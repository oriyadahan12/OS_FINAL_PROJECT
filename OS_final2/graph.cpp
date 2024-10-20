#include "vertex.cpp"

// Check if the graph is connected using BFS
bool Graph::isConnected() const
{ 
    size_t n = numVertices(); // Get the number of vertices
    std::vector<std::vector<size_t>> mat = adjacencyMatrix(); // Get the adjacency matrix
    std::vector<bool> visited(n, false); // Track visited vertices
    std::queue<size_t> q; // Queue for BFS
    q.push(0); // Start from the first vertex
    visited[0] = true; // Mark the first vertex as visited
    size_t count = 1; // Count of visited vertices
    
    while (!q.empty()){
        size_t curr = q.front(); // Get the current vertex
        q.pop();
        for (size_t i = 0; i < n; i++){
            // Check for an edge and whether the vertex is unvisited
            if (mat[curr][i] != INF && !visited[i]){
                visited[i] = true; // Mark as visited
                q.push(i); // Add to queue
                count++; // Increment count of visited vertices
            }
        }
    }
    return count == n; // Check if all vertices are visited
}

// Constructor to create an empty graph
Graph::Graph() :
    vertices(), 
    edges(), 
    distances(), 
    parent() {}

// Constructor to create a graph from a set of vertices that may already contain edges
Graph::Graph(std::unordered_set<Vertex> v) : 
    vertices(), 
    edges(), 
    distances(), 
    parent()
{
    // Add vertices to the graph
    for (auto vertex : v)
        vertices[vertex.getId()] = vertex; // Store vertex by ID
    // Add edges to the graph
    for (auto vertex : v){
        for (auto e : vertex) {/ Iterate through edges of vertex
            // Check if the other vertex of the edge is in the set
            if (v.find(e.getOther(vertex)) != v.end())
                edges.insert(e); // Add edge if valid
        }
    }
}

// Copy constructor with option to not copy edges
Graph::Graph(const Graph &other, bool copyEdges) : vertices(),
    edges(), 
    distances(), 
    parent()
{   
    // Copy all vertices
    for (const auto &pair : other.vertices){
        vertices[pair.first] = pair.second;  // Deep copy vertices
    }

    if (copyEdges){
        // Copy all edges
        for (const auto &e : other.edges){
            edges.insert(e);
        }

        // Copy all distances
        if (other.distances.size() != 0) {
            // Deep copy distances
            size_t n = other.distances.size();
            distances = std::vector<std::vector<size_t>>(n, std::vector<size_t>(n, INF));
            for (size_t i = 0; i < n; i++){
                for (size_t j = 0; j < n; j++){
                    distances[i][j] = other.distances[i][j]; // Copy distances
                }
            }
        }
        // Copy all parents
        if (other.parent.size() != 0)
        {
            // Deep copy parent matrix
            size_t n = other.parent.size();
            parent = std::vector<std::vector<size_t>>(n, std::vector<size_t>(n, INF));
            for (size_t i = 0; i < n; i++){
                for (size_t j = 0; j < n; j++){
                    parent[i][j] = other.parent[i][j]; // Copy parent information
                }
            }
        }
    }
    else{ // Remove all edges 
        for (auto &pair : vertices){
            pair.second.removeAllEdges(); // Remove edges from vertex
            pair.second.getAdj().clear(); // Clear adjacency list
        }
    }
}

// Get the number of vertices in the graph
size_t Graph::numVertices() const{
    return vertices.size(); // Return size of vertices map
}

// Get the number of edges in the graph
size_t Graph::numEdges() const{
    return edges.size(); // Return size of edges set
}

// Check if the graph has a vertex
bool Graph::hasVertex(Vertex v) const{
    return vertices.find(v.getId()) != vertices.end(); // Check if vertex ID exists
}

// Get an iterator for the start of edges in the graph
std::unordered_set<Edge>::iterator Graph::edgesBegin(){
    return edges.begin(); // Return iterator to the beginning of edges
}

// Get an iterator for the end of edges in the graph
std::unordered_set<Edge>::iterator Graph::edgesEnd(){
    return edges.end(); // Return iterator to the end of edges
}

// Add an edge to the graph, the edge is directed from start to end
void Graph::addEdge(Edge e){
    cleanDistParent(); // Clean up distance and parent matrices
    vertices[e.getStart().getId()].addEdge(e); // Add edge to start vertex
    vertices[e.getEnd().getId()].addEdge(e); // Add edge to end vertex
    // Update adjacency list for both vertices
    vertices[e.getStart().getId()].getAdj()[e.getOther(vertices[e.getStart().getId()]).getId()] = e.getWeight();
    vertices[e.getEnd().getId()].getAdj()[e.getOther(vertices[e.getEnd().getId()]).getId()] = e.getWeight();
    edges.insert(e); // Insert edge into the edges set
}

