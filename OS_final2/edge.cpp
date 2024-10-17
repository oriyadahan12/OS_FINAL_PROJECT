#include "vertex.cpp"

// Constructor to create a weighted edge
Edge::Edge(Vertex s, Vertex e, size_t w) : start(s), end(e), weight(w) {}

// Copy constructor to create an edge
Edge::Edge(const Edge &other) : start(other.start), end(other.end), weight(other.weight) {}

// Getters and setters for edge properties
Vertex &Edge::getStart() { return start; } // Return a reference to the start vertex
const Vertex &Edge::getStart() const { return start; } // Return a const reference to the start vertex

Vertex &Edge::getEnd() { return end; } // Return a reference to the end vertex
const Vertex &Edge::getEnd() const { return end; } // Return a const reference to the end vertex

size_t &Edge::getWeight() { return weight; } // Return a reference to the edge's weight
size_t Edge::getWeight() const { return weight; } // Return the edge's weight as a value

// Get the vertex at the other end of the edge
Vertex &Edge::getOther(Vertex v) {
    return start == v ? end : start; // Return the vertex opposite to the given vertex
}

const Vertex &Edge::getOther(Vertex v) const {
    return start == v ? end : start; // Return the opposite vertex as a const reference
}

// Check if the edge contains a specific vertex
bool Edge::contains(Vertex &target) const {
    return start == target || end == target; // Return true if the edge contains the target vertex
}

// Equality operator
bool Edge::operator==(const Edge &other) const {
    return start == other.start && end == other.end; // Check if two edges are equal by comparing vertices
}

// Assignment operator
Edge &Edge::operator=(const Edge &other) {
    start = other.start; // Assign the start vertex
    end = other.end;     // Assign the end vertex
    weight = other.weight; // Assign the weight
    return *this; // Return the current edge
}

// Less-than operator for comparing edges based on weight
bool Edge::operator<(const Edge &other) const {
    return weight < other.weight; // Return true if this edge's weight is less than the other's
}

// Overload the output stream operator for Edge
std::ostream& operator<<(std::ostream &os, const Edge &e) {
    os << e.start << " -- " << e.end << " (" << e.weight << ")"; // Output the edge in a readable format
    return os; // Return the output stream
}
