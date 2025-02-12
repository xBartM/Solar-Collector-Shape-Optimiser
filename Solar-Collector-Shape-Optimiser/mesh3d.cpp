#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <execution>
#include <ranges>

#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>


Mesh3d::Mesh3d() {}

Mesh3dSoA::Mesh3dSoA() : triangle_count(0) {}

Mesh3d::Mesh3d(const uint32_t triangle_count) : triangle_count(triangle_count) {
    // Allocate memory for the new mesh based on the copied triangle count
    mesh.resize(triangle_count);
    
}

Mesh3dSoA::Mesh3dSoA(const uint32_t triangle_count) : triangle_count(triangle_count) {
    // Reserve space to avoid reallocations.
    v0x.reserve(triangle_count);
    v0y.reserve(triangle_count);
    v0z.reserve(triangle_count);
    v1x.reserve(triangle_count);
    v1y.reserve(triangle_count);
    v1z.reserve(triangle_count);
    v2x.reserve(triangle_count);
    v2y.reserve(triangle_count);
    v2z.reserve(triangle_count);
    normx.reserve(triangle_count);
    normy.reserve(triangle_count);
    normz.reserve(triangle_count);
    midpx.reserve(triangle_count);
    midpy.reserve(triangle_count);
    midpz.reserve(triangle_count);

    // Resize to pre-allocate.  Values will be zero-initialized.
    v0x.resize(triangle_count);
    v0y.resize(triangle_count);
    v0z.resize(triangle_count);
    v1x.resize(triangle_count);
    v1y.resize(triangle_count);
    v1z.resize(triangle_count);
    v2x.resize(triangle_count);
    v2y.resize(triangle_count);
    v2z.resize(triangle_count);
    normx.resize(triangle_count);
    normy.resize(triangle_count);
    normz.resize(triangle_count);
    midpx.resize(triangle_count);
    midpy.resize(triangle_count);
    midpz.resize(triangle_count);

}

Mesh3d::Mesh3d(const std::string filename) { importSTL(filename); }

