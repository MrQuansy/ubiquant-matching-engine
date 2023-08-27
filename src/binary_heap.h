//
// Created by Yongzao Dan on 2023/8/25.
//

#ifndef UBIQUANTMATCHINGENGINE_BINARYHEAP_H
#define UBIQUANTMATCHINGENGINE_BINARYHEAP_H

#include <cstdio>

struct BinaryHeap {

    OrderLog* heapArray;
    int capacity;
    int _size;
    Compare* cmp;

    BinaryHeap(int capacity, Compare* cmp) : capacity(capacity), _size(0), cmp(cmp) {
        heapArray = new OrderLog[capacity + 5];
    }

    ~BinaryHeap() {
        delete[] heapArray;
    }

    int size() const {
        return _size;
    }

    bool isEmpty() const {
        return _size == 0;
    }

    void insert(const OrderLog& value) {
        int index = ++_size;
        for (int father = index >> 1;
            index > 1 && (*cmp)(value, heapArray[father]);
            index = father, father >>= 1) {
            heapArray[index] = heapArray[father];
        }
        heapArray[index] = value;
    }

    OrderLog top() const {
        return heapArray[1];
    }

    OrderLog* topPtr() const {
        return &heapArray[1];
    }

    OrderLog pop() {
        OrderLog top = heapArray[1];

        int index = 1;
        OrderLog last = heapArray[_size--];
        for (int maxChild = 2; maxChild <= _size; index = maxChild, maxChild <<= 1) {
            if (maxChild < _size && (*cmp)(heapArray[maxChild + 1], heapArray[maxChild])) {
                maxChild++;
            }

            if ((*cmp)(last, heapArray[maxChild])) {
                break;
            } else {
                heapArray[index] = heapArray[maxChild];
            }
        }

        heapArray[index] = last;
        return top;
    }

    // Test Only
    void printPrice() {
        for (int i = 1; i <= _size; i++) {
            std::printf("%lf ", heapArray[i].price);
        }
        std::printf("\n");
    }
};

#endif //UBIQUANTMATCHINGENGINE_BINARYHEAP_H