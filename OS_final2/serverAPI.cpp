#include <utility>
#include <unordered_set>
#include <shared_mutex>
#include "graph.cpp"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <netinet/in.h> // Include this header for sockaddr_in
#include <netdb.h>      // Include this header for addrinfo
#include <poll.h>       // Include this header for pollfd
#include "MST_Factory.cpp"
#include <string.h>
#define PORT "8080" // Port we're listening on
#include "LeaderFollower.hpp"

extern LF lf; // Leader-Follower pattern instance

// Initialize the graph by reading edges from the client
void initGraph(Graph *g, int m, int clientFd){
    std::string msg = "enter edge: u v w \n";
    if (send(clientFd, msg.c_str(), msg.size(), 0) < 0){
        perror("send"); // Handle send error
    }
    int stdin_save = dup(STDIN_FILENO); // Save the current state of STDIN
    dup2(clientFd, STDIN_FILENO);       // Redirect STDIN to the socket
    for (int i = 0; i < m; i++){
        // Read the edges
        size_t u, v, weight;
        std::cin >> u >> v >> weight; // Read edge input
        Edge e = Edge(g->getVertex(u - 1), g->getVertex(v - 1), weight);
        g->addEdge(e); // Add edge from u to v
    }
    dup2(stdin_save, STDIN_FILENO); // Restore the original STDIN
}

// Split a string into tokens by spaces
std::vector<std::string> split_spaces(const std::string &input){
    std::istringstream stream(input);
    std::vector<std::string> res;
    std::string temp;
    while (stream >> temp){
        result.push_back(temp);} // Push each token to result
    return res;
}

// Parse input from the client
void parseInput(char *buf, int numOfBytes, int &n, int &m, int &weight, std::string &strat, std::string &act, std::string &currentAct, const std::vector<std::string> &graphacts, const std::vector<std::string> &mstStrats){
    buf[numOfBytes] = '\0'; // Null-terminate the buffer
    act = toLowerCase(std::string(buf)); // Convert input to lowercase
    std::vector<std::string> tokens = split_spaces(act); // Split input into tokens
    if (tokens.size() > 0){
        currentAct = tokens[0]; // Get the first token as the act
        cout << "act received: " << currentAct << " tokens size: " << tokens.size() << endl;
    }
    else{
        currentAct = "There is no message"; // Handle empty input
    }

    // Check if the act is a valid graph act
    if (find(graphacts.begin(), graphacts.end(), currentAct) == graphacts.end()){
        currentAct = "message"; // Invalid act
    }
    else if (currentAct == "mst"){ // Check for MST act
        if (tokens.size() > 1){
            if (find(mstStrats.begin(), mstStrats.end(), tokens[1]) == mstStrats.end()){
                currentAct = "message"; // Invalid strategy
            }
            else{
                currentAct = "mst"; // Valid MST act
                act = tokens[0];
                n = -1;
                m = -1;
                weight = -1;
                strat = tokens[1]; // Set strategy
            }
        }
        else {
            currentAct = "message"; // No strategy provided
        }
    }
    else if (!isNumber(tokens)){ // Check if tokens are numbers
        currentAct = "message"; // Not valid numbers
        cout << "Not a number" << endl;
    }
    else if (currentAct == "newgraph"){ // Handle new graph creation
        if (tokens.size() != 3){ // Check token count
            currentAct = "message"; // Invalid command
        }
        else{
            n = stoi(tokens[1]); // Get number of vertices
            m = stoi(tokens[2]); // Get number of edges
            weight = -1; 
        }
    }
    else if (currentAct == "newedge"){ // Handle new edge creation
        if (tokens.size() != 4){ // Check token count
            currentAct = "message"; // Invalid command
        }
        else{
            n = stoi(tokens[1]); // Get starting vertex
            m = stoi(tokens[2]); // Get ending vertex
            weight = stoi(tokens[3]); // Get edge weight
        }
    }
    else if (currentAct == "removeedge"){ // Handle edge removal
        if (tokens.size() != 3){
            currentAct = "message"; // Invalid command
        }
        else{
            n = stoi(tokens[1]); // Get starting vertex
            m = stoi(tokens[2]); // Get ending vertex
            weight = -1; // Weight not needed for removal
        }
    }
}

// Initialize vertices for the graph
std::unordered_set<Vertex> initVertices(int n){
    std::unordered_set<Vertex> vertices;
    for (size_t i = 0; i < n; i++){
        vertices.insert(Vertex(i)); } // Insert vertices into the set
    return vertices;
}

// Create a new graph with n vertices and m edges
std::pair<std::string, Graph *> newGraph(int n, int m, int clientFd, Graph *g){
    std::cout << "Creating a new graph with " << n << " vertices and " << m << " edges" << std::endl;

    if (g != nullptr)
        delete g; // Delete the existing graph if not null
    std::unordered_set<Vertex> vertices = initVertices(n); // Initialize the vertices

    g = new Graph(vertices);   // Create a new graph of n vertices
    initGraph(g, m, clientFd); // Initialize the graph with m edges

    std::string msg = "Client " + std::to_string(clientFd) + " successfully created a new Graph with " + std::to_string(n) + " vertices and " + std::to_string(m) + " edges" + "\n";
    std::cout << "Graph created successfully\n";
    return {msg, g}; // Return success message and the new graph
}

