#include "edge.hpp"
#include "vertex.hpp"

// Constructor to create a weighted edge
Edge::Edge(Vertex s, Vertex e, size_t w ) : start(s), end(e), weight(w) {}

// Copy constructor to create an edge
Edge::Edge(const Edge &other) : start(other.start), end(other.end), weight(other.weight) {}

// Getters and setters for edge properties
Vertex &Edge::getStart() { return start; }
const Vertex &Edge::getStart() const { return start; }
Vertex &Edge::getEnd() { return end; }
const Vertex &Edge::getEnd() const { return end; }

size_t &Edge::getWeight() { return weight; }
size_t Edge::getWeight() const { return weight; }

// Get the vertex at the other end of the edge
Vertex &Edge::getOther(Vertex v)
{
    return start == v ? end : start;
}

const Vertex &Edge::getOther(Vertex v) const
{
    return start == v ? end : start;
}

// Check if the edge contains a specific vertex
bool Edge::contains(Vertex &target) const
{
    return start == target || end == target;
}

// Equality operator
bool Edge::operator==(const Edge &other) const
{
    return start == other.start && end == other.end;
}

//Assignment operator
Edge &Edge::operator=(const Edge &other)
{
    start = other.start;
    end = other.end;
    weight = other.weight;
    return *this;
}

    bool Edge::operator<(const Edge &other) const
{
    return weight < other.weight;
}

std::ostream& operator<<(std::ostream &os, const Edge &e)
{
    os << e.start << " -- " << e.end << " (" << e.weight << ")";
    return os;
}