#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include "GraphObj/graph.hpp"
#include "MST/MST_Strategy.hpp"
#include "MST/MST_Factory.hpp"
#include "ServerUtils/serverUtils.hpp"
#include "PAO/pipelineActiveObject.hpp"
#include <memory>
#include <mutex>
#include <signal.h>

#define PORT "9036"   // Port number where the server listens for connections
#define WELCOME_MSG_SIZE 480  // Size of the welcome message buffer

using namespace std;

/**
 * Struct to store the graph and the message to be sent to the client.
 * This structure is used in the PAO (Parallel Asynchronous Object) to 
 * execute various graph-related functions asynchronously.
 */
struct Triple{
    Graph* g;          // Pointer to the client's graph
    string msg;        // Message to be sent to the client
    int clientFd;      // File descriptor representing the client connection
};

// Global variables:
int fd_count = 0;     // Counter for number of file descriptors (clients)
PAO* pao = nullptr;   // Pointer to the PAO object managing tasks
mutex mtx;            // Mutex for global synchronization
map<int, pair<Graph*, Triple*>> clients_graphs;  // Maps client file descriptors to their graphs and associated Triple objects
map<int, mutex> clients_mtx;  // Maps client file descriptors to their corresponding mutex for thread safety
struct pollfd* pfds;  // Set of poll file descriptors, dynamically managed during client connections


/**
 * Handles an MST request from a client. This function creates a new Triple object
 * for each request, assigns the task to the PAO, and processes the MST calculation.
 *
 * @param g The graph object pointer
 * @param clientFd The file descriptor for the client
 * @param strat The MST algorithm strategy ("prim", "kruskal", etc.)
 * @return A pair consisting of the result message and the MST graph pointer
 */
std::pair<std::string, Graph *> MST(Graph *g, int clientFd, const std::string& strat)
{
    Graph* tmp = nullptr;
    Triple* t = nullptr;
    {
        unique_lock<mutex> lock(clients_mtx[clientFd]);  // Lock the mutex for thread safety

        // Clean up any existing Triple or graph for the client
        if (clients_graphs[clientFd].second != nullptr) {  
            if (clients_graphs[clientFd].second->g != nullptr) {
                delete clients_graphs[clientFd].second->g;  // Delete the old graph
                clients_graphs[clientFd].second->g = nullptr;
            }
            delete clients_graphs[clientFd].second;  // Delete the old Triple
        }

        // Create a new Triple object and store it in the client's record
        clients_graphs[clientFd].second = new Triple{g, strat, clientFd};  
        t = clients_graphs[clientFd].second;  // Assign the new Triple

        // Select the MST algorithm strategy (e.g., Prim, Kruskal)
        MST_Strategy* MST_strategy = MST_Factory::getInstance()->createMST(t->msg);  
        tmp = (*MST_strategy)(t->g);  // Generate the MST based on the selected strategy
    }

    // Update the Triple with the new MST graph and a success message
    t->g = tmp;  
    t->msg = "MST created using " + t->msg + " strategy\n";

    // Add the task to the PAO for further processing (like calculating metrics)
    pao->addTask(clients_graphs[clientFd].second);  
    std::cout << "User " << clientFd << " requested to find MST of the Graph" << std::endl;

    return {"", nullptr};  // Return empty result since the processing will be done asynchronously
}
/**
 * Signal handler for SIGINT (Ctrl+C). Cleans up resources and safely shuts down
 * the server by releasing allocated memory and closing client connections.
 */
void handleSig(int sig) {
    {
        for(auto& mtx : clients_mtx) {
            mtx.second.lock();
        }
        cout << "\nPAO-server: cleaning up resources..." << endl;

        // graphs:
        for(auto& graph_triple : clients_graphs) {
            if(graph_triple.second.first != nullptr) {  // freeing the graph
                delete graph_triple.second.first;
            }
            if(graph_triple.second.second != nullptr) {  // freeing the triple
                if(graph_triple.second.second->g != nullptr){  // freeing the graph in the triple
                    delete graph_triple.second.second->g;
                    graph_triple.second.second->g = nullptr;
                }
                delete graph_triple.second.second;  // freeing the triple itself
                graph_triple.second.second = nullptr;
            }
        }

        cout << "PAO-server: Graphs freed," << endl;
        // Clients: cleans the pfds
        for (int i = 0; i < fd_count; i++)
        {
            if (pfds[i].fd != -1)
            {
                close(pfds[i].fd);
            }
        }
        free(pfds);
        if (pao != nullptr) {
            delete pao;  // delete the PAO object
        }
        cout << "PAO-server: Clients freed,\n" << "Good Bye!" << endl;
        for (auto& mtx : clients_mtx) {
            mtx.second.unlock();
        }
        
    }
    exit(0);
}

 
/**
 * Main function of the server. Sets up the listener socket, manages client connections,
 * and handles incoming messages to perform graph-related actions (create graph, add/remove edge, MST).
 */

