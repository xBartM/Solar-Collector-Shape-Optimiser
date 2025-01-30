#ifndef GENOME_HPP
#define GENOME_HPP


class Genome {
public:
    uint32_t id;   // unique identifier for a set of chromosomes in this Genome (form 0 to a-lot)

    // add age, max age and "immortal" elites
    uint32_t chromosome_size; // number of chromosomes - size of dna vector
    std::vector<double> dna; // 1-d array of doubles (maybe type <T>?) that hold the instruction to create inheriting object
    double fitness; // fitness of this set of chromosomes (% of triangles that reflected the light)


    Genome (const uint32_t id, const uint32_t chromosome_size);
    Genome (const Genome & other);
    Genome (Genome&& other) noexcept;
    Genome& operator= (Genome&& other) noexcept;
    Genome& operator= (const Genome & other);
    auto operator<=>(const Genome& other) const { return std::compare_three_way{}(fitness, other.fitness); }
    ~Genome();

    // double getXY(const unsigned short x, const unsigned short y) const;
    // void setXY(const unsigned short x, const unsigned short y, const double val);
    // void showYourself(); // Removed: SolarCollector has its own version

    friend std::ostream& operator<<(std::ostream& os, const Genome& genome);

};


#endif // GENOME_HPP