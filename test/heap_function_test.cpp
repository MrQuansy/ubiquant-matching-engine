//
// Created by Yongzao Dan on 2023/8/25.
//


#include "../src/common.h"
#include "../src/binary_heap.h"

#include <random>

const int N = 1E7;

int main() {
    std::srand(std::time(nullptr));

    printf("Test max BinaryHeap...\n");

    BinaryHeap<OrderLog, MaxBinaryHeapCmp> maxBinaryHeap(N);
    for (int i = 0; i < N; i++) {
        OrderLog orderLog = {
                std::rand(),
                std::rand(),
                (double) (std::rand() % 100000) / 100.0,
                std::rand() % 256,
                std::rand() % 256
        };
        maxBinaryHeap.insert(orderLog);
    }

    double lastPrice = 1E9;
    int lastTime = 0;
    while (!maxBinaryHeap.isEmpty()) {
        OrderLog orderLog = maxBinaryHeap.pop();
        if (_abs(lastPrice - orderLog.price) > EPS) {
            assert(lastPrice > orderLog.price);
        } else {
            assert(lastTime < orderLog.timestamp);
        }
        lastPrice = orderLog.price;
        lastTime = orderLog.timestamp;
    }

    printf("max BinaryHeap correct!\n");

    printf("Test min BinaryHeap...\n");

    BinaryHeap<OrderLog, MinBinaryHeapCmp> minBinaryHeap(N);
    for (int i = 0; i < N; i++) {
        OrderLog orderLog = {
                std::rand(),
                std::rand(),
                (double) (std::rand() % 100000) / 100.0,
                std::rand() % 256,
                std::rand() % 256
        };
        minBinaryHeap.insert(orderLog);
    }

    lastPrice = -1E9;
    lastTime = 0;
    while (!minBinaryHeap.isEmpty()) {
        OrderLog orderLog = minBinaryHeap.pop();
        if (_abs(lastPrice - orderLog.price) > EPS) {
            assert(lastPrice < orderLog.price);
        } else {
            assert(lastTime < orderLog.timestamp);
        }
        lastPrice = orderLog.price;
        lastTime = orderLog.timestamp;
    }

    printf("min BinaryHeap correct!\n");
}