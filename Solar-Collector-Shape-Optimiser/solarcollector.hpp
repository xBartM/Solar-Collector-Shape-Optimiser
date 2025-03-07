#ifndef SOLARCOLLECTOR_HPP
#define SOLARCOLLECTOR_HPP


#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>
#include <Solar-Collector-Shape-Optimiser/genome.hpp>

class SolarCollector : public Genome { // Inherits from Genome
public:
    uint32_t xsize;   // size of the panel
    uint32_t ysize;
    uint32_t hmax;  // maximal height (dictated by max printing height)

    Mesh3d shape_mesh; // mesh calculated from 'dna' member

    const Mesh3d* obstacle; // pointer to obstacle to read its mesh

    std::vector<triangle> reflecting; // I think it's for checking the shape of the mesh built only from triangles that reflect the ray directly onto the obstacle - it's an overkill to store all triangles, just store bools or something

    SolarCollector (const uint32_t xs, const uint32_t ys, const uint32_t hm, const Mesh3d* obs);
    SolarCollector (const uint32_t xs, const uint32_t ys, const uint32_t hm, const Mesh3d* obs, const Genome &genome);
    // copy, move constructors, assignments can be default
    ~SolarCollector();

    double getXY(const uint32_t x, const uint32_t y) const;
    void setXY(const uint32_t x, const uint32_t y, const double val);
    void showYourself() const;
    
    // when having a ray == (0, -1, 0) we can remove all the triangles that are not in the plane
    // so when having a ray == (x, y, z) we can transform all the coordinates so that ray becomes (0, -1, 0) and the check for intersection is faster
    bool rayObstacleHit(const double& sourcex, const double& sourcey, const double& sourcez, const vertex& ray, bool invertRay) const;
    void computeFitness(const std::vector<vertex>& rays);
    void computeMesh();
    void exportAsSTL(std::string name) const;
    void exportAsBinarySTL(std::string name) const;
    // void exportReflectionAsSTL();
};


#endif // SOLARCOLLECTOR_HPP