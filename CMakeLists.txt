cmake_minimum_required(VERSION 3.20)
project(UbiquantMatchingEngine CXX)

# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# O3 optimization
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread")
endif()
#for profile
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -pthread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -pthread")

# UbiquantMatchingEngine Configuration
# include_directories(.)
add_executable(NetServer
        src/net/server.cpp)
find_package(Threads REQUIRED)
target_link_libraries(NetServer Threads::Threads)

add_executable(entrance src/binary_heap.h src/common.h src/trade_engine.h src/contract_engine.h src/config.h src/contract_engine.cpp src/trade_engine.cpp src/entrance.cpp)
add_executable(baseline_entrance src/binary_heap.h src/common.h src/trade_engine.h src/contract_engine.h src/config.h src/contract_engine.cpp src/trade_engine.cpp src/baseline_entrance.cpp)

# Google Test Configuration
set(GOOGLETEST_VERSION 1.14.0)
add_subdirectory(./googletest)
include_directories(./googletest/include)
add_executable(
        test_main test/test_main.cpp test/heap_function_test.cpp test/heap_efficiency_test.cpp test/heap_pack_test.cpp test/baseline_test.cpp
        src/binary_heap.h src/common.h src/trade_engine.h src/contract_engine.h src/contract_engine.cpp src/trade_engine.cpp test/trade_log_test.cpp test/file_comparator.h test/file_comparator.cpp test/single_contract_test.cpp)
target_link_libraries(test_main gtest)


add_executable(SenderTest
        test/net_sender_test.cpp src/net/client.h)

# Baseline Test
add_executable(BaselineTest
        test/baseline_test.cpp test/file_comparator.h test/file_comparator.cpp test/test_main.cpp
        src/binary_heap.h src/common.h src/trade_engine.h src/contract_engine.h src/contract_engine.cpp src/trade_engine.cpp)
target_link_libraries(BaselineTest gtest)


add_executable(ServerResultChecker
        test/file_comparator.h test/file_comparator.cpp test/result_check.cpp )
target_link_libraries(ServerResultChecker gtest)
