#ifndef SOLARCOLLECTOR_HPP
#define SOLARCOLLECTOR_HPP


#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>
#include <Solar-Collector-Shape-Optimiser/genome.hpp>

class SolarCollector {
public:
    ushort xsize;   // size of the panel
    ushort ysize;
    ushort hmax;  // maximal height (dictated by max printing height)

    Genome genes; // everything GA related
    // double* shape;  // table for heights at x and y coordinates -- access via (getter, but..) [y*xsize+x] -- this is chromosome

    Mesh3d shape_mesh; // mesh calculated from 'shape' member
    vertex* mesh_midpoints; // precalculated mid points of every triangle of a mesh

    std::vector<triangle> reflecting; // I think it's for checking the shape of the mesh built only from triangles that reflect the ray directly onto the obstacle - it's an overkill to store all triangles, just store bools or something

    Mesh3d* obstacle; // pointer to obstacle to read its mesh


    SolarCollector(const unsigned char num, const unsigned short xs, const unsigned short ys, const unsigned short hm, Mesh3d* obs);
    SolarCollector (const SolarCollector & other);
    SolarCollector& operator= (const SolarCollector & other);
    ~SolarCollector();

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

// move both of those to Genes
bool sortBestToWorst (SolarCollector* a, SolarCollector* b); 
SolarCollector* crossoverAndMutate (SolarCollector & a, SolarCollector & b, int id, double crossover_bias, int mutation_percent);


#endif // SOLARCOLLECTOR_HPP