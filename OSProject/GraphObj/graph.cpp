#include "graph.hpp"

// Check if the graph is connected
bool Graph::isConnected() const
{ // use bfs for connected
    size_t n = numVertices();
    std::vector<std::vector<size_t>> adjMat = adjacencyMatrix();
    std::vector<bool> visited(n, false);
    std::queue<size_t> q;
    q.push(0);
    visited[0] = true;
    size_t count = 1;
    while (!q.empty())
    {
        size_t current = q.front();
        q.pop();
        for (size_t i = 0; i < n; i++)
        {
            if (adjMat[current][i] != INF && !visited[i])
            {
                visited[i] = true;
                q.push(i);
                count++;
            }
        }
    }
    return count == n;
}

// Constructor to create an empty graph
Graph::Graph() : vertices(), edges(), distances(), parent() {}



// Constructor to create a graph from a set of vertices that may already contain edges
Graph::Graph(std::unordered_set<Vertex> inputVxs) : vertices(), edges(), distances(), parent()
{
    // Add vertices to the graph
    for (auto v : inputVxs)
        vertices[v.getId()] = v;
    // Add edges to the graph
    for (auto v : inputVxs)
    {
        for (auto e : v)
        {
            if (inputVxs.find(e.getOther(v)) != inputVxs.end())
                edges.insert(e);
        }
    }
    
}



// Copy constructor with option to not copy edges
Graph::Graph(const Graph &other, bool copyEdges) : vertices(), edges(), distances(), parent()
{   
    // Copy all vertices:
    for (const auto &pair : other.vertices)
    {
        vertices[pair.first] = pair.second;  // deep copy vertices
    }

    if (copyEdges)
    {
        // Copy all edges:
        for (const auto &e : other.edges)
        {
            edges.insert(e);
        }


        // Copy all distances:
        if (other.distances.size() != 0)
        {
            // deep copy distances:
            size_t n = other.distances.size();
            distances = std::vector<std::vector<size_t>>(n, std::vector<size_t>(n, INF));
            for (size_t i = 0; i < n; i++)
            {
                for (size_t j = 0; j < n; j++)
                {
                    distances[i][j] = other.distances[i][j];
                }
            }
        }
        
        // Copy all parents:
        if (other.parent.size() != 0)
        {
            // deep copy distances, notice it's unique_ptr
            size_t n = other.parent.size();
            parent = std::vector<std::vector<size_t>>(n, std::vector<size_t>(n, INF));
            for (size_t i = 0; i < n; i++)
            {
                for (size_t j = 0; j < n; j++)
                {
                    parent[i][j] = other.parent[i][j];
                }
            }
        }
    }

    else
    { // Remove all edges
        for (auto &pair : vertices)
        {
            pair.second.removeAllEdges();
            pair.second.getAdj().clear();
        }
    }
}



// Get the number of vertices in the graph
size_t Graph::numVertices() const
{
    return vertices.size();
}

// Get the number of edges in the graph
size_t Graph::numEdges() const
{
    return edges.size();
}

// Check if the graph has a vertex
bool Graph::hasVertex(Vertex v) const
{
    return vertices.find(v.getId()) != vertices.end();
}

// Get an iterator for the start of edges in the graph
std::unordered_set<Edge>::iterator Graph::edgesBegin()
{
    return edges.begin();
}

// Get an iterator for the end of edges in the graph
std::unordered_set<Edge>::iterator Graph::edgesEnd()
{
    return edges.end();
}





// Add an edge to the graph, the edge is directed from start to end
void Graph::addEdge(Edge e)
{
    cleanDistParent();
    vertices[e.getStart().getId()].addEdge(e);
    vertices[e.getEnd().getId()].addEdge(e);
    vertices[e.getStart().getId()].getAdj()[e.getOther(vertices[e.getStart().getId()]).getId()] = e.getWeight();
    vertices[e.getEnd().getId()].getAdj()[e.getOther(vertices[e.getEnd().getId()]).getId()] = e.getWeight();
    edges.insert(e);
}

