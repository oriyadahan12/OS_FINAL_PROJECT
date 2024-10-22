#include "MST_Factory.hpp"
//#include "Prim.hpp"
//#include "Kruskal.hpp"

#include "Algo.hpp"


MST_Factory *MST_Factory::instance = nullptr;

std::map<std::string, MST_Strategy *> MST_Factory::strats = {{"prim", nullptr}, {"kruskal", nullptr}, {"tarjan", nullptr}, {"boruvka", nullptr}};
std::mutex MST_Factory::instance_mutex;

MST_Factory *MST_Factory::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MST_Factory();
        strats["prim"] = new Prim{};
        strats["kruskal"] = new Kruskal{};
        std::atexit(cleanUp);
    }
    return instance;
}

MST_Strategy* MST_Factory::createMST(std::string type)
{
    std::lock_guard<std::mutex> lock(instance_mutex);  // Lock the mutex
    if(strats.find(type) != strats.end())
    {
        return strats[type];
    }
    throw std::invalid_argument("Invalid MST Strategy");
}

void MST_Factory::cleanUp()
{
    std::lock_guard<std::mutex> lock(instance_mutex);
    if (instance != nullptr)
    {
        for (auto &strat : strats)
        {
            delete strat.second;
            strat.second = nullptr;
        }
        delete instance;
        instance = nullptr;
    }
}