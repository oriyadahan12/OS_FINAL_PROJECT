
  
#pragma once
#include <vector>
#include <stddef.h>

  
class UnionFind { 

private:
    size_t n; 
    std::vector<size_t> rank, parent;

  
public: 
    
    // Constructor to create and 
    // initialize sets of n items 
    UnionFind(size_t n);
  
    // Creates n single item sets 
    void makeSet();
  
    // Finds set of given item x 
    size_t find(size_t x);
  
    // Do union of two sets by rank represented 
    // by x and y. 
    void Union(size_t x, size_t y);
}; 
  
