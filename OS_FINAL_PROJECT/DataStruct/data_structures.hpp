#include <vector>
#include <stdexcept>
#include <algorithm> // For std::find
#include <map>
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
    void create_set();
  
    // Finds set of given item x 
    size_t find(size_t x);
  
    // Do union of two sets by rank represented 
    // by x and y. 
    void Union(size_t x, size_t y);
}; 
  

////////////////////////////////////////////////////////////////////////////////////////////////////////

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

