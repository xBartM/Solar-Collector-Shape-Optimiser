#ifndef MESH3D_HPP
#define MESH3D_HPP

#include <cstdint>
#include <string>

struct vertex {
    double x;
    double y;
    double z;

    vertex() : x(0), y(0), z(0) {};
    vertex(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {};
};

struct triangle {
    vertex normal;
    vertex v[3];

    triangle() : normal(), v{} {};
    triangle(vertex n, vertex v1, vertex v2, vertex v3) : normal(n), v{v1, v2, v3} {};
};

class Mesh3d {
public:
    uint32_t triangle_count; // number of triangles in mesh
    triangle* mesh; // mesh consisting of multiple triangles


    Mesh3d();
    Mesh3d(const uint32_t triangle_count);
    Mesh3d(const std::string filename);
    ~Mesh3d();

    void moveXY(const double x, const double y);
    void importSTL(const std::string filename);
    void exportSTL(const std::string filename);
};

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

#endif // MESH3D