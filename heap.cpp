
#include <vector>
#include "heap.hpp"

static size_t heap_parent(size_t i) { 
    return (i + 1) / 2 - 1;
}

static size_t heap_left(size_t i) { 
    return i * 2 + 1; 
}

static size_t heap_right(size_t i) { 
    return i * 2 + 2; 
}

// when a node becomes greater than its parent
static void heap_up(HeapItem* arr, size_t pos) {
    HeapItem t = arr[pos];
    while (pos > 0 && arr[heap_parent(pos)].val > t.val) {
        // swap with parent
        arr[pos] = arr[heap_parent(pos)];
        *arr[pos].ref = pos;
        pos = heap_parent(pos);
    }

    arr[pos] = t;
    *arr[pos].ref = pos;
}

// if node is geater than its children
static void heap_down(HeapItem* arr, size_t pos, size_t len) {
    HeapItem t = arr[pos];
    while (true) {
        // find smallest kid
        size_t l = heap_left(pos);
        size_t r = heap_right(pos);
        size_t min_pos = pos;
        uint64_t min_val = t.val;
        if (l < len && arr[l].val < min_val) {
            min_pos = l;
            min_val = arr[l].val;
        }
        if (r < len && arr[r].val < min_val) {
            min_pos = r;
        }
        if (min_pos == pos) {
            break;
        }
        // swap with kid
        arr[pos] = arr[min_pos];
        *arr[pos].ref = pos;
        pos = min_pos;
    }
    arr[pos] = t;
    *arr[pos].ref = pos;

}

void heap_update(HeapItem* arr, size_t pos, size_t len) {
    if (pos > 0 && arr[heap_parent(pos)].val > arr[pos].val) {
        heap_up(arr, pos);
    } else {
        heap_down(arr, pos, len);
    }
}