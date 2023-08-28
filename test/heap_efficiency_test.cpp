//
// Created by Yongzao Dan on 2023/8/25.
//

#include "../src/common.h"
#include "../src/binary_heap.h"
#include "gtest/gtest.h"

#include <queue>

const int N = 1E7;

class HeapEfficiencyTest : public ::testing::Test {};

TEST_F(HeapEfficiencyTest, heap_efficiency_test) {
    std::srand(std::time(nullptr));

    printf("Test efficiency of BinaryHeap...\n");

    auto start = std::chrono::high_resolution_clock::now();
    BinaryHeap maxBinaryHeap(N, new MaxBinaryHeapCmp());
    for (int i = 0; i < N; i++) {
        OrderLog orderLog = {
                std::rand(),
                std::rand(),
                (double) (std::rand() % 100000) / 100.0,
                static_cast<unsigned char>(std::rand() % 256),
                static_cast<unsigned char>(std::rand() % 256)
        };
        maxBinaryHeap.insert(orderLog);
    }
    auto bhInsertTime = std::chrono::high_resolution_clock::now() - start;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        maxBinaryHeap.pop();
    }
    auto bhPopTime = std::chrono::high_resolution_clock::now() - start;

    std::priority_queue<OrderLog, std::vector<OrderLog>, MinBinaryHeapCmp> pq;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        //pq.push(std::rand());
        OrderLog orderLog = {
                std::rand(),
                std::rand(),
                (double) (std::rand() % 100000) / 100.0,
                static_cast<unsigned char>(std::rand() % 256),
                static_cast<unsigned char>(std::rand() % 256)
        };
        pq.push(orderLog);
    }
    auto pqInsertTime = std::chrono::high_resolution_clock::now() - start;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        pq.pop();
    }
    auto pqPopTime = std::chrono::high_resolution_clock::now() - start;

    printf("BinaryHeap insert time for %d OrderLog: %lld ms\n", N, std::chrono::duration_cast<std::chrono::milliseconds>(bhInsertTime).count());
    printf("priority_queue insert time for %d OrderLog: %lld ms\n", N, std::chrono::duration_cast<std::chrono::milliseconds>(pqInsertTime).count());
    printf("BinaryHeap pop time for %d OrderLog: %lld ms\n", N, std::chrono::duration_cast<std::chrono::milliseconds>(bhPopTime).count());
    printf("priority_queue pop time for %d OrderLog: %lld ms\n", N, std::chrono::duration_cast<std::chrono::milliseconds>(pqPopTime).count());
}