#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <string>

#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>

class Obstacle {
public:
    int triangle_count;
    triangle* mesh;

    void moveXY(const double x, const double y);
    void importSTL(const std::string filename);
    void exportSTL(const std::string filename);

    Obstacle();
    Obstacle(const std::string filename);
    ~Obstacle();
};

#endif