// Remove an edge from the graph
void Graph::removeEdge(Edge e){
    cleanDistParent(); // Clean up distance and parent matrices
    vertices[e.getStart().getId()].removeEdge(e); // Remove edge from start vertex
    vertices[e.getEnd().getId()].removeEdge(e); // Remove edge from end vertex
    // Erase from adjacency lists
    vertices[e.getStart().getId()].getAdj().erase(e.getOther(vertices[e.getStart().getId()]).getId());
    vertices[e.getEnd().getId()].getAdj().erase(e.getOther(vertices[e.getEnd().getId()]).getId());
    edges.erase(e); // Erase edge from edges set
    edges.erase(Edge(e.getEnd(), e.getStart(), e.getWeight())); // Remove reverse edge if it's undirected
}

// Add an edge using vertex references and weight
void Graph::addEdge(Vertex &start, Vertex &end, size_t weight){
    Edge e(start, end, weight); // Create edge
    addEdge(e); // Add edge to the graph
}

// Get an iterator for the vertices in the graph
std::map<int, Vertex>::iterator Graph::begin(){
    return vertices.begin(); // Return iterator to the beginning of vertices
}

// Get an iterator for the end of the vertices in the graph
std::map<int, Vertex>::iterator Graph::end(){
    return vertices.end(); // Return iterator to the end of vertices
}

// Get the adjacency matrix of the graph
std::vector<std::vector<size_t>> Graph::adjacencyMatrix() const{
    size_t n = numVertices(); // Get number of vertices
    std::vector<std::vector<size_t>> mat(n, std::vector<size_t>(n, INF)); // Initialize adjacency matrix with INF
    for (auto Edge : edges) // Iterate over edges
    {
        mat[Edge.getStart().getId()][Edge.getEnd().getId()] = Edge.getWeight(); // Set weight for directed edge
        mat[Edge.getEnd().getId()][Edge.getStart().getId()] = Edge.getWeight(); // Set weight for reverse edge (if undirected)
    }
    for (size_t i = 0; i < n; i++) {
        mat[i][i] = 0; // Set diagonal to 0 (distance to itself)
    }
    return mat; // Return the adjacency matrix
}

// Get a vertex by its ID
Vertex &Graph::getVertex(int id){
    return vertices[id]; // Return vertex reference
}

const Vertex &Graph::getVertex(int id) const{
    return vertices.at(id); // Return constant reference to vertex
}

// Calculate the total weight of all edges in the graph
size_t Graph::totalWeight() const{
    size_t total = 0;
    for (const auto &e : edges){
        total += e.getWeight(); // Sum up weights
    }
    return total; // Return total weight
}

// Find the longest path in the distance matrix
std::string Graph::longestPath(const std::vector<std::vector<size_t>> &dist) const{
    size_t n = numVertices();
    size_t maxDist = 0; // Initialize max distance
    size_t maxDistIndex = 0, maxDistIndex2 = 0; // Indices for longest path
    for (size_t i = 0; i < n; i++){
        for (size_t j = 0; j < n; j++){
            // Check for longest distance that isn't INF or between the same vertex
            if (dist[i][j] > maxDist && dist[i][j] != INF && i != j){
                maxDist = dist[i][j]; // Update max distance
                maxDistIndex = i; // Update start index
                maxDistIndex2 = j; // Update end index
            }
        }
    }
    return "Longest path is from " + std::to_string(maxDistIndex) + " to " + std::to_string(maxDistIndex2) + " with a distance of " + std::to_string(maxDist);
}

// Get the distances and parent matrices
std::pair<std::vector<std::vector<size_t>>, std::vector<std::vector<size_t>>> Graph::getDistances() const{
    if (this->distances.empty())
        throw std::runtime_error("Distances not calculated"); // Check if distances exist
    if (this->parent.empty())
        throw std::runtime_error("Parent not calculated"); // Check if parent exists
    return std::make_pair(distances, parent); // Return distances and parents
}

// Calculate the average distance between vertices
double Graph::avgDistance(const std::vector<std::vector<size_t>> &dist) const{
    size_t totalDist = 0; // Total distance accumulator
    size_t count = 0; // Count of valid distances
    size_t n = dist.size(); // Number of vertices
    for (size_t i = 0; i < n; i++){
        for (size_t j = i; j < n; j++){
            totalDist += dist[i][j]; // Accumulate distances
            count++; // Increment count
        }
    }
    count -= n; // Don't count the diagonal (distance to itself)
    return static_cast<double>(totalDist) / count; // Return average
}

// Find the shortest path between two vertices
std::string Graph::shortestPath(size_t start, size_t end, const std::vector<std::vector<size_t>> &dist, const std::vector<std::vector<size_t>> &parents) const{
    if (start >= numVertices() || end >= numVertices()){
        return "Invalid vertices\n"; // Check for valid vertices
    }

    if (parents[start][end] == INF){
        return "No path exists between " + std::to_string(start) + " and " + std::to_string(end) + "\n"; // No path found
    }

    std::string path = "Shortest path from " + std::to_string(start) + " to " + std::to_string(end) + " is: ";

    std::vector<int> pathVec; // Vector to store the path
    pathVec.push_back(end); // Start from the end vertex
    size_t current = end;
    while (current != start) // Backtrack to find the path{
        current = parents[start][current]; // Move to parent
        pathVec.push_back(current); // Add to path
    }
    // Reverse the path to get the correct order
    for (auto it = pathVec.rbegin(); it != pathVec.rend(); it++){
        path += std::to_string(*it) + " -> "; // Append each vertex to path string
    }
    path.pop_back();
    path.pop_back();
    path.pop_back(); // Remove the last arrow

    return path + " with a distance of " + std::to_string(dist[start][end]) + "\n"; // Return the full path
}

