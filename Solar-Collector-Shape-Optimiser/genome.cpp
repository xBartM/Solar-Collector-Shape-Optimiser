#include <cstdint>
#include <algorithm>
// #include <compare>


#include <Solar-Collector-Shape-Optimiser/genome.hpp>


Genome::Genome(const uint32_t id, const uint32_t chromosome_size)
    : id(id), chromosome_size(chromosome_size) { 
    shape = new double[chromosome_size];
    fitness = 0;
}

Genome::Genome (const Genome & other)
    : id(other.id), chromosome_size(other.chromosome_size), fitness(other.fitness) {
    shape = new double[other.chromosome_size];
    std::copy(other.shape, other.shape + other.chromosome_size, shape);    
}

Genome::Genome(Genome&& other) noexcept
    : id(std::move(other.id)),
      chromosome_size(std::move(other.chromosome_size)),
      shape(other.shape),
      fitness(std::move(other.fitness))
{
    other.shape = nullptr;
    other.chromosome_size = 0; // set to some default value to avoid problems later
}

Genome& Genome::operator= (Genome&& other) noexcept {
    if (this != &other)
    {
        delete[] shape;
        id = std::move(other.id);
        chromosome_size = std::move(other.chromosome_size);
        fitness = std::move(other.fitness);
        shape = other.shape;

        other.shape = nullptr;
        other.chromosome_size = 0; // set to some default value to avoid problems later
    }
    return *this;
}

Genome& Genome::operator= (const Genome & other)
{
    id = other.id;
    chromosome_size = other.chromosome_size;
    fitness = other.fitness;

    shape = new double[other.chromosome_size];
    std::copy(other.shape, other.shape + other.chromosome_size, shape);
    
    return *this;
}

Genome::~Genome() {
    delete[] shape;
}

// double Genome::getXY(const unsigned short x, const unsigned short y) const {
//     return shape[y * xsize + x];
// }

// void Genome::setXY(const unsigned short x, const unsigned short y, const double val) {
//     if (val < 0)
//         shape[y * xsize + x] = 0;
//     else if (val > hmax)
//         shape[y * xsize + x] = hmax;
//     else
//         shape[y * xsize + x] = val;
// }

// void Genome::showYourself() {
//     std::cout << (int)id << " " << xsize << " " << ysize << std::endl;
//     for (int y = 0; y < ysize; y++) {
//         for (int x = 0; x < xsize; x++)
//             std::cout << getXY(x, y) << " ";
//         std::cout << std::endl;
//     }
// }

