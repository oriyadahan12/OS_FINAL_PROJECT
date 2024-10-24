#include "data_structures.hpp"

UnionFind::UnionFind(size_t n): 
        n(n), 
        rank(n), 
        parent(n){ 

        create_set(); } 
  
    // Creates n single item sets 
    void UnionFind::create_set() { 
        for (size_t i = 0; i < n; i++) { 
            parent[i] = i; 
        } 
    } 
  
    // Finds set of given item x 
    size_t UnionFind::find(size_t x) { 
       // If i is the parent of itself 
    if (parent[x] == x) { 
        // Then i is the representative  
        return x; 
    } 
    else {  
        // Recursively find the representative. 
        size_t res = find(parent[x]); 
  
        // We cache the result by moving i’s node directly under the representative of this set 
        parent[x] = res; 
        
        // And then we return the result 
        return res; 
     } 
    } 
  
    // Do union of two sets by rank represented by x and y. 
    void UnionFind::Union(size_t x, size_t y) { 
        // Find current sets of x and y 
        size_t xp = find(x); 
        size_t yp = find(y); 

        // If they are already in same set 
        if (xp == yp) 
            return; 
  
        // Put smaller ranked item under bigger ranked item if ranks are different 
        if (rank[xp] < rank[yp]) { 
            parent[xp] = yp; 
        } 
        else if (rank[xp] > rank[yp]) { 
            parent[yp] = xp; 
        } 
        //If ranks are same, then increment rank. 
        else { 
            parent[yp] = xp; 
            rank[xp] = rank[xp] + 1; 
        } 
    } 


  
