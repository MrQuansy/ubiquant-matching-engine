cmake_minimum_required(VERSION 3.20)
project(UbiquantMatchingEngine CXX)

# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# O3 optimization
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

# UbiquantMatchingEngine Configuration
# include_directories(.)
add_executable(UbiquantMatchingEngine
        src/binary_heap.h src/common.h src/trade_engine.h src/contract_engine.h src/contract_engine.cpp src/trade_engine.cpp)

# Google Test Configuration
set(GOOGLETEST_VERSION 1.14.0)
add_subdirectory(./googletest)
include_directories(./googletest/include)
add_executable(
        test_main test/test_main.cpp test/heap_function_test.cpp test/heap_efficiency_test.cpp test/heap_pack_test.cpp test/baseline_test.cpp
        src/binary_heap.h src/common.h src/trade_engine.h src/contract_engine.h src/contract_engine.cpp src/trade_engine.cpp test/test_main.cpp test/trade_engine_test.cpp test/binary_file_compare.h test/binary_file_compare.cpp)
target_link_libraries(test_main gtest)