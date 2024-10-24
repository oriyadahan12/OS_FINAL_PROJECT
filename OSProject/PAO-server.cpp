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
#include <map>
#include <memory>
#include <mutex>
#include <signal.h>
#include <vector>
#include <functional>
#include "GraphObj/graph.hpp"
#include "MST/MST_Strategy.hpp"
#include "MST/MST_Factory.hpp"
#include "ServerUtils/serverUtils.hpp"
#include "PAO/pipelineActiveObject.hpp"

#define PORT "8080"   // Port number where the server listens for connections
#define SIZE 40  // Size of the welcome message buffer

using namespace std;

// Struct to store the graph and the message to be sent to the client.
struct Triple {
    Graph* g;          // Pointer to the client's graph
    string msg;        // Message to be sent to the client
    int client_fd;     // File descriptor representing the client connection
};
// Global variables
int fd_count = 0;     // Counter for number of file descriptors (clients)
PAO* pao = nullptr;   // Pointer to the PAO object managing tasks
mutex mtx;            // Mutex for global synchronization
map<int, pair<Graph*, Triple*>> clients_graphs;  // Maps client file descriptors to their graphs and associated Triple objects
map<int, mutex> clients_mutex;  // Maps client file descriptors to their corresponding mutex for thread safety
struct pollfd* pfds;  // Set of poll file descriptors, dynamically managed during client connections


/**
 * Signal handler for SIGINT (Ctrl+C).
 * Cleans up resources and safely shuts down the server by releasing allocated memory and closing client connections.
 */
void handle_signal(int sig) {
    for (auto& mtx : clients_mutex) {
        mtx.second.lock();
    }
    // Clean up graphs and triples
    for (auto& graph_triple : clients_graphs) {
        if (graph_triple.second.first != nullptr) {  // Free the graph
            delete graph_triple.second.first;}
        if (graph_triple.second.second != nullptr) {  // Free the triple
            if (graph_triple.second.second->g != nullptr) {  // Free the graph in the triple
                delete graph_triple.second.second->g;
                graph_triple.second.second->g = nullptr;}
            delete graph_triple.second.second;  // Free the triple itself
            graph_triple.second.second = nullptr;
        }
    }
    // Clean up clients
    for (int i = 0; i < fd_count; i++) {
        if (pfds[i].fd != -1) {
            close(pfds[i].fd);
        }
    }
    free(pfds);
    if (pao != nullptr) {
        delete pao;  // Delete the PAO object
    }
    for (auto& mtx : clients_mutex) {
        mtx.second.unlock();
    }
    exit(0);
}

/**
 * Main function of the server.
 * Sets up the listener socket, manages client connections, and handles incoming messages to perform graph-related actions.
 */