int main(void) {   

    // Create a list of functions to be executed by the PAO
    std::vector<std::function<void(void*)>> functions = {

        // second function calculates the total weight of the edges
        [](void* triple) { 
                            Triple* t = (Triple*)triple;  // cast the void* to Triple*
                            unique_lock<mutex> lock(clients_mtx[t->clientFd]);;  // lock the mutex
                            t->msg += "Total weight of edges: " + std::to_string((t->g)->totalWeight()) + "\n";
                            },

        // third function calculates the longest path
        [](void* triple) {Triple* t = (Triple*)triple;  // cast the void* to Triple*
                            unique_lock<mutex> lock(clients_mtx[t->clientFd]);;  // lock the mutex
                            t->msg += (t->g)->longestPath() + "\n";},

        // fourth function calculates the average distance between vertices
        [](void* triple) { Triple* t = (Triple*)triple;  // cast the void* to Triple*
                            unique_lock<mutex> lock(clients_mtx[t->clientFd]);;  // lock the mutex
                            t->msg += "The average distance between vertices is: " + std::to_string((t->g)->avgDistance()) + "\n";},

        // fifth function calculates the shortest paths
        [](void* triple) { Triple* t = (Triple*)triple;  // cast the void* to Triple*
                            unique_lock<mutex> lock(clients_mtx[t->clientFd]);;  // lock the mutex
                            t->msg += "The shortest paths are: \n" + (t->g)->allShortestPaths() + "\n"; 
                          
                            },  // delete the mst graph
        
        // sixth function sends the result msg to the clientFd and deletes the triple
        [](void* triple) { Triple* t = (Triple*)triple;  // cast the void* to Triple*
                            unique_lock<mutex> lock(clients_mtx[t->clientFd]);;  // lock the mutex
                            if (send(t->clientFd, t->msg.c_str(), t->msg.size(), 0) < 0)  // send the message to the client
                                perror("send");
                           
                            }  // delete the triple
    };

    pao = new PAO(functions);  // create a new PAO object with the functions
    pao->start();  // start the PAO object (start the threads). no need to stop it because it will be stopped in the destructor.
    const vector<string> graphActions = {"newgraph", "newedge", "removeedge", "mst"};
    const vector<string> mstStrats = {"prim", "kruskal", "tarjan", "boruvka"};

    string action = "";
    string actualAction = "";
    int m =0, n= 0, weight= 0;  // n := number of vertices, m := number of edges, weight := weight of the edge
    string strat = "";  // strategy for the MST
 
    char welcomeMsg[WELCOME_MSG_SIZE] = 
        "Welcome to the PAO-server!\n"
        "This server can perform the following actions:\n"
        "1. Create a new graph: newgraph n m where \"n\" is the number of vertices and \"m\" is the number of edges.\n"
        "2. Add an edge to the graph: newedge n m w where \"n\" and \"m\" are the vertices and \"w\" is the weight of the edge.\n"
        "3. Remove an edge from the graph: removeedge n m where \"n\" and \"m\" are the vertices.\n"
        "4. Find the Minimum Spanning Tree of the graph: mst strat -  where strat is either 'prim' or 'kruskal'\n";


    int newfd;                          // Newly accepted socket descriptor
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    char buf[256]= {0}; // Buffer for client data
    char remoteIP[INET6_ADDRSTRLEN] ={0};

    // Start off with room for 5 connections (isnt it four? because one is the listener..)
    // (We'll realloc as necessary)
    int fd_size = 5;

    // Set up and get a listening socket
    int listener = getListenerSocket();
    if (listener == -1){
        std::cerr << "Error getting listening socket on port " << PORT 
              << ": " << strerror(errno) << "\n";
        exit(1);
    }

    // Add the listener to set
    pfds = (struct pollfd *)malloc(sizeof *pfds * (size_t)fd_size);
    memset(pfds, 0, sizeof *pfds * (size_t)fd_size);
    pfds[0].fd = listener;
    pfds[0].events = POLLIN; // Report ready to read on incoming connection
    fd_count = 1; // For the listener
    cout << "waiting for connections..." << endl;

    signal(SIGINT, handleSig);  // handle the CTRL+C signal

    // Main loop
    while(true){
        int poll_count = poll(pfds, (size_t)fd_count, -1);

        if (poll_count == -1){
            perror("poll");
            exit(1);
        }

        // Run through the existing connections looking for data to read
        for (int i = 0; i < fd_count; i++){

            // Check if someone's ready to read
            if (pfds[i].revents & POLLIN){ // We got one!!

                if (pfds[i].fd == listener){ // If listener is ready to read, handle new connection

                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

                    if (newfd == -1){
                        perror("accept");
                    }
                    else{
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);

                        // Add the new client to the dictionary:
                        clients_graphs[newfd].first = nullptr;
                        clients_graphs[newfd].second = nullptr;

                        printf("pollserver: new connection from %s on socket %d\n",
                                    inet_ntop(remoteaddr.ss_family,
                                                getInAddr((struct sockaddr *)&remoteaddr),
                                                remoteIP, INET6_ADDRSTRLEN),
                                    newfd);
                        if (send(newfd, welcomeMsg, sizeof(welcomeMsg), 0) < 0){
                            perror("send");
                        }
                    }
                }
                else{ // handle existing connection 
                    int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0); // receiving the msg from the client
                    int sender_fd = pfds[i].fd;

                    if (nbytes <= 0){ // Got error or connection closed by client
                        if (nbytes == 0)
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        else
                            perror("ERROR: receiving data from client");

                        close(pfds[i].fd);  // close the connection 
                        del_from_pfds(pfds, i, &fd_count);  // remove the connection from the set of connections
                        {
                            unique_lock<mutex> lock(clients_mtx[sender_fd]);;
                            if (clients_graphs[sender_fd].first != nullptr){  // if the client has a graph, delete it
                            delete clients_graphs[sender_fd].first;
                            clients_graphs[sender_fd].first = nullptr;
                        }
                        if (clients_graphs[sender_fd].second != nullptr){  // if the client has a triple, delete it
                            if(clients_graphs[sender_fd].second->g != nullptr) {  // if the triple has a graph, delete it
                                delete clients_graphs[sender_fd].second->g;
                                clients_graphs[sender_fd].second->g = nullptr;
                            }
                            delete clients_graphs[sender_fd].second;  // delete the triple
                            clients_graphs[sender_fd].second = nullptr;
                        }
                        clients_graphs.erase(sender_fd);  // remove the client from the dictionary
                        }
                    }
                    else {  // the client sent a message
                        parseInput(buf, nbytes, n, m, weight, strat, action, actualAction, graphActions, mstStrats);
                        cout << "Action received: " << action << " from client " << sender_fd << endl;

                        // handling the input:
                        pair<string, Graph*> result = handleInput(clients_graphs[sender_fd].first, action, sender_fd, actualAction, n, m, weight, strat);
                        //string msg = result.first;
                        if (result.second != nullptr) {  // if the result is not null, store it in the dictionary for this client
                            clients_graphs[sender_fd].first = result.second;
                        }

                        // print the message to the server
                        if(actualAction == "message") {
                            cout << result.first << endl;
                            continue;
                        }

                        // if the actualAction is in the graphActions, then send the result to all the clients
                        if(find(graphActions.begin(), graphActions.end(), actualAction) != graphActions.end()) {
                            for (int j = 0; j < fd_count; j++) {
                                int dest_fd = pfds[j].fd;
                                if (dest_fd != listener)  // if the destination is not the listener
                                    if (send(dest_fd, result.first.c_str(), result.first.size() + 1, 0) < 0)  // send the result to the client include the null terminator
                                        perror("send");
                            }
                        }
                    }
                }
            } 
        } 
    } 
   

    return 0;
}