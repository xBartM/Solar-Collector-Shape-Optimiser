#include <cstdint>
#include <vector>
#include <ostream>
#include <execution>

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

Genome Genome::crossoverAndMutateSoA(const Genome &other, double crossover_bias, double mutation_probability, double mutation_range) const {
    Genome offspring(dna_size);

    // TODO: random device and distributions could be static (but that takes away the control to dynamically change them with each crossoverAnd.. call (can be passed to function))
    std::random_device rd;  // Used only to seed the mt19937 generator. Seeding should ideally happen once per thread
    std::mt19937 mt(rd()); // thread_local would ensure each thread has its own generator.

    // Crossover bias (probability of using genes from the first parent (this))
    std::bernoulli_distribution crossover_choice_dist(crossover_bias);
    // Mutation (probability of applying mutation and it's amount)
    std::bernoulli_distribution mutation_choice_dist(mutation_probability);
    std::uniform_real_distribution<double> mutation_amount_dist(-mutation_range, mutation_range);

    // Use std::transform with std::execution::par_unseq (parallel execution)
    std::transform(std::execution::par_unseq, dna.begin(), dna.end(), other.dna.begin(), offspring.dna.begin(),
        [&](double dna1_val, double dna2_val) {
            const bool crossover_choice = crossover_choice_dist(mt);
            const bool mutation_flag = mutation_choice_dist(mt);
            const double mutation_amount = mutation_amount_dist(mt);

            // merge two dnas
            return (crossover_choice ? dna1_val : dna2_val) + (mutation_flag ? mutation_amount : 0.0);
        }
    );

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