int main(void) {
    // Create a list of functions to be executed by the PAO
    std::vector<std::function<void(void*)>> functions = {
        [](void* triple) { 
            Triple* t = (Triple*)triple;  // Cast the void* to Triple*
            unique_lock<mutex> lock(clients_mutex[t->client_fd]);  // Lock the mutex
            t->msg += "Total weight of edges: " + std::to_string((t->g)->totalWeight()) + "\n";
        },
        [](void* triple) {
            Triple* t = (Triple*)triple;
            unique_lock<mutex> lock(clients_mutex[t->client_fd]);  
            t->msg += (t->g)->longestPath() + "\n";
        },
        [](void* triple) {
            Triple* t = (Triple*)triple;
            unique_lock<mutex> lock(clients_mutex[t->client_fd]);  
            t->msg += "The average distance between vertices is: " + std::to_string((t->g)->avgDistance()) + "\n";
        },
        [](void* triple) {
            Triple* t = (Triple*)triple;
            unique_lock<mutex> lock(clients_mutex[t->client_fd]);  
            t->msg += "The shortest paths are: \n" + (t->g)->allShortestPaths() + "\n"; 
        },
        [](void* triple) {
            Triple* t = (Triple*)triple;
            unique_lock<mutex> lock(clients_mutex[t->client_fd]);  
            if (send(t->client_fd, t->msg.c_str(), t->msg.size(), 0) < 0)  // Send the message to the client
                perror("send");
        }
    };

    pao = new PAO(functions);  // Create a new PAO object with the functions
    pao->start();  // Start the PAO object
    const vector<string> graphActions = {"newgraph", "newedge", "removeedge", "mst"};
    const vector<string> mstStrats = {"prim", "kruskal"};
    string action = "";
    string current_act = "";
    int m = 0, n = 0, weight = 0;  // n := number of vertices, m := number of edges, weight := weight of the edge
    string strat = "";  // Strategy for the MST
    char Msg[SIZE] = "Welcome to the PAO-server!\n";
    int new_fd;                          // Newly accepted socket descriptor
    struct sockaddr_storage remote_address; // Client address
    socklen_t addr_len;
    char buf[256] = {0}; // Buffer for client data
    char remoteIP[INET6_ADDRSTRLEN] = {0};
    // Start off with room for 5 connections
    int fd_size = 5;

    // Set up and get a listening socket
    int listener = getListenerSocket();
    if (listener == -1) {
        std::cerr << "Error getting listening socket on port " << PORT 
                  << ": " << strerror(errno) << "\n";
        exit(1);
    }

    // Add the listener to the set
    pfds = (struct pollfd *)malloc(sizeof *pfds * (size_t)fd_size);
    memset(pfds, 0, sizeof *pfds * (size_t)fd_size);
    pfds[0].fd = listener;
    pfds[0].events = POLLIN; // Report ready to read on incoming connection
    fd_count = 1; // For the listener
    cout << "Waiting for connections..." << endl;

    signal(SIGINT, handle_signal);  // Handle the CTRL+C signal

    // Main loop
    while (true) {
        int poll_count = poll(pfds, (size_t)fd_count, -1);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }
        // Run through the existing connections looking for data to read
        for (int i = 0; i < fd_count; i++) {
            // Check if someone's ready to read
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == listener) { // If listener is ready to read, handle new connection
                    addr_len = sizeof remote_address;
                    new_fd = accept(listener, (struct sockaddr *)&remote_address, &addr_len);
                    if (new_fd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, new_fd, &fd_count, &fd_size);
                        // Add the new client to the dictionary:
                        clients_graphs[new_fd] = {nullptr, nullptr};  // Initialize both pointers to nullptr
                        printf("pollserver: new connection from %s on socket %d\n",
                               inet_ntop(remote_address.ss_family,
                                         getInAddr((struct sockaddr *)&remote_address),
                                         remoteIP, INET6_ADDRSTRLEN),
                               new_fd);
                        if (send(new_fd, Msg, sizeof(Msg), 0) < 0) {
                            perror("send");
                        }
                    }
                } else { // Handle existing connection 
                    int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0); // Receiving the msg from the client
                    int sender_fd = pfds[i].fd;
                    if (nbytes <= 0) { // Got error or connection closed by client
                        if (nbytes == 0)
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        else
                            perror("ERROR: receiving data from client");
                        close(pfds[i].fd);  // Close the connection 
                        del_from_pfds(pfds, i, &fd_count);  // Remove the connection from the set of connections
                        {
                            unique_lock<mutex> lock(clients_mutex[sender_fd]);
                            if (clients_graphs[sender_fd].first != nullptr) {  // If the client has a graph, delete it
                                delete clients_graphs[sender_fd].first;
                                clients_graphs[sender_fd].first = nullptr;
                            }
                            if (clients_graphs[sender_fd].second != nullptr) {  // If the client has a triple, delete it
                                if (clients_graphs[sender_fd].second->g != nullptr) {  // If the triple has a graph, delete it
                                    delete clients_graphs[sender_fd].second->g;
                                    clients_graphs[sender_fd].second->g = nullptr;
                                }
                                delete clients_graphs[sender_fd].second;  // Delete the triple
                                clients_graphs[sender_fd].second = nullptr;
                            }
                            clients_graphs.erase(sender_fd);  // Remove the client from the dictionary
                        }
                    } else {  // The client sent a message
                        parseInput(buf, nbytes, n, m, weight, strat, action, current_act, graphActions, mstStrats);
                        cout << "Action received: " << action << " from client " << sender_fd << endl;
                        // Handling the input:
                        pair<string, Graph*> result = handleInput(clients_graphs[sender_fd].first, action, sender_fd, current_act, n, m, weight, strat);
                        if (result.second != nullptr) {  // If the result is not null, store it in the dictionary for this client
                            clients_graphs[sender_fd].first = result.second;
                        }
                        // Print the message to the server
                        if (current_act == "message") {
                            cout << result.first << endl;
                            continue;
                        }
                        // If the current_act is in the graphActions, then send the result to all the clients
                        if (find(graphActions.begin(), graphActions.end(), current_act) != graphActions.end()) {
                            for (int j = 0; j < fd_count; j++) {
                                int dest_fd = pfds[j].fd;
                                if (dest_fd != listener) {  // If the destination is not the listener
                                    if (send(dest_fd, result.first.c_str(), result.first.size() + 1, 0) < 0) {  // Send the result to the client
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                }
            } 
        } 
    } 
    return 0;
}


/////////////////////////////// more - fuction ///////////////////////////////////////////////////

/**
 * Handles an MST request from a client.
 * This function creates a new Triple object for each request, assigns the task to the PAO, and processes the MST calculation.
 *
 * @param g The graph object pointer
 * @param client_fd The file descriptor for the client
 * @param strat The MST algorithm strategy ("prim", "kruskal", etc.)
 * @return A pair consisting of the result message and the MST graph pointer
 */
std::pair<std::string, Graph*> MST(Graph* g, int client_fd, const std::string& strat) {
    Graph* graph_temp = nullptr;
    Triple* trip = nullptr; {
        unique_lock<mutex> lock(clients_mutex[client_fd]);  // Lock the mutex for thread safety
        // Clean up any existing Triple or graph for the client
        if (clients_graphs[client_fd].second != nullptr) {  
            if (clients_graphs[client_fd].second->g != nullptr) {
                delete clients_graphs[client_fd].second->g;  // Delete the old graph
                clients_graphs[client_fd].second->g = nullptr;  }
            delete clients_graphs[client_fd].second;  // Delete the old Triple
        }
        // Create a new Triple object and store it in the client's record
        clients_graphs[client_fd].second = new Triple{g, strat, client_fd};  
        trip = clients_graphs[client_fd].second;  // Assign the new Triple
        // Select the MST algorithm strategy
        MST_Strategy* MST_algo = MST_Factory::getInstance()->createMST(trip->msg);  
        graph_temp = (*MST_algo)(trip->g);  // Generate the MST based on the selected strategy
    }
    // Update the Triple with the new MST graph and a success message
    trip->g = graph_temp;  
    trip->msg = "MST created using " + trip->msg + " strategy\n";
    // Add the task to the PAO for further processing
    pao->addTask(clients_graphs[client_fd].second);  
    std::cout << "User " << client_fd << " requested to find MST of the Graph" << std::endl;
    return {"", nullptr};  // Return empty result since the processing will be done asynchronously
}