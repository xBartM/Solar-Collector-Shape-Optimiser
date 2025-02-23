#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>

#include <Solar-Collector-Shape-Optimiser/mesh3d.hpp>

class Config {
public:
    // Static members to hold the configuration
    static uint32_t xsize;
    static uint32_t ysize;
    static uint32_t hmax;
    static uint32_t popsize;

    static double crossover_bias;
    static double mutation_probability;
    static double mutation_range;

    static std::vector<vertex> rays;

    // Static method to load configuration from a file
    static void loadFromFile(const std::string& filename);

private:
    static std::map<std::string, std::string> settings; // Store key-value pairs

    // Helper function to trim whitespace from a string
    static std::string trim(const std::string& str);
};

#endif // CONFIG_HPP