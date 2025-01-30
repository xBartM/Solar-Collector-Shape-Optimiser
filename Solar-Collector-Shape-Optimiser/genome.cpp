#include <cstdint>
#include <vector>
#include <ostream>

#include <Solar-Collector-Shape-Optimiser/genome.hpp>


Genome::Genome(const uint32_t id, const uint32_t chromosome_size)
    : id(id), chromosome_size(chromosome_size) { 
    dna.resize(chromosome_size);
    fitness = 0;
}

Genome::Genome (const Genome & other)
    : id(other.id), chromosome_size(other.chromosome_size), fitness(other.fitness) {
    dna.resize(other.chromosome_size);
    std::copy(other.dna.begin(), other.dna.end(), dna.begin());    
}

Genome::Genome(Genome&& other) noexcept
    : id(std::move(other.id)),
      chromosome_size(std::move(other.chromosome_size)),
      dna(std::move(other.dna)),
      fitness(std::move(other.fitness))
{
    other.chromosome_size = 0; // set to some default value to avoid problems later
}

Genome& Genome::operator= (Genome&& other) noexcept {
    if (this != &other)
    {
        dna.clear(); // or dna.resize(0); to release memory
        id = std::move(other.id);
        chromosome_size = std::move(other.chromosome_size);
        fitness = std::move(other.fitness);
        dna = std::move(other.dna);

        other.chromosome_size = 0; // set to some default value to avoid problems later
    }
    return *this;
}

Genome& Genome::operator= (const Genome & other)
{
    id = other.id;
    chromosome_size = other.chromosome_size;
    fitness = other.fitness;

    dna.resize(other.chromosome_size);
    std::copy(other.dna.begin(), other.dna.end(), dna.begin());
    
    return *this;
}

Genome::~Genome() {
    // No need to manually delete as std::vector handles its own memory.
}

std::ostream& operator<<(std::ostream& os, const Genome& genome) {
    os << "ID: " << genome.id << ", Chromosome Size: " << genome.chromosome_size << ", Fitness: " << genome.fitness;
    return os;
}


// void Genome::showYourself() {
//     std::cout << (int)id << " " << xsize << " " << ysize << std::endl;
//     for (int y = 0; y < ysize; y++) {
//         for (int x = 0; x < xsize; x++)
//             std::cout << getXY(x, y) << " ";
//         std::cout << std::endl;
//     }
// }

