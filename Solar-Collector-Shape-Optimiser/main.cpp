#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>

#include <vector>

class Specimen;
bool sortBestToWorst (Specimen* a, Specimen* b);
Specimen* crossoverAndMutate (Specimen & a, Specimen & b, int id, double crossover_bias, int mutation_percent);

struct vertex {
    double x;
    double y;
    double z;

    vertex () : x(0), y(0), z(0) {};
    vertex (double xx, double yy, double zz) : x(xx), y(yy), z(zz) {};
};

struct triangle {
    vertex normal;
    vertex v[3];

    triangle () : normal(), v{} {};
    triangle (vertex n, vertex v1, vertex v2, vertex v3) : normal(n), v{v1, v2, v3} {};
};

class Obstacle
{
public:

    int triangle_count;
    triangle* mesh;

    void moveXY (const double x, const double y)
    {
        for (int i = 0; i < triangle_count; i++)
        {
            mesh[i].v[0].x += x; mesh[i].v[1].x += x; mesh[i].v[2].x += x;
            mesh[i].v[0].y += y; mesh[i].v[1].y += y; mesh[i].v[2].y += y;
        }

        return;
    }

    void importSTL (const std::string filename)
    {
        std::string line;
        std::ifstream file(filename, std::ifstream::in);

        size_t pos;

        if (file.is_open()) 
        {
            triangle_count = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
            triangle_count -= 2;   
            triangle_count /= 7;

            mesh = new triangle[triangle_count];

            file.seekg(0, file.beg);    // return to beginning
            file.clear();               // clear flags (especially eof flag)

            std::getline(file, line);   // skip the name of the solid
            std::getline(file, line);   // get the "facet normal" line ready

            for (int i = 0; line.find("endsolid") == std::string::npos; i++) // read untill you find "endsolid";
            {
                // get normal
                pos = line.rfind(' ');      // find last ' '
                mesh[i].normal.z = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                pos = line.rfind(' ');      // find last ' '
                mesh[i].normal.y = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                pos = line.rfind(' ');      // find last ' '
                mesh[i].normal.x = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                
                std::getline(file, line);   // skip the "outer loop" line
                
                // vertex 0
                std::getline(file, line);   // get the first "vertex" line ready
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[0].z = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[0].y = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[0].x = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                
                // vertex 1
                std::getline(file, line);   // get the second "vertex" line ready
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[1].z = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[1].y = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[1].x = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);

                // vertex 2
                std::getline(file, line);   // get the third "vertex" line ready
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[2].z = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[2].y = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);
                pos = line.rfind(' ');      // find last ' '
                mesh[i].v[2].x = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
                line = line.substr(0, pos);


                std::getline(file, line);   // skip "endloop"
                std::getline(file, line);   // skip "endfacet"

                std::getline(file, line);   // get the "facet normal" line ready (or "endsolid" line)
                
            }
            file.close();
        }
        return;
    }

    void exportSTL (const std::string filename) 
    {
        std::ofstream stlout;

        stlout.open(filename, std::ofstream::trunc);

        stlout << "solid obstacle" << std::endl;

        for (int i = 0; i < triangle_count; i++)
            {
                stlout << "facet normal " << mesh[i].normal.x << " " << mesh[i].normal.y << " " << mesh[i].normal.z << std::endl;
                stlout << "   outer loop" << std::endl;
                stlout << "      vertex " << mesh[i].v[0].x << " " << mesh[i].v[0].y << " " << mesh[i].v[0].z << std::endl;
                stlout << "      vertex " << mesh[i].v[1].x << " " << mesh[i].v[1].y << " " << mesh[i].v[1].z << std::endl;
                stlout << "      vertex " << mesh[i].v[2].x << " " << mesh[i].v[2].y << " " << mesh[i].v[2].z << std::endl;
                stlout << "   endloop" << std::endl;
                stlout << "endfacet" << std::endl;
                
            }

        stlout << "endsolid obstacle" << std::endl;

        stlout.close();
    };

    Obstacle () {};

    Obstacle (const std::string filename) {importSTL(filename);};

    ~Obstacle () { delete [] mesh; };

};

