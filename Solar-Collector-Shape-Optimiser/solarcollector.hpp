#ifndef SOLARCOLLECTOR_HPP
#define SOLARCOLLECTOR_HPP


#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>
#include <Solar-Collector-Shape-Optimiser/genome.hpp>

class SolarCollector : public Genome { // Inherits from Genome
public:
    uint32_t xsize;   // size of the panel
    uint32_t ysize;
    uint32_t hmax;  // maximal height (dictated by max printing height)

    Mesh3d shape_mesh; // mesh calculated from 'shape' member
    vertex* mesh_midpoints; // precalculated mid points of every triangle of a mesh

    std::vector<triangle> reflecting; // I think it's for checking the shape of the mesh built only from triangles that reflect the ray directly onto the obstacle - it's an overkill to store all triangles, just store bools or something

    Mesh3d* obstacle; // pointer to obstacle to read its mesh


    SolarCollector (const uint32_t xs, const uint32_t ys, const uint32_t hm, Mesh3d* obs);
    SolarCollector (const SolarCollector & other);
    SolarCollector (SolarCollector&& other) noexcept;
    SolarCollector& operator= (SolarCollector&& other) noexcept;
    SolarCollector& operator= (const SolarCollector & other);
    ~SolarCollector();

    double getXY(const uint32_t x, const uint32_t y) const;
    void setXY(const uint32_t x, const uint32_t y, const double val);
    void showYourself();
    
    bool rayIsBlocked(const vertex& source, const vertex& ray, const triangle& target);
    bool rayMeetsObstacle(const vertex& source, const vertex& ray, const triangle& target);
    void computeMesh();
    void computeMeshMidpoints();
    void computeFitness(const vertex* rays, const uint32_t count_rays);
    void exportAsSTL(std::string name);
    void exportReflectionAsSTL();
};

// SolarCollector crossoverAndMutate (SolarCollector & a, SolarCollector & b, uint32_t id, double crossover_bias, int mutation_percent);

#endif // SOLARCOLLECTOR_HPP