#include "PAO.hpp"

/**
 * Construct a new PAO object.
 * Get the functions to be executed by the workers.
 */
PAO::PAO(const std::vector<std::function<void(void*)>>& functions): stopFlag(false) {
    // filling the workers vector with the struct Worker:
    for (const auto& func : functions) {
        // std::thread* trd = new std::thread();  // creating a new thread
        std::mutex* mtx = new std::mutex();  // creating a new mutex
        std::condition_variable* cond = new std::condition_variable();  // creating a new condition variable
        workers.push_back({nullptr, func, std::queue<void*>(), mtx, cond, nullptr});
    }
    // Set the nextTaskQueue pointer for each worker except the last one:
    for (size_t i = 0; i < workers.size() - 1; ++i) {
        workers[i].nextTaskQueue = &workers[i + 1].taskQueue;
    }
}

PAO::~PAO() {

    stop();  // stop the threads

    // going over all the workers and deleting the threads, mutexes and condition variables
    for (auto& worker : workers) {
        // if the thread is joinable, join it (:= wait for it to finish)
        if (worker.thread && worker.thread->joinable()) {
            worker.thread->join();
        }

        delete worker.thread;
        delete worker.queueMutex;
        delete worker.condition;
    }
}

/**
 * in this function, the object will give the first thread the mst and the string and it will start working on it.
 */
void PAO::addTask(void* task) {
    
    std::lock_guard<std::mutex> lock(*(workers[0].queueMutex));
    workers[0].taskQueue.push(task);
    workers[0].condition->notify_one();  // notify the first worker to start working
}

/**
 * this function will start all the threads.
 * it will iterate over all the workers and start the thread with the workerFunction.
 */
void PAO::start() {
    stopFlag = false;
    
    for (size_t i = 0; i < workers.size(); ++i) {
        Worker* nextWorker = (i + 1 < workers.size()) ? &workers[i + 1] : nullptr;  // if the worker is not the last one, set the nextWorker to the next worker
        workers[i].thread = new std::thread(&PAO::workerFunction, this, std::ref(workers[i]), nextWorker);
    }
}

void PAO::stop() {
    stopFlag = true;
    for (auto& worker : workers) {  // notify all the workers to stop
        std::lock_guard<std::mutex> lock(*worker.queueMutex);
        worker.condition->notify_all();
        // if (worker.thread->joinable()) {
        //     worker.thread->join();
        // }
    }
}

/**
 * this function actually wrap the function that the worker will execute so it will not end until the stopFlag is true.
 */
void PAO::workerFunction(Worker& worker, Worker* nextWorker) {
    while (!stopFlag) {

        // creating a task:
        void* task = nullptr;
        {
            std::unique_lock<std::mutex> lock(*worker.queueMutex);
            // Note that in the wait function, the queueMutex is unlocked and locked again when the condition is met.
            worker.condition->wait(lock, [&]() { return stopFlag || !worker.taskQueue.empty(); });  // while the taskQueue is empty and the stopFlag is false, wait

            if (stopFlag && worker.taskQueue.empty()) return;
            task = worker.taskQueue.front();
            worker.taskQueue.pop();
        }

        // if the worker has a function to execute:
        if (worker.function) {
            worker.function(task);  // operate the function with the task
        }

        // if the worker is not the last one
        if (nextWorker) {  
            // pushing the task to the next worker:
            std::lock_guard<std::mutex> lock(*nextWorker->queueMutex);
            nextWorker->taskQueue.push(task);  // actually pushes a reserence to the mst and the string
            nextWorker->condition->notify_one();  // notify the next worker to start working
        }
        if(stopFlag) return;
    }
}