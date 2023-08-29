cmake_minimum_required(VERSION 3.20)
project(UbiquantMatchingEngine)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(UbiquantMatchingEngine src/sender.cpp)
add_executable(receiver src/receiver.cpp)
add_executable(server src/server.cpp)
# Add the -pthread option to link with the pthread library
target_link_libraries(UbiquantMatchingEngine PRIVATE -pthread)
target_link_libraries(receiver PRIVATE -pthread)
