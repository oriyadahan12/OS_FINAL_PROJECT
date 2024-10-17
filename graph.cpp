#include "vertex.cpp"
#include "edge.cpp"

// Check if the graph is connected using BFS
bool Graph::isConnected() const
{ 
    size_t n = numVertices(); // Get the number of vertices
    std::vector<std::vector<size_t>> adjMat = adjacencyMatrix(); // Get adjacency matrix
    std::vector<bool> visited(n, false); // Vector to track visited vertices
    std::queue<size_t> q; // Queue for BFS
    q.push(0); // Start BFS from vertex 0
    visited[0] = true; // Mark the start vertex as visited
    size_t count = 1; // Count of visited vertices
    while (!q.empty())
    {
        size_t current = q.front(); // Get the front vertex
        q.pop();
        for (size_t i = 0; i < n; i++)
        {
            if (adjMat[current][i] != INF && !visited[i]) // Check for unvisited neighbors
            {
                visited[i] = true; // Mark neighbor as visited
                q.push(i); // Add to queue
                count++; // Increment visited count
            }
        }
    }
    return count == n; // Return true if all vertices are visited
}

// Constructor to create an empty graph
Graph::Graph() : vertices(), edges(), distances(), parent() {}

// Constructor to create a graph from a set of vertices that may already contain edges
Graph::Graph(std::unordered_set<Vertex> inputVxs) : vertices(), edges(), distances(), parent()
{
    // Add vertices to the graph
    for (auto v : inputVxs)
        vertices[v.getId()] = v; // Map vertex ID to vertex
    // Add edges to the graph
    for (auto v : inputVxs)
    {
        for (auto e : v) // Assuming `v` can iterate over edges
        {
            // Check if the edge connects to another vertex in the set
            if (inputVxs.find(e.getOther(v)) != inputVxs.end())
                edges.insert(e); // Insert edge if valid
        }
    }
}

// Copy constructor with option to not copy edges
Graph::Graph(const Graph &other, bool copyEdges) : vertices(), edges(), distances(), parent()
{   
    // Copy all vertices:
    for (const auto &pair : other.vertices)
    {
        vertices[pair.first] = pair.second;  // Deep copy vertices
    }

    if (copyEdges)
    {
        // Copy all edges:
        for (const auto &e : other.edges)
        {
            edges.insert(e); // Insert edges from the other graph
        }

        // Copy all distances:
        if (other.distances.size() != 0)
        {
            // Deep copy distances:
            size_t n = other.distances.size();
            distances = std::vector<std::vector<size_t>>(n, std::vector<size_t>(n, INF)); // Initialize with INF
            for (size_t i = 0; i < n; i++)
            {
                for (size_t j = 0; j < n; j++)
                {
                    distances[i][j] = other.distances[i][j]; // Copy distances
                }
            }
        }
        
        // Copy all parents:
        if (other.parent.size() != 0)
        {
            size_t n = other.parent.size();
            parent = std::vector<std::vector<size_t>>(n, std::vector<size_t>(n, INF)); // Initialize with INF
            for (size_t i = 0; i < n; i++)
            {
                for (size_t j = 0; j < n; j++)
                {
                    parent[i][j] = other.parent[i][j]; // Copy parents
                }
            }
        }
    }
    else
    { 
        // Remove all edges if not copying edges
        for (auto &pair : vertices)
        {
            pair.second.removeAllEdges(); // Clear edges from each vertex
            pair.second.getAdj().clear(); // Clear adjacency map
        }
    }
}

// Get the number of vertices in the graph
size_t Graph::numVertices() const
{
    return vertices.size(); // Return size of vertices map
}

// Get the number of edges in the graph
size_t Graph::numEdges() const
{
    return edges.size(); // Return size of edges set
}

// Check if the graph has a vertex
bool Graph::hasVertex(Vertex v) const
{
    return vertices.find(v.getId()) != vertices.end(); // Check if vertex exists
}

// Get an iterator for the start of edges in the graph
std::unordered_set<Edge>::iterator Graph::edgesBegin()
{
    return edges.begin(); // Return beginning iterator for edges
}

// Get an iterator for the end of edges in the graph
std::unordered_set<Edge>::iterator Graph::edgesEnd()
{
    return edges.end(); // Return end iterator for edges
}

// Add an edge to the graph; the edge is directed from start to end
void Graph::addEdge(Edge e)
{
    cleanDistParent(); // Clean distance and parent matrices
    vertices[e.getStart().getId()].addEdge(e); // Add edge to start vertex
    vertices[e.getEnd().getId()].addEdge(e); // Add edge to end vertex
    // Update adjacency maps for both vertices
    vertices[e.getStart().getId()].getAdj()[e.getOther(vertices[e.getStart
