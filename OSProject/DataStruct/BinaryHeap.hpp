#include <vector>
#include <stdexcept>
#include <algorithm> // For std::find
#include <map>

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