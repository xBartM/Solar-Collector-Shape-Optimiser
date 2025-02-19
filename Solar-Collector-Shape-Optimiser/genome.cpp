#include <cstdint>
#include <vector>
#include <ostream>
#include <execution>

#include <Solar-Collector-Shape-Optimiser/genome.hpp>


Genome::Genome(const uint32_t dna_size)
    : dna_size(dna_size)  
    , dna(dna_size)
    , fitness(0.0)
{}

Genome::Genome(const Genome &parent1, const Genome &parent2, const double& crossover_bias, const double& mutation_probability, const double& mutation_range) 
    : Genome(parent1.dna_size) 
{

    // TODO: random device and distributions could be static (but that takes away the control to dynamically change them with each crossoverAnd.. call (can be passed to function))
    std::random_device rd;  // Used only to seed the mt19937 generator. Seeding should ideally happen once per thread
    std::mt19937 mt(rd()); // thread_local would ensure each thread has its own generator.

    // Crossover bias (probability of using genes from the first parent (this))
    std::bernoulli_distribution crossover_choice_dist(crossover_bias);
    // Mutation (probability of applying mutation and it's amount)
    std::bernoulli_distribution mutation_choice_dist(mutation_probability);
    std::uniform_real_distribution<double> mutation_amount_dist(-mutation_range, mutation_range);

    // Use std::transform with std::execution::par_unseq (parallel execution)
    std::transform(std::execution::par_unseq, parent1.dna.begin(), parent1.dna.end(), parent2.dna.begin(), dna.begin(),
        [&](double dna1_val, double dna2_val) {
            const bool crossover_choice = crossover_choice_dist(mt);
            const bool mutation_flag = mutation_choice_dist(mt);
            const double mutation_amount = mutation_amount_dist(mt);

            // merge two dnas
            return (crossover_choice ? dna1_val : dna2_val) + (mutation_flag ? mutation_amount : 0.0);
        }
    );

}

Genome::~Genome() {}

std::ostream& operator<<(std::ostream& os, const Genome& genome) {
    os << "Chromosome Size: " << genome.dna_size << ", Fitness: " << genome.fitness;
    return os;
}

