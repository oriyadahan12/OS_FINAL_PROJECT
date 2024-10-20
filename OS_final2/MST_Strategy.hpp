#pragma once
#include "graph.cpp"

// Abstract base class for Minimum Spanning Tree (MST) strategies
class MST_Strategy
{
public:
    // Pure virtual function that will be implemented by derived classes
    // This function takes a pointer to a Graph and returns a pointer to a Graph
    virtual Graph* operator()(Graph *g) = 0;

    // Virtual destructor to ensure proper cleanup of derived classes
    virtual ~MST_Strategy() = default;
};
