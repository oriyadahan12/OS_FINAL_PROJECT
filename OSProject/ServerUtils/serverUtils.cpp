#include "serverUtils.hpp"

extern LFP lfp; // Leader-Follower pattern instance

// Handle client input and perform acts based on the received command
std::pair<std::string, Graph *> handleInput(Graph *g, std::string act, int fd_client, std::string current_act, int n, int m, int w, std::string algo){
    std::string msg;
    std::vector<std::string> command = split_spaces(act);
    if (command.size() < 1){
        msg = "Empty message\n";
        return {msg, nullptr}; // Handle empty message
    }

    if (current_act == "newgraph"){ 
        return newGraph(n, m, fd_client, g); // Create a new graph
    }
    else if (current_act == "newedge"){
        if (g != nullptr){
            return newEdge(static_cast<size_t>(n), static_cast<size_t>(m), static_cast<size_t>(w), fd_client, g); // Add an edge
        }
        else{ msg = "There is no graph\n";
              return {msg, nullptr}; // Handle case where graph doesn't exist
        }
    }
    else if (current_act == "removeedge"){
        if (g != nullptr){
            return removeedge(n, m, fd_client, g); // Remove an edge
        }
        else{ msg = "There is no graph\n";
              return {msg, nullptr}; // Handle case where graph doesn't exist
        }
    }
    else if (current_act == "mst"){ 
        if (g == nullptr){
            msg = "There is no graph\n";
            return {msg, nullptr}; // Handle case where graph doesn't exist
        }
        else if (!g->isConnected()){
            msg = " The graph is not connected\n";
            return {msg, nullptr}; // Handle case where graph is not connected
        }
        else{ return MST(g, fd_client, algo); // Compute MST
        }
    }
    else{ msg = "Client sent a message: " + act; // Log the message
          return {msg, nullptr}; // Return the message without act
    }
}


// Get sockaddr, IPv4 or IPv6
void *getInAddr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr); } // Handle IPv4
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
    if (p == NULL){
        return -1; // Bind failure
    }
    if (listen(listener, 10) == -1){
        return -1; // Listen failure
    }
    return listener; // Return the listener socket
}

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int new_fd, int *count, int *size){
    // If we don't have room, add more space in the pfds array
    if (*count == *size){
        *size *= 2; // Double the size of the array
        *pfds = (struct pollfd *)realloc(*pfds, sizeof(**pfds) * (size_t)(*size)); // Reallocate memory
    }
    (*pfds)[*count].fd = new_fd; // Set new file descriptor
    (*pfds)[*count].events = POLLIN; // Check for ready-to-read
    *count = (*count)+1; // Increment count of file descriptors
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *count){
    pfds[i] = pfds[*count - 1]; // Replace removed index with last one
    *count = (*count)-1; // Decrement the count of file descriptors
}

//////////////////////////// Graph - function ///////////////////////

// Initialize vertices for the graph
std::unordered_set<Vertex> initVertices(int n){
    std::unordered_set<Vertex> vertices;
    for (size_t i = 0; i < n; i++){
        vertices.insert(Vertex(i)); } // Insert vertices into the set
    return vertices;
}

void initGraph(Graph *g, int m, int clientFd)
{
    std::string msg = "To create an edge u->v with weight w please enter the edge number in the format: u v w \n";
    if (send(clientFd, msg.c_str(), msg.size(), 0) < 0)
    {
        perror("send");
    }
    int stdin_save = dup(STDIN_FILENO); // Save the current state of STDIN
    dup2(clientFd, STDIN_FILENO);       // Redirect STDIN to the socket
    for (int i = 0; i < m; i++)
    { // Read the edges
        size_t u, v, weight;
        std::cin >> u >> v >> weight;
        Edge e = Edge(g->getVertex(u - 1), g->getVertex(v - 1), weight);
        g->addEdge(e); // Add edge from u to v
    }
    dup2(stdin_save, STDIN_FILENO); // Restore the original STDIN
}

// Create a new graph with n vertices and m edges
std::pair<std::string, Graph *> newGraph(int n, int m, int fd_client, Graph *g){
    std::cout << "Creating new graph with " << n << " vertices and " << m << " edges" << std::endl;
    if (g != nullptr)
        delete g; // Delete the existing graph if not null
    std::unordered_set<Vertex> vertices = initVertices(n); // Initialize the vertices
    g = new Graph(vertices);   // Create a new graph of n vertices
    initGraph(g, m, fd_client); // Initialize the graph with m edges
    std::string msg = "Client successfully created a new Graph with " + std::to_string(n) + " vertices and " + std::to_string(m) + " edges" + "\n";
    std::cout << "Graph created successfully\n";
    return {msg, g}; 
}

