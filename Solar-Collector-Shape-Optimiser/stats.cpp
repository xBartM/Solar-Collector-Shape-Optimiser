#include <iostream>
#include <algorithm>
#include <numeric>
#include <iomanip>

#include "Solar-Collector-Shape-Optimiser/stats.hpp"

std::map<std::string, std::vector<stat_fields>> Stats::stats; // Definition of the static member

void Stats::begin(const std::string& stat_name) {
    // Create a new stat_fields object and push it into the vector.
    // The constructor automatically captures the start time.
    if (stats.find(stat_name) == stats.end()) {
        // If the key doesn't exist, create a new vector for it.
        stats[stat_name] = std::vector<stat_fields>();
    }
    stats[stat_name].push_back(stat_fields());
}

void Stats::end(const std::string& stat_name) {
    // Check if the vector for the given stat_name exists and is not empty.
    if (stats.find(stat_name) != stats.end() && !stats[stat_name].empty()) {
        // Get the last (most recent) stat_fields object from the vector.
        stat_fields& current_stat = stats[stat_name].back();

        // Update the finish time and calculate the duration.
        current_stat.finish = std::chrono::high_resolution_clock::now();
        current_stat.delta = current_stat.finish - current_stat.start;
    } else {
        // Handle the case where begin() was not called or the vector is empty.
        std::cerr << "Warning: Stats::end() called for '" << stat_name
                  << "' without a corresponding Stats::begin() or vector is empty." << std::endl;
    }
}
void Stats::show() {
    std::cerr << std::fixed << std::setprecision(1); // Set precision for output
    std::cerr << "---------------------------------- Statistics ----------------------------------" << std::endl;

    // First, calculate the total time across all statistics.
    double total_all_time = 0.0;
    for (const auto& [stat_name, history] : stats) {
        total_all_time += std::accumulate(history.begin(), history.end(), 0.0,
            [](const double& sum, const stat_fields& s) {
                return sum + s.delta.count();
            });
    }

    for (auto& [stat_name, history] : stats) {
        std::cerr << stat_name << ":\t";

        // Calculate statistics
        if (!history.empty()) {
            // last time
            const double last_time = history.back().delta.count();

            // min max time
            const auto [min_it, max_it] = std::minmax_element(history.begin(), history.end(), 
                [](const stat_fields& a, const stat_fields& b) {
                    return a.delta.count() < b.delta.count();
                });
            const double min_time = min_it->delta.count();
            const double max_time = max_it->delta.count();

            // total time
            const double total_time = std::accumulate(history.begin(), history.end(), 0.0, 
                [](const double& sum, const stat_fields& sf) {
                    return sum + sf.delta.count();
                }); 

            // avg time
            const double avg_time = total_time / history.size();

            // Calculate percentage of total time.
            const double percentage = (total_all_time > 0.0) ? (total_time / total_all_time) * 100.0 : 0.0;

            std::cerr << last_time << "s (last); " 
                      << avg_time << "s (avg); " 
                      << min_time << "s (min); " 
                      << max_time << "s (max); " 
                      << total_time/60.0 << "m (total); "
                      << percentage << "% (total); " << std::endl;

        }
    }

    std::cerr << "--------------------------------------------------------------------------------" << std::endl;
}

void Stats::clear() {
    stats.clear();
}