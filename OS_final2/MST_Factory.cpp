#pragma once
#include <string>
#include <map>
#include <mutex>
#include "MST_Strategy.hpp"
#include "Prim.cpp"
#include "Kruskal.cpp"

// Singleton pattern implementation for MST_Factory
MST_Factory *MST_Factory::instance = nullptr;  // Pointer to the single instance of MST_Factory

// Map to hold various MST strategy instances, initialized to nullptr
std::map<std::string, MST_Strategy *> MST_Factory::strats = {
    {"prim", nullptr},
    {"kruskal", nullptr},
};

std::mutex MST_Factory::instance_mutex;  // Mutex for thread-safe access to the singleton

// Method to get the single instance of MST_Factory
MST_Factory *MST_Factory::getInstance() {
    // Check if instance is not already created
    if (instance == nullptr) {
        instance = new MST_Factory();  // Create the instance
        // Initialize the strategies
        strats["prim"] = new Prim{};
        strats["kruskal"] = new Kruskal{};
        
        // Register cleanup function to be called at program exit
        std::atexit(cleanUp);
    }
    return instance;  // Return the singleton instance
}

// Method to create an MST strategy based on the provided type
MST_Strategy* MST_Factory::createMST(std::string algo) {
    std::lock_guard<std::mutex> lock(instance_mutex);  // Lock the mutex for thread safety
    
    // Check if the requested strategy type exists in the map
    if (strats.find(algo) != strats.end()) {
        return strats[algo];  // Return the corresponding strategy
    }
    // Throw an exception if the strategy type is invalid
    throw std::invalid_argument("Invalid MST Strategy input");
}

// Cleanup method to delete allocated strategies and the instance
void MST_Factory::cleanUp() {
    std::lock_guard<std::mutex> lock(instance_mutex);  // Lock the mutex for thread safety
    
    // Check if the instance is not null
    if (instance != nullptr) {
        // Delete all strategy instances
        for (auto &strat : strats) {
            delete strat.second;  // Delete the strategy object
            strat.second = nullptr;  // Set pointer to nullptr
        }
        delete instance;  // Delete the singleton instance
        instance = nullptr;  // Set the instance pointer to nullptr
    }
}
