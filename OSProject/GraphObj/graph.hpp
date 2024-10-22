#pragma once
#include "vertex.hpp"
#include "edge.hpp"
#include <map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <queue>
#include <cstddef>
#include <memory>
#define INF static_cast<size_t>(-1)

class Graph
{

private:
    // Map to store vertices by their IDs
    std::map<int, Vertex> vertices;
    // Set to store edges in the graph
    std::unordered_set<Edge, std::hash<Edge>> edges;

    std::vector<std::vector<size_t>> distances;  // Matrix to store the distances between vertices
    std::vector<std::vector<size_t>> parent;  // Matrix to store the parent of each vertex in the shortest path

    // Get the longest path in the graph given the distances
    std::string longestPath(const std::vector<std::vector<size_t>> &dist) const;
    double avgDistance(const std::vector<std::vector<size_t>> &dist) const;
    // Get the shortest path in the graph given the distances
    std::string shortestPath(size_t start, size_t end, const std::vector<std::vector<size_t>> &dist, const std::vector<std::vector<size_t>> &parent) const;
    // Get the distances between vertices in the graph and the parent matrix
    std::string allShortestPaths(const std::vector<std::vector<size_t>> &dist, const std::vector<std::vector<size_t>> &parent) const;

    void cleanDistParent();

   
    


public:
    // Constructor to create an empty graph
    Graph();




    // Constructor to create a graph from a set of vertices that may already contain edges
    Graph(std::unordered_set<Vertex> inputVxs);

    //Copy constructor with option to not copy edges
    Graph(const Graph &other, bool copyEdges = false);

    // Get the number of vertices in the graph
    size_t numVertices() const;
    // Get the number of edges in the graph
    size_t numEdges() const;
    // Check if the graph has a vertex
    bool hasVertex(Vertex v) const;
    // Get an iterator for the start of edges in the graph
    std::unordered_set<Edge>::iterator edgesBegin();
    // Get an iterator for the end of edges in the graph
    std::unordered_set<Edge>::iterator edgesEnd();

    // Add an edge to the graph, the edge is directed from start to end
    void addEdge(Edge e);
    // Remove an edge from the graph
    void removeEdge(Edge e);
 
    //add edge to the graph by vertices
    void addEdge(Vertex &start, Vertex &end, size_t weight = 1);

    // Get an iterator for the vertices in the graph
    std::map<int, Vertex>::iterator begin();

    // Get an iterator for the end of the vertices in the graph
    std::map<int, Vertex>::iterator end();

    // Get the adjacency matrix of the graph
    std::vector<std::vector<size_t>> adjacencyMatrix() const;

    // Check if the graph is connected
    bool isConnected() const;

    // Get a vertex by its ID
    Vertex &getVertex(int id);
    const Vertex &getVertex(int id) const;

    // Get the distances between vertices in the graph and the parent matrix
    std::pair<std::vector<std::vector<size_t>>, std::vector<std::vector<size_t>>> getDistances() const;

    std::string stats() const;

    // Get total weight of the graph
    size_t totalWeight() const;
    
    void setDistances(std::vector<std::vector<size_t>>);
    void setParent(std::vector<std::vector<size_t>>);

     // Get the distances between vertices in the graph and the parent matrix
    std::pair<std::vector<std::vector<size_t>>, std::vector<std::vector<size_t>>> floydWarshall() const;

    std::string longestPath() const;
    std::string allShortestPaths() const;
    double avgDistance() const;




};


