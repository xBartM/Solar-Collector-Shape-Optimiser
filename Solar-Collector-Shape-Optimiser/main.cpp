#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>

#include <Solar-Collector-Shape-Optimiser/solarcollector.hpp>

#define MAIN_TIMED

using namespace std;

void findProperHmaxDist (const uint32_t xsize, const uint32_t ysize, const uint32_t hmax, const Mesh3d* obs);

int main (int argc, char** argv)
{
    #ifdef MAIN_TIMED
    auto start = chrono::high_resolution_clock::now();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> deltatime = end - start;
    #endif // MAIN_TIMED

    uint32_t xsize;   // size of the panel (in mm preferably?)
    uint32_t ysize;   // -||-
    uint32_t hmax;    // max height of a panel
    uint32_t popsize; // size of the population

    uint32_t generation = 0;  // number of current generation
    vertex ray(0, -1, 0);
    std::vector<vertex> rays({ray});

    // +1 because, ex.: xsize, ysize = 2, 2 gives 2 triangles/1 square with a side of length 1mm
    if (argc == 5) {
        xsize   = stoul(argv[1])+1;
        ysize   = stoul(argv[2])+1;
        hmax    = stoul(argv[3])+1;
        popsize = stoul(argv[4]);
    } else {
        xsize   = 180+1;    // size of printbed (minus some spare space)
        ysize   = 94+1;    // length of aluminum rod
        hmax    = 180+1;    // height of printbed (minus some spare space)
        popsize = 4;       // make popsize divisible by 4 xd
    }

    random_device rd;
    mt19937 mt(rd());
    uniform_real_distribution<double> dist(0.0, 0.45);//(double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start

    #ifdef MAIN_TIMED
    end = chrono::high_resolution_clock::now();
    deltatime = end - start;
    std::cout << "Setting up main() variables: " << deltatime.count() << " s\n";
    start = chrono::high_resolution_clock::now();
    #endif // MAIN_TIMED

    // obs2 should be const
    Mesh3d obs("obstacle.stl");
    Mesh3dSoA obs2("obstacle.stl");
    obs.moveXY((xsize-1.0)/2.0, (hmax-1.0)/2.0);
    obs2.moveXY((xsize-1.0)/2.0, (hmax-1.0)/2.0);
    obs2.findEdges(); // needed later for Moller-.. intersection algorithm
    //obs.exportSTL("my_obstacle.stl");

    #ifdef MAIN_TIMED
    end = chrono::high_resolution_clock::now();
    deltatime = end - start;
    std::cout << "Setting up the obstacle: " << deltatime.count() << " s\n";
    start = chrono::high_resolution_clock::now();
    #endif // MAIN_TIMED

    // findProperHmaxDist(xsize, ysize, hmax, &obs);

    // vector<Genome> gene_pool;
    vector<SolarCollector> population;
    population.reserve(popsize); // reserve space to avoid reallocations
    for (uint32_t i = 0, j = 0; i < popsize; i++, j++) {
        population.emplace_back(xsize, ysize, hmax, &obs);
        for (uint32_t k = 0; k < xsize*ysize; k++)
            population[i].setXY(k, 0, dist(mt));
        population[i].computeMesh();
        // population[i].computeMeshSoA();

    }

    vector<SolarCollector> population2;
    population2.reserve(popsize); // reserve space to avoid reallocations
    for (uint32_t i = 0, j = 0; i < popsize; i++, j++) {
        population2.emplace_back(xsize, ysize, hmax, &obs2);
        for (uint32_t k = 0; k < xsize*ysize; k++)
            population2[i].setXY(k, 0, dist(mt));
        population2[i].computeMeshSoA();

    }

    // format text for CSV integration
    cout << "Gen";
    for (uint32_t i = 0; i < popsize; i++)
        cout << ";F" << to_string(i);
    cout << endl;

    #ifdef MAIN_TIMED
    end = chrono::high_resolution_clock::now();
    deltatime = end - start;
    std::cout << "Setting up population: " << deltatime.count() << " s\n";
    start = chrono::high_resolution_clock::now();
    #endif // MAIN_TIMED

    while (true)
    {
        #pragma omp parallel for num_threads(4)
        for (auto pop = population.rbegin(); pop != population.rend(); pop++)
            if (pop->fitness == 0)
                pop->computeFitness(&ray, 1);
                // else cout << "skipped" << endl;
            // pop.exportReflectionsAsSTL(); // ??

        #ifdef MAIN_TIMED
        end = chrono::high_resolution_clock::now();
        deltatime = end - start;
        std::cout << "Computing fitness: " << deltatime.count() << " s\n";
        start = chrono::high_resolution_clock::now();
        #endif // MAIN_TIMED

        #pragma omp parallel for num_threads(4)
        for (auto pop = population2.rbegin(); pop != population2.rend(); pop++)
            if (pop->fitness == 0)
                pop->computeFitnessSoA(rays);
                // else cout << "skipped" << endl;
            // pop.exportReflectionsAsSTL(); // ??

        #ifdef MAIN_TIMED
        end = chrono::high_resolution_clock::now();
        deltatime = end - start;
        std::cout << "Computing fitness: " << deltatime.count() << " s\n";
        start = chrono::high_resolution_clock::now();
        #endif // MAIN_TIMED

        sort(population.begin(), population.end()); // WRONG WAY (but it works - i think even better.. 2gen max 15k...) // sorted best to worst
        sort(population2.begin(), population2.end()); // WRONG WAY (but it works - i think even better.. 2gen max 15k...) // sorted best to worst

        cout << to_string(generation);
        for (auto pop = population.begin(); pop != population.end(); pop++)
            cout << ";" << to_string(pop->fitness);
            // cout << *pop << endl;
        cout << endl;

        cout << to_string(generation);
        for (auto pop = population2.begin(); pop != population2.end(); pop++)
            cout << ";" << to_string(pop->fitness);
            // cout << *pop << endl;
        cout << endl;
  
        #ifdef MAIN_TIMED
        end = chrono::high_resolution_clock::now();
        deltatime = end - start;
        std::cout << "Sorting and printing: " << deltatime.count() << " s\n";
        start = chrono::high_resolution_clock::now();
        #endif // MAIN_TIMED

        for (uint32_t i = 0; i < popsize/3 ; i++)    // remove the weak
        {
            // delete population.back();
            population.pop_back();
            population2.pop_back();
        }
        
        #ifdef MAIN_TIMED
        end = chrono::high_resolution_clock::now();
        deltatime = end - start;
        std::cout << "Erasing: " << deltatime.count() << " s\n";
        start = chrono::high_resolution_clock::now();
        #endif // MAIN_TIMED

        // start = chrono::high_resolution_clock::now();
        for (auto pop = population.begin(); population.size() < popsize; pop++)
        {
            // this could be offspring constructor xDD
            // population.push_back(crossoverAndMutate(*pop, *(pop+1), 60, 5));

            // Use the crossoverAndMutate method from the Genome class
            // auto offspring = pop->crossoverAndMutateSoA(*(pop + 1), 60.0, 5.0, 0.225);
            auto offspring = pop->crossoverAndMutateSoA(*(pop + 1), 0.6, 0.05, 0.225);

            // Convert Genome to SolarCollector (assuming constructor compatibility)
            SolarCollector child(xsize, ysize, hmax, &obs);
            child.dna = offspring.dna;

            child.computeMesh();

            population.push_back(std::move(child));
  
        }
         
       for (auto pop = population2.begin(); population2.size() < popsize; pop++)
        {
            // this could be offspring constructor xDD
            // population.push_back(crossoverAndMutate(*pop, *(pop+1), 60, 5));

            // Use the crossoverAndMutate method from the Genome class
            // auto offspring = pop->crossoverAndMutateSoA(*(pop + 1), 60.0, 5.0, 0.225);
            auto offspring = pop->crossoverAndMutateSoA(*(pop + 1), 0.6, 0.05, 0.225);

            // Convert Genome to SolarCollector (assuming constructor compatibility)
            SolarCollector child(xsize, ysize, hmax, &obs2);
            child.dna = offspring.dna;

            child.computeMeshSoA();

            population2.push_back(std::move(child));
  
        }

        #ifdef MAIN_TIMED
        end = chrono::high_resolution_clock::now();
        deltatime = end - start;
        std::cout << "Crossover and mutation: " << deltatime.count() << " s\n";
        start = chrono::high_resolution_clock::now();
        #endif // MAIN_TIMED

        // dont mutate the elites
        // cout << "crossovered" << endl;

        ++generation;
        return 0;
    }

    return 0;
}

