#ifndef SPECIMEN_HPP
#define SPECIMEN_HPP


#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>

class Specimen {
public:
    uint32_t id;   // unique identifier for a Specimen (form 0 to a-lot)

    ushort xsize;   // size of the panel
    ushort ysize;
    ushort hmax;  // maximal height (dictated by max printing height)

    double* shape;  // table for heights at x and y coordinates -- access via (getter, but..) [y*xsize+x] -- this is chromosome

    double fitness; // fitness of a specimen (% of triangles that reflected the light)
    double norm_fitness;    // normalized fitness

    Mesh3d shape_mesh; // mesh calculated from 'shape' member
    vertex* mesh_midpoints; // precalculated mid points of every triangle of a mesh

    std::vector<triangle> reflecting; // I think it's for checking the shape of the mesh built only from triangles that reflect the ray directly onto the obstacle - it's an overkill to store all triangles, just store bools or something

    Mesh3d* obstacle; // pointer to obstacle to read its mesh


    Specimen(const unsigned char num, const unsigned short xs, const unsigned short ys, const unsigned short hm, Mesh3d* obs);
    Specimen (const Specimen & other);
    Specimen& operator= (const Specimen & other);
    ~Specimen();

    double getXY(const unsigned short x, const unsigned short y) const;
    void setXY(const unsigned short x, const unsigned short y, const double val);
    void showYourself();
    
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