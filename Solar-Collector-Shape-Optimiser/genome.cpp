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

Genome::~Genome() {}

Genome Genome::crossoverAndMutate(const Genome &other, uint32_t new_id, double crossover_bias, int mutation_percent, double mutation_range) const {
    Genome offspring(new_id, chromosome_size);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> mutation_dist(-mutation_range, mutation_range);
    std::uniform_real_distribution<double> crossover_dist(0.0, 100.0);

    for (size_t i = 0; i < chromosome_size; ++i) {
        if (crossover_dist(mt) < crossover_bias) {
            if (mutation_percent > 0 && crossover_dist(mt) < mutation_percent) {
                offspring.dna[i] = dna[i] + mutation_dist(mt);
            } else {
                offspring.dna[i] = dna[i];
            }
        } else {
            if (mutation_percent > 0 && crossover_dist(mt) < mutation_percent) {
                offspring.dna[i] = other.dna[i] + mutation_dist(mt);
            } else {
                offspring.dna[i] = other.dna[i];
            }
        }
    }

    return offspring;
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

