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
    : SolarCollector(xs, ys, hm, obs, Genome((xs-1)*(ys-1)*2)) // same size as the mesh - ex. 3x3 shape has 4 rectangles -> 8 triangles
{}

SolarCollector::SolarCollector (const uint32_t xs, const uint32_t ys, const uint32_t hm, const Mesh3d* obs, const Genome &genome)
    : Genome(genome)
    , xsize(xs)
    , ysize(ys)
    , hmax(hm)
    , shape_mesh((xs-1)*(ys-1)*2) // see comment above
    , obstacle(obs) 
{
    computeMesh();
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

void SolarCollector::showYourself() const {
    std::cout << xsize << " " << ysize << std::endl;
    for (uint32_t y = 0; y < ysize; y++) {
        for (uint32_t x = 0; x < xsize; x++)
            std::cout << getXY(x, y) << " ";
        std::cout << std::endl;
    }
}

bool SolarCollector::rayObstacleHit(const double& sourcex, const double& sourcey, const double& sourcez, 
                                    const vertex& ray, bool invertRay) const {
    const double EPSILON = 0.0000001;
    const uint32_t obs_tri_count = obstacle->triangle_count;

    const vertex usedRay = invertRay ? vertex{-ray.x, -ray.y, -ray.z} : ray;

    for (uint32_t obs_idx = 0; obs_idx < obs_tri_count; ++obs_idx) {
        
        // Use a simplified ray-triangle intersection check (see below)

        // Pre-calculated edges are now class members (obs_edge1, obs_edge2)
        const double edge1x = obstacle->e1x[obs_idx];
        const double edge1y = obstacle->e1y[obs_idx];
        const double edge1z = obstacle->e1z[obs_idx];

        const double edge2x = obstacle->e2x[obs_idx];
        const double edge2y = obstacle->e2y[obs_idx];
        const double edge2z = obstacle->e2z[obs_idx];

        const double hx = usedRay.y * edge2z - usedRay.z * edge2y;
        const double hy = usedRay.z * edge2x - usedRay.x * edge2z;
        const double hz = usedRay.x * edge2y - usedRay.y * edge2x;

        const double a = edge1x * hx + edge1y * hy + edge1z * hz;

        if (std::abs(a) < EPSILON)
            continue;    // This ray is parallel to this triangle.

        const double f = 1.0 / a;
        const double sx = sourcex - obstacle->v0x[obs_idx];
        const double sy = sourcey - obstacle->v0y[obs_idx];
        const double sz = sourcez - obstacle->v0z[obs_idx];

        const double u = f * (sx * hx + sy * hy + sz * hz);
        if (u < 0.0 || u > 1.0)
            continue;

        const double qx = sy * edge1z - sz * edge1y;
        const double qy = sz * edge1x - sx * edge1z;
        const double qz = sx * edge1y - sy * edge1x;

        const double v = f * (usedRay.x * qx + usedRay.y * qy + usedRay.z * qz);
        if (v < 0.0 || u + v > 1.0)
            continue;

        const double t = f * (edge2x * qx + edge2y * qy + edge2z * qz);
        if (t > EPSILON) // ray intersection
            return true;
    }
    return false;
}

void SolarCollector::computeFitness(const std::vector<vertex>& rays) {
    // fitness is based on the amount of `mesh` triangles that reflect the `ray` directly onto an `obstacle`
    // add bounding boxes to obstacle as it doesn't change (AABB, BVH, Slab method); even a BB that covers the whole obstacle would speed the process up significantly

    const uint32_t mesh_tri_count = shape_mesh.triangle_count;

    fitness = 0.0; // Reset fitness


    // --- Step 1: Iterate through rays ---
    for (const auto& ray : rays) {

        // --- Step 2 & 3: Iterate through *mesh* triangles ---
        for (uint32_t mesh_idx = 0; mesh_idx < mesh_tri_count; ++mesh_idx) {
            // 2.1 Calculate reflected ray for the *current mesh triangle*.
            const vertex mesh_normal(shape_mesh.normx[mesh_idx], shape_mesh.normy[mesh_idx], shape_mesh.normz[mesh_idx]); // Get precomputed normal

            const vertex reflection = calculateReflection(mesh_normal, ray); // Simpler reflection calculation


            // --- Step 4: Check against *all obstacle triangles* ---
            const double midx = shape_mesh.midpx[mesh_idx];            
            const double midy = shape_mesh.midpy[mesh_idx];
            const double midz = shape_mesh.midpz[mesh_idx];

            // if ray hits (is blocked by) the obstacle continue to the next mesh triangle
            if (rayObstacleHit(midx, midy, midz, ray, true)) {
                continue;
            }

            // Check against all obstacle triangles (for reflected rays)
            // if reflected ray hits the obstacle increment fitness
            if (rayObstacleHit(midx, midy, midz, reflection, false)) {
                fitness += 1.0;
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

void SolarCollector::exportAsSTL(std::string name) const {
    shape_mesh.exportSTL(name);
}

// void SolarCollector::exportReflectionAsSTL() {
//     Mesh3d reflecting_mesh(static_cast<uint32_t>(reflecting.size()));
//     for (size_t i = 0; i < reflecting.size(); ++i) {
//         reflecting_mesh[static_cast<uint32_t>(i)] = reflecting[i];
//     }

//     reflecting_mesh.exportSTL(std::to_string(id) + "_reflection.stl");
// }
