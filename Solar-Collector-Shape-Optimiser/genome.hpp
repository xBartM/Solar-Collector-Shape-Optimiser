#ifndef GENOME_HPP
#define GENOME_HPP

#include <cstdint>
#include <vector>
#include <random>


// note to self: it's good that dna is unique, meaning that the order of "genes" matters (ex. 100101 gives a different specimen than any other permutation of genes of this length)
class Genome {
public:
    // add age, max age and "immortal" elites
    uint32_t dna_size; // number of chromosomes - size of dna vector
    std::vector<double> dna; // 1-d array of doubles (maybe type <T>?) that hold the instruction to create inheriting object
    double fitness; // fitness of this set of chromosomes


    Genome(const uint32_t dna_size);
    Genome(const Genome &parent1, const Genome &parent2, const double& crossover_bias = 0.5, const double& mutation_probability = 0.0, const double& mutation_range = 0.0);
    // copy, move constructors, assignments can be default
    auto operator<=>(const Genome &other) const { return std::compare_three_way{}(fitness, other.fitness); }
    virtual ~Genome();

    friend std::ostream& operator<<(std::ostream& os, const Genome& genome);

};


#endif // GENOME_HPP