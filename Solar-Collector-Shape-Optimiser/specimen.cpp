#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>

#include <Solar-Collector-Shape-Optimiser/specimen.hpp>


Specimen::Specimen(const unsigned char num, const unsigned short xs, const unsigned short ys, const unsigned short hm, Obstacle* obs)
    : id(num), xsize(xs), ysize(ys), hmax(hm), triangle_count((xs - 1) * (ys - 1) * 2), obstacle(obs) {
    shape = new double[xs*ys];
    mesh  = new triangle[(xs-1)*(ys-1)*2];   // ex. 3x3 shape has 4 rectangles -> 8 triangles
    mesh_midpoints = new vertex[(xs-1)*(ys-1)*2]; // same size as the mesh

    fitness = 0;
    norm_fitness = 0;
}

Specimen::Specimen (const Specimen & other)
: id(other.id), xsize(other.xsize), ysize(other.ysize), hmax(other.hmax), fitness(other.fitness), norm_fitness(other.norm_fitness), triangle_count(other.triangle_count)
{
    obstacle = other.obstacle;
    reflecting = other.reflecting;

    shape = new double[other.xsize*other.ysize];
    mesh  = new triangle[(other.xsize-1)*(other.ysize-1)*2];   // ex. 3x3 shape has 4 rectangles -> 8 triangles
    mesh_midpoints = new vertex[(other.xsize-1)*(other.ysize-1)*2]; // same size as the mesh

    std::copy(other.shape, other.shape + other.xsize*other.ysize, shape);
    std::copy(other.mesh, other.mesh + (other.xsize-1)*(other.ysize-1)*2, mesh);
    std::copy(other.mesh_midpoints, other.mesh_midpoints + (other.xsize-1)*(other.ysize-1)*2, mesh_midpoints);
    
}

Specimen& Specimen::operator= (const Specimen & other)
{
    id = other.id;
    xsize = other.xsize;
    ysize = other.ysize;
    hmax = other.hmax;
    fitness = other.fitness;
    norm_fitness = other.norm_fitness;
    triangle_count = other.triangle_count;

    obstacle = other.obstacle;
    reflecting = other.reflecting;

    shape = new double[other.xsize*other.ysize];
    mesh  = new triangle[(other.xsize-1)*(other.ysize-1)*2];   // ex. 3x3 shape has 4 rectangles -> 8 triangles
    mesh_midpoints = new vertex[(other.xsize-1)*(other.ysize-1)*2]; // same size as the mesh

    std::copy(other.shape, other.shape + other.xsize*other.ysize, shape);
    std::copy(other.mesh, other.mesh + (other.xsize-1)*(other.ysize-1)*2, mesh);
    std::copy(other.mesh_midpoints, other.mesh_midpoints + (other.xsize-1)*(other.ysize-1)*2, mesh_midpoints);
    
    return *this;
}

Specimen::~Specimen() {
    delete[] shape;
    delete[] mesh;
    delete[] mesh_midpoints;
}

double Specimen::getXY(const unsigned short x, const unsigned short y) const {
    return shape[y * xsize + x];
}

void Specimen::setXY(const unsigned short x, const unsigned short y, const double val) {
    if (val < 0)
        shape[y * xsize + x] = 0;
    else if (val > hmax)
        shape[y * xsize + x] = hmax;
    else
        shape[y * xsize + x] = val;
}

void Specimen::showYourself() {
    std::cout << (int)id << " " << xsize << " " << ysize << std::endl;
    for (int y = 0; y < ysize; y++) {
        for (int x = 0; x < xsize; x++)
            std::cout << getXY(x, y) << " ";
        std::cout << std::endl;
    }
}

