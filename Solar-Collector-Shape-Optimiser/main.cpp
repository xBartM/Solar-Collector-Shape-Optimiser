#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <execution>

#include <Solar-Collector-Shape-Optimiser/solarcollector.hpp>

#define MAIN_TIMED

void printTime(const std::string str)
{
    static auto start = std::chrono::high_resolution_clock::now();
    static auto end = std::chrono::high_resolution_clock::now();
    static std::chrono::duration<double> deltatime = end - start;

    if (str == "") return;

    #ifdef MAIN_TIMED
    end = std::chrono::high_resolution_clock::now();
    deltatime = end - start;
    std::cout << str << deltatime.count() << " s\n";
    start = std::chrono::high_resolution_clock::now();
    #endif // MAIN_TIMED

}

void findProperHmaxDist (const uint32_t xsize, const uint32_t ysize, const uint32_t hmax, const Mesh3d* obs);

int main (int argc, char** argv)
{

    uint32_t xsize;   // size of the panel (in mm preferably?)
    uint32_t ysize;   // -||-
    uint32_t hmax;    // max height of a panel
    uint32_t popsize; // size of the population

    uint32_t generation = 0;  // number of current generation
    const vertex ray(0, -1, 0);
    const std::vector<vertex> rays({ray});

    // +1 because, ex.: xsize, ysize = 2, 2 gives 2 triangles/1 square with a side of length 1mm
    if (argc == 5) {
        xsize   = std::stoul(argv[1])+1;
        ysize   = std::stoul(argv[2])+1;
        hmax    = std::stoul(argv[3])+1;
        popsize = std::stoul(argv[4]);
    } else {
        xsize   = 180+1;    // size of printbed (minus some spare space)
        ysize   = 940+1;    // length of aluminum rod
        hmax    = 180+1;    // height of printbed (minus some spare space)
        popsize = 40;       // make popsize divisible by 4 xd
    }

    std::random_device rd;
    std::mt19937 mt(rd());
    // std::uniform_real_distribution<double> dist(0.0, 0.45);//(double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start
    std::uniform_real_distribution<double> hdist(0.0, (double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start

    printTime(""); // prime all MAIN_TIMED variables

    const Mesh3d obs("obstacle.stl", (xsize-1.0)/2.0, (hmax-1.0)/2.0);
    //obs.exportSTL("my_obstacle.stl");

    printTime("Setting up the obstacle: "); 

    // findProperHmaxDist(xsize, ysize, hmax, &obs);

    // create a population
    std::vector<SolarCollector> population;
    population.reserve(popsize); // reserve space to avoid reallocations
    for (uint32_t i = 0, j = 0; i < popsize; i++, j++) {
        population.emplace_back(xsize, ysize, hmax, &obs);
        for (uint32_t k = 0; k < xsize*ysize; k++)
            population[i].setXY(k, 0, hdist(mt));
        population[i].computeMesh();

    }

    // create a vector for future parents
    std::vector<SolarCollector> parents;
    parents.reserve(2);

    // format text for CSV integration
    std::cout << "Gen";
    for (uint32_t i = 0; i < popsize; i++)
        std::cout << ";F" << std::to_string(i);
    std::cout << std::endl;

   printTime("Setting up the population: "); 

    while (true)
    {

        std::for_each(std::execution::par_unseq, population.begin(), population.end(), [&](auto& pop) {
            if (pop.fitness == 0) {
                pop.computeFitness(rays);
            }
        });

        printTime("Computing fitness: "); 

        sort(population.begin(), population.end(), std::greater<>()); // sorted best to worst -- slow, consider using pointers

        std::cout << std::to_string(generation);
        for (auto pop = population.begin(); pop != population.end(); pop++)
            std::cout << ";" << std::to_string(pop->fitness);
            // std::cout << *pop << std::endl;
        std::cout << std::endl;
  
        printTime("Sorting and printing: "); 

        for (uint32_t i = 0; i < popsize/3 ; i++)    // remove the weak
        {
            population.pop_back();
        }
        
        printTime("Erasing: "); 

        while (population.size() < popsize)
        {
            // get two random parents
            std::sample(population.begin(), population.end(), std::back_inserter(parents), 2, mt);
            // create an offspring
            const Genome offspring(parents[0], parents[1], 0.6, 0.05, 0.225);
            // add it to population
            population.push_back(SolarCollector(xsize, ysize, hmax, &obs, offspring));
            // remove parents to make place for new ones
            parents.clear();
        }
         
        printTime("Crossover and mutation: "); 

        // dont mutate the elites
        // std::cout << "crossovered" << std::endl;

        ++generation;
        // return 0;
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