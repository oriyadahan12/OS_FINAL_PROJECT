#include "serverUtils.hpp"

extern LFP lfp; // Leader-Follower pattern instance

// Function to convert a string to lowercase
std::string toLowerCase(std::string s)
{
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool isNumber(const std::vector<std::string> &s)
{
    for (size_t i = 1; i < s.size(); i++)
    {
        std::string str = s[i];
        if (str.empty())
        {
            return false;
        }
        if (!std::all_of(str.begin(), str.end(), ::isdigit))
        {
            return false;
        }
    }
    return true;
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

std::vector<std::string> splitStringBySpaces(const std::string &input)
{
    std::istringstream stream(input);
    std::vector<std::string> result;
    std::string temp;

    while (stream >> temp)
    {
        result.push_back(temp);
    }

    return result;
}

void parseInput(char *buf, int nbytes, int &n, int &m, int &weight, std::string &strat, std::string &action, std::string &actualAction, const std::vector<std::string> &graphActions, const std::vector<std::string> &mstStrats)
{

    buf[nbytes] = '\0';
    action = toLowerCase(std::string(buf));
    std::vector<std::string> tokens = splitStringBySpaces(action);
    if (tokens.size() > 0)
    {
        actualAction = tokens[0];
        cout << "Action received: " << actualAction << " tokens size: " << tokens.size() << endl;
    }
    else
    {
        actualAction = "emptyMessage";
    }
    if (find(graphActions.begin(), graphActions.end(), actualAction) == graphActions.end())
    {
        actualAction = "message";
    }
    else if (actualAction == "mst")
    {
        if (tokens.size() > 1)
        {
            if (find(mstStrats.begin(), mstStrats.end(), tokens[1]) == mstStrats.end())
            {
                actualAction = "message";
            }
            else
            {
                actualAction = "mst";
                action = tokens[0];
                n = -1;
                m = -1;
                weight = -1;
                strat = tokens[1];
            }
        }
        else {
            actualAction = "message";
        }
    }
    else if (!isNumber(tokens))
    {
        actualAction = "message";
        cout << "Not a number" << endl;
    }
    else if (actualAction == "newgraph")
    {
        if (tokens.size() != 3)
        {
            actualAction = "message";
        }
        else
        {
            n = stoi(tokens[1]);
            m = stoi(tokens[2]);
            weight = -1;
        }
    }
    else if (actualAction == "newedge")
    {
        if (tokens.size() != 4)
        {
            actualAction = "message";
        }
        else
        {
            n = stoi(tokens[1]);
            m = stoi(tokens[2]);
            weight = stoi(tokens[3]);
        }
    }
    else if (actualAction == "removeedge") // removeedge
    {
        if (tokens.size() != 3)
        {
            actualAction = "message";
        }
        else
        {
            n = stoi(tokens[1]);
            m = stoi(tokens[2]);
            weight = -1;
        }
    }
}

std::unordered_set<Vertex> initVertices(int n)
{
    std::unordered_set<Vertex> vertices;
    for (size_t i = 0; i < n; i++)
    {
        vertices.insert(Vertex(i));
    }
    return vertices;
}

std::pair<std::string, Graph *> newGraph(int n, int m, int clientFd, Graph *g)
{
    std::cout << "Creating a new graph with " << n << " vertices and " << m << " edges" << std::endl;

    if (g != nullptr)
        delete g;
    std::unordered_set<Vertex> vertices = initVertices(n); // Initialize the vertices

    g = new Graph(vertices);   // Create a new graph of n vertices
    initGraph(g, m, clientFd); // Initialize the graph with m edges

    std::string msg = "Client " + std::to_string(clientFd) + " successfully created a new Graph with " + std::to_string(n) + " vertices and " + std::to_string(m) + " edges" + "\n";
    std::cout << "Graph created successfully\n";
    return {msg, g};
}

std::pair<std::string, Graph *> newEdge(size_t n, size_t m, size_t weight, int clientFd, Graph *g)
{
    std::cout << "Adding an edge from " << n << " to " << m << std::endl;
    g->addEdge(Edge(g->getVertex(n - 1), g->getVertex(m - 1), weight)); // Add edge from u to v
    std::string msg = "Client " + std::to_string(clientFd) + " added an edge from " + std::to_string(n) + " to " + std::to_string(m) + " with weight " + std::to_string(weight) + "\n";

    return {msg, g};
}

std::pair<std::string, Graph *> removeedge(int n, int m, int clientFd, Graph *g)
{
    std::cout << "Removing an edge from " << n << " to " << m << std::endl;
    g->removeEdge(Edge{g->getVertex(n - 1), g->getVertex(m - 1)}); // Remove edge from u to v
    std::string msg = "Client " + std::to_string(clientFd) + " removed an edge from " + std::to_string(n) + " to " + std::to_string(m) + "\n";

    return {msg, g};
}

std::pair<std::string, Graph *> handleInput(Graph *g, std::string action, int clientFd, std::string actualAction, int n, int m, int w, std::string strat)
{
    std::string msg;
    std::vector<std::string> tokens = splitStringBySpaces(action);
    if (tokens.size() < 1)
    {
        msg = "User " + std::to_string(clientFd) + " sent an empty message\n";
        return {msg, nullptr};
    }

    if (actualAction == "newgraph")
    { // format: newgraph n m
        return newGraph(n, m, clientFd, g);
    }
    else if (actualAction == "newedge")
    { // format: newedge n m (add an edge from n to m)
        if (g != nullptr)
        {
            return newEdge(static_cast<size_t>(n), static_cast<size_t>(m), static_cast<size_t>(w), clientFd, g);
        }
        else
        {
            msg = "Client " + std::to_string(clientFd) + " tried to perform the operation but there is no graph\n";
            return {msg, nullptr};
        }
    }
    else if (actualAction == "removeedge")
    { // format: removeedge n m (remove an edge from n to m)
        if (g != nullptr)
        {
            return removeedge(n, m, clientFd, g);
        }
        else
        {
            msg = "Client " + std::to_string(clientFd) + " tried to perform the operation but there is no graph\n";
            return {msg, nullptr};
        }
    }
    else if (actualAction == "mst")
    { // format: MST
        if (g == nullptr)
        {
            msg = "Client " + std::to_string(clientFd) + " tried to perform the operation but there is no graph\n";
            return {msg, nullptr};
        }
        else if (!g->isConnected())
        {
            msg = "Client " + std::to_string(clientFd) + " tried to perform the operation but the graph is not connected therefore it doesn't have a MST\n";
            return {msg, nullptr};
        }
        else
        {
            return MST(g, clientFd, strat);
        }
    }
    else
    {
        msg = "Client " + std::to_string(clientFd) + " sent a message: " + action;
        return {msg, nullptr};
    }
}

// Get sockaddr, IPv4 or IPv6:
void *getInAddr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Return a listening socket
int getListenerSocket(void)
{
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL)
    {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1)
    {
        return -1;
    }

    return listener;
}

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size)
    {
        *fd_size *= 2; // Double it

        *pfds = (struct pollfd *)realloc(*pfds, sizeof(**pfds) * (size_t)(*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count - 1];

    (*fd_count)--;
}