void findProperHmaxDist (const uint32_t xsize, const uint32_t ysize, const uint32_t hmax, const Mesh3d* obs) {

    constexpr uint32_t test_sample = 12;
    constexpr uint32_t test_batches = 9;
    vertex ray(0, -1, 0);

    random_device rd;
    mt19937 mt(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);//(double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start

    vector<SolarCollector> seed;//(0, xsize, ysize, hmax, &obs);
    seed.reserve(test_sample); // reserve space to avoid reallocations
    cout << "hub"; // hmax upper bound
    for (uint32_t i = 0; i < test_sample; ++i)
    {
        seed.emplace_back(xsize, ysize, hmax, obs);
        cout << ";S" << to_string(i);
    }
    cout << endl;

    for (double i = 0.0; i <= (double)hmax; i+=(double)hmax/(double)test_batches)//i+=0.0025)
    {
        #pragma omp parallel for num_threads(4)
        for (auto pop = seed.begin(); pop != seed.end(); pop++)
        {
            for (uint32_t j = 0; j < xsize*ysize; j++)
                pop->setXY(j, 0, dist(mt)*i);
            pop->computeMesh();
            pop->computeFitness(&ray, 1);
        }

        cout << to_string(i);
        for (auto pop = seed.begin(); pop != seed.end(); pop++)
            cout << ";" << to_string(pop->fitness);
        cout << endl;

    }
    exit(0);
}