// Remove an edge from the graph
void Graph::removeEdge(Edge e)
{
    cleanDistParent();
    vertices[e.getStart().getId()].removeEdge(e);
    vertices[e.getEnd().getId()].removeEdge(e);
    vertices[e.getStart().getId()].getAdj().erase(e.getOther(vertices[e.getStart().getId()]).getId());
    vertices[e.getEnd().getId()].getAdj().erase(e.getOther(vertices[e.getEnd().getId()]).getId());
    edges.erase(e);
    edges.erase(Edge(e.getEnd(),e.getStart(),e.getWeight()));

}

void Graph::addEdge(Vertex &start, Vertex &end, size_t weight)
{
    Edge e(start, end, weight);
    addEdge(e);
}

// Get an iterator for the vertices in the graph
std::map<int, Vertex>::iterator Graph::begin()
{
    return vertices.begin();
}

// Get an iterator for the end of the vertices in the graph
std::map<int, Vertex>::iterator Graph::end()
{
    return vertices.end();
}

// Get the adjacency matrix of the graph
std::vector<std::vector<size_t>> Graph::adjacencyMatrix() const
{
    size_t n = numVertices();
    std::vector<std::vector<size_t>> adjMat(n, std::vector<size_t>(n, INF)); // Initialize all distances to -1, actually infinity, because of using size_t
    for (auto Edge : edges)
    {
        adjMat[Edge.getStart().getId()][Edge.getEnd().getId()] = Edge.getWeight();
        adjMat[Edge.getEnd().getId()][Edge.getStart().getId()] = Edge.getWeight();
    }
    for (size_t i = 0; i < n; i++)
    {
        adjMat[i][i] = 0;
    }
    return adjMat;
}

// Get a vertex by its ID
Vertex &Graph::getVertex(int id)
{
    return vertices[id];
}

const Vertex &Graph::getVertex(int id) const
{
    return vertices.at(id);
}

size_t Graph::totalWeight() const
{
    size_t total = 0;
    for (const auto &e : edges)
    {
        total += e.getWeight();
    }
    return total;
}

std::string Graph::longestPath(const std::vector<std::vector<size_t>> &dist) const
{
    size_t n = numVertices();
    size_t maxDist = 0;
    size_t maxDistIndex = 0, maxDistIndex2 = 0;
    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = 0; j < n; j++)
        {
            if (dist[i][j] > maxDist && dist[i][j] != INF && i != j)
            {
                maxDist = dist[i][j];
                maxDistIndex = i;
                maxDistIndex2 = j;
            }
        }
    }
    return "Longest path is from " + std::to_string(maxDistIndex) + " to " + std::to_string(maxDistIndex2) + " with a distance of " + std::to_string(maxDist);
}

// gets the distances between vertices in the graph and the parent matrix for undirected graph
std::pair<std::vector<std::vector<size_t>>, std::vector<std::vector<size_t>>> Graph::getDistances() const
{
    if (this->distances.empty())
        throw std::runtime_error("Distances not calculated");
    if (this->parent.empty())
        throw std::runtime_error("Parent not calculated");
    return std::make_pair(distances, parent);
}

double Graph::avgDistance(const std::vector<std::vector<size_t>> &dist) const
{
    size_t totalDist = 0;
    size_t count = 0;
    size_t n = dist.size();
    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = i; j < n; j++)
        {
            totalDist += dist[i][j];
            count++;
        }
    }
    count -= n; // Don't count the diagonal
    return static_cast<double>(totalDist) / count;
}

std::string Graph::shortestPath(size_t start, size_t end, const std::vector<std::vector<size_t>> &dist,const std::vector<std::vector<size_t>> &parents) const
{

    if (start >= numVertices() || end >= numVertices())
    {
        return "Invalid vertices\n";
    }

    if (parents[start][end] == INF)
    {
        return "No path exists between " + std::to_string(start) + " and " + std::to_string(end) + "\n";
    }

    std::string path = "Shortest path from " + std::to_string(start) + " to " + std::to_string(end) + " is: ";

    std::vector<int> pathVec;
    pathVec.push_back(end);
    size_t current = end;
    while (current != start)
    {
        current = parents[start][current];
        pathVec.push_back(current);
    }
    // using reverse iterator to get the path in the correct order
    for (auto it = pathVec.rbegin(); it != pathVec.rend(); it++)
    {
        path += std::to_string(*it) + " -> ";
    }
    path.pop_back();
    path.pop_back();
    path.pop_back(); // Remove the last arrow

    return path + " with a distance of " + std::to_string(dist[start][end]) + "\n";
}

