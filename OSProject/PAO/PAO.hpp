#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <string>
#include <utility>
#include "../ServerUtils/serverUtils.hpp"
#include "../GraphObj/graph.hpp"


class PAO {
    private:

        // Worker struct that will represents a worker thread
        struct Worker {
            std::thread* thread;  // The worker thread
            std::function<void(void*)> function;  // The function that the worker will execute
            std::queue<void*> taskQueue;  // The queue of tasks for the worker. task = void* because the function can get any type of task
            std::mutex* queueMutex;  // Mutex for the task queue
            std::condition_variable* condition;  // Condition variable for the worker to wait on
            std::queue<void*>* nextTaskQueue;  // Pointer to the next worker's task queue
        };

        void workerFunction(Worker& worker, Worker* nextWorker);

        std::vector<Worker> workers;
        std::atomic<bool> stopFlag;   // atomic flag to stop the threads
    
    public:
        PAO(const std::vector<std::function<void(void*)>>& functions);  // the constructor get the functions to be executed by the workers
        ~PAO();
        PAO() = default;

        void addTask(void* task);
        void start();
        void stop();
};