// Add a new edge to the existing graph
std::pair<std::string, Graph *> newEdge(size_t n, size_t m, size_t weight, int clientFd, Graph *g){
    std::cout << "Adding an edge from " << n << " to " << m << std::endl;
    g->addEdge(Edge(g->getVertex(n - 1), g->getVertex(m - 1), weight)); // Add edge from u to v
    std::string msg = "Client " + std::to_string(clientFd) + " added an edge from " + std::to_string(n) + " to " + std::to_string(m) + " with weight " + std::to_string(weight) + "\n";

    return {msg, g}; // Return success message and the updated graph
}

// Remove an edge from the existing graph
std::pair<std::string, Graph *> removeedge(int n, int m, int clientFd, Graph *g){
    std::cout << "Removing an edge from " << n << " to " << m << std::endl;
    g->removeEdge(Edge{g->getVertex(n - 1), g->getVertex(m - 1)}); // Remove edge from u to v
    std::string msg = "Client " + std::to_string(clientFd) + " removed an edge from " + std::to_string(n) + " to " + std::to_string(m) + "\n";

    return {msg, g}; // Return success message and the updated graph
}

// Handle client input and perform acts based on the received command
std::pair<std::string, Graph *> handleInput(Graph *g, std::string act, int clientFd, std::string currentAct, int n, int m, int w, std::string strat){
    std::string msg;
    std::vector<std::string> tokens = split_spaces(act);
    if (tokens.size() < 1){
        msg = "User " + std::to_string(clientFd) + " sent an empty message\n";
        return {msg, nullptr}; // Handle empty message
    }

    if (currentAct == "newgraph"){ 
        // format: newgraph n m
        return newGraph(n, m, clientFd, g); // Create a new graph
    }
    else if (currentAct == "newedge"){
         // format: newedge n m (add an edge from n to m)
        if (g != nullptr){
            return newEdge(static_cast<size_t>(n), static_cast<size_t>(m), static_cast<size_t>(w), clientFd, g); // Add an edge
        }
        else{
            msg = "Client " + std::to_string(clientFd) + " tried to perform the operation but there is no graph\n";
            return {msg, nullptr}; // Handle case where graph doesn't exist
        }
    }
    else if (currentAct == "removeedge"){
         // format: removeedge n m (remove an edge from n to m)
        if (g != nullptr){
            return removeedge(n, m, clientFd, g); // Remove an edge
        }
        else{
            msg = "Client " + std::to_string(clientFd) + " tried to perform the operation but there is no graph\n";
            return {msg, nullptr}; // Handle case where graph doesn't exist
        }
    }
    else if (currentAct == "mst"){ 
        // format: MST
        if (g == nullptr){
            msg = "Client " + std::to_string(clientFd) + " tried to perform the operation but there is no graph\n";
            return {msg, nullptr}; // Handle case where graph doesn't exist
        }
        else if (!g->isConnected()){
            msg = "Client " + std::to_string(clientFd) + " tried to perform the operation but the graph is not connected therefore it doesn't have a MST\n";
            return {msg, nullptr}; // Handle case where graph is not connected
        }
        else{
            return MST(g, clientFd, strat); // Compute MST
        }
    }
    else{
        msg = "Client " + std::to_string(clientFd) + " sent a message: " + act; // Log the message
        return {msg, nullptr}; // Return the message without act
    }
}

// Get sockaddr, IPv4 or IPv6
void *getInAddr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr); // Handle IPv4
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr); // Handle IPv6
}

// Return a listening socket
int getListenerSocket(void){
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;
    struct addrinfo hints, *ai, *p;
    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE; // Use my IP

    // Get address info
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0){
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1); // Handle error in getting address info
    }

    // Loop through all the results and bind to the first we can
    for (p = ai; p != NULL; p = p->ai_next){
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol); // Create socket
        if (listener < 0){
            continue; // Skip on error
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0){
            close(listener); // Close on bind error
            continue; // Try next address
        }
        break; // Successfully bound
    }

    freeaddrinfo(ai); // Free address info

    // If we got here, it means we didn't get bound
    if (p == NULL){
        return -1; // Bind failure
    }

    // Listen for incoming connections
    if (listen(listener, 10) == -1){
        return -1; // Listen failure
    }
    return listener; // Return the listener socket
}

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size){
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size){
        *fd_size *= 2; // Double the size of the array

        *pfds = (struct pollfd *)realloc(*pfds, sizeof(**pfds) * (size_t)(*fd_size)); // Reallocate memory
    }
    (*pfds)[*fd_count].fd = newfd; // Set new file descriptor
    (*pfds)[*fd_count].events = POLLIN; // Check for ready-to-read

    (*fd_count)++; // Increment count of file descriptors
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count){
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count - 1]; // Replace removed index with last one

    (*fd_count)--; // Decrement the count of file descriptors
}


// Function to convert a string to lowercase
std::string toLowerCase(std::string s){
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Check if the provided strings represent numbers
bool isNumber(const std::vector<std::string> &s){
    for (size_t i = 1; i < s.size(); i++){
        std::string str = s[i];
        if (str.empty()){
            return false; // Check for empty strings
        }
        if (!std::all_of(str.begin(), str.end(), ::isdigit)){
            return false; // Check if all characters are digits
        }
    }
    return true; // All strings are valid numbers
}
