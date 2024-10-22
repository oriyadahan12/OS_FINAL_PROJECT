#pragma once

#include <string>
#include <map>
#include <mutex>
#include <stdexcept>
#include "MST_Strategy.hpp"  // Base class for MST strategies
#include "Prim.hpp"          // Prim algorithm implementation
#include "Kruskal.hpp"       // Kruskal algorithm implementation

/**
 * @class MST_Factory
 * @brief Singleton class for creating MST strategy objects.
 *
 * The MST_Factory class implements the Singleton design pattern to ensure 
 * that only one instance of the factory exists. The factory is responsible 
 * for creating and managing instances of different MST strategies like Prim 
 * and Kruskal.
 */
class MST_Factory {
private:
    // Pointer to the single instance of the factory (Singleton)
    static MST_Factory *instance;
    
    // Mutex to ensure thread-safe access to the singleton
    static std::mutex instance_mutex;

    // Map to hold instances of different MST strategies (e.g., "prim", "kruskal")
    static std::map<std::string, MST_Strategy *> strats;

    // Private constructor to prevent external instantiation
    MST_Factory() = default;

public:
    /**
     * @brief Get the singleton instance of the MST_Factory.
     * 
     * This method ensures that only one instance of the factory is created and 
     * initializes the map of MST strategies if it hasn't been done already.
     * 
     * @return MST_Factory* The singleton instance of the MST_Factory.
     */
    static MST_Factory *getInstance();

    /**
     * @brief Create an MST strategy based on the provided algorithm name.
     * 
     * This method looks up the appropriate MST strategy in the map based on 
     * the algorithm name (e.g., "prim", "kruskal") and returns a pointer to 
     * the corresponding strategy instance.
     * 
     * @param algo The name of the MST algorithm (e.g., "prim", "kruskal").
     * @return MST_Strategy* A pointer to the corresponding MST strategy object.
     * @throws std::invalid_argument If the provided algorithm name is invalid.
     */
    MST_Strategy* createMST(std::string algo);

    /**
     * @brief Clean up the resources by deleting all strategy instances and the singleton.
     * 
     * This method is called at the end of the program to clean up dynamically 
     * allocated resources, including the MST strategy objects and the factory instance itself.
     */
    static void cleanUp();
};

#endif // MST_FACTORY_HPP
