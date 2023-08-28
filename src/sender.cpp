//
// Created by 陈彦泽 on 2023/8/27.
//
#include "common.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>


long long getSystemMillis() {
    struct timespec ts{};
    (void) clock_gettime(CLOCK_REALTIME, &ts);
    long long milliseconds = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    return milliseconds;
}


const int NUM_THREADS = 10;
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
    std::vector<OrderLog> orderLogs;
    std::ifstream orderLogFile(FILE_PATH+ std::to_string(index), std::ios::binary);
    char buffer[BUFFER_SIZE];
    long long cnt = 0;
    while (!orderLogFile.eof()) {
        orderLogFile.read(buffer, BUFFER_SIZE);
        int gcount = orderLogFile.gcount();
        if (send(clientSocket, buffer, gcount, 0) == -1) {
            std::cerr << "Failed to send data" << std::endl;
            break;
        }
        cnt += gcount;
    }
    orderLogFile.close();
    close(clientSocket);
    std::cout << "port：" << port << " 传输完成，大小：" << cnt << std::endl;
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