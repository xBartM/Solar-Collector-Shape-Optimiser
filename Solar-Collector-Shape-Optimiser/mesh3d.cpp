#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <execution>
#include <ranges>

#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>


Mesh3d::Mesh3d() 
    : Mesh3d(0) 
{}

Mesh3d::Mesh3d(const uint32_t triangle_count) 
    : triangle_count(triangle_count) 
    , v0x(triangle_count), v0y(triangle_count), v0z(triangle_count)
    , v1x(triangle_count), v1y(triangle_count), v1z(triangle_count)
    , v2x(triangle_count), v2y(triangle_count), v2z(triangle_count)
    , normx(triangle_count), normy(triangle_count), normz(triangle_count)
    , midpx(triangle_count), midpy(triangle_count), midpz(triangle_count)
    , e1x(triangle_count), e1y(triangle_count), e1z(triangle_count)
    , e2x(triangle_count), e2y(triangle_count), e2z(triangle_count)
{}

// Constructor from file (STL)
Mesh3d::Mesh3d(const std::string filename, const double xmove, const double ymove)
    : Mesh3d(importBinarySTL(filename)) {  
    if (xmove != 0.0 && ymove != 0.0)
        moveXY(xmove, ymove);
    findCircumcentres();
    findEdges();
    findBoundingBox();
}

Mesh3d::~Mesh3d() {}

void Mesh3d::moveXY(const double& x, const double& y) {

    // Apply the translation to all x and y components of vertices and midpoints.
    auto translate_x = [x](double& val) { val += x; };
    auto translate_y = [y](double& val) { val += y; };

    #ifndef NO_STD_EXECUTION

        // Vertices v0
        std::for_each(std::execution::unseq, v0x.begin(), v0x.end(), translate_x);
        std::for_each(std::execution::unseq, v0y.begin(), v0y.end(), translate_y);

        // Vertices v1
        std::for_each(std::execution::unseq, v1x.begin(), v1x.end(), translate_x);
        std::for_each(std::execution::unseq, v1y.begin(), v1y.end(), translate_y);

        // Vertices v2
        std::for_each(std::execution::unseq, v2x.begin(), v2x.end(), translate_x);
        std::for_each(std::execution::unseq, v2y.begin(), v2y.end(), translate_y);

        // Midpoints
        std::for_each(std::execution::unseq, midpx.begin(), midpx.end(), translate_x);
        std::for_each(std::execution::unseq, midpy.begin(), midpy.end(), translate_y);
    #else 

        // Vertices v0
        std::for_each(v0x.begin(), v0x.end(), translate_x);
        std::for_each(v0y.begin(), v0y.end(), translate_y);

        // Vertices v1
        std::for_each(v1x.begin(), v1x.end(), translate_x);
        std::for_each(v1y.begin(), v1y.end(), translate_y);

        // Vertices v2
        std::for_each(v2x.begin(), v2x.end(), translate_x);
        std::for_each(v2y.begin(), v2y.end(), translate_y);

        // Midpoints
        std::for_each(midpx.begin(), midpx.end(), translate_x);
        std::for_each(midpy.begin(), midpy.end(), translate_y);
    #endif // NO_STD_EXECUTION
    
    // Do NOT transform normals.  Normals represent direction and are invariant under translation.
    findEdges();
}

void Mesh3d::exportSTL(const std::string& filename) const {
    std::ofstream stlout(filename, std::ofstream::trunc); // open file in a constructor, will be closed by destructor
    std::stringstream stlmem; // Build the entire string in memory

    stlmem << std::scientific << "solid Mesh3d" << std::endl;

    for (uint32_t i = 0; i < triangle_count; ++i) {
        stlmem << "  facet normal " << normx[i] << " " << normy[i] << " " << normz[i] << std::endl;
        stlmem << "    outer loop" << std::endl;
        stlmem << "      vertex " << v0x[i] << " " << v0y[i] << " " << v0z[i] << std::endl;
        stlmem << "      vertex " << v1x[i] << " " << v1y[i] << " " << v1z[i] << std::endl;
        stlmem << "      vertex " << v2x[i] << " " << v2y[i] << " " << v2z[i] << std::endl;
        stlmem << "    endloop" << std::endl;
        stlmem << "  endfacet" << std::endl;
    }

    stlmem << "endsolid Mesh3d" << std::endl;

    stlout << stlmem.str(); // Write the entire string at once
}

