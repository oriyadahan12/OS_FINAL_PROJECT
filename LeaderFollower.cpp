#include "LeaderFollower.hpp"

using namespace std;

LF::LF(int num_threads) : stopFlag(false), leader(0){  // Constructor
    for (int i = 0; i < num_threads; ++i) {  // Create threads and add them to the vector
        threads.emplace_back(&LF::worker, this, i);  // arguments: function, object, id
        threadIDs.push_back(i);
    }
}

LF::~LF() {
    stop();  
}

void LF::addTask(function<void()> task) {
    // cout << "Adding task to the LF queue\n" << endl;
    {
        lock_guard<mutex> lock(queueMutex);  // Lock the mutex
        taskQueue.push(task);  // Add the task to the queue
        condition.notify_one();  // Notify one of the threads
    }
}


void LF::start() {
    {
        lock_guard<mutex> lock(stopMutex);  // Lock the mutex
        stopFlag = false;  
    }
}

void LF::stop() {
    {
        lock_guard<mutex> lock(stopMutex);  // Lock the mutex
        stopFlag = true;
    }
    
    {
        lock_guard<mutex> lock(queueMutex);
        condition.notify_all();
    }
    for (thread &thread : threads) {
        if (thread.joinable()) {
            thread.join();  // Join the threads
        }
    }
}

void LF::worker(int id) {
    while (true) {  // Worker function to process tasks in the queue
        function<void()> task;  // Task to be executed
        {
            unique_lock<mutex> lock(queueMutex);
            condition.wait(lock, [this]() { 
                lock_guard<mutex> stopLock(stopMutex);  // Lock the stop mutex
                return stopFlag || !taskQueue.empty(); 
            });
            // std::cout << "Executing task from the LF queue by thread " << id << endl << endl;
            {
                lock_guard<mutex> stopLock(stopMutex);  // Lock the stop mutex
                if (stopFlag && taskQueue.empty()) return;
            }
            if (!taskQueue.empty() && this->leader == id) {
                // std::cout << "Executing task from the LF queue by thread " << id << endl << endl;
                task = taskQueue.front();
                taskQueue.pop();
            } else {
                continue;
            }
        }
        // std::cout << "Executing task from the LF queue by thread " << id << endl << endl;
        // locking the leader:
        {
            lock_guard<mutex> lock(queueMutex);
            leader = (size_t)(leader+1) % threads.size();
        }
        task();  // Execute the task
        
    }
}