#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <random>

#include <Solar-Collector-Shape-Optimiser/specimen.hpp>


Specimen::Specimen(const uint32_t id, const uint32_t chromosome_size)
    : id(id), chromosome_size(chromosome_size) { 
    shape = new double[chromosome_size];
    fitness = 0;
}

Specimen::Specimen (const Specimen & other)
    : id(other.id), chromosome_size(other.chromosome_size), fitness(other.fitness) {
    shape = new double[other.chromosome_size];
    std::copy(other.shape, other.shape + other.chromosome_size, shape);    
}

Specimen& Specimen::operator= (const Specimen & other)
{
    id = other.id;
    chromosome_size = other.chromosome_size;
    fitness = other.fitness;

    shape = new double[other.chromosome_size];
    std::copy(other.shape, other.shape + other.chromosome_size, shape);
    
    return *this;
}

Specimen::~Specimen() {
    delete[] shape;
}

// double Specimen::getXY(const unsigned short x, const unsigned short y) const {
//     return shape[y * xsize + x];
// }

// void Specimen::setXY(const unsigned short x, const unsigned short y, const double val) {
//     if (val < 0)
//         shape[y * xsize + x] = 0;
//     else if (val > hmax)
//         shape[y * xsize + x] = hmax;
//     else
//         shape[y * xsize + x] = val;
// }

// void Specimen::showYourself() {
//     std::cout << (int)id << " " << xsize << " " << ysize << std::endl;
//     for (int y = 0; y < ysize; y++) {
//         for (int x = 0; x < xsize; x++)
//             std::cout << getXY(x, y) << " ";
//         std::cout << std::endl;
//     }
// }

