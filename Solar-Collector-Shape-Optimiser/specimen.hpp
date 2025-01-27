#ifndef SPECIMEN_HPP
#define SPECIMEN_HPP


#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>
#include <Solar-Collector-Shape-Optimiser/obstacle.hpp>

class Specimen {
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


    Specimen(const unsigned char num, const unsigned short xs, const unsigned short ys, const unsigned short hm, Obstacle* obs);
    Specimen (const Specimen & other);
    Specimen& operator= (const Specimen & other);
    ~Specimen();

    double getXY(const unsigned short x, const unsigned short y) const;
    void setXY(const unsigned short x, const unsigned short y, const double val);
    void showYourself();
    
    vertex xProduct(const vertex& a, const vertex& b);
    double dotProduct(const vertex& a, const vertex& b);
    vertex substract(const vertex& a, const vertex& b);
    vertex add(const vertex& a, const vertex& b);
    vertex multiply(const vertex& a, double c);
    vertex divide(const vertex& a, double c);
    vertex unitNormal(const vertex& v0, const vertex& v1, const vertex& v2);
    vertex unitNormal(const triangle& t);
    vertex tMidPoint(const triangle& t);
    vertex calculateReflection(const triangle& t, const vertex& ray);
    bool rayIsBlocked(const vertex& source, const vertex& ray, const triangle& target);
    bool rayMeetsObstacle(const vertex& source, const vertex& ray, const triangle& target);
    void computeMesh();
    void computeMeshMidpoints();
    void computeFitness(const vertex* rays, const int count_rays);
    void exportAsSTL(std::string name);
    void exportReflectionAsSTL();
};

bool sortBestToWorst (Specimen* a, Specimen* b);
Specimen* crossoverAndMutate (Specimen & a, Specimen & b, int id, double crossover_bias, int mutation_percent);

#endif