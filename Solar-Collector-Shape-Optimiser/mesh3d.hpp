#ifndef MESH3D_HPP
#define MESH3D_HPP

#include <cstdint>
#include <string>
#include <vector>

struct vertex {
    double x;
    double y;
    double z;

    vertex() : x(0), y(0), z(0) {};
    vertex(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {};
};

struct triangle {
    vertex v[3]; // three points for vertices
    vertex normal; // normal to the triangle
    vertex midpoint; // midpoint of the triangle

    triangle() : v{}, normal(), midpoint() {};
    triangle(vertex v1, vertex v2, vertex v3, vertex n, vertex m) : v{v1, v2, v3}, normal(n), midpoint(m) {};
};

class Mesh3d {
public:
    uint32_t triangle_count; // number of triangles in mesh

    // vertices defining triangles
    std::vector<double> v0x, v0y, v0z; // x, y, z components of v0 vertex
    std::vector<double> v1x, v1y, v1z; // x, y, z components of v1 vertex
    std::vector<double> v2x, v2y, v2z; // x, y, z components of v2 vertex
    // vertices defining normals
    std::vector<double> normx, normy, normz; // x, y, z components of a normal
    // vertices defining midpoints
    std::vector<double> midpx, midpy, midpz; // x, y, z components of a midpoint

    // OPTIONAL: edges for finding intersections in with obstacle
    std::vector<double> e1x; std::vector<double> e1y; std::vector<double> e1z; // v1 - v0
    std::vector<double> e2x; std::vector<double> e2y; std::vector<double> e2z; // v2 - v0

    // TODO bounding box; make it recursive up to the depth of 4(?)
    vertex bbmin;
    vertex bbmax;

    // constructors and a destructor
    Mesh3d();
    explicit Mesh3d(const uint32_t triangle_count);
    Mesh3d(const std::string filename, const double xmove = 0.0, const double ymove = 0.0);

    // copy, move constructors, assignments can be defualt
    ~Mesh3d();

    // overloaded operators
    // triangle& operator[](const uint32_t index);

    // methods
    void findCircumcentres();
    void findNormals();
    void findEdges();
    void findBoundingBox();
    void moveXY(const double& x, const double& y);
    void exportSTL(const std::string& filename) const;
    void exportBinarySTL(const std::string& filename) const;
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
vertex calculateReflection(const vertex& normal, const vertex& ray);

Mesh3d importSTL(const std::string& filename);
Mesh3d importBinarySTL(const std::string& filename);


#endif // MESH3D