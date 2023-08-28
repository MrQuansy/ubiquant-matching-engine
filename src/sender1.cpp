//
// Created by 陈彦泽 on 2023/8/27.
//
#include "common.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


long long getSystemMillis() {
    struct timespec ts{};
    (void) clock_gettime(CLOCK_REALTIME, &ts);
    long long milliseconds = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    return milliseconds;
}


const int NUM_THREADS = 1;
const int BASE_PORT = 20000;
const char *const FILE_PATH = "/mnt/data/20150101/order_log";
const int BUFFER_SIZE = 64 * 1024 * 1024;

void sendFile(int port, int index) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("172.28.142.141"); // Change to the receiver's IP address
    serverAddr.sin_port = htons(port);
    if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server on port " << port << std::endl;
        close(clientSocket);
        return;
    }

    // Read order_log
    int fileDescriptor = open(FILE_PATH, O_RDONLY | O_DIRECT);
    if (fileDescriptor == -1) {
        std::cerr << "Error opening file" << std::endl;
    }
    void* buffer;
    if (posix_memalign(&buffer, BUFFER_SIZE, BUFFER_SIZE) != 0) {
        std::cerr <<"Memory allocation failed"<< std::endl;
    }
    ssize_t bytesRead = 0;
    long long offset = 0;
    while ((bytesRead = pread(fileDescriptor, buffer, BUFFER_SIZE, offset))>0) {
        if (send(clientSocket, buffer, bytesRead, 0) == -1) {
            std::cerr << "Failed to send data" << std::endl;
            break;
        }
        offset += bytesRead;
    }
    close(fileDescriptor);
    free(buffer);
    std::cout << "port：" << port << " 传输完成, offset=" <<  offset <<  std::endl;
}

int main() {
    auto start = getSystemMillis();
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(sendFile, BASE_PORT + i, i);
    }
    for (auto &thread: threads) {
        thread.join();
    }

    auto end = getSystemMillis();
    std::cout << "总耗时：" << end - start << " 毫秒" << std::endl;
    return 0;
}