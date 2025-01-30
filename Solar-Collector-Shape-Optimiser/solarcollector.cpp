#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>

#include <Solar-Collector-Shape-Optimiser/solarcollector.hpp>

SolarCollector::SolarCollector(const uint32_t xs, const uint32_t ys, const uint32_t hm, Mesh3d* obs)
    : Genome((xs-1)*(ys-1)*2), xsize(xs), ysize(ys), hmax(hm), shape_mesh((xs-1)*(ys-1)*2), obstacle(obs) { 
    mesh_midpoints = new vertex[(xs-1)*(ys-1)*2]; // same size as the mesh - ex. 3x3 shape has 4 rectangles -> 8 triangles

}

SolarCollector::SolarCollector (const SolarCollector & other)
    : Genome(other), xsize(other.xsize), ysize(other.ysize), hmax(other.hmax), shape_mesh(other.shape_mesh) {
    obstacle = other.obstacle;
    reflecting = other.reflecting;

    mesh_midpoints = new vertex[(other.xsize-1)*(other.ysize-1)*2]; // same size as the mesh

    std::copy(other.mesh_midpoints, other.mesh_midpoints + (other.xsize-1)*(other.ysize-1)*2, mesh_midpoints);
    
}

SolarCollector::SolarCollector (SolarCollector&& other) noexcept
    : Genome(std::move(other)),
      xsize(std::move(other.xsize)),
      ysize(std::move(other.ysize)),
      hmax(std::move(other.hmax)),
      shape_mesh(std::move(other.shape_mesh)),
      mesh_midpoints(other.mesh_midpoints),
      reflecting(std::move(other.reflecting)),
      obstacle(std::move(other.obstacle))
{
    other.mesh_midpoints = nullptr;
}

SolarCollector& SolarCollector::operator= (SolarCollector&& other) noexcept {
    if (this != &other)
    {
        Genome::operator=(std::move(other)); // Call the base class assignment operator
        xsize = std::move(other.xsize);
        ysize = std::move(other.ysize);
        hmax = std::move(other.hmax);
        // genes = std::move(other.genes); // Removed
        shape_mesh = std::move(other.shape_mesh);
        reflecting = std::move(other.reflecting);
        obstacle = std::move(other.obstacle);

        delete[] mesh_midpoints;
        mesh_midpoints = other.mesh_midpoints;
        other.mesh_midpoints = nullptr;
    }
    return *this;
}

SolarCollector& SolarCollector::operator= (const SolarCollector & other)
{
    Genome::operator=(other); // Call the base class assignment operator
    xsize = other.xsize;
    ysize = other.ysize;
    hmax = other.hmax;

    obstacle = other.obstacle;
    reflecting = other.reflecting;

    shape_mesh = Mesh3d(other.shape_mesh); // we can use Mesh3d operator=
    mesh_midpoints = new vertex[(other.xsize-1)*(other.ysize-1)*2]; // same size as the mesh

    std::copy(other.mesh_midpoints, other.mesh_midpoints + (other.xsize-1)*(other.ysize-1)*2, mesh_midpoints);
    
    return *this;
}

SolarCollector::~SolarCollector() {
    delete[] mesh_midpoints;
}

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

