#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>

class Edge;



class Vertex
{

private:
    // ID of the vertex
    size_t id;

    // List to store the edges connected to the vertex
    std::vector<Edge> edges;

    // <neighbour, weight to the neighbour>
    std::map<size_t,size_t> adj;

public:
    // Constructor to create a vertex with a given ID
    Vertex(size_t id);

    // Default constructor
    Vertex() = default;

    // Getters and setters for vertex properties
    size_t &getId();
    const size_t &getId() const;

    // Add an edge to the vertex
    void addEdge(Edge e);

    // Remove an edge from the vertex
    void removeEdge(Edge e);

    //Remove all edges from the vertex
    void removeAllEdges();

    // Get an iterator for the edges connected to the vertex
    std::vector<Edge>::iterator begin();
    std::vector<Edge>::iterator end();

    const std::map<size_t,size_t>& getAdj() const;
    std::map<size_t,size_t>& getAdj();

    //iterator for the adj map
    std::map<size_t,size_t>::iterator adjBegin();

    std::map<size_t,size_t>::iterator adjEnd();

    // Check if the vertex has an edge connecting to a specific target vertex
    bool hasEdge(Vertex target) const;

    bool operator==(const Vertex &other) const;

    Vertex &operator=(const Vertex &other);

   
    bool operator<(const Vertex &other) const
    {
        return id < other.id;
    }

    friend std::ostream &operator<<(std::ostream &os, const Vertex &v);
   
};

namespace std
{
    template <>
    struct hash<Vertex>
    {
        std::size_t operator()(const Vertex &v) const
        {
            return v.getId();
        }
    };
}

