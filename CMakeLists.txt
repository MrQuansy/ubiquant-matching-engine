cmake_minimum_required(VERSION 3.25)
project(UbiquantMatchingEngine C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(UbiquantMatchingEngine
        src/binary_heap.h src/common.h test/heap_function_test.cpp test/heap_efficiency_test.cpp test/heap_pack_test.cpp)
