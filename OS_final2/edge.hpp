#pragma once

#include "vertex.hpp"  // Include Vertex class
#include <iostream>    // For overloading the output stream operator
#include <cstddef>     // For size_t

/**
 * @class Edge
 * @brief Represents a weighted edge connecting two vertices in a graph.
 * 
 * The Edge class models a connection between two vertices with a specific weight.
 * It provides functionality to get the vertices, weight, and check if an edge contains a vertex.
 */
class Edge {
private:
    Vertex start;   // The starting vertex of the edge
    Vertex end;     // The ending vertex of the edge
    size_t weight;  // The weight of the edge

public:
    /**
     * @brief Constructor to create an edge with given start, end vertices and weight.
     * 
     * @param s The start vertex of the edge.
     * @param e The end vertex of the edge.
     * @param w The weight of the edge.
     */
    Edge(Vertex s, Vertex e, size_t w);

    /**
     * @brief Copy constructor to create a copy of an existing edge.
     * 
     * @param other The edge to copy.
     */
    Edge(const Edge &other);

    // Getters and setters for edge properties

    /**
     * @brief Get the start vertex.
     * @return Vertex& Reference to the start vertex.
     */
    Vertex &getStart();

    /**
     * @brief Get the start vertex (const version).
     * @return const Vertex& Const reference to the start vertex.
     */
    const Vertex &getStart() const;

    /**
     * @brief Get the end vertex.
     * @return Vertex& Reference to the end vertex.
     */
    Vertex &getEnd();

    /**
     * @brief Get the end vertex (const version).
     * @return const Vertex& Const reference to the end vertex.
     */
    const Vertex &getEnd() const;

    /**
     * @brief Get the weight of the edge.
     * @return size_t& Reference to the edge's weight.
     */
    size_t &getWeight();

    /**
     * @brief Get the weight of the edge (const version).
     * @return size_t The weight of the edge.
     */
    size_t getWeight() const;

    /**
     * @brief Get the vertex at the other end of the edge given a vertex.
     * 
     * @param v The vertex at one end of the edge.
     * @return Vertex& Reference to the opposite vertex.
     */
    Vertex &getOther(Vertex v);

    /**
     * @brief Get the vertex at the other end of the edge given a vertex (const version).
     * 
     * @param v The vertex at one end of the edge.
     * @return const Vertex& Const reference to the opposite vertex.
     */
    const Vertex &getOther(Vertex v) const;

    /**
     * @brief Check if the edge contains the specified vertex.
     * 
     * @param target The vertex to check for.
     * @return true If the edge contains the vertex.
     * @return false If the edge does not contain the vertex.
     */
    bool contains(Vertex &target) const;

    /**
     * @brief Equality operator to compare two edges.
     * 
     * @param other The edge to compare with.
     * @return true If the two edges are equal.
     * @return false If the two edges are not equal.
     */
    bool operator==(const Edge &other) const;

    /**
     * @brief Assignment operator to assign one edge to another.
     * 
     * @param other The edge to assign from.
     * @return Edge& Reference to the current edge.
     */
    Edge &operator=(const Edge &other);

    /**
     * @brief Less-than operator for comparing two edges based on their weight.
     * 
     * @param other The edge to compare with.
     * @return true If the current edge's weight is less than the other edge's weight.
     * @return false Otherwise.
     */
    bool operator<(const Edge &other) const;

    /**
     * @brief Overload the output stream operator to print the edge in a readable format.
     * 
     * @param os The output stream object.
     * @param e The edge to print.
     * @return std::ostream& The modified output stream object.
     */
    friend std::ostream& operator<<(std::ostream &os, const Edge &e);
};

