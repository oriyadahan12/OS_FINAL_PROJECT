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
#include "../Graph/graph.hpp"


class Pipeline {
public:
    // Constructor: Accepts a list of functions to be executed by worker threads
    Pipeline(const std::vector<std::function<void(void*)>>& functions);

    // Destructor: Cleans up resources and ensures threads are stopped
    ~Pipeline();
    
    // Default constructor (in case we need it)
    Pipeline() = default;

    // Adds a task to be executed by the workers
    void addTask(void* task);
    
    // Starts the worker threads and begins processing tasks
    void start();

    // Signals the workers to stop processing and shuts down the threads
    void stop();

private:
    // Worker struct: Represents an individual worker thread and its associated data
    struct Worker {
        std::thread* thread;  // Pointer to the thread running the worker
        std::function<void(void*)> function;  // Function that the worker will execute on tasks
        std::queue<void*> taskQueue;  // Queue of tasks assigned to the worker (tasks are of generic type `void*`)
        std::mutex* queueMutex;  // Mutex for synchronizing access to the worker's task queue
        std::condition_variable* condition;  // Condition variable to notify the worker of new tasks
        std::queue<void*>* nextTaskQueue;  // Pointer to the task queue of the next worker in the pipeline
    };

    // Function that defines the behavior of a worker thread. Processes tasks and passes them to the next worker.
    void workerFunction(Worker& worker, Worker* nextWorker);

    std::vector<Worker> workers;  // Vector holding all worker threads
    std::atomic<bool> stopFlag;   // Atomic flag used to signal workers to stop
};
