#ifndef STATS_HPP
#define STATS_HPP

#include <chrono>
#include <string>
#include <map>
#include <vector>

struct stat_fields {

    const std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> finish;
    std::chrono::duration<double> delta;

    stat_fields() : start(std::chrono::high_resolution_clock::now())
                  , finish(std::chrono::high_resolution_clock::now())
                  , delta(finish - start) 
    {};
};

class Stats {
    // Static members to hold the statistics
    static std::map<std::string, std::vector<stat_fields> > stats; // Store key-value pairs

public:
    // Static methods to time and show statistics
    static void begin(const std::string& stat_name);
    static void end(const std::string& stat_name);
    static void show();
    static void clear();

};

#endif // STATS_HPP