#include "vertex.hpp"
#include "edge.hpp"


// Constructor to create a vertex with a given ID
Vertex::Vertex(size_t id) : id(id) {}

// Getters and setters for vertex properties

size_t &Vertex::getId() { return id; } // Return a reference to the vertex ID
const size_t &Vertex::getId() const { return id; } // Return a const reference to the vertex ID

// Add an edge to the vertex
void Vertex::addEdge(Edge e) {
    // Check if the edge is not already in the edges vector
    if (std::find(edges.begin(), edges.end(), e) == edges.end())
        edges.push_back(e); // Add the edge if it's not present
}

const std::map<size_t, size_t> &Vertex::getAdj() const {
    return adj; // Return a const reference to the adjacency map
}

std::map<size_t, size_t> &Vertex::getAdj() {
    return adj; // Return a reference to the adjacency map
}

// Iterator for the adjacency map
std::map<size_t, size_t>::iterator Vertex::adjBegin() {
    return adj.begin(); // Return an iterator to the beginning of the adjacency map
}

std::map<size_t, size_t>::iterator Vertex::adjEnd() {
    return adj.end(); // Return an iterator to the end of the adjacency map
}

// Check if the vertex has an edge connecting to a specific target vertex
bool Vertex::hasEdge(Vertex target) const {
    // Iterate through edges to check if any edge contains the target vertex
    for (auto e : edges) {
        if (e.contains(target))
            return true; // Return true if an edge contains the target
    }
    return false; // Return false if no edge contains the target
}

// Remove an edge from the vertex
void Vertex::removeEdge(Edge e) {
    // Remove the edge from the edges vector
    edges.erase(std::remove(edges.begin(), edges.end(), e), edges.end());
    // Remove the edge from the adjacency map using the other vertex
    adj.erase(e.getOther(*this).getId());
}

// Remove all edges from the vertex
void Vertex::removeAllEdges() {
    edges.clear(); // Clear the edges vector
    adj.clear();   // Clear the adjacency map
}

// Get an iterator for the edges connected to the vertex
std::vector<Edge>::iterator Vertex::begin() {
    return edges.begin(); // Return an iterator to the beginning of edges
}
std::vector<Edge>::iterator Vertex::end() {
    return edges.end(); // Return an iterator to the end of edges
}

// Equality operator to compare two vertices
bool Vertex::operator==(const Vertex &v) const {
    return id == v.id; // Compare vertex IDs for equality
}

// Assignment operator to assign one vertex to another
Vertex &Vertex::operator=(const Vertex &v) {
    id = v.id; // Assign the ID
    // Copy edges from the source vertex
    for (Edge e : v.edges) {
        edges.push_back(e);
    }
    return *this; // Return the current vertex
}

// Overload the output stream operator for Vertex
std::ostream& operator<<(std::ostream &os, const Vertex &v) {
    os << "Vertex " << v.getId(); // Output the vertex ID
    return os; // Return the output stream
}