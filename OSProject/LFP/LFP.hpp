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

class LFP {
    private:
        void worker(int id);  // Worker function
        vector<thread> threads;  // Vector to store threads
        vector<int> threadIDs;  // Vector to store thread IDs
        queue<function<void()>> taskQueue;  // Queue to store tasks
        mutex queueMutex;  // Mutex to protect the tasks queue
        mutex stopMutex;  // Mutex to protect the stop flag
        condition_variable condition;  // used to notify the threads that there is a task in the queue
        bool stopFlag;  // Flag to stop the threads if set to true
        int leader;  // Leader thread
    public:
        LFP(int num_threads);
        ~LFP();
        void addTask(function<void()> task);  // Add a task to the queue
        void start();
        void stop();
};

#endif // LFP_HPP
