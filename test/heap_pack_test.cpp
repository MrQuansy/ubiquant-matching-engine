//
// Created by Yongzao Dan on 2023/8/25.
//

#include "../src/common.h"
#include "../src/binary_heap.h"

#include <random>

const int M = 100;
const int N = 1E6;

int main() {
    std::srand(std::time(nullptr));

    printf("Test pack of BinaryHeap...\n");

    long long totInsert = 0, totPop = 0;
    for (int round = 0; round < M; round++) {
        auto start = std::chrono::high_resolution_clock::now();
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
        auto bhInsertTime = std::chrono::high_resolution_clock::now() - start;
        totInsert += std::chrono::duration_cast<std::chrono::milliseconds>(bhInsertTime).count();

        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i++) {
            maxBinaryHeap.pop();
        }
        auto bhPopTime = std::chrono::high_resolution_clock::now() - start;
        totPop += std::chrono::duration_cast<std::chrono::milliseconds>(bhPopTime).count();

        printf("Round %d: BinaryHeap insert time for %d OrderLog: %lld ms\n", round, N, std::chrono::duration_cast<std::chrono::milliseconds>(bhInsertTime).count());
        printf("Round %d: BinaryHeap pop time for %d OrderLog: %lld ms\n", round, N, std::chrono::duration_cast<std::chrono::milliseconds>(bhPopTime).count());
    }

    printf("==============================================================================\n");
    printf("BinaryHeap avg insert time for %d OrderLog: %lld ms\n", N, totInsert / M);
    printf("BinaryHeap avg pop time for %d OrderLog: %lld ms\n", N, totPop / M);
}