vertex Specimen::xProduct(const vertex& a, const vertex& b) {
    return vertex(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

double Specimen::dotProduct(const vertex& a, const vertex& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vertex Specimen::substract(const vertex& a, const vertex& b) {
    return vertex(a.x - b.x, a.y - b.y, a.z - b.z);
}

vertex Specimen::add(const vertex& a, const vertex& b) {
    return vertex(a.x + b.x, a.y + b.y, a.z + b.z);
}

vertex Specimen::multiply(const vertex& a, double c) {
    return vertex(a.x * c, a.y * c, a.z * c);
}

vertex Specimen::divide(const vertex& a, double c) {
    return vertex(a.x / c, a.y / c, a.z / c);
}

vertex Specimen::unitNormal(const vertex& v0, const vertex& v1, const vertex& v2) {
    vertex normal;
    float vmod;

    normal = xProduct(substract(v1, v0), substract(v2, v1));
    vmod = sqrt(pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2));

    if (vmod > (float)1.0e-9) {
        normal.x /= vmod;
        normal.y /= vmod;
        normal.z /= vmod;
    }

    return normal;
}

vertex Specimen::unitNormal(const triangle& t) {
    vertex normal;
    float vmod;

    normal = xProduct(substract(t.v[1], t.v[0]), substract(t.v[2], t.v[1]));
    vmod = sqrt(pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2));

    if (vmod > (float)1.0e-9)
        normal = divide(normal, vmod);

    return normal;
}

vertex Specimen::tMidPoint(const triangle& t) {
    // vertex circumcenter;
    const vertex a = substract (t.v[0], t.v[2]);  // move t.v[2] to origin
    const vertex b = substract (t.v[1], t.v[2]);  // -||-

    const vertex axb = xProduct(a, b);

    return add(divide(xProduct(substract(multiply(b, dotProduct(a, a)), multiply(a, dotProduct(b, b))), axb), dotProduct(axb, axb) * 2), t.v[2]);
}

vertex Specimen::calculateReflection(const triangle& t, const vertex& ray) {
    const vertex invray(substract(vertex(0, 0, 0), ray));
    return substract(multiply(t.normal, 2 * dotProduct(t.normal, invray)), invray);
}

bool Specimen::rayIsBlocked(const vertex& source, const vertex& ray, const triangle& target) {
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

bool Specimen::rayMeetsObstacle(const vertex& source, const vertex& ray, const triangle& target) {
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

void Specimen::computeMesh() {
    int i = 0;
    for (int y = 0; y < ysize - 1; y++) {
        for (int x = 0; x < xsize - 1; x++) {
            mesh[i].v[0].x = x;     mesh[i].v[0].z = y;     mesh[i].v[0].y = getXY(x, y);   // swapped coordinates
            mesh[i].v[1].x = x;     mesh[i].v[1].z = y + 1; mesh[i].v[1].y = getXY(x, y + 1);
            mesh[i].v[2].x = x + 1; mesh[i].v[2].z = y;     mesh[i].v[2].y = getXY(x + 1, y);
            mesh[i].normal = unitNormal(mesh[i]);
            ++i;

            mesh[i].v[0].x = x + 1; mesh[i].v[0].z = y + 1; mesh[i].v[0].y = getXY(x + 1, y + 1);
            mesh[i].v[1].x = x + 1; mesh[i].v[1].z = y;     mesh[i].v[1].y = getXY(x + 1, y);
            mesh[i].v[2].x = x;     mesh[i].v[2].z = y + 1; mesh[i].v[2].y = getXY(x, y + 1);
            mesh[i].normal = unitNormal(mesh[i]);
            ++i;
        }
    }
}

void Specimen::computeMeshMidpoints() {
    for (uint32_t i = 0; i < triangle_count; i++)
        mesh_midpoints[i] = tMidPoint(mesh[i]);
}

void Specimen::computeFitness(const vertex* rays, const int count_rays) {
    bool blocked = false;
    vertex reflection;
    
    fitness = 0;

    for (int rayno = 0; rayno < count_rays; rayno++)
    {
        // main (per ray) loop
        for (uint32_t trino = 0; trino < triangle_count; trino++)
        {
            // per triangle loop
            blocked = false;
            for (int obsno = 0; obsno < obstacle->triangle_count; obsno++)
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
                reflection = calculateReflection(mesh[trino], rays[rayno]);
                // std::cout << reflection.x << ' ' << reflection.y << ' ' << reflection.z << std::endl;
                for (int obsno = 0; obsno < obstacle->triangle_count; obsno++)
                {
                    if(rayMeetsObstacle(mesh_midpoints[trino], reflection, obstacle->mesh[obsno]) == true)
                    {
                        // reflecting.push_back(mesh[trino]);
                        fitness +=  1;
                        break;
                    }
                }
            }

        }
    }
}

void Specimen::exportAsSTL(std::string name) {
    std::ofstream stlout;
    std::stringstream ss;

    for (uint32_t i = 0; i < triangle_count; i += 2) {
        ss << "facet normal " << mesh[i].normal.x << " " << mesh[i].normal.y << " " << mesh[i].normal.z << std::endl
           << "   outer loop" << std::endl
           << "      vertex " << mesh[i].v[0].x << " " << mesh[i].v[0].y << " " << mesh[i].v[0].z << std::endl
           << "      vertex " << mesh[i].v[1].x << " " << mesh[i].v[1].y << " " << mesh[i].v[1].z << std::endl
           << "      vertex " << mesh[i].v[2].x << " " << mesh[i].v[2].y << " " << mesh[i].v[2].z << std::endl
           << "   endloop" << std::endl
           << "endfacet" << std::endl

           << "facet normal " << mesh[i + 1].normal.x << " " << mesh[i + 1].normal.y << " " << mesh[i + 1].normal.z << std::endl
           << "   outer loop" << std::endl
           << "      vertex " << mesh[i + 1].v[0].x << " " << mesh[i + 1].v[0].y << " " << mesh[i + 1].v[0].z << std::endl
           << "      vertex " << mesh[i + 1].v[1].x << " " << mesh[i + 1].v[1].y << " " << mesh[i + 1].v[1].z << std::endl
           << "      vertex " << mesh[i + 1].v[2].x << " " << mesh[i + 1].v[2].y << " " << mesh[i + 1].v[2].z << std::endl
           << "   endloop" << std::endl
           << "endfacet" << std::endl;
    }

    stlout.open(name, std::ofstream::trunc);

    stlout << "solid " << std::to_string(id) << std::endl;
    stlout << ss.rdbuf();
    stlout << "endsolid " << std::to_string(id) << std::endl;
    
    stlout.close();
}

void Specimen::exportReflectionAsSTL() {
    std::ofstream stlout;
    std::stringstream ss;

    for (auto i = reflecting.begin(); i != reflecting.end(); i++) {
        ss << "facet normal " << i->normal.x << " " << i->normal.y << " " << i->normal.z << std::endl
           << "   outer loop" << std::endl
           << "      vertex " << i->v[0].x << " " << i->v[0].y << " " << i->v[0].z << std::endl
           << "      vertex " << i->v[1].x << " " << i->v[1].y << " " << i->v[1].z << std::endl
           << "      vertex " << i->v[2].x << " " << i->v[2].y << " " << i->v[2].z << std::endl
           << "   endloop" << std::endl
           << "endfacet" << std::endl;
    }

    stlout.open(std::to_string(id) + "_reflection.stl", std::ofstream::trunc);
    
    stlout << "solid " << std::to_string(id) << std::endl;
    stlout << ss.rdbuf();
    stlout << "endsolid " << std::to_string(id) << std::endl;
    
    stlout.close();
}

bool sortBestToWorst (Specimen* a, Specimen* b) { return (a->fitness > b->fitness); }

Specimen* crossoverAndMutate (Specimen & a, Specimen & b, int id, double crossover_bias, int mutation_percent)
{
    Specimen* temp = new Specimen(id, a.xsize, a.ysize, a.hmax, a.obstacle);

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

    return temp;
}