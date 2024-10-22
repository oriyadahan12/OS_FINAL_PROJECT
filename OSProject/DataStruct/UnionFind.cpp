
  
#include "UnionFind.hpp"    


    // Constructor to create and 
    // initialize sets of n items 
    UnionFind::UnionFind(size_t n): n(n), rank(n), parent(n)
    { 
        makeSet(); 
    } 
  
    // Creates n single item sets 
    void UnionFind::makeSet() 
    { 
        for (size_t i = 0; i < n; i++) { 
            parent[i] = i; 
        } 
    } 
  
    // Finds set of given item x 
    size_t UnionFind::find(size_t x) 
    { 
       // If i is the parent of itself 
    if (parent[x] == x) { 
  
        // Then i is the representative  
        return x; 
    } 
    else {  
  
        // Recursively find the representative. 
        size_t result = find(parent[x]); 
  
        // We cache the result by moving i’s node  
        // directly under the representative of this 
        // set 
        parent[x] = result; 
        
        // And then we return the result 
        return result; 
     } 
    } 
  
    // Do union of two sets by rank represented 
    // by x and y. 
    void UnionFind::Union(size_t x, size_t y) 
    { 
        // Find current sets of x and y 
        size_t xset = find(x); 
        size_t yset = find(y); 
  
        // If they are already in same set 
        if (xset == yset) 
            return; 
  
        // Put smaller ranked item under 
        // bigger ranked item if ranks are 
        // different 
        if (rank[xset] < rank[yset]) { 
            parent[xset] = yset; 
        } 
        else if (rank[xset] > rank[yset]) { 
            parent[yset] = xset; 
        } 
  
        // If ranks are same, then increment 
        // rank. 
        else { 
            parent[yset] = xset; 
            rank[xset] = rank[xset] + 1; 
        } 
    } 

  
