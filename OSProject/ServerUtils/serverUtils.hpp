// #pragma once
// #include <iostream>
// #include <queue>
// #include <mutex>
// #include <thread>
// #include <vector>
// #include <string>
// #include <utility>
// #include <unordered_set>
// #include <shared_mutex>
// #include "../GraphObj/graph.hpp"
// #include <sys/socket.h>
// #include <unistd.h>
// #include <sstream>
// #include <netinet/in.h> // Include this header for sockaddr_in
// #include <netdb.h>      // Include this header for addrinfo
// #include <poll.h>       // Include this header for pollfd
// #include "../MST/MST_Factory.hpp"
// #include <string.h>
// #define PORT "9036" // Port we're listening on
// #include "../LFP/LFP.hpp"


// extern std::pair<std::string, Graph *> MST(Graph *g, int clientFd, const std::string &strat);

// // Function to convert a string to lowercase
// std::string toLowerCase(std::string s);


// void initGraph(Graph *g, int m, int clientFd);

// std::vector<std::string> splitStringBySpaces(const std::string &input);

// void parseInput(char *buf, int nbytes, int &n, int &m, int &weight, std::string &strat, std::string &action, std::string &actualAction, const std::vector<std::string> &graphActions, const std::vector<std::string> &mstStrats);


// std::unordered_set<Vertex> initVertices(int n);

// std::pair<std::string, Graph *> newGraph(int n, int m, int clientFd, Graph *g);

// std::pair<std::string, Graph *> newEdge(size_t n, size_t m, size_t weight, int clientFd, Graph *g);

// std::pair<std::string, Graph *> removeedge(int n, int m, int clientFd, Graph *g);


// std::pair<std::string, Graph *> handleInput(Graph *g, std::string action, int clientFd, std::string actualAction, int n, int m, int w, std::string strat);


// // Get sockaddr, IPv4 or IPv6:
// void *getInAddr(struct sockaddr *sa);


// // Return a listening socket
// int getListenerSocket();


// // Add a new file descriptor to the set
// void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size);


// // Remove an index from the set
// void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);


#ifndef SERVER_UTILS_HPP
#define SERVER_UTILS_HPP

#include <utility>
#include <unordered_set>
#include <shared_mutex>
#include "../GraphObj/graph.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <netinet/in.h> // Include this header for sockaddr_in
#include <netdb.h>      // Include this header for addrinfo
#include <poll.h>       // Include this header for pollfd
#include "../MST/MST_Factory.hpp"
#include <string.h>
#define PORT "9036" // Port we're listening on
#include "../LFP/LFP.hpp"

// Declare the MST function as extern
extern std::pair<std::string, Graph *> MST(Graph *g, int clientFd, const std::string &strat);

// Function to convert a string to lowercase
std::string toLowerCase(std::string s);

void initGraph(Graph *g, int m, int clientFd);

std::vector<std::string> splitStringBySpaces(const std::string &input);

void parseInput(char *buf, int nbytes, int &n, int &m, int &weight, std::string &strat, std::string &action, std::string &actualAction, const std::vector<std::string> &graphActions, const std::vector<std::string> &mstStrats);

std::unordered_set<Vertex> initVertices(int n);

std::pair<std::string, Graph *> newGraph(int n, int m, int clientFd, Graph *g);

std::pair<std::string, Graph *> newEdge(size_t n, size_t m, size_t weight, int clientFd, Graph *g);

std::pair<std::string, Graph *> removeedge(int n, int m, int clientFd, Graph *g);

std::pair<std::string, Graph *> handleInput(Graph *g, std::string action, int clientFd, std::string actualAction, int n, int m, int w, std::string strat);


// Get sockaddr, IPv4 or IPv6:
void *getInAddr(struct sockaddr *sa);


// Return a listening socket
int getListenerSocket();


// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size);


// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

#endif // SERVER_UTILS_HPP