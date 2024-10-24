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
#include "Graph/graph.hpp"
#include "MST/MST_Strategy.hpp"
#include "MST/MST_Factory.hpp"
#include "LF/LeaderFollower.hpp"
#include "ServerUtils/serverUtils.hpp"
#include <signal.h>
#include <atomic>
#define PORT "8080"   
#define SIZE 40

using namespace std;

// global variable:
LFP lf(4);             // Create an instance of LF
map<int, Graph *> clients_graphs; // dictionary to store the client file descriptor and its graph
struct pollfd *pfds;              // set of file descriptors (global to maintain correct memory management when interrupting the server)
int fd_count = 0;

//Signal handler to clean up resources when the server is stopped
void handle_signal(int sig) {
    // Free allocated graphs for each client
    for (auto &graph : clients_graphs) {
        if (graph.second != nullptr) {
            delete graph.second; // Delete graph if it exists
            graph.second = nullptr;
        } }
    // Close all client connections
    for (int i = 0; i < fd_count; i++) {
        if (pfds[i].fd != -1) {
            close(pfds[i].fd); // Close the file descriptor
        } }
    free(pfds); // Free the pollfd array
    cout << "LF: exit" << endl;
    exit(0); // Exit the server
}

int main(void) {
    lf.start(); // Start the Leader-Follower threads
    const vector<string> commands_graph = {"newgraph", "newedge", "removeedge", "mst"}; // Supported graph commands
    const vector<string> mstStrats = {"prim", "kruskal"}; // Supported MST strategies

    // Variable declarations for input handling
    string action = "";
    string current_act = "";
    int n = 0; 
    int m = 0;
    int weight = 0;
    string strat = "";
    int new_fd; // Newly accepted socket descriptor
    struct sockaddr_storage remote_address; // Client address structure
    socklen_t addr_len; // Length of client address
    char buf[256] = {0}; // Buffer for client data
    char remoteIP[INET6_ADDRSTRLEN] = {0}; // Buffer for remote IP address
    // Initialize polling file descriptor array
    fd_count = 0; // Reset active file descriptor count
    int fd_size = 5; // Initial size for file descriptor array
    pfds = (struct pollfd *)malloc(sizeof *pfds * (size_t)fd_size); // Allocate memory for pollfd array
    memset(pfds, 0, sizeof *pfds * (size_t)fd_size); // Zero-initialize the array
    char start_messege[SIZE] = "Start LF-server!\n";

    // Set up and get a listening socket
    int listener = getListenerSocket(); // Obtain the listening socket descriptor
    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    cout << "LF: Try to connect..." << endl;
    // Add the listener to the pollfd array
    pfds[0].fd = listener; // Assign listener to the first position
    pfds[0].events = POLLIN; // Monitor for incoming connections
    fd_count = 1; // Count of active file descriptors (starting with listener)

    signal(SIGINT, handle_signal); // Set signal handler for CTRL+C

    // Main loop for handling client connections
    while (true) {
        int poll_count = poll(pfds, (size_t)fd_count, -1); // Wait for an event on any file descriptor
        if (poll_count == -1) {
            perror("poll");
            exit(1); // Exit if an error occurs
        }
        // Iterate through active connections to check for data
        for (int i = 0; i < fd_count; i++) {
            // Check if a file descriptor is ready for reading
            if (pfds[i].revents & POLLIN) { // Data is ready to read
                if (pfds[i].fd == listener) {
                    // Handle new incoming connection
                    addr_len = sizeof remote_address; // Get size of the address
                    new_fd = accept(listener, (struct sockaddr *)&remote_address, &addr_len); // Accept the connection
                    if (new_fd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, new_fd, &fd_count, &fd_size); // Add new client to the pollfd array
                        // Initialize the client's graph to null
                        clients_graphs[new_fd] = nullptr; 
                        printf("LF: New connection\n");
                        if (send(new_fd, start_messege, sizeof(start_messege), 0) < 0) {
                            perror("send"); // Send welcome message to the new client
                        }
                    }
                } else {
                    // Handle data from an existing connection
                    int num_of_bytes = recv(pfds[i].fd, buf, sizeof buf, 0); // Receive data from client
                    int sender_fd = pfds[i].fd; // Store sender's file descriptor
                    if (num_of_bytes <= 0) {
                        // Handle error or closed connection
                        if (num_of_bytes == 0)
                            printf("LF: Client disconnected, socket %d\n", sender_fd); 
                        else
                            perror("ERROR: receiving");

                        close(pfds[i].fd); // Close the connection
                        del_from_pfds(pfds, i, &fd_count); // Remove the file descriptor from the pollfd array
                        if (clients_graphs[sender_fd] != nullptr) {
                            // Delete the client's graph if it exists
                            delete clients_graphs[sender_fd];
                            clients_graphs[sender_fd] = nullptr; // Set to nullptr
                        }
                        clients_graphs.erase(sender_fd); // Remove client from the dictionary
                    } else {
                        // Process received message from the client
                        parseInput(buf, num_of_bytes, n, m, weight, strat, action, current_act, commands_graph, mstStrats);
                        cout << "Act received: " << action << " from client: " << sender_fd << endl;

                        // Handle input and perform appropriate actions
                        pair<string, Graph *> result = handleInput(clients_graphs[sender_fd], action, sender_fd, current_act, n, m, weight, strat);
                        // If a new graph was created, store it in the clients_graphs map
                        if (result.second != nullptr) {
                            clients_graphs[sender_fd] = result.second;
                        }
                        // Print the message to the server
                        if (current_act == "message") {
                            cout << result.first << endl; // Log the message
                            continue; // Skip sending to other clients
                        }
                        // If the action is a graph command, broadcast the result to all clients
                        if (find(commands_graph.begin(), commands_graph.end(), current_act) != commands_graph.end()) {
                            for (int j = 0; j < fd_count; j++) {
                                int dest_fd = pfds[j].fd; // Destination file descriptor
                                if (dest_fd != listener) // Ensure not to send to the listener
                                    if (send(dest_fd, result.first.c_str(), result.first.size() + 1, 0) < 0) // Send message to clients
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


/////////////////////////// More - Functions ///////////////////////

// Function to handle Minimum Spanning Tree (MST) requests
pair<string, Graph *> MST(Graph *g, int client_fd, const string &strat) {
    // Create the MST based on the provided strategy
    Graph *mst = (*MST_Factory::getInstance()->createMST(strat))(g);
    
    // Add a task to the Leader-Follower instance for handling the MST response
    lf.addTask([client_fd, strat, mst]() {
        string msg = "Client request the MST\n";
        msg += "MST statistics: \n" + mst->stats(); // Get statistics of the MST
        send(client_fd, msg.c_str(), msg.size(), 0); // Send the response to the client
        delete mst; // Free the MST graph after use
    });
    return {"", nullptr}; // No message needed for the main loop
}