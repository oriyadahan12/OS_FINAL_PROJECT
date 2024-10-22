#include "LeaderFollower.hpp"

using namespace std;

// Constructor for the Leader-Follower class
LF::LF(int num_threads) : stopFlag(false), leader(0) {  
    for (int i = 0; i < num_threads; ++i) {  // Create the specified number of threads
        threads.emplace_back(&LF::worker, this, i);  // Create and start worker threads
        threadIDs.push_back(i); // Store thread IDs
    }
}

// Destructor for the Leader-Follower class
LF::~LF() {
    stop();  // Stop all threads upon destruction
}

// Add a new task to the task queue
void LF::addTask(function<void()> task) {
    {
        lock_guard<mutex> lock(queueMutex);  // Lock the mutex for thread-safe access
        taskQueue.push(task);  // Push the task into the queue
        c.notify_one();  // Notify one waiting thread
    }
}

// Start the Leader-Follower mechanism
void LF::start() {
    {
        lock_guard<mutex> lock(stopMutex);  // Lock the mutex to change stopFlag
        stopFlag = false;  // Set the flag to allow tasks to be processed
    }
}

// Stop the Leader-Follower mechanism
void LF::stop() {
    {
        lock_guard<mutex> lock(stopMutex);  // Lock the mutex to change stopFlag
        stopFlag = true;  // Set the flag to stop processing tasks
    }
    
    {
        lock_guard<mutex> lock(queueMutex); // Lock the queue mutex
        c.notify_all();  // Wake up all waiting threads
    }
    for (thread &thread : threads) {  // Join all threads
        if (thread.joinable()) {
            thread.join();  // Wait for the thread to finish
        }
    }
}

// Worker function for processing tasks in the queue
void LF::worker(int id) {
    while (true) {  
        function<void()> task;  // Variable to hold the task
        {
            unique_lock<mutex> lock(queueMutex); // Lock the queue mutex
            // Wait for a task to be available or for stop signal
            c.wait(lock, [this]() { 
                lock_guard<mutex> stopLock(stopMutex);  // Lock the stop mutex
                return stopFlag || !taskQueue.empty(); // Condition to wake up
            });
            {
                lock_guard<mutex> stopLock(stopMutex);  // Lock the stop mutex
                if (stopFlag && taskQueue.empty()) return; // Exit if stopping and no tasks
            }
            // Check if there's a task and if this thread is the leader
            if (!taskQueue.empty() && this->leader == id) {
                task = taskQueue.front(); // Get the next task
                taskQueue.pop(); // Remove the task from the queue
            } else {
                continue; // If not the leader or no task, continue waiting
            }
        }
        // Locking the leader for the next task assignment
        {
            lock_guard<mutex> lock(queueMutex);
            leader = (size_t)(leader + 1) % threads.size(); // Rotate leader to the next thread
        }
        task();  // Execute the task
    }
}
