#ifndef GENOME_HPP
#define GENOME_HPP

#include <cstdint>
#include <vector>
#include <random>
#include <limits>
#include <string>


// note to self: it's good that dna is unique, meaning that the order of "genes" matters (ex. 100101 gives a different specimen than any other permutation of genes of this length)
class Genome {
public:
    // add age, max age and "immortal" elites
    uint32_t dna_size; // number of chromosomes - size of dna vector
    std::vector<double> dna; // 1-d array of doubles (maybe type <T>?) that hold the instruction to create inheriting object
    double fitness; // fitness of this set of chromosomes

    // these should be const but it breaks implicit copy, move constructors, assignments
    double dna_min;
    double dna_max;


    Genome(const uint32_t dna_size, const double dna_min = std::numeric_limits<double>::lowest(), const double dna_max = std::numeric_limits<double>::max());
    Genome(const Genome &parent1, const Genome &parent2, const double& crossover_bias = 0.5, const double& mutation_probability = 0.0, const double& mutation_range = 0.0);
    // copy, move constructors, assignments can be default
    auto operator<=>(const Genome &other) const { return std::compare_three_way{}(fitness, other.fitness); }
    virtual ~Genome();

    double calcSimilarity(const Genome &other) const;

    friend std::ostream& operator<<(std::ostream& os, const Genome& genome);

};

void serializeToFile(const Genome& genome, const std::string& filename);
Genome deserializeFromFile(const std::string& filename);

#endif // GENOME_HPP