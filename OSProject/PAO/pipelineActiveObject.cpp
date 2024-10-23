#include "pipelineActiveObject.hpp"

/**
 * Constructor: Initializes a PAO object.
 * Takes a list of functions to be executed by the worker threads.
 */
PAO::PAO(const std::vector<std::function<void(void*)>>& functions) : stopFlag(false) {
    // Populate the workers vector with Worker structs
    for (const auto& func : functions) {
        // Creating a mutex and condition variable for each worker
        std::mutex* taskQueueMutex = new std::mutex();
        std::condition_variable* taskCondition = new std::condition_variable();
        // Add a new worker to the vector with the given function, empty task queue, mutex, and condition variable
        workers.push_back({nullptr, func, std::queue<void*>(), taskQueueMutex, taskCondition, nullptr});
    }
    // Set the nextTaskQueue pointer for each worker, except the last one, to point to the next worker's task queue
    for (size_t i = 0; i < workers.size() - 1; ++i) {
        workers[i].nextTaskQueue = &workers[i + 1].taskQueue;
    }
}

/**
 * Destructor: Cleans up resources by stopping the worker threads and freeing memory.
 */
PAO::~PAO() {
    stop();  // Stop all worker threads

    // Iterate over all workers and free associated resources (threads, mutexes, condition variables)
    for (auto& worker : workers) {
        if (worker.thread && worker.thread->joinable()) {
            worker.thread->join();  // Ensure the thread has finished executing
        }
        // Free allocated memory for the thread, mutex, and condition variable
        delete worker.thread;
        delete worker.queueMutex;
        delete worker.condition;
    }
}

/**
 * Add a new task to the first worker's task queue.
 * This method locks the queue and notifies the first worker to start processing.
 */
void PAO::addTask(void* newTask) {
    std::lock_guard<std::mutex> lock(*(workers[0].queueMutex));
    workers[0].taskQueue.push(newTask);
    workers[0].condition->notify_one();  // Notify the first worker to start working on the new task
}

/**
 * Start all worker threads.
 * Iterates over all workers and creates threads for each, passing the workerFunction.
 */
void PAO::start() {
    stopFlag = false;  // Reset the stop flag

    // Iterate over all workers and start a thread for each one
    for (size_t i = 0; i < workers.size(); ++i) {
        Worker* nextWorker = (i + 1 < workers.size()) ? &workers[i + 1] : nullptr;  // If it's not the last worker, set the next worker
        workers[i].thread = new std::thread(&PAO::workerFunction, this, std::ref(workers[i]), nextWorker);
    }
}

/**
 * Stop all worker threads.
 * Sets the stop flag to true and notifies all workers to stop processing.
 */
void PAO::stop() {
    stopFlag = true;  // Signal the stop condition to all workers

    // Notify all workers to stop by unlocking their mutex and signaling their condition variables
    for (auto& worker : workers) {
        std::lock_guard<std::mutex> lock(*worker.queueMutex);
        worker.condition->notify_all();  // Notify all workers to exit
    }
}

/**
 * The worker function that continuously processes tasks until the stop flag is set.
 * Executes the assigned function on each task and forwards tasks to the next worker.
 */
void PAO::workerFunction(Worker& currentWorker, Worker* nextWorker) {
    while (!stopFlag) {
        // Task processing:
        void* task = nullptr;
        {
            std::unique_lock<std::mutex> lock(*currentWorker.queueMutex);
            // Wait until there's a task or the stop flag is set
            currentWorker.condition->wait(lock, [&]() { return stopFlag || !currentWorker.taskQueue.empty(); });

            if (stopFlag && currentWorker.taskQueue.empty()) return;  // Exit if stop flag is set and no tasks remain

            task = currentWorker.taskQueue.front();  // Get the task from the queue
            currentWorker.taskQueue.pop();  // Remove the task from the queue
        }

        // Execute the worker's function with the task, if available
        if (currentWorker.function) {
            currentWorker.function(task);
        }

        // If there is a next worker, forward the task to their queue
        if (nextWorker) {
            std::lock_guard<std::mutex> lock(*nextWorker->queueMutex);
            nextWorker->taskQueue.push(task);  // Forward the task to the next worker
            nextWorker->condition->notify_one();  // Notify the next worker to start working
        }

        if (stopFlag) return;  // Check if stop flag was set mid-processing
    }
}
