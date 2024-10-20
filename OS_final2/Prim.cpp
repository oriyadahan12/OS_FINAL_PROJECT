#include "MST_Strategy.hpp"
#include <limits>
#include <vector>
#include <stdexcept>
#include <algorithm> // For std::find
#include <map>


// Comparator for priority queue (min-heap), compares based on the key values of vertices
struct CompareVertex{
    bool operator()(std::pair<size_t, int> const &v1, std::pair<size_t, int> const &v2){
        return v1.second < v2.second; // Min-heap based on key values (second element of the pair)
    }
};

// Prim's algorithm based on the pseudocode
Graph* Prim::operator()(Graph *g){
    size_t V = g->numVertices();

    // Create a new graph with the same vertices as the input graph but no edges
    Graph *mst = new Graph(*g, false);

    const int INTINF = std::numeric_limits<int>::max(); // Infinity value for key values

    // Fibonacci min Heap to select the next vertex with the minimum key value
    BinaryHeap<std::pair<size_t, int>, CompareVertex> pq;

    // Key values (weights) used to pick the minimum weight edge
    std::vector<int> key(V, INTINF);

    // Array to store the parent of each vertex in the MST
    std::vector<int> parent(V, -1);

    // Boolean array to track vertices already included in the MST
    std::vector<bool> inMST(V, false);

    // Start from the first vertex (arbitrarily chosen as 0)
    size_t startVertex = 0;
    key[startVertex] = 0;
    for (auto v : *g){
        pq.push({v.first, key[(size_t)v.first]});
    }

    while (!pq.empty()){
        // Extract the vertex with the minimum key value
       auto minNode = pq.top();
       pq.pop();
       size_t u = minNode.first;

        // Iterate over all edges of the vertex u (Adj[u])
        for (auto v = g->getVertex(u).adjBegin(); v != g->getVertex(u).adjEnd(); v++){
            size_t vertex = v->first; // Get the vertex v adjacent to u
            int weight = v->second; // Get the weight of the edge (u, v)

            // If v is not yet in MST and the weight of (u, v) is less than key[v]
            if (!inMST[vertex]&& weight < key[vertex]){
                size_t index = pq.getIndex({vertex, key[vertex]});
                key[vertex] = weight; // Update the key value of vertex v
                pq.decreaseKey(index,{vertex,key[vertex]}); // Decrease the key value of the vertex in the priority queue
                parent[vertex] = u;        // Update parent[v]
            }
        }
       
         // Mark the vertex as included in the MST
        inMST[u] = true;
    }

    // Adding all edges to the MST
    for (size_t i = 0; i < V; i++){
        if (parent[i] != -1){
            mst->addEdge(g->getVertex((size_t)parent[i]), g->getVertex(i), (size_t)key[i]);
        }
    }

    std::vector<std::vector<size_t>> dist, per;
    std::tie(dist, per) = mst->floydWarshall(); // Get the distance and parent matrices of the MST
    // Update distance and parent matrices in mst
    mst->setDistances(dist);
    mst->setParent(per);

    return mst; // Return the MST
}



///////////// Binary Heap struct /////////////


template <typename T, typename Comparator = std::less<T>>
class BinaryHeap {
public:
    BinaryHeap() = default;

    void push(const T& value) {
        heap.push_back(value);
        indexMap[value] = heap.size() - 1;
        heapifyUp(heap.size() - 1);
    }

    void pop() {
        if (heap.empty()) {
            throw std::out_of_range("Heap is empty");
        }
        indexMap.erase(heap.front());
        std::swap(heap.front(), heap.back());
        heap.pop_back();
        if (!heap.empty()) {
            indexMap[heap.front()] = 0;
            heapifyDown(0);
        }
    }

    const T& top() const {
        if (heap.empty()) {
            throw std::out_of_range("Heap is empty");
        }
        return heap.front();
    }

    bool empty() const {
        return heap.empty();
    }

    size_t size() const {
        return heap.size();
    }

    size_t getIndex(const T& value) const {
        auto it = indexMap.find(value);
        if (it == indexMap.end()) {
            throw std::invalid_argument("Value not found in heap");
        }
        return it->second;
    }

    void decreaseKey(size_t index,const T& newKey) {
        if (index >= heap.size()) {
            throw std::out_of_range("Index out of range");
        }
        if (comp(heap[index], newKey)) {
            throw std::invalid_argument("New key is greater than current key");
        }
        heap[index] = newKey;
        indexMap[newKey] = index;
        heapifyUp(index);
    }

private:
    std::vector<T> heap;
    Comparator comp;
    std::map<T, size_t> indexMap;

    void heapifyUp(size_t index) {
        while (index > 0) {
            size_t parentIndex = (index - 1) / 2;
            if (!comp(heap[index], heap[parentIndex])) {
                break;
            }
            std::swap(heap[index], heap[parentIndex]);
            indexMap[heap[index]] = index;
            indexMap[heap[parentIndex]] = parentIndex;
            index = parentIndex;
        }
    }

    void heapifyDown(size_t index) {
        size_t leftChild, rightChild, smallest;
        while (true) {
            leftChild = 2 * index + 1;
            rightChild = 2 * index + 2;
            smallest = index;

            if (leftChild < heap.size() && comp(heap[leftChild], heap[smallest])) {
                smallest = leftChild;
            }
            if (rightChild < heap.size() && comp(heap[rightChild], heap[smallest])) {
                smallest = rightChild;
            }
            if (smallest == index) {
                break;
            }
            std::swap(heap[index], heap[smallest]);
            indexMap[heap[index]] = index;
            indexMap[heap[smallest]] = smallest;
            index = smallest;
        }
    }
};