// gets the shortest path between all vertices in the graph, returns a string with all the paths in the graph for undirected graph
std::string Graph::allShortestPaths(const std::vector<std::vector<size_t>> &dist, const std::vector<std::vector<size_t>> &parent) const
{
    size_t n = numVertices();
    std::string paths = "Shortest paths between all vertices in the graph are: \n";
    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = i + 1; j < n; j++)
        {
            paths += shortestPath(i, j, dist, parent);
        }
    }
    return paths;
}

std::pair<std::vector<std::vector<size_t>>, std::vector<std::vector<size_t>>> Graph::floydWarshall() const
{
    size_t n = numVertices();
    std::vector<std::vector<size_t>> dist = adjacencyMatrix();
    std::vector<std::vector<size_t>> parent(n, std::vector<size_t>(n, INF));
    // initialize parent matrix
    for (size_t i = 0; i < n; i++)
    {
        for (size_t j = 0; j < n; j++)
        {
            if (dist[i][j] != INF)
            {
                parent[i][j] = i;
            }
        }
    }

    for (size_t k = 0; k < n; k++)
    {
        for (size_t i = 0; i < n; i++)
        {
            for (size_t j = 0; j < n; j++)
            {
                if (dist[i][k] != INF && dist[k][j] != INF && dist[i][j] > dist[i][k] + dist[k][j])
                {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    parent[i][j] = parent[k][j];
                }
            }
        }
    }

    return {dist, parent};
}

std::string Graph::longestPath() const
{
    if (distances.empty())
    {
        std::vector<std::vector<size_t>> dist = getDistances().first;
        return longestPath(dist);
    }
    return longestPath(distances);
}
double Graph::avgDistance() const
{
    if (distances.empty())
    {
        std::vector<std::vector<size_t>> dist = getDistances().first;
        return avgDistance(dist);
    }
    return avgDistance(distances);
}
std::string Graph::allShortestPaths() const
{
    // If distances are not calculated, calculate them
    if (distances.empty())
    {
        // Get the distances between vertices in the graph and the parent matrix
        std::vector<std::vector<size_t>> dist, parent;
        std::tie(dist, parent) = floydWarshall();
        return allShortestPaths(dist, parent);
    }
    return allShortestPaths(distances, parent);
}


std::string Graph::stats() const
{
    std::vector<std::vector<size_t>> dist, parents;
    // Get the distances between vertices in the graph and the parent matrix
    if (distances.empty() || parent.empty())
    {
        std::tie(dist, parents) = floydWarshall();
    }
    else
    {
        std::tie(dist, parents) = getDistances();
    }
    std::string stats = "Graph with " + std::to_string(numVertices()) + " vertices and " + std::to_string(edges.size()) + " edges\n";
    stats += "Total weight of edges: " + std::to_string(totalWeight()) + "\n";
    stats += longestPath(dist) + "\n";
    stats += "The average distance between vertices is: " + std::to_string(avgDistance(dist)) + "\n";
    stats += "The shortest paths are: \n" + allShortestPaths(dist, parents) + "\n";
    return stats;
}

void Graph::setDistances(std::vector<std::vector<size_t>> dist)
{
    distances = std::move(dist);
}

void Graph::setParent(std::vector<std::vector<size_t>> pare)
{
   parent = std::move(pare);
}

void Graph::cleanDistParent()
{
    //clearing the dist matrix and parent matrix
    for(size_t i = 0; i< distances.size(); i++){
        distances[i].clear();
    }
    for(size_t i = 0; i< parent.size(); i++){
        parent[i].clear();
    }
    parent.clear();
    distances.clear();
}
  
