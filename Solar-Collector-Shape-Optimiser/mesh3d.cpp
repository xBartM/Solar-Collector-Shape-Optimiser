#include <algorithm>
#include <cmath>
#include <fstream>
// #include <string>

#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>

void Obstacle::moveXY(const double x, const double y) {
    for (int i = 0; i < triangle_count; i++) {
        mesh[i].v[0].x += x; mesh[i].v[1].x += x; mesh[i].v[2].x += x;
        mesh[i].v[0].y += y; mesh[i].v[1].y += y; mesh[i].v[2].y += y;
    }
}

void Obstacle::importSTL(const std::string filename) {
    std::string line;
    std::ifstream file(filename, std::ifstream::in);

    size_t pos;

    if (file.is_open()) {
        triangle_count = std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), '\n');
        triangle_count -= 2;
        triangle_count /= 7;

        mesh = new triangle[triangle_count];

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

void Obstacle::exportSTL(const std::string filename) {
    std::ofstream stlout;
    
    stlout.open(filename, std::ofstream::trunc);
    stlout << "solid obstacle" << std::endl;

    for (int i = 0; i < triangle_count; i++) {
        stlout << "facet normal " << mesh[i].normal.x << " " << mesh[i].normal.y << " " << mesh[i].normal.z << std::endl;
        stlout << "   outer loop" << std::endl;
        stlout << "      vertex " << mesh[i].v[0].x << " " << mesh[i].v[0].y << " " << mesh[i].v[0].z << std::endl;
        stlout << "      vertex " << mesh[i].v[1].x << " " << mesh[i].v[1].y << " " << mesh[i].v[1].z << std::endl;
        stlout << "      vertex " << mesh[i].v[2].x << " " << mesh[i].v[2].y << " " << mesh[i].v[2].z << std::endl;
        stlout << "   endloop" << std::endl;
        stlout << "endfacet" << std::endl;
    }

    stlout << "endsolid obstacle" << std::endl;
    stlout.close();
}

Obstacle::Obstacle() {}

Obstacle::Obstacle(const std::string filename) { importSTL(filename); }

Obstacle::~Obstacle() { delete[] mesh; }


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

vertex calculateReflection(const triangle& t, const vertex& ray) {
    const vertex invray(substract(vertex(0, 0, 0), ray));
    return substract(multiply(t.normal, 2 * dotProduct(t.normal, invray)), invray);
}
