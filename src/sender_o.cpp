//
// Created by 陈彦泽 on 2023/8/27.
//
#include "common.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <chrono>
#include <sys/stat.h>


long long getSystemMillis() {
    struct timespec ts{};
    (void) clock_gettime(CLOCK_REALTIME, &ts);
    long long milliseconds = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    return milliseconds;
}


const int NUM_THREADS = 1;
const int BASE_PORT = 20000;
const char *const FILE_PATH = "/mnt/nettest/order_log_";
const int BUFFER_SIZE = sizeof(OrderLog) * 100000;

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
    int file_fd = open(FILE_PATH + std::to_string(index), O_RDONLY);
    if (file_fd == -1) {
        std::cerr << "Error opening file" << std::endl;
        close(file_fd);
        return;
    }
    struct stat stat_buf;
    fstat(file_fd, &stat_buf);

    off_t offset = 0;
    ssize_t total_sent = 0;

    while (total_sent < stat_buf.st_size) {
        ssize_t sent = sendfile(clientSocket, file_fd, &offset, stat_buf.st_size - total_sent);
        if (sent == -1) {
            std::cerr << "Error sending file" << std::endl;
            break;
        }
        total_sent += sent;
    }

    close(file_fd);
    close(clientSocket);
    std::cout << "port：" << port << " 传输完成" << std::endl;
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