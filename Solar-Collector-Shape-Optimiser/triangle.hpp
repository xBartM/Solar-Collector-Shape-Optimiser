#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <Solar-Collector-Shape-Optimiser/vertex.hpp>

struct triangle {
    vertex normal;
    vertex v[3];

    triangle() : normal(), v{} {};
    triangle(vertex n, vertex v1, vertex v2, vertex v3) : normal(n), v{v1, v2, v3} {};
};

#endif