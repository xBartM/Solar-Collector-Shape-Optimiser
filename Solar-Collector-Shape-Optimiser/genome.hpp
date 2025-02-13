#ifndef GENOME_HPP
#define GENOME_HPP

#include <cstdint>
#include <vector>
#include <random>


// note to self: it's good that dna is unique, meaning that the order of "genes" matters (ex. 100101 gives a different specimen than any other permutation of genes of this length)
class Genome {
public:
    uint32_t id;   // unique identifier for a set of chromosomes in this Genome (form 0 to a-lot)

    // add age, max age and "immortal" elites
    uint32_t dna_size; // number of chromosomes - size of dna vector
    std::vector<double> dna; // 1-d array of doubles (maybe type <T>?) that hold the instruction to create inheriting object
    double fitness; // fitness of this set of chromosomes


    Genome(const uint32_t dna_size);
    Genome(const Genome &other);
    Genome(Genome &&other) noexcept;
    Genome &operator=(Genome &&other) noexcept;
    Genome &operator=(const Genome &other);
    auto operator<=>(const Genome &other) const { return std::compare_three_way{}(fitness, other.fitness); }
    virtual ~Genome();

    // double getXY(const unsigned short x, const unsigned short y) const;
    // void setXY(const unsigned short x, const unsigned short y, const double val);
    // void showYourself(); // Removed: SolarCollector has its own version

    // Function to perform crossover and mutation between two Genomes
    Genome crossoverAndMutate(const Genome &other, double crossover_bias, double mutation_percent, double mutation_range) const;

    friend std::ostream& operator<<(std::ostream& os, const Genome& genome);

private:
    static uint32_t next_id;

};


#endif // GENOME_HPP