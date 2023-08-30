//
// Created by Yongzao Dan on 2023/8/25.
//


#include "../src/common.h"
#include "../src/binary_heap.h"
#include "gtest/gtest.h"

#include <random>

const int N = 1E7;

class HeapFunctionTest : public ::testing::Test {};

TEST_F(HeapFunctionTest, heap_function_test) {
    std::srand(std::time(nullptr));

    printf("Test max BinaryHeap...\n");

    BinaryHeap maxBinaryHeap(N, new MaxBinaryHeapCmp());
    for (int i = 0; i < N; i++) {
        compact_order_log orderLog = {
                std::rand() % 86400,
                std::rand(),
                (double) (std::rand() % 100000) / 100.0,
                static_cast<unsigned char>(std::rand() % 256),
                static_cast<unsigned char>(((std::rand() % 2) << 3) | (std::rand() % 7))
        };
        maxBinaryHeap.insert(orderLog);
    }

    double lastPrice = 1E9;
    int lastTime = 0, lastType = 0;
    while (!maxBinaryHeap.isEmpty()) {
        compact_order_log orderLog = maxBinaryHeap.pop();
        if (_neq(lastPrice, orderLog.price)) {
            EXPECT_GT(lastPrice, orderLog.price);
        } else if (lastTime != orderLog.timestamp) {
            EXPECT_LT(lastTime, orderLog.timestamp);
        } else {
            EXPECT_LT(lastType, (orderLog.directionAndType & TYPE_MASK));
        }
        lastPrice = orderLog.price;
        lastTime = orderLog.timestamp;
        lastType = orderLog.directionAndType & TYPE_MASK;
    }

    printf("max BinaryHeap correct!\n");

    printf("Test min BinaryHeap...\n");

    BinaryHeap minBinaryHeap(N, new MinBinaryHeapCmp());
    for (int i = 0; i < N; i++) {
        compact_order_log orderLog = {
                std::rand() % 86400,
                std::rand(),
                (double) (std::rand() % 100000) / 100.0,
                static_cast<unsigned char>(std::rand() % 256),
                static_cast<unsigned char>(((std::rand() % 2) << 3) | (std::rand() % 7))
        };
        minBinaryHeap.insert(orderLog);
    }

    lastPrice = -1E9;
    lastTime = 0;
    while (!minBinaryHeap.isEmpty()) {
        compact_order_log orderLog = minBinaryHeap.pop();
        if (_neq(lastPrice, orderLog.price)) {
            EXPECT_LT(lastPrice, orderLog.price);
        } else if (lastTime != orderLog.timestamp) {
            EXPECT_LT(lastTime, orderLog.timestamp);
        } else {
            EXPECT_LT(lastType, (orderLog.directionAndType & TYPE_MASK));
        }
        lastPrice = orderLog.price;
        lastTime = orderLog.timestamp;
        lastType = orderLog.directionAndType & TYPE_MASK;
    }

    printf("min BinaryHeap correct!\n");
}