/*
** pollserver.c -- a cheezy multiperson chat server
*/

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
#include "LFP/LeaderFollower.hpp"
#include "ServerUtils/serverUtils.hpp"

// to handle the CTRL+C signal
#include <signal.h>
#include <atomic>

#define NUM_THREADS 4 // Number of threads in LFP
#define PORT "9036"   // Port we're listening on
#define WELCOME_MSG_SIZE 510

using namespace std;

// global variable:
LFP lfp(NUM_THREADS);             // Create an instance of LFP
map<int, Graph *> clients_graphs; // dictionary to store the client file descriptor and its graph
struct pollfd *pfds;              // set of file descriptors (global to maintain correct memory management when interrupting the server)
int fd_count = 0;

pair<string, Graph *> MST(Graph *g, int clientFd, const string &strat) // many to do here
{
    
    // Perform the operation
    Graph *mst = (*MST_Factory::getInstance()->createMST(strat))(g); // the strategy will create a new graph and return a pointer to it
    // implementing Leader-Follower with global variable "lfp":
    lfp.addTask([clientFd, strat, mst]()
                {
                    // sleep(7);
                    string msg = "Client " + to_string(clientFd) + " requested to find MST of the Graph" + "\n";
                    msg += "MST Strategy: " + strat + "\n";
                    msg += "MSTs' stats: \n" + mst->stats();
                    send(clientFd, msg.c_str(), msg.size(), 0);
                    //cout << "User " << clientFd << "succesfuly finished finding MST of the Graph" << endl;
                    delete mst; // deleting the mst graph
                });
    return {"", nullptr};
}

/**
 * Handle the signal, actually stopping the server's while loop
 */
void handleSig(int sig)
{
    cout << "\nLF-server: cleaning up resources..." << endl;

    // graphs:
    for (auto &graph : clients_graphs)
    {
        if (graph.second != nullptr)
        {
            delete graph.second;
            graph.second = nullptr;
        }
    }

    cout << "LF-server: Graphs freed," << endl;

    // Clients: cleans the pfds
    for (int i = 0; i < fd_count; i++)
    {
        if (pfds[i].fd != -1)
        {
            close(pfds[i].fd);
        }
    }
    free(pfds);
    cout << "LF-server: Clients freed,\n"
         << "Good Bye!" << endl;
    exit(0);
}

// Main
int main(void)
{
    lfp.start(); // Start the threads in LFP
    const vector<string> graphActions = {"newgraph", "newedge", "removeedge", "mst"};
    const vector<string> mstStrats = {"prim", "kruskal", "tarjan", "boruvka"};

    string action= "";
    string actualAction = "";
    int n = 0; 
    int m = 0;
    int weight = 0;
    string strat = "";
    map<int, bool> connections_in_use;
    char welcomeMsg[WELCOME_MSG_SIZE] =
        "Welcome to the LF-server!\n"
        "This server can perform the following actions:\n"
        "1. Create a new graph: newgraph n m where \"n\" is the number of vertices and \"m\" is the number of edges.\n"
        "2. Add an edge to the graph: newedge n m w where \"n\" and \"m\" are the vertices and \"w\" is the weight of the edge.\n"
        "3. Remove an edge from the graph: removeedge n m where \"n\" and \"m\" are the vertices.\n"
        "4. Find the Minimum Spanning Tree of the graph: mst strat -  where strat is either 'prim', 'kruskal', 'tarjan' or 'boruvka'\n";

    int newfd;                          // Newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;
    char buf[256]={0}; // Buffer for client data
    char remoteIP[INET6_ADDRSTRLEN]={0};

    // Start off with room for 5 connections (isnt it four? because one is the listener..)
    // (We'll realloc as necessary)
    fd_count = 0;
    int fd_size = 5;
    pfds = (struct pollfd *)malloc(sizeof *pfds * (size_t)fd_size);
    memset(pfds, 0, sizeof *pfds * (size_t)fd_size);

    // Set up and get a listening socket
    int listener = getListenerSocket(); // Listening socket descriptor
    if (listener == -1)
    {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    cout << "LF- waiting for connections..." << endl;
    // Add the listener to set
    pfds[0].fd = listener;
    pfds[0].events = POLLIN; // Report ready to read on incoming connection
    fd_count = 1;            // For the listener

    signal(SIGINT, handleSig); // handle the CTRL+C signal

    // Main loop
    while (true)
    {
        int poll_count = poll(pfds, (size_t)fd_count, -1);

        if (poll_count == -1)
        {

            perror("poll");
            exit(1);
        }

        // Run through the existing connections looking for data to read
        for (int i = 0; i < fd_count; i++)
        {

            // Check if someone's ready to read
            if (pfds[i].revents & POLLIN)
            { // We got one!!

                if (pfds[i].fd == listener)
                { // If listener is ready to read, handle new connection

                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);

                        // Add the new client to the dictionary:
                        clients_graphs[newfd] = nullptr;

                        printf("LF-server new connection from %s on socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         getInAddr((struct sockaddr *)&remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                        if (send(newfd, welcomeMsg, sizeof(welcomeMsg), 0) < 0)
                        {
                            perror("send");
                        }
                    }
                }
                else
                {                                                      // handle existing connection
                    int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0); // receiving the msg from the client
                    int sender_fd = pfds[i].fd;

                    if (nbytes <= 0)
                    { // Got error or connection closed by client
                        if (nbytes == 0)
                            printf("LF-server: socket %d hung up\n", sender_fd);
                        else
                            perror("ERROR: receiving data from client");

                        close(pfds[i].fd);                 // close the connection
                        del_from_pfds(pfds, i, &fd_count); // remove the connection from the set of connections
                        if (clients_graphs[sender_fd] != nullptr)
                        { // if the client has a graph, delete it
                            delete clients_graphs[sender_fd];
                            clients_graphs[sender_fd] = nullptr;
                        }
                        clients_graphs.erase(sender_fd); // remove the client from the dictionary
                    }
                    else
                    { // the client sent a message
                        parseInput(buf, nbytes, n, m, weight, strat, action, actualAction, graphActions, mstStrats);
                        cout << "Action received: " << action << " from client " << sender_fd << endl;

                        // handling the input:
                        pair<string, Graph *> result = handleInput(clients_graphs[sender_fd], action, sender_fd, actualAction, n, m, weight, strat);
                        // string msg = result.first;
                        if (result.second != nullptr)
                        { // if the result is not null, store it in the dictionary for this client
                            clients_graphs[sender_fd] = result.second;
                        }

                        // print the message to the server
                        if (actualAction == "message")
                        {
                            cout << result.first << endl;
                            continue;
                        }

                        // if the actualAction is in the graphActions, then send the result to all the clients
                        if (find(graphActions.begin(), graphActions.end(), actualAction) != graphActions.end())
                        {
                            for (int j = 0; j < fd_count; j++)
                            {
                                int dest_fd = pfds[j].fd;
                                if (dest_fd != listener)                                                     // if the destination is not the listener
                                    if (send(dest_fd, result.first.c_str(), result.first.size() + 1, 0) < 0) // send the result to the client include the null terminator
                                        perror("send");
                            }
                        }
                    }
                }
            } // END handle data from client
        } // END got ready-to-read from poll()
    } // END looping through file descriptors
    // END for(;;)--and you thought it would never end!

    return 0;
}
