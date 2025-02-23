#include <cstdint>
#include <vector>
#include <ostream>
#include <execution>
#include <sstream>
#include <fstream>
// #include <stdexcept>

#include <Solar-Collector-Shape-Optimiser/genome.hpp>


Genome::Genome(const uint32_t dna_size, const double dna_min, const double dna_max)
    : dna_size(dna_size)  
    , dna(dna_size)
    , fitness(0.0)
    , dna_min(dna_min)
    , dna_max(dna_max)
{}

Genome::Genome(const Genome &parent1, const Genome &parent2, const double& crossover_bias, const double& mutation_probability, const double& mutation_range) 
    : Genome(parent1.dna_size, parent1.dna_min, parent1.dna_max) 
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
    std::transform(
    #ifndef NO_STD_EXECUTION
        std::execution::unseq, 
    #endif // NO_STD_EXECUTIOn
        parent1.dna.begin(), parent1.dna.end(), 
        parent2.dna.begin(), 
        dna.begin(),
        [&](double dna1_val, double dna2_val) {
            const bool crossover_choice = crossover_choice_dist(mt);
            const bool mutation_flag = mutation_choice_dist(mt);
            const double mutation_amount = mutation_amount_dist(mt);

            // merge two dnas
            const double ret = (crossover_choice ? dna1_val : dna2_val) + (mutation_flag ? mutation_amount : 0.0);
            // clamp to desired dna_min and max values
            return std::clamp(ret, dna_min, dna_max);
        }
    );

}

Genome::~Genome() {}

// 0.0 should mean that they are the same
// 1.0 should mean that they are 100% opposite (same, but "negative")
// 0.5 should mean that they are distinct (think - bit error rate: 0.5 carries no information as opposed to 0.0 or 1.0)
double Genome::calcSimilarity(const Genome &other) const {

    const double elem_count = dna_size;
    const double magnitude = dna_max - dna_min;

    // Use std::inner_product (sum of absolute values of differences on each field of DNA)
    const double acc = 
        std::inner_product(
            dna.begin(), dna.end(), 
            other.dna.begin(), 
            (double)0.0,
            std::plus<>(), 
            [&](double dna1_val, double dna2_val) {
                return std::abs(dna1_val - dna2_val);
            }
        );
    
    return acc / (elem_count * magnitude) ;
}


std::ostream& operator<<(std::ostream& os, const Genome& genome) {
    os << "Chromosome Size: " << genome.dna_size << ", Fitness: " << genome.fitness;
    return os;
}

void serializeToFile(const Genome& genome, const std::string& filename) {
    std::stringstream ss;

    std::ofstream file(filename, std::ios::out); // Open in text mode
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    
    // Serialize basic members.  Use a consistent, easily parsable format.
    ss << genome.dna_size << " ";
    ss << genome.fitness << " ";
    ss << genome.dna_min << " ";
    ss << genome.dna_max << " ";

    // Serialize the DNA vector.  Iterate and add each element to the stream.
    for (const double& gene : genome.dna) {
        ss << gene << " ";
    }

    file << ss.rdbuf();

    if (file.fail()) {
        throw std::runtime_error("Error writing to file: " + filename);
    }
    file.close(); // Explicit close for clarity (RAII handles it, but this is good practice)
}

Genome deserializeFromFile(const std::string& filename) {
    uint32_t dna_size;
    double fitness;
    double dna_min, dna_max;
    std::vector<double> dna;
    double gene;
        
    std::ifstream file(filename, std::ios::in);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read entire file into a stringstream
        if (file.fail() && !file.eof()) {  // failbit set, but not due to EOF (e.g., read error, permission)
        throw std::runtime_error("Error reading from file: " + filename);
    }
    file.close(); // Explicit close (RAII handles it, but it's good practice)


    // Deserialize basic members.  Match the order and format used in serialize().
    if (!(buffer >> dna_size)) {
        throw std::runtime_error("Failed to deserialize dna_size.");
    }
    if (!(buffer >> fitness)) {
        throw std::runtime_error("Failed to deserialize fitness.");
    }
    if (!(buffer >> dna_min)) {
        throw std::runtime_error("Failed to deserialize dna_min.");
    }
    if (!(buffer >> dna_max)) {
        throw std::runtime_error("Failed to deserialize dna_max.");
    }

    // Deserialize the DNA vector. Read until the end of the stream.
    while (buffer >> gene) {
        dna.push_back(gene);
    }
        // Check for errors during DNA vector deserialization
    if (buffer.fail() && !buffer.eof()) {  // failbit set, but not due to EOF
        throw std::runtime_error("Failed to deserialize dna vector: invalid data.");
    }

    if (dna.size() != dna_size) {
        throw std::runtime_error("Deserialized DNA size mismatch. Expected: " + std::to_string(dna_size) + ", got: " + std::to_string(dna.size()));
    }

    // Create and return the Genome object.
    Genome genome(dna_size, dna_min, dna_max); // Use constructor to initialize
    genome.dna = std::move(dna);             // Efficient move assignment
    genome.fitness = fitness;

    return genome;

}
