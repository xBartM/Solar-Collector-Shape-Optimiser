#ifndef SOLARCOLLECTOR_HPP
#define SOLARCOLLECTOR_HPP


#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>
#include <Solar-Collector-Shape-Optimiser/genome.hpp>

class SolarCollector : public Genome { // Inherits from Genome
public:
    ushort xsize;   // size of the panel
    ushort ysize;
    ushort hmax;  // maximal height (dictated by max printing height)

    // Genome genes; // everything GA related - REMOVED (inherited)
    // double* shape;  // table for heights at x and y coordinates -- access via (getter, but..) [y*xsize+x] -- this is chromosome - REMOVED (inherited)

    Mesh3d shape_mesh; // mesh calculated from 'shape' member
    vertex* mesh_midpoints; // precalculated mid points of every triangle of a mesh

    std::vector<triangle> reflecting; // I think it's for checking the shape of the mesh built only from triangles that reflect the ray directly onto the obstacle - it's an overkill to store all triangles, just store bools or something

    Mesh3d* obstacle; // pointer to obstacle to read its mesh


    SolarCollector (const unsigned char num, const unsigned short xs, const unsigned short ys, const unsigned short hm, Mesh3d* obs);
    SolarCollector (const SolarCollector & other);
    SolarCollector (SolarCollector&& other) noexcept;
    SolarCollector& operator= (SolarCollector&& other) noexcept;
    SolarCollector& operator= (const SolarCollector & other);
    // Removed spaceship operator, because it is inherited
    ~SolarCollector();

    // getXY and setXY moved to Genome, because they are now accessing the member of the parent class
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

// Changed parameters
SolarCollector crossoverAndMutate (SolarCollector & a, SolarCollector & b, uint32_t id, double crossover_bias, int mutation_percent);

#endif // SOLARCOLLECTOR_HPP