#ifndef MESH3D_HPP
#define MESH3D_HPP


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

#endif // MESH3D