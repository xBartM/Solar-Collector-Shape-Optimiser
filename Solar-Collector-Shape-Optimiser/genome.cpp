#include <cstdint>
#include <vector>
#include <ostream>

#include <Solar-Collector-Shape-Optimiser/genome.hpp>

uint32_t Genome::next_id = 0;

Genome::Genome(const uint32_t dna_size)
    : id(next_id++), dna_size(dna_size) { 
    dna.resize(dna_size);
    fitness = 0;
}

Genome::Genome (const Genome & other)
    : id(other.id), dna_size(other.dna_size), fitness(other.fitness) {
    dna.resize(other.dna_size);
    std::copy(other.dna.begin(), other.dna.end(), dna.begin());    
}

Genome::Genome(Genome&& other) noexcept
    : id(other.id),
      dna_size(std::move(other.dna_size)),
      dna(std::move(other.dna)),
      fitness(std::move(other.fitness))
{
    other.dna_size = 0; // set to some default value to avoid problems later
}

Genome& Genome::operator= (Genome&& other) noexcept {
    if (this != &other)
    {
        dna.clear(); // or dna.resize(0); to release memory
        id = other.id;
        dna_size = std::move(other.dna_size);
        fitness = std::move(other.fitness);
        dna = std::move(other.dna);

        other.dna_size = 0; // set to some default value to avoid problems later
    }
    return *this;
}

Genome& Genome::operator= (const Genome & other)
{
    id = other.id;
    dna_size = other.dna_size;
    fitness = other.fitness;

    dna.resize(other.dna_size);
    std::copy(other.dna.begin(), other.dna.end(), dna.begin());
    
    return *this;
}

Genome::~Genome() {}

Genome Genome::crossoverAndMutate(const Genome &other, double crossover_bias, double mutation_percent, double mutation_range) const {
    Genome offspring(dna_size);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> mutation_dist(-mutation_range, mutation_range); // how much to add/remove from dna point
    std::uniform_real_distribution<double> crossover_dist(0.0, 100.0);
    std::uniform_real_distribution<double> mutation_chance_dist(0.0, 100.0); // added for comparing with mutation_percent

    for (uint32_t i = 0; i < dna_size; ++i) {
        if (crossover_dist(mt) < crossover_bias) {
            if (mutation_percent > 0 && mutation_chance_dist(mt) < mutation_percent) { // changed comparison to use mutation_chance_dist
                offspring.dna[i] = dna[i] + mutation_dist(mt);
            } else {
                offspring.dna[i] = dna[i];
            }
        } else {
            if (mutation_percent > 0 && mutation_chance_dist(mt) < mutation_percent) { // changed comparison to use mutation_chance_dist
                offspring.dna[i] = other.dna[i] + mutation_dist(mt);
            } else {
                offspring.dna[i] = other.dna[i];
            }
        }
    }

    return offspring;
}

std::ostream& operator<<(std::ostream& os, const Genome& genome) {
    os << "ID: " << genome.id << ", Chromosome Size: " << genome.dna_size << ", Fitness: " << genome.fitness;
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