// Add a new edge to the existing graph
std::pair<std::string, Graph *> newEdge(size_t n, size_t m, size_t weight, int fd_client, Graph *g){
    std::cout << "Adding an edge from " << n << " to " << m << std::endl;
    g->addEdge(Edge(g->getVertex(n - 1), g->getVertex(m - 1), weight)); // Add edge from u to v
    std::string msg = "Client " + std::to_string(fd_client) + " added an edge from " + std::to_string(n) + " to " + std::to_string(m) + " with weight " + std::to_string(weight) + "\n";

    return {msg, g}; // Return success message and the updated graph
}

// Remove an edge from the existing graph
std::pair<std::string, Graph *> removeedge(int n, int m, int fd_client, Graph *g){
    std::cout << "Removing an edge from " << n << " to " << m << std::endl;
    g->removeEdge(Edge{g->getVertex(n - 1), g->getVertex(m - 1)}); // Remove edge from u to v
    std::string msg = "Client " + std::to_string(fd_client) + " removed an edge from " + std::to_string(n) + " to " + std::to_string(m) + "\n";

    return {msg, g}; // Return success message and the updated graph
}

//////////////////////////////// More - Function ////////////////////////////

// Function to convert a string to lowercase
std::string lower_case(std::string s){
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Check if the provided strings represent numbers
bool string_is_num(const std::vector<std::string> &s){
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


// Split a string into command by spaces
std::vector<std::string> split_spaces(const std::string &input){
    std::istringstream stream(input);
    std::vector<std::string> res;
    std::string temp;
    while (stream >> temp){
        res.push_back(temp);} // Push each token to result
    return res;
}

// Parse input from the client
void parseInput(char *buf, int numOfBytes, int &n, int &m, int &weight, std::string &strat, std::string &act, std::string &current_act, const std::vector<std::string> &commands_graph, const std::vector<std::string> &mst_starts){
    buf[numOfBytes] = '\0'; // Null-terminate the buffer
    act = lower_case(std::string(buf)); // Convert input to lowercase
    std::vector<std::string> command = split_spaces(act); // Split input into command
    if (command.size() > 0){
        current_act = command[0]; // Get the first token as the act
        cout << "act received: " << current_act << " command size: " << command.size() << endl;
    }
    else{ current_act = "There is no message"; // Handle empty input
    }
    // Check if the act is a valid graph act
    if (find(commands_graph.begin(), commands_graph.end(), current_act) == commands_graph.end()){
        current_act = "message"; // Invalid act
    }
    else if (current_act == "mst"){ // Check for MST act
        if (command.size() > 1){
            if (find(mst_starts.begin(), mst_starts.end(), command[1]) == mst_starts.end()){
                current_act = "message"; // Invalid strategy
            }
            else{
                current_act = "mst"; // Valid MST act
                act = command[0];
                n = -1;
                m = -1;
                weight = -1;
                strat = command[1]; // Set strategy
            }
        }
        else {
            current_act = "message"; // No strategy provided
        }
    }
    else if (!string_is_num(command)){ // Check if command are numbers
        current_act = "message"; // Not valid numbers
        cout << "Not a number" << endl;
    }
    else if (current_act == "newgraph"){ // Handle new graph creation
        if (command.size() != 3){ // Check token count
            current_act = "message"; // Invalid command
        }
        else{
            n = stoi(command[1]); // Get number of vertices
            m = stoi(command[2]); // Get number of edges
            weight = -1; 
        }
    }
    else if (current_act == "newedge"){ // Handle new edge creation
        if (command.size() != 4){ // Check token count
            current_act = "message"; // Invalid command
        }
        else{
            n = stoi(command[1]); // Get starting vertex
            m = stoi(command[2]); // Get ending vertex
            weight = stoi(command[3]); // Get edge weight
        }
    }
    else if (current_act == "removeedge"){ // Handle edge removal
        if (command.size() != 3){
            current_act = "message"; // Invalid command
        }
        else{
            n = stoi(command[1]); // Get starting vertex
            m = stoi(command[2]); // Get ending vertex
            weight = -1; // Weight not needed for removal
        }
    }
}
