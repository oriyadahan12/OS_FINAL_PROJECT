#include "Algo.hpp"
#include <limits>


// Comparator for priority queue (min-heap), compares based on the key values of vertices
struct CompareVertex
{
    bool operator()(std::pair<size_t, int> const &value_1, std::pair<size_t, int> const &value_2){
        return value_1.second < value_2.second; // Min-heap based on key values (second element of the pair)
    }
};


// Prim's algorithm implementation
Graph* Prim::operator()(Graph *g) {
    size_t V = g->numVertices(); // Number of vertices in the input graph

    // Create a new graph for the Minimum Spanning Tree (MST) with the same vertices but no edges
    Graph *mst = new Graph(*g, false);

    const int INTINF = std::numeric_limits<int>::max(); // Define infinity value for initialization

    // Fibonacci min Heap to select the next vertex with the minimum key value
    BinaryHeap<std::pair<size_t, int>, CompareVertex> minHeap;

    // Key values (weights) used to pick the minimum weight edge for each vertex
    std::vector<int> key(V, INTINF);

    // Array to store the parent of each vertex in the MST
    std::vector<int> parent(V, -1);

    // Boolean array to track vertices already included in the MST
    std::vector<bool> inMST(V, false);

    // Start from the first vertex (arbitrarily chosen as 0)
    size_t v_start = 0;
    key[v_start] = 0; // Initialize the key value of the start vertex
    for (auto v : *g) {
        minHeap.push({v.first, key[(size_t)v.first]}); // Push all vertices into the priority queue
    }

    // Main loop of Prim's algorithm
    while (!minHeap.empty()) {
        // Extract the vertex with the minimum key value
        auto minNode = minHeap.top();
        minHeap.pop();
        size_t u = minNode.first; // Vertex with minimum key value

        // Iterate over all edges of the vertex u (Adj[u])
        for (auto v = g->getVertex(u).adjBegin(); v != g->getVertex(u).adjEnd(); v++) {
            size_t vertex = v->first; // Get the vertex v adjacent to u
            int weight = v->second; // Get the weight of the edge (u, v)

            // If v is not yet in MST and the weight of (u, v) is less than key[v]
            if (!inMST[vertex] && weight < key[vertex]) {
                size_t index = minHeap.getIndex({vertex, key[vertex]}); // Get index of vertex in the heap
                key[vertex] = weight; // Update the key value of vertex v
                minHeap.decreaseKey(index, {vertex, key[vertex]}); // Decrease the key value of the vertex in the priority queue
                parent[vertex] = u; // Update parent[v]
            }
        }
        // Mark the vertex as included in the MST
        inMST[u] = true;
    }

    // Adding all edges to the MST
    for (size_t i = 0; i < V; i++) {
        if (parent[i] != -1) { // If parent[i] is valid
            mst->addEdge(g->getVertex((size_t)parent[i]), g->getVertex(i), (size_t)key[i]); // Add edge to MST
        }
    }

    // Get the distance and parent matrices of the MST using Floyd-Warshall algorithm
    std::vector<std::vector<size_t>> dist, per;
    std::tie(dist, per) = mst->floydWarshall();
    
    // Update distance and parent matrices in mst
    mst->setDistances(dist);
    mst->setParent(per);

    return mst; // Return the constructed MST
}



////////////////////////////////////////////////////////////////////////////////////



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
        std::vector<std::vector<size_t>> dist, parent;
        // Get the distance and parent matrices of the MST
        std::tie(dist, parent) = mst->floydWarshall();
        //update distance and parent matrices in mst
        mst->setDistances(dist);
        mst->setParent(parent);
        return mst;
    }