bool SolarCollector::rayMeetsObstacle(const vertex& source, const vertex& ray, const triangle& target, bool invertRay) {
    //Moller–Trumbore intersection algorithm

    //const vertex invray = substract(vertex(0,0,0), ray);    // while checking if ray is blocked it is sent from the reflecting triangle

    //const vertex src_midpoint = tMidPoint(source);
    const float EPSILON = 0.0000001;
    const vertex vertex0 = target.v[0];
    const vertex vertex1 = target.v[1];  
    const vertex vertex2 = target.v[2];
    const vertex edge1 = substract(vertex1, vertex0);
    const vertex edge2 = substract(vertex2, vertex0); 

    const vertex usedRay = invertRay ? substract(vertex(0,0,0), ray) : ray; // Invert the ray if needed

    const vertex h = xProduct(usedRay, edge2); // Use the potentially inverted ray
    vertex s, q;
    const float a = dotProduct(edge1, h);
    float f,u,v;

    // edge1 = substract(vertex1, vertex0);
    // edge2 = substract(vertex2, vertex0);
    // h = xProduct(ray, edge2);
    // a = dotProduct(edge1, h);
    
    if (a > -EPSILON && a < EPSILON)
        return false;    // This ray is parallel to this triangle.
    
    f = 1.0/a;
    s = substract(source, vertex0);
    u = f * (dotProduct(s, h));
    if (u < 0.0 || u > 1.0)
        return false;

    q = xProduct(s, edge1);
    v = f * dotProduct(usedRay, q); // Use the potentially inverted ray
    if (v < 0.0 || u + v > 1.0)
        return false;

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dotProduct(edge2, q);
    if (t > EPSILON) // ray intersection
    {
        //outIntersectionPoint = rayOrigin + rayVector * t;
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return false;
}

void SolarCollector::computeMesh() {
    uint32_t i = 0;
    for (uint32_t y = 0; y < ysize - 1; y++) {
        for (uint32_t x = 0; x < xsize - 1; x++) {
            shape_mesh[i].v[0].x = x;     shape_mesh[i].v[0].z = y;     shape_mesh[i].v[0].y = getXY(x, y);   // swapped coordinates
            shape_mesh[i].v[1].x = x;     shape_mesh[i].v[1].z = y + 1; shape_mesh[i].v[1].y = getXY(x, y + 1);
            shape_mesh[i].v[2].x = x + 1; shape_mesh[i].v[2].z = y;     shape_mesh[i].v[2].y = getXY(x + 1, y);
            shape_mesh[i].normal = unitNormal(shape_mesh[i]);
            ++i;

            shape_mesh[i].v[0].x = x + 1; shape_mesh[i].v[0].z = y + 1; shape_mesh[i].v[0].y = getXY(x + 1, y + 1);
            shape_mesh[i].v[1].x = x + 1; shape_mesh[i].v[1].z = y;     shape_mesh[i].v[1].y = getXY(x + 1, y);
            shape_mesh[i].v[2].x = x;     shape_mesh[i].v[2].z = y + 1; shape_mesh[i].v[2].y = getXY(x, y + 1);
            shape_mesh[i].normal = unitNormal(shape_mesh[i]);
            ++i;
        }
    }
}

void SolarCollector::computeMeshMidpoints() {
    for (uint32_t i = 0; i < shape_mesh.triangle_count; i++)
        mesh_midpoints[i] = tMidPoint(shape_mesh[i]);
}

void SolarCollector::computeFitness(const vertex* rays, const uint32_t count_rays) {
    bool blocked = false;
    vertex reflection;
    
    fitness = 0;

    for (uint32_t rayno = 0; rayno < count_rays; rayno++)
    {
        // main (per ray) loop
        for (uint32_t trino = 0; trino < shape_mesh.triangle_count; trino++)
        {
            // per triangle loop
            blocked = false;
            for (uint32_t obsno = 0; obsno < obstacle->triangle_count; obsno++)
            {
                // per obstacle triangle loop
                if (rayMeetsObstacle(mesh_midpoints[trino], rays[rayno], obstacle->mesh[obsno], true)) ///  optimization potential (?) - collides(...,..., obstacle->mesh)
                { 
                    blocked = true; // if a ray to the triangle is blocked then stop checking
                    break;
                }
                // make sure the reflecting triangle faces the ray
            }

            if (blocked == false)    // ray not blocked: check the reflection
            {   // check if reflected ray hits the obstacle
                reflection = calculateReflection(shape_mesh[trino], rays[rayno]);
                // std::cout << reflection.x << ' ' << reflection.y << ' ' << reflection.z << std::endl;
                for (uint32_t obsno = 0; obsno < obstacle->triangle_count; obsno++)
                {
                    if(rayMeetsObstacle(mesh_midpoints[trino], reflection, obstacle->mesh[obsno], false) == true)
                    {
                        // reflecting.push_back(shape_mesh[trino]);
                        fitness +=  1;
                        break;
                    }
                }
            }

        }
    }
}

void SolarCollector::exportAsSTL(std::string name) {
    shape_mesh.exportSTL(name);
}

void SolarCollector::exportReflectionAsSTL() {
    Mesh3d reflecting_mesh(static_cast<uint32_t>(reflecting.size()));
    for (size_t i = 0; i < reflecting.size(); ++i) {
        reflecting_mesh[static_cast<uint32_t>(i)] = reflecting[i];
    }

    reflecting_mesh.exportSTL(std::to_string(id) + "_reflection.stl");
}