class Specimen 
{
public:
    uint32_t id;   // 0 to 255

    ushort xsize;   // size of the panel
    ushort ysize;
    ushort hmax;  // maximal height (dictated by max printing height)

    double* shape;  // table for heights at x and y coordinates -- access via (getter, but..) [y*xsize+x] -- this is chromosome

    double fitness; // fitness of a specimen (% of triangles that reflected the light)
    double norm_fitness;    // normalized fitness

    triangle* mesh; // mesh for computing reflections
    uint32_t triangle_count;
    vertex* mesh_midpoints; // precalculated mid points of every triangle of a mesh

    std::vector<triangle> reflecting;

    Obstacle* obstacle; // pointer to obstacle to read its mesh


    Specimen (const unsigned char num, const ushort xs, const ushort ys, const ushort hm, Obstacle* obs) 
    : id(num), xsize(xs), ysize(ys), hmax(hm), triangle_count((xs-1)*(ys-1)*2), obstacle(obs)
    {
        shape = new double[xs*ys];
        mesh  = new triangle[(xs-1)*(ys-1)*2];   // ex. 3x3 shape has 4 rectangles -> 8 triangles
        mesh_midpoints = new vertex[(xs-1)*(ys-1)*2]; // same size as the mesh

        fitness = 0;
        norm_fitness = 0;
    };

    Specimen (const Specimen & other)
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

    Specimen& operator= (const Specimen & other)
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

    ~Specimen () { delete [] shape; delete [] mesh; delete [] mesh_midpoints; };

    double getXY (const ushort x, const ushort y) const { return shape[y*xsize+x]; };

    void setXY (const ushort x, const ushort y, const double val) 
    { 
        if (val < 0) 
            shape[y*xsize+x] = 0; 
        else if (val > hmax)
            shape[y*xsize+x] = hmax;
        else
            shape[y*xsize+x] = val; 
        return; 
    };

    void showYourself () 
    {
        std::cout << (int)id << " " << xsize << " " << ysize << std::endl;
        for (int y = 0; y < ysize; y++) {
            for (int x = 0; x < xsize; x++)
                std::cout << getXY(x,y) << " ";
            std::cout << std::endl;
        }
        return;
    };

    vertex xProduct     (const vertex & a, const vertex & b) { return  vertex(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x) ; };

    double dotProduct   (const vertex & a, const vertex & b) { return a.x * b.x + a.y * b.y + a.z * b.z; } ;

    vertex substract    (const vertex & a, const vertex & b) { return vertex(a.x-b.x, a.y-b.y, a.z-b.z); } ;

    vertex add          (const vertex & a, const vertex & b) { return vertex(a.x+b.x, a.y+b.y, a.z+b.z); } ;

    vertex multiply     (const vertex & a, double c) { return vertex(a.x*c, a.y*c, a.z*c); };

    vertex divide       (const vertex & a, double c) { return vertex(a.x/c, a.y/c, a.z/c); };

    vertex unitNormal   (const vertex & v0, const vertex & v1, const vertex & v2) 
    {
        vertex normal;
        float vmod;

        normal = xProduct(substract(v1, v0), substract(v2, v1));

        vmod = sqrt(pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2));

        if (vmod > (float)1.0e-9) {
            normal.x /= vmod;
            normal.y /= vmod;
            normal.z /= vmod;
        };

