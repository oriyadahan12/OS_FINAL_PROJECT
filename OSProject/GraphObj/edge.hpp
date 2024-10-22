#pragma once
#include <functional>
#include "vertex.hpp"
#include <iostream>


class Edge
{
private:
    // The start and end vertices of the edge
    Vertex start;
    Vertex end;
    // Whether the edge is weighted and the weight of the edge
    size_t weight;

public:
    // Constructor to create a weighted edge
    Edge(Vertex s, Vertex e, size_t w = 1);

    // Default constructor
    Edge() = default;

    // Copy constructor to create an edge
    Edge(const Edge &other);

    // Getters and setters for edge properties
    Vertex &getStart();
    const Vertex &getStart() const;
    Vertex &getEnd();
    const Vertex &getEnd() const;

    size_t &getWeight();
    size_t getWeight() const;

    // Get the vertex at the other end of the edge
    Vertex &getOther(Vertex v);
    const Vertex &getOther(Vertex v) const;

    // Check if the edge contains a specific vertex
    bool contains(Vertex &target) const;


    // Equality operator
    bool operator==(const Edge &other) const;

    //Assignment operator
    Edge &operator=(const Edge &other);

    //Less than operator
    bool operator<(const Edge &other) const;

    //greater than operator for priority queue
    bool operator>(const Edge &other) const;

    friend std::ostream &operator<<(std::ostream &os, const Edge &e);

    };



// Hash function for the Edge class, used in unordered_set
namespace std
{
    template <>
    struct hash<Edge>
    {
        std::size_t operator()(const Edge &e) const
        {
            std::size_t h1 = std::hash<Vertex>{}(e.getStart());
            std::size_t h2 =std::hash<Vertex>{}(e.getEnd());
            return h1 ^ (h2 << 1);
        }
    };
}




