#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>
#include <execution>
#include <ranges>

#include <Solar-Collector-Shape-Optimiser/solarcollector.hpp>

SolarCollector::SolarCollector(const uint32_t xs, const uint32_t ys, const uint32_t hm, const Mesh3d* obs)
    : Genome((xs-1)*(ys-1)*2) // same size as the mesh - ex. 3x3 shape has 4 rectangles -> 8 triangles
    , xsize(xs)
    , ysize(ys)
    , hmax(hm)
    , shape_mesh((xs-1)*(ys-1)*2) // see comment above
    , obstacle(obs) 
{}

SolarCollector::SolarCollector (const SolarCollector & other)
    : Genome(other)
    , xsize(other.xsize)
    , ysize(other.ysize)
    , hmax(other.hmax)
    , shape_mesh(other.shape_mesh)
    , obstacle(other.obstacle)
    , reflecting(other.reflecting)
{}

SolarCollector::SolarCollector (SolarCollector&& other) noexcept
    : Genome(std::move(other))
    , xsize(std::move(other.xsize))
    , ysize(std::move(other.ysize))
    , hmax(std::move(other.hmax))
    , shape_mesh(std::move(other.shape_mesh))
    , obstacle(std::move(other.obstacle))
    , reflecting(std::move(other.reflecting))
{}

SolarCollector& SolarCollector::operator= (SolarCollector&& other) noexcept {
    if (this != &other)
    {
        Genome::operator=(std::move(other)); // Call the base class assignment operator
        xsize = std::move(other.xsize);
        ysize = std::move(other.ysize);
        hmax = std::move(other.hmax);
        shape_mesh = std::move(other.shape_mesh);
        obstacle = std::move(other.obstacle);
        reflecting = std::move(other.reflecting);

    }
    return *this;
}

SolarCollector& SolarCollector::operator= (const SolarCollector & other)
{
    Genome::operator=(other); // Call the base class assignment operator
    xsize = other.xsize;
    ysize = other.ysize;
    hmax = other.hmax;

    shape_mesh = Mesh3d(other.shape_mesh); // we can use Mesh3d operator=

    obstacle = other.obstacle;
 
    reflecting = other.reflecting;
    
    return *this;
}

SolarCollector::~SolarCollector() {}

double SolarCollector::getXY(const uint32_t x, const uint32_t y) const {
    return dna[y * xsize + x];
}

void SolarCollector::setXY(const uint32_t x, const uint32_t y, const double val) {
    if (val < 0)
        dna[y * xsize + x] = 0;
    else if (val > hmax)
        dna[y * xsize + x] = hmax;
    else
        dna[y * xsize + x] = val;
}

void SolarCollector::showYourself() {
    std::cout << id << " " << xsize << " " << ysize << std::endl;
    for (uint32_t y = 0; y < ysize; y++) {
        for (uint32_t x = 0; x < xsize; x++)
            std::cout << getXY(x, y) << " ";
        std::cout << std::endl;
    }
}


bool SolarCollector::rayTriangleIntersect(
    // Moller–Trumbore intersection algorithm - I think

    double v0x, double v0y, double v0z,
    double v1x, double v1y, double v1z,
    double v2x, double v2y, double v2z,
    double sourcex, double sourcey, double sourcez,
    const vertex& ray, bool invertRay) {

    const double EPSILON = 0.0000001;

    // Pre-calculated edges are now class members (obs_edge1, obs_edge2)
    double edge1x = v1x - v0x;
    double edge1y = v1y - v0y;
    double edge1z = v1z - v0z;

    double edge2x = v2x - v0x;
    double edge2y = v2y - v0y;
    double edge2z = v2z - v0z;

    const vertex usedRay = invertRay ? vertex{-ray.x, -ray.y, -ray.z} : ray;

    double hx = usedRay.y * edge2z - usedRay.z * edge2y;
    double hy = usedRay.z * edge2x - usedRay.x * edge2z;
    double hz = usedRay.x * edge2y - usedRay.y * edge2x;

    double a = edge1x * hx + edge1y * hy + edge1z * hz;

    if (a > -EPSILON && a < EPSILON)
        return false;    // This ray is parallel to this triangle.

    double f = 1.0 / a;
    double sx = sourcex - v0x;
    double sy = sourcey - v0y;
    double sz = sourcez - v0z;

    double u = f * (sx * hx + sy * hy + sz * hz);
    if (u < 0.0 || u > 1.0)
        return false;

    double qx = sy * edge1z - sz * edge1y;
    double qy = sz * edge1x - sx * edge1z;
    double qz = sx * edge1y - sy * edge1x;

    double v = f * (usedRay.x * qx + usedRay.y * qy + usedRay.z * qz);
    if (v < 0.0 || u + v > 1.0)
        return false;

    double t = f * (edge2x * qx + edge2y * qy + edge2z * qz);
    if (t > EPSILON) // ray intersection
    {
        return true;
    }

    return false;
}