        return normal;
    };

    vertex unitNormal   (const triangle & t) 
    {
        vertex normal;
        float vmod;

        normal = xProduct(substract(t.v[1], t.v[0]), substract(t.v[2], t.v[1]));

        vmod = sqrt(pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2));

        if (vmod > (float)1.0e-9) 
            normal = divide(normal, vmod);

        return normal;
    };

    vertex tMidPoint    (const triangle & t) 
    { 
        // vertex circumcenter;
        const vertex a = substract (t.v[0], t.v[2]);  // move t.v[2] to origin
        const vertex b = substract (t.v[1], t.v[2]);  // -||-

        const vertex axb = xProduct(a, b);

        return add(divide(  xProduct( substract(multiply(b, dotProduct(a, a)), multiply(a, dotProduct(b, b))), axb)
                           ,dotProduct(axb, axb) * 2)
                  ,t.v[2]);
        // return circumcenter;
        // return vertex( (t.v[0].x+t.v[1].x+t.v[2].x)/3, (t.v[0].y+t.v[1].y+t.v[2].y)/3, (t.v[0].z+t.v[1].z+t.v[2].z)/3); 
    };

    vertex calculateReflection(const triangle & t, const vertex & ray) 
    {
        const vertex invray(substract(vertex(0,0,0), ray));
        return substract( multiply(t.normal, 2*dotProduct(t.normal, invray)), invray);
    };

    bool rayIsBlocked (const vertex & source, const vertex & ray, const triangle & target)
    {
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

    bool rayMeetsObstacle (const vertex & source, const vertex & ray, const triangle & target)
    {
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

    void computeMesh ()
    {
        int i = 0;

        for (int y = 0; y < ysize-1; y++)
            for (int x = 0; x < xsize-1; x++)
            {
                mesh[i].v[0].x = x  ; mesh[i].v[0].z = y  ; mesh[i].v[0].y = getXY(x  ,y  );  // swapped coordinates
                mesh[i].v[1].x = x  ; mesh[i].v[1].z = y+1; mesh[i].v[1].y = getXY(x  ,y+1);
                mesh[i].v[2].x = x+1; mesh[i].v[2].z = y  ; mesh[i].v[2].y = getXY(x+1,y  );

                mesh[i].normal = unitNormal(mesh[i]);

                ++i;

                mesh[i].v[0].x = x+1; mesh[i].v[0].z = y+1; mesh[i].v[0].y = getXY(x+1,y+1);
                mesh[i].v[1].x = x+1; mesh[i].v[1].z = y  ; mesh[i].v[1].y = getXY(x+1,y  );
                mesh[i].v[2].x = x  ; mesh[i].v[2].z = y+1; mesh[i].v[2].y = getXY(x  ,y+1);

                mesh[i].normal = unitNormal(mesh[i]);

                ++i;

            }
        return;
    }

    void computeMeshMidpoints ()
    {
        for (uint32_t i = 0; i < triangle_count; i++)
            mesh_midpoints[i] = tMidPoint(mesh[i]);
        return;
    }

    void computeFitness (const vertex* rays, const int count_rays)
    {
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

        return;
    };

    void exportAsSTL (std::string name) // slow because of saving small chunks of data
    {
        std::ofstream stlout;
        std::stringstream ss;   // sped up even more by saving to memory first and then to file
        

        for (uint32_t i = 0; i < triangle_count; i +=2) // sped up a bit by making chunks bigger (number of triangles is always divisible by 2)
        {
            ss << "facet normal " << mesh[i].normal.x << " " << mesh[i].normal.y << " " << mesh[i].normal.z << std::endl
             << "   outer loop" << std::endl
             << "      vertex " << mesh[i].v[0].x << " " << mesh[i].v[0].y << " " << mesh[i].v[0].z << std::endl
             << "      vertex " << mesh[i].v[1].x << " " << mesh[i].v[1].y << " " << mesh[i].v[1].z << std::endl
             << "      vertex " << mesh[i].v[2].x << " " << mesh[i].v[2].y << " " << mesh[i].v[2].z << std::endl
             << "   endloop" << std::endl
             << "endfacet" << std::endl
             
             << "facet normal " << mesh[i+1].normal.x << " " << mesh[i+1].normal.y << " " << mesh[i+1].normal.z << std::endl
             << "   outer loop" << std::endl
             << "      vertex " << mesh[i+1].v[0].x << " " << mesh[i+1].v[0].y << " " << mesh[i+1].v[0].z << std::endl
             << "      vertex " << mesh[i+1].v[1].x << " " << mesh[i+1].v[1].y << " " << mesh[i+1].v[1].z << std::endl
             << "      vertex " << mesh[i+1].v[2].x << " " << mesh[i+1].v[2].y << " " << mesh[i+1].v[2].z << std::endl
             << "   endloop" << std::endl
             << "endfacet" << std::endl;
            
        }

        stlout.open(name, std::ofstream::trunc);

        stlout << "solid " << std::to_string(id) << std::endl;
        stlout << ss.rdbuf();
        stlout << "endsolid " << std::to_string(id) << std::endl;

        stlout.close();
    };

    void exportReflectionAsSTL () // slow because of saving small chunks of data
    {
        std::ofstream stlout;
        std::stringstream ss;   // sped up even more by saving to memory first and then to file
        

        for (auto i = reflecting.begin(); i != reflecting.end(); i++) // sped up a bit by making chunks bigger (number of triangles is always divisible by 2)
        {
            ss << "facet normal " << i->normal.x << " " << i->normal.y << " " << i->normal.z << std::endl
             << "   outer loop" << std::endl
             << "      vertex " << i->v[0].x << " " << i->v[0].y << " " << i->v[0].z << std::endl
             << "      vertex " << i->v[1].x << " " << i->v[1].y << " " << i->v[1].z << std::endl
             << "      vertex " << i->v[2].x << " " << i->v[2].y << " " << i->v[2].z << std::endl
             << "   endloop" << std::endl
             << "endfacet" << std::endl;
            
        }

        stlout.open(std::to_string(id)+ "_reflection.stl", std::ofstream::trunc);

        stlout << "solid " << std::to_string(id) << std::endl;
        stlout << ss.rdbuf();
        stlout << "endsolid " << std::to_string(id) << std::endl;

        stlout.close();
    };

};


using namespace std;

int main (int argc, char** argv) 
{
    // auto start = chrono::high_resolution_clock::now();
    // auto end = chrono::high_resolution_clock::now();
    // std::chrono::duration<double> deltatime = end - start;

    ushort xsize;   // size of the panel (in mm preferably?)
    ushort ysize;   // -||-
    ushort hmax;    // max height of a panel

    vertex ray(0, -1, 0);

    ushort popsize; // size of the population

    ushort generation = 1;  // number of current generation

    // +1 because, ex.: xsize, ysize = 2, 2 gives 2 triangles/1 square with a side of length 1mm 
    if (argc == 5) {
        xsize   = atoi(argv[1])+1;
        ysize   = atoi(argv[2])+1;
        hmax    = atoi(argv[3])+1;
        popsize = atoi(argv[4]);
    } else {
        xsize   = 180+1;    // size of printbed (minus some spare space)
        ysize   = 940+1;    // length of aluminum rod
        hmax    = 180+1;    // height of printbed (minus some spare space)
        popsize = 60;       // make popsize divisible by 4 xd
    }

    random_device rd;
    mt19937 mt(rd());
    // uniform_real_distribution<double> dist(0.0, (double)hmax/10.0);
    uniform_real_distribution<double> dist(0.0, 0.45);//(double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start
    uniform_int_distribution<uint32_t> rand_id(0, numeric_limits<uint32_t>::max());

    //std::cout << dist(mt) << "\n";

    Obstacle obs("obstacle.stl");
    obs.moveXY((xsize-1)/2, (hmax-1)/2);
    //obs.exportSTL("my_obstacle.stl");

    // vector<Specimen*> seed;//(0, xsize, ysize, hmax, &obs);
    // // cout << "hmax";
    // for (int i = 0; i < 12; i++)
    // {
    //     seed.push_back(new Specimen(rand_id(mt), xsize, ysize, hmax, &obs));
    //     // cout << ";S" << to_string(i);
    // }
    // cout << endl;

    // while (true)
    // for (double i = 0.4; i <= 0.55; i+=0.0025)
    // {
    //     #pragma omp parallel for num_threads(4)
    //     for (auto pop = seed.begin(); pop != seed.end(); pop++)
    //     {
    //         for (int j = 0; j < xsize*ysize; j++)
    //             (*pop)->setXY(j, 0, dist(mt)*(i/(double)hmax));
    //         (*pop)->computeMesh();
    //         (*pop)->computeMeshMidpoints();
    //         (*pop)->computeFitness(&ray, 1);        
    //     }

    //     cout << to_string(i);
    //     for (auto pop = seed.begin(); pop != seed.end(); pop++)
    //         cout << ";" << to_string((*pop)->fitness);
    //     cout << endl;
        
    // }


    // return 0;

    // start = chrono::high_resolution_clock::now();
    vector<Specimen*> population;
    for (int i = 0, j = 0; i < popsize; i++, j++)
    {
        population.push_back(new Specimen(rand_id(mt), xsize, ysize, hmax, &obs));
        for (int k = 0; k < xsize*ysize; k++)
            population[i]->setXY(k, 0, dist(mt));
        population[i]->computeMesh();
        population[i]->computeMeshMidpoints();
        // population[i].computeFitness(&ray, 1);
        // if(population[i].fitness < 1000)
        // {
        //     cout << to_string(j) << "th try; fitness found: " << to_string(population[i].fitness) << endl;
        //     population.pop_back();
        //     i--;
        // }
        // else
        // {
        //     cout << to_string(i) << " out of " << to_string(popsize) << "; found after " << to_string(j) << " tries." << endl;
        //     j = 0;
        // }
        // population[i].exportAsSTL();    

    }
    // end = chrono::high_resolution_clock::now();
    // deltatime = end - start;
    // std::cout << "Setting up population: " << deltatime.count() << " s\n";

    cout << "Gen";
    for (int i = 0; i < popsize; i++)
        cout << ";F" << to_string(i);
    cout << endl;
    while (true)
    {
        // start = chrono::high_resolution_clock::now();
        #pragma omp parallel for num_threads(3)
        for (auto pop = population.rbegin(); pop != population.rend(); pop++)
            if ((*pop)->fitness == 0)
                (*pop)->computeFitness(&ray, 1);
                // else cout << "skipped" << endl;
            // pop.exportReflectionsAsSTL(); // ??
        // end = chrono::high_resolution_clock::now();
        // deltatime = end - start;
        // std::cout << "Computing fitness: " << deltatime.count() << " s\n";
        sort(population.begin(), population.end(), sortBestToWorst); // sorted best to worst

        cout << to_string(generation);
        for (auto pop = population.begin(); pop != population.end(); pop++)
            cout << ";" << to_string((*pop)->fitness);
        cout << endl;

        // start = chrono::high_resolution_clock::now();
        for (int i = 0; i < popsize/3 ; i++)    // remove the weak
        {
            delete population.back();
            population.pop_back();
        }

        // end = chrono::high_resolution_clock::now();
        // deltatime = end - start;
        // std::cout << "Erasing: " << deltatime.count() << " s\n";
        
        // start = chrono::high_resolution_clock::now();
        for (auto pop = population.begin(); population.size() < popsize; pop++)
        {
            population.push_back(crossoverAndMutate(*(*pop), *(*(pop+1)), rand_id(mt), 60, 5));
        }
        // end = chrono::high_resolution_clock::now();
        // deltatime = end - start;
        // std::cout << "Crossover: " << deltatime.count() << " s\n";

        // dont mutate the elites
        // cout << "crossovered" << endl;

        ++generation;
    }


    return 0;

}


bool sortBestToWorst (Specimen* a, Specimen* b) { return (a->fitness > b->fitness); }

Specimen* crossoverAndMutate (Specimen & a, Specimen & b, int id, double crossover_bias, int mutation_percent)
{
    Specimen* temp = new Specimen(id, a.xsize, a.ysize, a.hmax, a.obstacle);

    random_device rd;
    mt19937 mt(rd());
    // uniform_real_distribution<double> height(0.0, (double)a.hmax);
    uniform_real_distribution<double> height(-0.225, 0.225);
    uniform_real_distribution<double> diceroll(0.0, (double)100.0);
    

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

