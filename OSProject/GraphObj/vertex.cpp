#include "vertex.hpp"
#include "edge.hpp"


// Constructor to create a vertex with a given ID
Vertex::Vertex(size_t id) : id(id) {}

// Getters and setters for vertex properties

size_t &Vertex::getId() { return id; }
const size_t &Vertex::getId() const { return id; }

// Add an edge to the vertex
void Vertex::addEdge(Edge e)
{
    if (std::find(edges.begin(), edges.end(), e) == edges.end())
    edges.push_back(e);
}

// Remove an edge from the vertex
void Vertex::removeEdge(Edge e)
{
    edges.erase(std::remove(edges.begin(), edges.end(), e), edges.end());
    adj.erase(e.getOther(*this).getId());
}

//Remove all edges from the vertex
void Vertex::removeAllEdges()
{
    edges.clear();
    adj.clear();
}

// Get an iterator for the edges connected to the vertex
std::vector<Edge>::iterator Vertex::begin()
{
    return edges.begin();
}
std::vector<Edge>::iterator Vertex::end()
{
    return edges.end();
}

const std::map<size_t, size_t> &Vertex::getAdj() const
{
    return adj;
}

std::map<size_t, size_t> &Vertex::getAdj()
{
    return adj;
}

//iterator for the adj map
std::map<size_t, size_t>::iterator Vertex::adjBegin()
{
    return adj.begin();
}

std::map<size_t, size_t>::iterator Vertex::adjEnd()
{
    return adj.end();
}

// Check if the vertex has an edge connecting to a specific target vertex
bool Vertex::hasEdge(Vertex target) const
{
    for (auto e : edges)
    {
        if (e.contains(target))
            return true;
    }
    return false;
}

bool Vertex::operator==(const Vertex &other) const
{
    return id == other.id;
}

Vertex &Vertex::operator=(const Vertex &other)
{
    id = other.id;
    for (Edge e : other.edges)
    {
        edges.push_back(e);
    }
    return *this;
}

std::ostream& operator<<(std::ostream &os, const Vertex &v)
{
    os << "Vertex " << v.getId();
    return os;
}