void SolarCollector::computeFitness(const std::vector<vertex>& rays) {
    // fitness is based on the amount of `mesh` triangles that reflect the `ray` directly onto an `obstacle`
    // add bounding boxes to obstacle as it doesn't change (AABB, BVH, Slab method); even a BB that covers the whole obstacle would speed the process up significantly

    // go through all rays
    // 1. compute if source ray intersects an obstacle
    // obstacle is the target
    // 2. compute reflected rays vector for every mesh triangle
    // 3. compute if relfected ray intersects an obstacle
    // 4. sum nonblocked reflections


    const uint32_t obs_tri_count = obstacle->triangle_count;
    const uint32_t mesh_tri_count = shape_mesh.triangle_count;

    fitness = 0; // Reset fitness


    // --- Step 1: Iterate through rays ---
    for (const auto& ray : rays) {

        // --- Step 2 & 3: Iterate through *mesh* triangles ---
        for (uint32_t mesh_idx = 0; mesh_idx < mesh_tri_count; ++mesh_idx) {
            // 2.1 Calculate reflected ray for the *current mesh triangle*.
            const vertex mesh_normal(shape_mesh.normx[mesh_idx], shape_mesh.normy[mesh_idx], shape_mesh.normz[mesh_idx]); // Get precomputed normal

            const vertex reflection = calculateReflection(mesh_normal, ray); // Simpler reflection calculation (see below)


            // --- Step 4: Check against *all obstacle triangles* (in parallel) ---
            bool obstacle_hit = false;
            
            // Use std::for_each with parallel execution
            std::for_each(std::execution::par_unseq,
                std::views::iota(0u, obs_tri_count).begin(),
                std::views::iota(0u, obs_tri_count).end(),
                [&](uint32_t obs_idx) {
                    if (obstacle_hit) return;
                    
                    // Use a simplified ray-triangle intersection check (see below)

                    if (rayTriangleIntersect(
                            obstacle->v0x[obs_idx], obstacle->v0y[obs_idx], obstacle->v0z[obs_idx],
                            obstacle->v1x[obs_idx], obstacle->v1y[obs_idx], obstacle->v1z[obs_idx],
                            obstacle->v2x[obs_idx], obstacle->v2y[obs_idx], obstacle->v2z[obs_idx],
                            shape_mesh.midpx[mesh_idx], shape_mesh.midpy[mesh_idx], shape_mesh.midpz[mesh_idx], //PASS MIDPOINT
                            ray,
                            true// Invert for initial block check (from mesh to obstacle)
                        ))
                        {
                            obstacle_hit = true;
                        } 
                }
            );

            if (obstacle_hit) continue;

            // Check against all obstacle triangles (for reflected rays)
            bool reflected_ray_hit = false;
             std::for_each(std::execution::par_unseq,
                std::views::iota(0u, obs_tri_count).begin(),
                std::views::iota(0u, obs_tri_count).end(),
                [&](uint32_t obs_idx) {
                    if (reflected_ray_hit) return;
                    if (rayTriangleIntersect(
                        obstacle->v0x[obs_idx], obstacle->v0y[obs_idx], obstacle->v0z[obs_idx],
                        obstacle->v1x[obs_idx], obstacle->v1y[obs_idx], obstacle->v1z[obs_idx],
                        obstacle->v2x[obs_idx], obstacle->v2y[obs_idx], obstacle->v2z[obs_idx],
                        shape_mesh.midpx[mesh_idx], shape_mesh.midpy[mesh_idx], shape_mesh.midpz[mesh_idx],  //PASS MIDPOINT
                        reflection,
                        false // Do *not* invert for reflection check
                    ))
                    {
                            reflected_ray_hit = true;
                    }
               });

            // Update fitness
            if(reflected_ray_hit){
                fitness += 1; // Increment fitness for each successful hit.
            }
        }
    }
}

void SolarCollector::computeMesh() {
    uint32_t i = 0;
    for (uint32_t y = 0; y < ysize - 1; y++) {
        for (uint32_t x = 0; x < xsize - 1; x++) {
            shape_mesh.v0x[i] = x;     shape_mesh.v0z[i] = y;     shape_mesh.v0y[i] = getXY(x, y);   // swapped coordinates
            shape_mesh.v1x[i] = x;     shape_mesh.v1z[i] = y + 1; shape_mesh.v1y[i] = getXY(x, y + 1);
            shape_mesh.v2x[i] = x + 1; shape_mesh.v2z[i] = y;     shape_mesh.v2y[i] = getXY(x + 1, y);
            ++i;

            shape_mesh.v0x[i] = x + 1; shape_mesh.v0z[i] = y + 1; shape_mesh.v0y[i] = getXY(x + 1, y + 1);
            shape_mesh.v1x[i] = x + 1; shape_mesh.v1z[i] = y;     shape_mesh.v1y[i] = getXY(x + 1, y);
            shape_mesh.v2x[i] = x;     shape_mesh.v2z[i] = y + 1; shape_mesh.v2y[i] = getXY(x, y + 1);
            ++i;
        }
    }
    shape_mesh.findNormals();
    shape_mesh.findCircumcentres();
}

void SolarCollector::exportAsSTL(std::string name) {
    shape_mesh.exportSTL(name);
}

// void SolarCollector::exportReflectionAsSTL() {
//     Mesh3d reflecting_mesh(static_cast<uint32_t>(reflecting.size()));
//     for (size_t i = 0; i < reflecting.size(); ++i) {
//         reflecting_mesh[static_cast<uint32_t>(i)] = reflecting[i];
//     }

//     reflecting_mesh.exportSTL(std::to_string(id) + "_reflection.stl");
// }
