#pragma once
#include "MST_Strategy.hpp"
#include <string>
#include <map>
#include <mutex>

class MST_Factory
{
    private:
        MST_Factory() = default;
        ~MST_Factory() = default;
        static MST_Factory* instance;  // Singleton instance
        MST_Factory(const MST_Factory&) = delete;  // No copy constructor
        MST_Factory& operator=(const MST_Factory&) = delete;  // No copy assignment
        static std::map<std::string, MST_Strategy*> strats;  // Map to store the strategies
        static void cleanUp();
        static std::mutex instance_mutex;
   
    public:
        static MST_Strategy* createMST(std::string type);
        static MST_Factory* getInstance();

};