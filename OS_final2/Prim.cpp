#include "MST_Strategy.hpp"
#include <limits>
#include <vector>
#include <stdexcept>
#include <algorithm> 
#include <map>

// Comparator for priority queue (min-heap), compares based on the key values of vertices
struct CompareVertex {
    bool operator()(std::pair<size_t, int> const &v1, std::pair<size_t, int> const &v2) {
        return v1.second < v2.second; // Min-heap based on key values (second element of the pair)
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

///////////// Binary Heap struct /////////////

template <typename T, typename Comparator = std::less<T>>
class BinaryHeap {
public:
    BinaryHeap() = default; // Default constructor

    // Insert a new value into the heap
    void push(const T& val) {
        heap.push_back(val); // Add value to the end of the heap
        indexMap[val] = heap.size() - 1; // Store index of the new value
        heapifyUp(heap.size() - 1); // Restore heap property
    }

    // Remove the top value from the heap
    void pop() {
        if (heap.empty()) {
            throw std::out_of_range("Out of range: Heap is empty");
        }
        indexMap.erase(heap.front()); // Remove the top element from the index map
        std::swap(heap.front(), heap.back()); // Swap top and last elements
        heap.pop_back(); // Remove last element (previously top)
        if (!heap.empty()) {
            indexMap[heap.front()] = 0; // Update index of the new top
            heapifyDown(0); // Restore heap property
        }
    }

    // Get the top value of the heap
    const T& top() const {
        if (heap.empty()) {
            throw std::out_of_range("Out of range: Heap is empty");
        }
        return heap.front(); // Return top element
    }

    // Check if the heap is empty
    bool empty() const {
        return heap.empty();
    }

    // Get the size of the heap
    size_t size() const {
        return heap.size();
    }

    // Get the index of a value in the heap
    size_t getIndex(const T& val) const {
        auto it = indexMap.find(val);
        if (it == indexMap.end()) {
            throw std::invalid_argument("Cannot find the value in heap");
        }
        return it->second; // Return index of the value
    }

    // Decrease the key of a value at a given index
    void decreaseKey(size_t index, const T& newKey) {
        if (index >= heap.size()) {
            throw std::out_of_range("Index out of range: index bigger than heap");
        }
        if (comp(heap[index], newKey)) {
            throw std::invalid_argument("New key is greater than current key");
        }
        heap[index] = newKey; // Update the key value
        indexMap[newKey] = index; // Update index map
        heapifyUp(index); // Restore heap property
    }

private:
    std::vector<T> heap; // Vector to store heap elements
    Comparator comp; // Comparator for heap property
    std::map<T, size_t> indexMap; // Map to track indices of values

    // Restore heap property upwards
    void heapifyUp(size_t index) {
        while (index > 0) {
            size_t indx_parent = (index - 1) / 2; // Calculate parent index
            if (!comp(heap[index], heap[indx_parent])) {
                break; // If heap property is satisfied, break
            }
            // Swap current index with parent
            std::swap(heap[index], heap[indx_parent]);
            indexMap[heap[index]] = index; // Update index map
            indexMap[heap[indx_parent]] = indx_parent; // Update parent index
            index = indx_parent; // Move up to parent index
        }
    }

    // Restore heap property downwards
    void heapifyDown(size_t index) {
        size_t leftChild, rightChild, smallest;
        while (true) {
            leftChild = 2 * index + 1; // Left child index
            rightChild = 2 * index + 2; // Right child index
            smallest = index; // Assume current index is smallest

            // Check if left child exists and is smaller
            if (leftChild < heap.size() && comp(heap[leftChild], heap[smallest])) {
                smallest = leftChild;
            }
            // Check if right child exists and is smaller
            if (rightChild < heap.size() && comp(heap[rightChild], heap[smallest])) {
                smallest = rightChild;
            }
            if (smallest == index) {
                break; // If no swaps needed, break
            }
            // Swap current index with smallest
            std::swap(heap[index], heap[smallest]);
            indexMap[heap[index]] = index; // Update index map
            indexMap[heap[smallest]] = smallest; // Update smallest index
            index = smallest; // Move down to smallest index
        }
    }
};
