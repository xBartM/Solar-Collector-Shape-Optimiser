#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <random>
//#include <chrono>


#include <Solar-Collector-Shape-Optimiser/solarcollector.hpp>

using namespace std;

int main (int argc, char** argv)
{
    // auto start = chrono::high_resolution_clock::now();
    // auto end = chrono::high_resolution_clock::now();
    // std::chrono::duration<double> deltatime = end - start;

    ushort xsize;   // size of the panel (in mm preferably?)
    ushort ysize;   // -||-
    ushort hmax;    // max height of a panel

    vertex ray(0, -1, 0);

    ushort popsize; // size of the population

    ushort generation = 1;  // number of current generation

    // +1 because, ex.: xsize, ysize = 2, 2 gives 2 triangles/1 square with a side of length 1mm
    if (argc == 5) {
        xsize   = atoi(argv[1])+1;
        ysize   = atoi(argv[2])+1;
        hmax    = atoi(argv[3])+1;
        popsize = atoi(argv[4]);
    } else {
        xsize   = 180+1;    // size of printbed (minus some spare space)
        ysize   = 940+1;    // length of aluminum rod
        hmax    = 180+1;    // height of printbed (minus some spare space)
        popsize = 8;       // make popsize divisible by 4 xd
    }

    random_device rd;
    mt19937 mt(rd());
    // uniform_real_distribution<double> dist(0.0, (double)hmax/10.0);
    uniform_real_distribution<double> dist(0.0, 0.45);//(double)hmax); // educated guess? for example take the average of 20 runs with same settings, and increment denominator. find best place to start
    uniform_int_distribution<uint32_t> rand_id(0, numeric_limits<uint32_t>::max());

    //std::cout << dist(mt) << "\n";

    Mesh3d obs("obstacle.stl");
    obs.moveXY((xsize-1)/2, (hmax-1)/2);
    //obs.exportSTL("my_obstacle.stl");

    // vector<SolarCollector*> seed;//(0, xsize, ysize, hmax, &obs);
    // // cout << "hmax";
    // for (int i = 0; i < 12; i++)
    // {
    //     seed.push_back(new SolarCollector(rand_id(mt), xsize, ysize, hmax, &obs));
    //     // cout << ";S" << to_string(i);
    // }
    // cout << endl;

    // while (true)
    // for (double i = 0.4; i <= 0.55; i+=0.0025)
    // {
    //     #pragma omp parallel for num_threads(4)
    //     for (auto pop = seed.begin(); pop != seed.end(); pop++)
    //     {
    //         for (int j = 0; j < xsize*ysize; j++)
    //             (*pop)->setXY(j, 0, dist(mt)*(i/(double)hmax));
    //         (*pop)->computeMesh();
    //         (*pop)->computeMeshMidpoints();
    //         (*pop)->computeFitness(&ray, 1);
    //     }

    //     cout << to_string(i);
    //     for (auto pop = seed.begin(); pop != seed.end(); pop++)
    //         cout << ";" << to_string((*pop)->fitness);
    //     cout << endl;

    // }


    // return 0;

    // start = chrono::high_resolution_clock::now();
    vector<Genome> gene_pool;
    vector<SolarCollector> population;
    population.reserve(popsize); // reserve space to avoid reallocations
    for (int i = 0, j = 0; i < popsize; i++, j++)
    {
        population.emplace_back(xsize, ysize, hmax, &obs);
        for (int k = 0; k < xsize*ysize; k++)
            population[i].setXY(k, 0, dist(mt));
        population[i].computeMesh();
        population[i].computeMeshMidpoints();
        // population[i].computeFitness(&ray, 1);
        // if(population[i].fitness < 1000)
        // {
        //     cout << to_string(j) << "th try; fitness found: " << to_string(population[i].fitness) << endl;
        //     population.pop_back();
        //     i--;
        // }
        // else
        // {
        //     cout << to_string(i) << " out of " << to_string(popsize) << "; found after " << to_string(j) << " tries." << endl;
        //     j = 0;
        // }
        // population[i].exportAsSTL();

    }
    // end = chrono::high_resolution_clock::now();
    // deltatime = end - start;
    // std::cout << "Setting up population: " << deltatime.count() << " s\n";

    cout << "Gen";
    for (int i = 0; i < popsize; i++)
        cout << ";F" << to_string(i);
    cout << endl;
    while (true)
    {
        // start = chrono::high_resolution_clock::now();
        #pragma omp parallel for num_threads(4)
        for (auto pop = population.rbegin(); pop != population.rend(); pop++)
            if (pop->fitness == 0)
                pop->computeFitness(&ray, 1);
                // else cout << "skipped" << endl;
            // pop.exportReflectionsAsSTL(); // ??
        // end = chrono::high_resolution_clock::now();
        // deltatime = end - start;
        // std::cout << "Computing fitness: " << deltatime.count() << " s\n";
        sort(population.begin(), population.end()); // WRONG WAY (but it works - i think even better.. 2gen max 15k...) // sorted best to worst

        cout << to_string(generation);
        for (auto pop = population.begin(); pop != population.end(); pop++)
            cout << ";" << to_string(pop->fitness);
            // cout << *pop << endl;
        cout << endl;

        // start = chrono::high_resolution_clock::now();
        for (int i = 0; i < popsize/3 ; i++)    // remove the weak
        {
            // delete population.back();
            population.pop_back();
        }

        // end = chrono::high_resolution_clock::now();
        // deltatime = end - start;
        // std::cout << "Erasing: " << deltatime.count() << " s\n";

        // start = chrono::high_resolution_clock::now();
        for (auto pop = population.begin(); population.size() < popsize; pop++)
        {
            // this could be offspring constructor xDD
            // population.push_back(crossoverAndMutate(*pop, *(pop+1), rand_id(mt), 60, 5));

            // Use the crossoverAndMutate method from the Genome class
            auto offspring = pop->crossoverAndMutate(*(pop + 1), 60, 5, 0.225);

            // Convert Genome to SolarCollector (assuming constructor compatibility)
            SolarCollector child(xsize, ysize, hmax, &obs);
            child.dna = offspring.dna;

            child.computeMesh();
            child.computeMeshMidpoints();

            population.push_back(std::move(child));
  
        }
        // end = chrono::high_resolution_clock::now();
        // deltatime = end - start;
        // std::cout << "Crossover: " << deltatime.count() << " s\n";

        // dont mutate the elites
        // cout << "crossovered" << endl;

        ++generation;
    }

    return 0;
}
