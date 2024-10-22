#include "LFP.hpp"

using namespace std;

/**
 * @brief Constructor to initialize the thread pool with a specified number of threads.
 * Each thread is assigned a unique ID and added to the `threads` vector.
 * @param num_threads The number of threads in the thread pool.
 */
LFP::LFP(int num_threads) : stopFlag(false), leader(0) {  // Initialize stopFlag to false and leader to thread 0
    for (int i = 0; i < num_threads; ++i) {
        // Create threads and assign worker function, passing thread ID
        threads.emplace_back(&LFP::worker, this, i);
        threadIDs.push_back(i);  // Store thread IDs
    }
}

/**
 * @brief Destructor to stop all threads and clean up resources.
 * Calls the stop function to ensure all threads are properly joined.
 */
LFP::~LFP() {
    stop();  // Ensure all threads are stopped when the object is destroyed
}

/**
 * @brief Adds a new task to the task queue.
 * Tasks are represented as `std::function<void()>` objects, and the task
 * is added to the queue with thread-safety. One of the threads is notified 
 * to wake up and execute the task.
 * @param task A function representing the task to be executed.
 */
void LFP::addTask(function<void()> task) {
    {
        lock_guard<mutex> lock(queueMutex);  // Lock the queue mutex to ensure safe access
        taskQueue.push(task);  // Add the task to the queue
        condition.notify_one();  // Notify one waiting thread that a task is available
    }
}

/**
 * @brief Starts the thread pool, allowing threads to begin processing tasks.
 * Sets the stop flag to `false` so threads can begin working if they are not already.
 */
void LFP::start() {
    {
        lock_guard<mutex> lock(stopMutex);  // Lock the stop mutex to safely change stopFlag
        stopFlag = false;  // Ensure stopFlag is reset to false to allow threads to run
    }
}

/**
 * @brief Stops the thread pool, signaling threads to finish their current tasks and terminate.
 * This sets the `stopFlag` to true, notifies all threads, and waits for each to finish.
 */
void LFP::stop() {
    {
        lock_guard<mutex> lock(stopMutex);  // Lock the stop mutex
        stopFlag = true;  // Signal threads to stop after completing current tasks
    }
    
    // Notify all threads that they should wake up (either to finish tasks or terminate)
    {
        lock_guard<mutex> lock(queueMutex);  // Lock the queue mutex
        condition.notify_all();  // Notify all threads
    }

    // Join all threads to ensure they have finished executing
    for (thread &thread : threads) {
        if (thread.joinable()) {
            thread.join();  // Wait for the thread to complete
        }
    }
}

/**
 * @brief Worker function executed by each thread.
 * Threads continuously check for tasks in the queue and execute them if available.
 * They also handle the stop flag to exit when instructed to stop.
 * @param id The unique ID of the thread running this worker function.
 */
void LFP::worker(int id) {
    while (true) {  // Continuously run unless stopped
        function<void()> task;  // Placeholder for the task to be executed

        // Lock the queue and wait for a task or stop signal
        {
            unique_lock<mutex> lock(queueMutex);  // Lock the queue mutex
            condition.wait(lock, [this]() {
                lock_guard<mutex> stopLock(stopMutex);  // Lock stop mutex during check
                return stopFlag || !taskQueue.empty();  // Continue if stopFlag is set or there are tasks
            });

            // Check if the stop flag is set and the task queue is empty, exit if true
            {
                lock_guard<mutex> stopLock(stopMutex);  // Lock the stop mutex
                if (stopFlag && taskQueue.empty()) return;  // Exit worker loop
            }

            // If the current thread is the leader and there are tasks, pop a task from the queue
            if (!taskQueue.empty() && this->leader == id) {
                task = taskQueue.front();  // Get the task from the front of the queue
                taskQueue.pop();  // Remove the task from the queue
            } else {
                continue;  // Continue if no task is assigned to this thread
            }
        }

        // Update the leader to the next thread (round-robin mechanism)
        {
            lock_guard<mutex> lock(queueMutex);  // Lock the queue mutex
            leader = (size_t)(leader + 1) % threads.size();  // Cycle leader ID to next thread
        }

        // Execute the task outside of the lock
        task();  
    }
}
