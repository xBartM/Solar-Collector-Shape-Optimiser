#include <fstream>
#include <string>
#include <algorithm>

#include <Solar-Collector-Shape-Optimiser/obstacle.hpp>

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