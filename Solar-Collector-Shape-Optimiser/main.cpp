#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

#ifndef NO_STD_EXECUTION
    #include <execution>
#else 
    #include <omp.h>
#endif // NO_STD_EXECUTION

#include <Solar-Collector-Shape-Optimiser/solarcollector.hpp>
#include <Solar-Collector-Shape-Optimiser/config.hpp>
#include <Solar-Collector-Shape-Optimiser/stats.hpp>


void findProperHmaxDist (const uint32_t xsize, const uint32_t ysize, const uint32_t hmax, const Mesh3d* obs);

int main (int argc, char** argv)
{

    // --- Load Configuration ---
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
        return 1;
    }

    try {
        Config::loadFromFile(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    // +1 because, ex.: xsize, ysize = 2, 2 gives 2 triangles/1 square with a side of length 1mm
    const uint32_t xsize   = Config::xsize+1; // size of the panel (in mm preferably?)
    const uint32_t ysize   = Config::ysize+1; // -||-
    const uint32_t hmax    = Config::hmax+1; // max height of a panel
    const uint32_t popsize = Config::popsize; // size of the population

    const double crossover_bias       = Config::crossover_bias;
    const double mutation_probability = Config::mutation_probability;
    const double mutation_range       = Config::mutation_range;

    const double termination_ratio = Config::termination_ratio;

    const uint32_t checkpoint_every = Config::checkpoint_every;
    const uint32_t export_every     = Config::export_every;

    const bool start_from_checkpoint = Config::start_from_checkpoint;

    const std::vector<vertex> rays = Config::rays; 

    const std::string obs_load_time = "1.ObstacleLoad";
    const std::string populating_time = "2.Populating";
    const std::string fitness_comp_time = "1.FitnessComp";
    const std::string crossover_and_mutate_time = "2.CrossMut";
    const std::string export_time = "3.Export";
    const std::string checkpoint_time = "4.Checkpoint";

    uint32_t generation = 0;  // number of current generation


    std::random_device rd;
    std::mt19937 mt(rd());
    // std::uniform_real_distribution<double> dist(0.0, 0.45);//(double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start
    std::uniform_real_distribution<double> hdist(0.0, (double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start

    Stats::begin(obs_load_time);

    const Mesh3d obs("./obstacleBin.stl", (xsize-1.0)/2.0, (hmax-1.0)/2.0);
    // obs.exportBinarySTL("obstacleBin2.stl");

    Stats::end(obs_load_time); 
    Stats::begin(populating_time);

    // findProperHmaxDist(xsize, ysize, hmax, &obs);

    // create a population and a pointer to each index
    std::vector<SolarCollector> population;
    std::vector<uint32_t> pop_idx;
    bool start_from_checkpoint_noerror = true; // if error is encountered upon reading from file ignore the rest
    // reserve space to avoid reallocations
    population.reserve(popsize); 
    pop_idx.reserve(popsize); 
    for (uint32_t i = 0; i < popsize; i++) {
        if (start_from_checkpoint && start_from_checkpoint_noerror) {
            try {
                const Genome temp_g = deserializeFromFile("./checkpoint/" + std::to_string(i) + ".genome");
                population.emplace_back(xsize, ysize, hmax, &obs, temp_g);
            } catch (const std::runtime_error& e) {
                std::cerr << "Deserialization error: " << e.what() << std::endl;
                // set the flag to ignore the rest of serialized Genomes
                start_from_checkpoint_noerror = false;
                // go back one step in iteration (no Genome inserted this time)
                --i;
                // skip recomputing the mesh and DON'T push_back the same index again
                continue;
            }
        }
        else {
            population.emplace_back(xsize, ysize, hmax, &obs);
            for (uint32_t k = 0; k < xsize*ysize; k++)
                population[i].setXY(k, 0, hdist(mt));
        }
        population[i].computeMesh();
        // populate pop_idx with indices (from 0 to population)
        pop_idx.push_back(i); 
    }

    // create a vector for future parents
    std::vector<uint32_t> parents;
    parents.reserve(2);

    // format text for CSV integration
    std::cout << "Gen";
    for (const auto& idx : pop_idx) { 
        std::cout << ";F" << std::to_string(idx);
    }
    std::cout << std::endl;

    Stats::end(populating_time); Stats::show(); Stats::clear();

    while (true)
    {
        Stats::begin(fitness_comp_time);

        #ifndef NO_STD_EXECUTION
            std::for_each(std::execution::par_unseq, population.begin(), population.end(), [&](auto& pop) {
                if (pop.fitness == 0) {
                    pop.computeFitness(rays);
                }
            });
        #else 
            #pragma omp parallel for
            for (size_t i = 0; i < population.size(); ++i) {
                if (population[i].fitness == 0) {
                    population[i].computeFitness(rays);
                }
            }
        #endif // NO_STD_EXECUTION

        Stats::end(fitness_comp_time);

        // sorted best to worst using indices
        sort(pop_idx.begin(), pop_idx.end(), [&](const uint32_t a, const uint32_t b) {
            return population[a].fitness > population[b].fitness; // Sort in descending order of fitness
        });

        // print fitness for every SolarCollector
        std::cout << std::to_string(generation);
        for (const auto& idx : pop_idx) { // Use const auto& for efficiency
            std::cout << ";" << std::to_string(population[idx].fitness);
        }
        std::cout << std::endl;

        Stats::begin(crossover_and_mutate_time);

        // replace weak indivituals
        for (uint32_t i = popsize * (1 - termination_ratio); i < popsize; ++i) {
            // Clear parents 
            parents.clear(); 
            // Select two random parents from the *top* 1-termination_ratio of the population.
            std::sample(pop_idx.begin(), pop_idx.begin() + (popsize * (1 - termination_ratio)), std::back_inserter(parents), 2, mt);

            // Create an offspring using the selected parents.
            const Genome offspring(population[parents[0]], population[parents[1]], crossover_bias, mutation_probability, mutation_range);

            // Replace the weak individual (at pop_idx[i]) with the new offspring.
            population[pop_idx[i]] = SolarCollector(xsize, ysize, hmax, &obs, offspring);
        }
         
        Stats::end(crossover_and_mutate_time);


        // export the best from the population once in a while
        if (!(generation % export_every)) {
            Stats::begin(export_time);
            population[pop_idx[0]].exportAsBinarySTL("Gen" + std::to_string(generation) + "Fit" + std::to_string(int(population[pop_idx[0]].fitness)) + ".stl");
            Stats::end(export_time);

        }

        ++generation;

        // make a checkpoint of current population AFTER ++gen so that gen0 isn't checkpointed (huge QOL :))
        if (!(generation % checkpoint_every)) {
            Stats::begin(checkpoint_time);
            for (uint32_t i = 0 ; i < popsize; ++i) {
                serializeToFile(population[pop_idx[i]], "./checkpoint/" + std::to_string(i) + ".genome");
            }
            Stats::end(checkpoint_time); 
        }

        // if (generation == 3) return 0;
        Stats::show();

    }

    return 0;
}

void findProperHmaxDist (const uint32_t xsize, const uint32_t ysize, const uint32_t hmax, const Mesh3d* obs) {

    constexpr uint32_t test_sample = 12;
    constexpr uint32_t test_batches = 9;
    vertex ray(0, -1, 0);
    std::vector<vertex> rays({ray});

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);//(double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start

    std::vector<SolarCollector> seed;//(0, xsize, ysize, hmax, &obs);
    seed.reserve(test_sample); // reserve space to avoid reallocations
    std::cout << "hub"; // hmax upper bound
    for (uint32_t i = 0; i < test_sample; ++i)
    {
        seed.emplace_back(xsize, ysize, hmax, obs);
        std::cout << ";S" << std::to_string(i);
    }
    std::cout << std::endl;

    for (double i = 0.0; i <= (double)hmax; i+=(double)hmax/(double)test_batches)//i+=0.0025)
    {
        for (auto pop = seed.begin(); pop != seed.end(); pop++)
        {
            for (uint32_t j = 0; j < xsize*ysize; j++)
                pop->setXY(j, 0, dist(mt)*i);
            pop->computeMesh();
            pop->computeFitness(rays);
        }

        std::cout << std::to_string(i);
        for (auto pop = seed.begin(); pop != seed.end(); pop++)
            std::cout << ";" << std::to_string(pop->fitness);
        std::cout << std::endl;

    }
    exit(0);
}