#ifndef LFP_HPP
#define LFP_HPP

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>

using namespace std;

/**
 * @brief Leader-Follower Pattern (LFP) class manages a thread pool where 
 * threads take turns becoming the leader and executing tasks from a queue.
 */
class LFP {

    public:
        /**
         * @brief Constructor that initializes the thread pool with a specified number of threads.
         * @param num_threads Number of threads to be created in the pool.
         */
        LFP(int num_threads);

        /**
         * @brief Destructor that stops all threads and cleans up resources.
         */
        ~LFP();

        /**
         * @brief Add a new task to the queue. The task is a function to be executed by a worker thread.
         * @param task A function representing the task to add.
         */
        void addTask(function<void()> task);

        /**
         * @brief Start the thread pool by launching all threads and processing tasks.
         */
        void start();

        /**
         * @brief Stop the thread pool and signal all threads to terminate once tasks are completed.
         */
        void stop();

        private:
        /**
         * @brief Worker function for the threads in the pool.
         * @param id The ID of the thread.
         */
        void worker(int id);

        vector<thread> threads;             // Vector to store the pool of threads
        vector<int> threadIDs;              // Vector to store thread IDs
        queue<function<void()>> taskQueue;  // Queue to store pending tasks
        mutex queueMutex;                   // Mutex to protect access to the task queue
        mutex stopMutex;                    // Mutex to protect the stop flag
        condition_variable condition;       // Condition variable to notify threads of new tasks
        bool stopFlag;                      // Flag to signal the termination of the thread pool
        int leader;                         // ID of the leader thread (current leader)

};

#endif // LFP_HPP