// Constructor from file (STL)
Mesh3dSoA::Mesh3dSoA(const std::string filename) {
    std::string line;
    std::ifstream file(filename, std::ifstream::in);

    size_t pos;

    if (file.is_open()) {
        triangle_count = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
        triangle_count -= 2;
        triangle_count /= 7;

        // Reserve space to avoid reallocations.
        v0x.reserve(triangle_count);
        v0y.reserve(triangle_count);
        v0z.reserve(triangle_count);
        v1x.reserve(triangle_count);
        v1y.reserve(triangle_count);
        v1z.reserve(triangle_count);
        v2x.reserve(triangle_count);
        v2y.reserve(triangle_count);
        v2z.reserve(triangle_count);
        normx.reserve(triangle_count);
        normy.reserve(triangle_count);
        normz.reserve(triangle_count);
        midpx.reserve(triangle_count);
        midpy.reserve(triangle_count);
        midpz.reserve(triangle_count);

        // Resize to access elements via operator[]
        v0x.resize(triangle_count);
        v0y.resize(triangle_count);
        v0z.resize(triangle_count);
        v1x.resize(triangle_count);
        v1y.resize(triangle_count);
        v1z.resize(triangle_count);
        v2x.resize(triangle_count);
        v2y.resize(triangle_count);
        v2z.resize(triangle_count);
        normx.resize(triangle_count);
        normy.resize(triangle_count);
        normz.resize(triangle_count);
        midpx.resize(triangle_count);
        midpy.resize(triangle_count);
        midpz.resize(triangle_count);


        file.seekg(0, file.beg);    // return to beginning
        file.clear();               // clear flags (especially eof flag)

        std::getline(file, line);   // skip the name of the solid
        std::getline(file, line);   // get the "facet normal" line ready

        for (int i = 0; line.find("endsolid") == std::string::npos; i++) { // read untill you find "endsolid";
            // get normal
            pos = line.rfind(' ');      // find last ' '
            normz[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            normy[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            normx[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);

            std::getline(file, line);   // skip the "outer loop" line
                
            // vertex 0
            std::getline(file, line);   // get the first "vertex" line ready
            pos = line.rfind(' ');      // find last ' '
            v0z[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            v0y[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            v0x[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);

            // vertex 1
            std::getline(file, line);   // get the second "vertex" line ready
            pos = line.rfind(' ');      // find last ' '
            v1z[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            v1y[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            v1x[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);

            // vertex 2
            std::getline(file, line);   // get the third "vertex" line ready
            pos = line.rfind(' ');      // find last ' '
            v2z[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            v2y[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            v2x[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);


            std::getline(file, line);   // skip "endloop"
            std::getline(file, line);   // skip "endfacet"

            std::getline(file, line);   // get the "facet normal" line ready (or "endsolid" line)
            
        }
        file.close();

        findCircumcentres();
    }

}

Mesh3d::Mesh3d(const Mesh3d& other) : triangle_count(other.triangle_count), mesh(other.mesh) {}

Mesh3dSoA::Mesh3dSoA(const Mesh3dSoA& other) : triangle_count(other.triangle_count) {
    v0x = other.v0x;
    v0y = other.v0y;
    v0z = other.v0z;
    v1x = other.v1x;
    v1y = other.v1y;
    v1z = other.v1z;
    v2x = other.v2x;
    v2y = other.v2y;
    v2z = other.v2z;
    normx = other.normx;
    normy = other.normy;
    normz = other.normz;
    midpx = other.midpx;
    midpy = other.midpy;
    midpz = other.midpz;
}

Mesh3d::~Mesh3d() {}

Mesh3dSoA::~Mesh3dSoA() {}


triangle& Mesh3d::operator[] (const uint32_t index) {
    // if (index >= triangle_count) {
    //     throw std::out_of_range("Index out of bounds");
    // }
    return mesh[index];
}

Mesh3d& Mesh3d::operator= (const Mesh3d& other) {
    if (this != &other) { // Self-assignment check
        triangle_count = other.triangle_count;
        mesh = other.mesh; // Use vector's assignment operator
    }
    return *this;
}

// Assignment operator
Mesh3dSoA& Mesh3dSoA::operator=(const Mesh3dSoA& other) {
    if (this != &other) { // Protect against self-assignment
        triangle_count = other.triangle_count;

        v0x = other.v0x;
        v0y = other.v0y;
        v0z = other.v0z;
        v1x = other.v1x;
        v1y = other.v1y;
        v1z = other.v1z;
        v2x = other.v2x;
        v2y = other.v2y;
        v2z = other.v2z;
        normx = other.normx;
        normy = other.normy;
        normz = other.normz;
        midpx = other.midpx;
        midpy = other.midpy;
        midpz = other.midpz;
    }
    return *this;
}


void Mesh3d::moveXY(const double x, const double y) {
    for (auto& t : mesh) {
        t.v[0].x += x; t.v[1].x += x; t.v[2].x += x;
        t.v[0].y += y; t.v[1].y += y; t.v[2].y += y;
    }
}

void Mesh3dSoA::moveXY(const double& x, const double& y) {
    // Use std::for_each with std::execution::par_unseq for parallel execution.

    // Apply the translation to all x and y components of vertices and midpoints.
    auto translate_x = [x](double& val) { val += x; };
    auto translate_y = [y](double& val) { val += y; };

    // Vertices v0
    std::for_each(std::execution::par_unseq, v0x.begin(), v0x.end(), translate_x);
    std::for_each(std::execution::par_unseq, v0y.begin(), v0y.end(), translate_y);

    // Vertices v1
    std::for_each(std::execution::par_unseq, v1x.begin(), v1x.end(), translate_x);
    std::for_each(std::execution::par_unseq, v1y.begin(), v1y.end(), translate_y);

    // Vertices v2
    std::for_each(std::execution::par_unseq, v2x.begin(), v2x.end(), translate_x);
    std::for_each(std::execution::par_unseq, v2y.begin(), v2y.end(), translate_y);

    // Midpoints
    std::for_each(std::execution::par_unseq, midpx.begin(), midpx.end(), translate_x);
    std::for_each(std::execution::par_unseq, midpy.begin(), midpy.end(), translate_y);

    // Do NOT transform normals.  Normals represent direction and are invariant under translation.
}

void Mesh3d::importSTL(const std::string filename) {
    std::string line;
    std::ifstream file(filename, std::ifstream::in);

    size_t pos;

    if (file.is_open()) {
        triangle_count = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
        triangle_count -= 2;
        triangle_count /= 7;

        mesh.resize(triangle_count);

        file.seekg(0, file.beg);    // return to beginning
        file.clear();               // clear flags (especially eof flag)

        std::getline(file, line);   // skip the name of the solid
        std::getline(file, line);   // get the "facet normal" line ready

        for (int i = 0; line.find("endsolid") == std::string::npos; i++) { // read untill you find "endsolid";
            // get normal
            pos = line.rfind(' ');      // find last ' '
            mesh[i].normal.z = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            mesh[i].normal.y = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            mesh[i].normal.x = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);

            std::getline(file, line);   // skip the "outer loop" line
                
            // vertex 0
            std::getline(file, line);   // get the first "vertex" line ready
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[0].z = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[0].y = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[0].x = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);

            // vertex 1
            std::getline(file, line);   // get the second "vertex" line ready
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[1].z = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[1].y = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[1].x = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);

            // vertex 2
            std::getline(file, line);   // get the third "vertex" line ready
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[2].z = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[2].y = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);
            pos = line.rfind(' ');      // find last ' '
            mesh[i].v[2].x = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
            line = line.substr(0, pos);


            std::getline(file, line);   // skip "endloop"
            std::getline(file, line);   // skip "endfacet"

            std::getline(file, line);   // get the "facet normal" line ready (or "endsolid" line)
            
        }
        file.close();
    }
}

void Mesh3d::exportSTL(const std::string filename) {
    std::ofstream stlout;
    
    stlout.open(filename, std::ofstream::trunc);
    stlout << "solid Mesh3d" << std::endl;

    for (const auto& t : mesh) {
        stlout << "facet normal " << t.normal.x << " " << t.normal.y << " " << t.normal.z << std::endl;
        stlout << "   outer loop" << std::endl;
        stlout << "      vertex " << t.v[0].x << " " << t.v[0].y << " " << t.v[0].z << std::endl;
        stlout << "      vertex " << t.v[1].x << " " << t.v[1].y << " " << t.v[1].z << std::endl;
        stlout << "      vertex " << t.v[2].x << " " << t.v[2].y << " " << t.v[2].z << std::endl;
        stlout << "   endloop" << std::endl;
        stlout << "endfacet" << std::endl;
    }

    stlout << "endsolid Mesh3d" << std::endl;
    stlout.close();
}

void Mesh3dSoA::exportSTL(const std::string filename) {
    std::ofstream stlout(filename, std::ofstream::trunc); // open file in a constructor, will be closed by destructor
    std::stringstream stlmem; // Build the entire string in memory

    stlmem << "solid Mesh3d" << std::endl;

    for (uint32_t i = 0; i < triangle_count; ++i) {
        stlmem << "facet normal " << normx[i] << " " << normy[i] << " " << normz[i] << std::endl;
        stlmem << "   outer loop" << std::endl;
        stlmem << "      vertex " << v0x[i] << " " << v0y[i] << " " << v0z[i] << std::endl;
        stlmem << "      vertex " << v1x[i] << " " << v1y[i] << " " << v1z[i] << std::endl;
        stlmem << "      vertex " << v2x[i] << " " << v2y[i] << " " << v2z[i] << std::endl;
        stlmem << "   endloop" << std::endl;
        stlmem << "endfacet" << std::endl;
    }

    stlmem << "endsolid Mesh3d" << std::endl;

    stlout << stlmem.str(); // Write the entire string at once
}

vertex xProduct(const vertex& a, const vertex& b) {
    return vertex(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

double dotProduct(const vertex& a, const vertex& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vertex substract(const vertex& a, const vertex& b) {
    return vertex(a.x - b.x, a.y - b.y, a.z - b.z);
}

vertex add(const vertex& a, const vertex& b) {
    return vertex(a.x + b.x, a.y + b.y, a.z + b.z);
}

vertex multiply(const vertex& a, double c) {
    return vertex(a.x * c, a.y * c, a.z * c);
}

vertex divide(const vertex& a, double c) {
    return vertex(a.x / c, a.y / c, a.z / c);
}

vertex unitNormal(const vertex& v0, const vertex& v1, const vertex& v2) {
    vertex normal;
    float vmod;

    normal = xProduct(substract(v1, v0), substract(v2, v1));
    vmod = sqrt(pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2));

    if (vmod > (float)1.0e-9) {
        normal.x /= vmod;
        normal.y /= vmod;
        normal.z /= vmod;
    }

    return normal;
}

vertex unitNormal(const triangle& t) {
    vertex normal;
    float vmod;

    normal = xProduct(substract(t.v[1], t.v[0]), substract(t.v[2], t.v[1]));
    vmod = sqrt(pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2));

    if (vmod > (float)1.0e-9)
        normal = divide(normal, vmod);

    return normal;
}

vertex tMidPoint(const triangle& t) {
    // vertex circumcenter;
    const vertex a = substract (t.v[0], t.v[2]);  // move t.v[2] to origin
    const vertex b = substract (t.v[1], t.v[2]);  // -||-

    const vertex axb = xProduct(a, b);

    return add(divide(xProduct(substract(multiply(b, dotProduct(a, a)), multiply(a, dotProduct(b, b))), axb), dotProduct(axb, axb) * 2), t.v[2]);
}

void Mesh3dSoA::findCircumcentres() {

    // Use parallel execution policy for maximum performance.
    std::for_each(std::execution::par_unseq,
                  std::views::iota(0u, triangle_count).begin(), // Use iota view
                  std::views::iota(0u, triangle_count).end(),
                  [this](uint32_t i) {
        // Calculate circumcenter for triangle i.

        // Fetch vertices.  Use 'const' to allow the compiler to optimize more.
        const double ax = v0x[i];
        const double ay = v0y[i];
        const double az = v0z[i];
        const double bx = v1x[i];
        const double by = v1y[i];
        const double bz = v1z[i];
        const double cx = v2x[i];
        const double cy = v2y[i];
        const double cz = v2z[i];

        // Calculate intermediate values.  Minimize redundant calculations.
        const double bax = bx - ax;
        const double bay = by - ay;
        const double baz = bz - az;
        const double cax = cx - ax;
        const double cay = cy - ay;
        const double caz = cz - az;

        const double ba_mag2 = bax * bax + bay * bay + baz * baz;
        const double ca_mag2 = cax * cax + cay * cay + caz * caz;

        // Compute cross product of (B - A) and (C - A).
        const double cross_x = bay * caz - baz * cay;
        const double cross_y = baz * cax - bax * caz;
        const double cross_z = bax * cay - bay * cax;

        //  Compute the denominator of the circumcenter calculation.
        const double denom = 0.5 / (cross_x * cross_x + cross_y * cross_y + cross_z * cross_z);

        // Compute circumcenter coordinates.
        const double ox = denom * (ba_mag2 * (cay * cross_z - caz * cross_y) + ca_mag2 * (baz * cross_y - bay * cross_z));
        const double oy = denom * (ba_mag2 * (caz * cross_x - cax * cross_z) + ca_mag2 * (bax * cross_z - baz * cross_x));
        const double oz = denom * (ba_mag2 * (cax * cross_y - cay * cross_x) + ca_mag2 * (bay * cross_x - bax * cross_y));
        
        midpx[i] = ox + ax;
        midpy[i] = oy + ay;
        midpz[i] = oz + az;
    });
}


vertex calculateReflection(const triangle& t, const vertex& ray) {
    const vertex invray(substract(vertex(0, 0, 0), ray));
    return substract(multiply(t.normal, 2 * dotProduct(t.normal, invray)), invray);
}