void Mesh3d::exportBinarySTL(const std::string& filename) const {
    // Use a vector<char> as our in-memory buffer
    std::vector<char> buffer;

    // 1. Reserve space for the header (80 bytes)
    buffer.resize(80);
    std::strcpy(buffer.data(), "Binary STL Mesh3d"); // Add an identifier

    // 2. Calculate and reserve space for the number of triangles (4 bytes)
    const uint32_t numTriangles = triangle_count;
    const size_t numTrianglesOffset = buffer.size(); // Store the offset where numTriangles will be
    buffer.resize(buffer.size() + sizeof(numTriangles));
    std::memcpy(buffer.data() + numTrianglesOffset, &numTriangles, sizeof(numTriangles));

    // 3. Reserve space for all triangle data (50 bytes per triangle)
    const size_t triangleDataOffset = buffer.size();
    buffer.resize(buffer.size() + (triangle_count * 50));
    char* triangleDataPtr = buffer.data() + triangleDataOffset;

    // 4. Write the triangle data to the buffer
    for (uint32_t i = 0; i < triangle_count; ++i) {
        const float normal[3] = {(float)normx[i], (float)normy[i], (float)normz[i]};
        const float v0[3] = {(float)v0x[i], (float)v0y[i], (float)v0z[i]};
        const float v1[3] = {(float)v1x[i], (float)v1y[i], (float)v1z[i]};
        const float v2[3] = {(float)v2x[i], (float)v2y[i], (float)v2z[i]};
        const uint16_t attributeByteCount = 0; // Usually 0

        std::memcpy(triangleDataPtr, normal, sizeof(normal));
        triangleDataPtr += sizeof(normal);
        std::memcpy(triangleDataPtr, v0, sizeof(v0));
        triangleDataPtr += sizeof(v0);
        std::memcpy(triangleDataPtr, v1, sizeof(v1));
        triangleDataPtr += sizeof(v1);
        std::memcpy(triangleDataPtr, v2, sizeof(v2));
        triangleDataPtr += sizeof(v2);
        std::memcpy(triangleDataPtr, &attributeByteCount, sizeof(attributeByteCount));
        triangleDataPtr += sizeof(attributeByteCount);
    }

    // 5. Write the entire buffer to the file
    std::ofstream stlout(filename, std::ios::binary | std::ios::trunc);
    if (!stlout.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return; // Or throw an exception
    }
    stlout.write(buffer.data(), buffer.size());
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

void Mesh3d::findCircumcentres() {

    std::for_each(
    #ifndef NO_STD_EXECUTION
                  std::execution::unseq,
    #endif // NO_STD_EXECUTION
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

void Mesh3d::findNormals() {
    if (triangle_count == 0) {
        return; // Nothing to do for an empty mesh.
    }

    std::for_each(
    #ifndef NO_STD_EXECUTION
                  std::execution::unseq,
    #endif // NO_STD_EXECUTION
                  std::views::iota(0u, triangle_count).begin(), // Use iota view
                  std::views::iota(0u, triangle_count).end(),
                  [this](uint32_t i) {
        // Compute the vectors representing two sides of the triangle.
        const double edge1x = v1x[i] - v0x[i];
        const double edge1y = v1y[i] - v0y[i];
        const double edge1z = v1z[i] - v0z[i];

        const double edge2x = v2x[i] - v0x[i];
        const double edge2y = v2y[i] - v0y[i];
        const double edge2z = v2z[i] - v0z[i];

        // Compute the cross product (normal vector).
        const double nx = edge1y * edge2z - edge1z * edge2y;
        const double ny = edge1z * edge2x - edge1x * edge2z;
        const double nz = edge1x * edge2y - edge1y * edge2x;

        // Normalize the normal vector.
        const double magnitude = std::sqrt(nx * nx + ny * ny + nz * nz);

        // Handle the case where the triangle is degenerate (magnitude is zero or very close to zero).
        if (magnitude > 1e-12) // Use a small tolerance to avoid division by zero.
        {
            normx[i] = nx / magnitude;
            normy[i] = ny / magnitude;
            normz[i] = nz / magnitude;
        } else {
            // For degenerate triangles, set the normal to a default value (e.g., zero).
            normx[i] = 0.0;
            normy[i] = 0.0;
            normz[i] = 0.0;
        }
    });
}

void Mesh3d::findEdges() {
    // resize vectors to acces via operator[]
    e1x.resize(triangle_count); e1y.resize(triangle_count); e1z.resize(triangle_count);
    e2x.resize(triangle_count); e2y.resize(triangle_count); e2z.resize(triangle_count);

    // Precompute obstacle edges
    std::for_each(
    #ifndef NO_STD_EXECUTION
                std::execution::unseq,
    #endif // NO_STD_EXECUTION
                std::views::iota(0u, triangle_count).begin(), // Use iota view
                std::views::iota(0u, triangle_count).end(),
                [this](uint32_t i) {
        
        e1x[i] = v1x[i] - v0x[i];
        e1y[i] = v1y[i] - v0y[i];
        e1z[i] = v1z[i] - v0z[i];

        e2x[i] = v2x[i] - v0x[i];
        e2y[i] = v2y[i] - v0y[i];
        e2z[i] = v2z[i] - v0z[i];
    });

}

void Mesh3d::findBoundingBox() {
    if (triangle_count == 0) {
        return; // Nothing to do for an empty mesh
    }

    // Initialize bounding box with the first vertex as a starting point
    bbmin.x = bbmax.x = v0x[0];
    bbmin.y = bbmax.y = v0y[0];
    bbmin.z = bbmax.z = v0z[0];

    // Use a lambda for concise min/max updates.  Could also use std::min and std::max.
    auto updateMinMax = [&](double x, double y, double z) {
        bbmin.x = std::min(bbmin.x, x);
        bbmin.y = std::min(bbmin.y, y);
        bbmin.z = std::min(bbmin.z, z);
        bbmax.x = std::max(bbmax.x, x);
        bbmax.y = std::max(bbmax.y, y);
        bbmax.z = std::max(bbmax.z, z);
    };

    // Iterate through all vertices.
    for (uint32_t i = 0; i < triangle_count; ++i) {
        updateMinMax(v0x[i], v0y[i], v0z[i]);
        updateMinMax(v1x[i], v1y[i], v1z[i]);
        updateMinMax(v2x[i], v2y[i], v2z[i]);
    }

}

vertex calculateReflection(const vertex& normal, const vertex& ray) {
    // reflection = ray - 2 * dotProduct(ray, normal) * normal
    const double dot = dotProduct(ray, normal);
    return vertex{ray.x - 2.0 * dot * normal.x,
                  ray.y - 2.0 * dot * normal.y,
                  ray.z - 2.0 * dot * normal.z};
}

Mesh3d importSTL(const std::string& filename) {
    std::string line;
    std::ifstream file(filename, std::ifstream::in);

    size_t pos;
    uint32_t triangle_count;

    if (!file.is_open()) return Mesh3d();

    triangle_count = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
    triangle_count -= 2;
    triangle_count /= 7;

    Mesh3d ret(triangle_count);

    file.seekg(0, file.beg);    // return to beginning
    file.clear();               // clear flags (especially eof flag)

    std::getline(file, line);   // skip the name of the solid
    std::getline(file, line);   // get the "facet normal" line ready

    for (int i = 0; line.find("endsolid") == std::string::npos; i++) { // read untill you find "endsolid";
        // get normal
        pos = line.rfind(' ');      // find last ' '
        ret.normz[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);
        pos = line.rfind(' ');      // find last ' '
        ret.normy[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);
        pos = line.rfind(' ');      // find last ' '
        ret.normx[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);

        std::getline(file, line);   // skip the "outer loop" line
            
        // vertex 0
        std::getline(file, line);   // get the first "vertex" line ready
        pos = line.rfind(' ');      // find last ' '
        ret.v0z[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);
        pos = line.rfind(' ');      // find last ' '
        ret.v0y[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);
        pos = line.rfind(' ');      // find last ' '
        ret.v0x[i] = strtod(line.substr(pos + 1, std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);

        // vertex 1
        std::getline(file, line);   // get the second "vertex" line ready
        pos = line.rfind(' ');      // find last ' '
        ret.v1z[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);
        pos = line.rfind(' ');      // find last ' '
        ret.v1y[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);
        pos = line.rfind(' ');      // find last ' '
        ret.v1x[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);

        // vertex 2
        std::getline(file, line);   // get the third "vertex" line ready
        pos = line.rfind(' ');      // find last ' '
        ret.v2z[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);
        pos = line.rfind(' ');      // find last ' '
        ret.v2y[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);
        pos = line.rfind(' ');      // find last ' '
        ret.v2x[i] = strtod(line.substr(pos+1,std::string::npos).c_str(), nullptr);
        line = line.substr(0, pos);


        std::getline(file, line);   // skip "endloop"
        std::getline(file, line);   // skip "endfacet"

        std::getline(file, line);   // get the "facet normal" line ready (or "endsolid" line)
        
    }

    return ret;
}

Mesh3d importBinarySTL(const std::string& filename) {
    std::ifstream stlin(filename, std::ios::binary);
    if (!stlin.is_open()) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return Mesh3d(); // Or throw an exception
    }

    // 1. Read the entire file into a buffer
    stlin.seekg(0, std::ios::end);
    const size_t fileSize = stlin.tellg();
    stlin.seekg(0, std::ios::beg);

    std::vector<char> buffer(fileSize);
    stlin.read(buffer.data(), fileSize);

    // 2. Validate the file size (at least 84 bytes, and 84 + 50*N)
    if (fileSize < 84) {
        std::cerr << "Error: Invalid STL file size (too small)." << std::endl;
        return Mesh3d();
    }
    
    // 3. Skip the header (80 bytes)
    const char* dataPtr = buffer.data() + 80;

    // 4. Read the number of triangles (uint32_t)
    uint32_t numTriangles;
    std::memcpy(&numTriangles, dataPtr, sizeof(numTriangles));

    dataPtr += sizeof(numTriangles);

    // 5. Validate file size against the number of triangles
    if (fileSize != 84 + (numTriangles * 50)) {
        std::cerr << "Error: Invalid STL file size (mismatch with triangle count)." << std::endl;
        return Mesh3d();
    }

    // 6. Resize member vectors
    Mesh3d ret(numTriangles);

    // 7. Read triangle data
    for (uint32_t i = 0; i < numTriangles; ++i) {
        float normal[3];
        float v0[3];
        float v1[3];
        float v2[3];
        uint16_t attributeByteCount;

        std::memcpy(normal, dataPtr, sizeof(normal));
        dataPtr += sizeof(normal);
        std::memcpy(v0, dataPtr, sizeof(v0));
        dataPtr += sizeof(v0);
        std::memcpy(v1, dataPtr, sizeof(v1));
        dataPtr += sizeof(v1);
        std::memcpy(v2, dataPtr, sizeof(v2));
        dataPtr += sizeof(v2);
        std::memcpy(&attributeByteCount, dataPtr, sizeof(attributeByteCount));
        dataPtr += sizeof(attributeByteCount);

        // Assign to member variables
        ret.normx[i] = normal[0];
        ret.normy[i] = normal[1];
        ret.normz[i] = normal[2];
        ret.v0x[i] = v0[0];
        ret.v0y[i] = v0[1];
        ret.v0z[i] = v0[2];
        ret.v1x[i] = v1[0];
        ret.v1y[i] = v1[1];
        ret.v1z[i] = v1[2];
        ret.v2x[i] = v2[0];
        ret.v2y[i] = v2[1];
        ret.v2z[i] = v2[2];
        // You might want to do something with attributeByteCount, but it's often ignored.
    }

    return ret;
}

