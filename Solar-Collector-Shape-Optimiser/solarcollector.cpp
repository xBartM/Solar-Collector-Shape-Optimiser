#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>

#include <Solar-Collector-Shape-Optimiser/solarcollector.hpp>


SolarCollector::SolarCollector(const unsigned char num, const unsigned short xs, const unsigned short ys, const unsigned short hm, Mesh3d* obs)
    : xsize(xs), ysize(ys), hmax(hm), genes(num, (xs-1)*(ys-1)*2), shape_mesh((xs-1)*(ys-1)*2), obstacle(obs) { 
    mesh_midpoints = new vertex[(xs-1)*(ys-1)*2]; // same size as the mesh - ex. 3x3 shape has 4 rectangles -> 8 triangles

}

SolarCollector::SolarCollector (const SolarCollector & other)
    : xsize(other.xsize), ysize(other.ysize), hmax(other.hmax), genes(other.genes), shape_mesh(other.shape_mesh) {
    obstacle = other.obstacle;
    reflecting = other.reflecting;

    mesh_midpoints = new vertex[(other.xsize-1)*(other.ysize-1)*2]; // same size as the mesh

    std::copy(other.mesh_midpoints, other.mesh_midpoints + (other.xsize-1)*(other.ysize-1)*2, mesh_midpoints);
    
}

SolarCollector::SolarCollector (SolarCollector&& other) noexcept
    : xsize(std::move(other.xsize)),
      ysize(std::move(other.ysize)),
      hmax(std::move(other.hmax)),
      genes(std::move(other.genes)),
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
        xsize = std::move(other.xsize);
        ysize = std::move(other.ysize);
        hmax = std::move(other.hmax);
        genes = std::move(other.genes);
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
    xsize = other.xsize;
    ysize = other.ysize;
    hmax = other.hmax;

    genes = other.genes;

    obstacle = other.obstacle;
    reflecting = other.reflecting;

    shape_mesh = Mesh3d(other.shape_mesh); // we can use Mesh3d operator=
    mesh_midpoints = new vertex[(other.xsize-1)*(other.ysize-1)*2]; // same size as the mesh

    std::copy(other.mesh_midpoints, other.mesh_midpoints + (other.xsize-1)*(other.ysize-1)*2, mesh_midpoints);
    
    return *this;
}

SolarCollector::~SolarCollector() {
    // delete[] shape;
    delete[] mesh_midpoints;
}

double SolarCollector::getXY(const unsigned short x, const unsigned short y) const {
    return genes.shape[y * xsize + x];
}

void SolarCollector::setXY(const unsigned short x, const unsigned short y, const double val) {
    if (val < 0)
        genes.shape[y * xsize + x] = 0;
    else if (val > hmax)
        genes.shape[y * xsize + x] = hmax;
    else
        genes.shape[y * xsize + x] = val;
}

void SolarCollector::showYourself() {
    std::cout << (int)genes.id << " " << xsize << " " << ysize << std::endl;
    for (int y = 0; y < ysize; y++) {
        for (int x = 0; x < xsize; x++)
            std::cout << getXY(x, y) << " ";
        std::cout << std::endl;
    }
}

bool SolarCollector::rayIsBlocked(const vertex& source, const vertex& ray, const triangle& target) {
    //Moller–Trumbore intersection algorithm

    const vertex invray = substract(vertex(0,0,0), ray);    // while checking if ray is blocked it is sent from the reflecting triangle

    //const vertex src_midpoint = tMidPoint(source);
    const float EPSILON = 0.0000001;
    const vertex vertex0 = target.v[0];
    const vertex vertex1 = target.v[1];  
    const vertex vertex2 = target.v[2];
    const vertex edge1 = substract(vertex1, vertex0);
    const vertex edge2 = substract(vertex2, vertex0); 
    const vertex h = xProduct(invray, edge2);
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
    v = f * dotProduct(invray, q);
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

bool SolarCollector::rayMeetsObstacle(const vertex& source, const vertex& ray, const triangle& target) {
    //Moller–Trumbore intersection algorithm

    //const vertex invray = substract(vertex(0,0,0), ray);    // while checking if ray is blocked it is sent from the reflecting triangle

    //const vertex src_midpoint = tMidPoint(source);
    const float EPSILON = 0.0000001;
    const vertex vertex0 = target.v[0];
    const vertex vertex1 = target.v[1];  
    const vertex vertex2 = target.v[2];
    const vertex edge1 = substract(vertex1, vertex0);
    const vertex edge2 = substract(vertex2, vertex0); 
    const vertex h = xProduct(ray, edge2);
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
    v = f * dotProduct(ray, q);
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
    int i = 0;
    for (int y = 0; y < ysize - 1; y++) {
        for (int x = 0; x < xsize - 1; x++) {
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

void SolarCollector::computeFitness(const vertex* rays, const int count_rays) {
    bool blocked = false;
    vertex reflection;
    
    genes.fitness = 0;

    for (int rayno = 0; rayno < count_rays; rayno++)
    {
        // main (per ray) loop
        for (uint32_t trino = 0; trino < shape_mesh.triangle_count; trino++)
        {
            // per triangle loop
            blocked = false;
            for (uint32_t obsno = 0; obsno < obstacle->triangle_count; obsno++)
            {
                // per obstacle triangle loop
                if (rayIsBlocked(mesh_midpoints[trino], rays[rayno], obstacle->mesh[obsno])) ///  optimization potential (?) - collides(...,..., obstacle->mesh)
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
                    if(rayMeetsObstacle(mesh_midpoints[trino], reflection, obstacle->mesh[obsno]) == true)
                    {
                        // reflecting.push_back(shape_mesh[trino]);
                        genes.fitness +=  1;
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

    reflecting_mesh.exportSTL(std::to_string(genes.id) + "_reflection.stl");
}

SolarCollector crossoverAndMutate (SolarCollector & a, SolarCollector & b, int id, double crossover_bias, int mutation_percent)
{
    SolarCollector* temp = new SolarCollector(id, a.xsize, a.ysize, a.hmax, a.obstacle);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> height(-0.225, 0.225);
    std::uniform_real_distribution<double> diceroll(0.0, (double)100.0);

    for (int i = 0; i < a.xsize*a.ysize; i++)
    {
        if (diceroll(mt) < crossover_bias)  // take first
        {
            if (diceroll(mt) < mutation_percent)
                temp->setXY(i, 0, a.getXY(i, 0) + height(mt));  // mutate 
            else 
                temp->setXY(i, 0, a.getXY(i, 0));               // take first (no mutation)
        }
        else    // take second
        {
            if (diceroll(mt) < mutation_percent)
                temp->setXY(i, 0, b.getXY(i, 0) + height(mt));  // mutate second
            else
                temp->setXY(i, 0, b.getXY(i, 0));               // take second (no mutation)
        }
        
    }
    temp->computeMesh();
    temp->computeMeshMidpoints();

    SolarCollector ret(*temp);
    delete temp;
    return ret;
}