// Get all shortest paths in the graph
std::string Graph::allShortestPaths(const std::vector<std::vector<size_t>> &dist, const std::vector<std::vector<size_t>> &parent) const{
    size_t n = numVertices();
    std::string paths = "Shortest paths between all vertices in the graph are: \n";
    for (size_t i = 0; i < n; i++){
        for (size_t j = i + 1; j < n; j++){ // Avoid duplicates by starting j from i+1
            paths += shortestPath(i, j, dist, parent); // Get shortest path for each pair
        }
    }
    return paths; // Return all paths
}

// Floyd-Warshall algorithm to compute shortest paths
std::pair<std::vector<std::vector<size_t>>, std::vector<std::vector<size_t>>> Graph::floydWarshall() const
{
    size_t n = numVertices(); // Get number of vertices
    std::vector<std::vector<size_t>> dist = adjacencyMatrix(); // Get distance matrix from adjacency matrix
    std::vector<std::vector<size_t>> parent(n, std::vector<size_t>(n, INF)); // Initialize parent matrix
    // Initialize parent matrix
    for (size_t i = 0; i < n; i++){
        for (size_t j = 0; j < n; j++){
            if (dist[i][j] != INF){
                parent[i][j] = i; // Set parent to itself if there is an edge
            }
        }
    }

    // Floyd-Warshall algorithm
    for (size_t k = 0; k < n; k++){
        for (size_t i = 0; i < n; i++){
            for (size_t j = 0; j < n; j++){
                // Update distance and parent if a shorter path is found
                if (dist[i][k] != INF && dist[k][j] != INF && dist[i][j] > dist[i][k] + dist[k][j]){
                    dist[i][j] = dist[i][k] + dist[k][j]; // Update distance
                    parent[i][j] = parent[k][j]; // Update parent
                }
            }
        }
    }

    return {dist, parent}; // Return distance and parent matrices
}

// Get the longest path from the precomputed distances
std::string Graph::longestPath() const
{
    if (distances.empty()){ // Check if distances are computed
        std::vector<std::vector<size_t>> dist = getDistances().first; // Get distances
        return longestPath(dist); // Find longest path
    }
    return longestPath(distances); // Use precomputed distances
}

// Calculate the average distance
double Graph::avgDistance() const{
    if (distances.empty()){ // Check if distances are computed
        std::vector<std::vector<size_t>> dist = getDistances().first; // Get distances
        return avgDistance(dist); // Calculate average distance
    }
    return avgDistance(distances); // Use precomputed distances
}

// Get all shortest paths
std::string Graph::allShortestPaths() const{
    // If distances are not calculated, calculate them
    if (distances.empty()){
        // Get the distances between vertices in the graph and the parent matrix
        std::vector<std::vector<size_t>> dist, parent;
        std::tie(dist, parent) = floydWarshall(); // Compute distances and parents
        return allShortestPaths(dist, parent); // Get all shortest paths
    }
    return allShortestPaths(distances, parent); // Use precomputed distances
}

// Get graph statistics
std::string Graph::stats() const{
    std::vector<std::vector<size_t>> dist, parents;
    // Get the distances between vertices in the graph and the parent matrix
    if (distances.empty() || parent.empty()){
        std::tie(dist, parents) = floydWarshall(); // Compute distances and parents
    }
    else{
        std::tie(dist, parents) = getDistances(); // Use precomputed distances
    }
    std::string stats = "Graph with " + std::to_string(numVertices()) + " vertices and " + std::to_string(edges.size()) + " edges\n";
    stats += "Total weight of edges: " + std::to_string(totalWeight()) + "\n"; // Display total weight
    stats += longestPath(dist) + "\n"; // Display longest path
    stats += "The average distance between vertices is: " + std::to_string(avgDistance(dist)) + "\n"; // Display average distance
    stats += "The shortest paths are: \n" + allShortestPaths(dist, parents) + "\n"; // Display all shortest paths
    return stats; // Return statistics
}

// Set the distance matrix
void Graph::setDistances(std::vector<std::vector<size_t>> dist){
    distances = std::move(dist); // Move new distances
}

// Set the parent matrix
void Graph::setParent(std::vector<std::vector<size_t>> pare){
   parent = std::move(pare); // Move new parents
}

// Clean the distance and parent matrices
void Graph::cleanDistParent(){
    // Clearing the distance matrix and parent matrix
    for(size_t i = 0; i< distances.size(); i++){
        distances[i].clear(); // Clear each row of distances
    }
    for(size_t i = 0; i< parent.size(); i++){
        parent[i].clear(); // Clear each row of parents
    }
    parent.clear(); // Clear parent vector
    distances.clear(); // Clear distance vector
}
