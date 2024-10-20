#include "MST_Strategy.hpp"
#pragma once
#include <vector>
#include <stddef.h>

    Graph* Kruskal::operator()(Graph *g){ 
        Graph* mst = new Graph(*g, false); // Create a new graph with the same vertices as the input graph but no edges

        std::vector<Edge> edges;  // Create a vector to store the edges
        for (auto e = g->edgesBegin(); e != g->edgesEnd(); e++){
            edges.push_back(*e);
        }
        std::sort(edges.begin(), edges.end());  // Sort the edges in non decreasing order of weight

        UnionFind uf(g->numVertices());
         /* for each edge E = u,v in G taken in non decreasing order of weight,
            if u and v are not in the same set, add E to the MST */
        for (auto e : edges){
            if (uf.find(e.getStart().getId()) != uf.find(e.getEnd().getId())){
                mst->addEdge(e);
                uf.Union(e.getStart().getId(), e.getEnd().getId());
            }
        }
        std::vector<std::vector<size_t>> dist, per;
        // Get the distance and parent matrices of the MST
        std::tie(dist, per) = mst->floydWarshall();
        //update distance and parent matrices in mst
        mst->setDistances(dist);
        mst->setParent(per);
        return mst;
    }


                  ///////////////////////////////// Union - Find //////////////////////////////////


    UnionFind::UnionFind(size_t n): 
        n(n), 
        rank(n), 
        parent(n){ 

        makeSet(); } 
  
    // Creates n single item sets 
    void UnionFind::makeSet() { 
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
  
    /* 
      Do union of two sets by rank represented by x and y. 
    */
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

      

