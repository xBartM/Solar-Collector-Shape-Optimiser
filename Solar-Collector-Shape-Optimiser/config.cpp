#include <cstdint>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "Solar-Collector-Shape-Optimiser/config.hpp"

// Initialize static members
uint32_t Config::xsize = 0;   // Default values, will be overwritten by config file
uint32_t Config::ysize = 0;
uint32_t Config::hmax = 0;
uint32_t Config::popsize = 0;

double Config::crossover_bias = 0.0;
double Config::mutation_probability = 0.0;
double Config::mutation_range = 0.0;

double Config::termination_ratio = 0.0;

uint32_t Config::checkpoint_every = 0;
uint32_t Config::export_every = 0;

bool Config::start_from_checkpoint = false;

std::vector<vertex> Config::rays;
std::map<std::string, std::string> Config::settings;

// Trim whitespace from a string
std::string Config::trim(const std::string& str) {
    const size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    const size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Load configuration from file
void Config::loadFromFile(const std::string& filename) {
    // Check if the path is valid and the file exists
    if (!std::filesystem::exists(filename) || !std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error("Invalid configuration file path: " + filename);
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open configuration file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line); // Remove leading/trailing whitespace

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) {
            throw std::runtime_error("Invalid configuration line: " + line);
        }

        const std::string key = trim(line.substr(0, delimiterPos));
        const std::string value = trim(line.substr(delimiterPos + 1));

        // okay, this is ugly (ray is being set here, but the rest gets set in a try block further down)
        if (key == "ray") {
            std::vector<double> tokens;
            std::istringstream iss{value};
            for (std::string token; std::getline(iss, token, ','); )
                tokens.push_back(std::stod(token));
            rays.emplace_back(tokens[0], tokens[1], tokens[2]);
        }
        else {
            settings[key] = value; // Store in the map
        }

    }

    file.close();

    // Assign values to static members, with error checking
    try {
        xsize = std::stoul(settings.at("xsize"));
        ysize = std::stoul(settings.at("ysize"));
        hmax = std::stoul(settings.at("hmax"));
        popsize = std::stoul(settings.at("popsize"));

        crossover_bias = std::stod(settings.at("crossover_bias"));
        mutation_probability = std::stod(settings.at("mutation_probability"));
        mutation_range = std::stod(settings.at("mutation_range"));

        termination_ratio = std::stod(settings.at("termination_ratio"));

        checkpoint_every = std::stoul(settings.at("checkpoint_every"));
        export_every = std::stoul(settings.at("export_every"));

        start_from_checkpoint = settings.at("start_from_checkpoint")=="true";
    
    } catch (const std::out_of_range& oor) {
        throw std::runtime_error("Missing or invalid configuration value: " + std::string(oor.what()));
    } catch (const std::invalid_argument& ia) {
         throw std::runtime_error("Invalid configuration value: " + std::string(ia.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Error reading config file: " + std::string(e.what()));
    }
    // Sanity check, it is important to specify these checks in case user defined their own .txt file
    if( xsize < 2 || ysize < 2)
      throw std::runtime_error("xsize and ysize need to be greater than 1!");

    if( popsize == 0 )
      throw std::runtime_error("popsize needs to be greater than 0!");

    if(popsize % 4)
        std::cerr << "Warning: Population Size is not divisible by 4!\n"; //not an error, just a qol warning

}