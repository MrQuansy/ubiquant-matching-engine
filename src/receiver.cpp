//
// Created by 陈彦泽 on 2023/8/27.
//
#include "common.h"
#include <iostream>
#include <vector>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>

const int NUM_THREADS = 1;
const int BASE_PORT = 20000;
const int BUFFER_SIZE = sizeof(OrderLog) * 100000;

void handleConnection(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("172.28.142.141");
    serverAddr.sin_port = htons(port);
    if (bind(serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding socket to port " << port << std::endl;
        close(serverSocket);
        return;
    }
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on port " << port << std::endl;
        close(serverSocket);
        return;
    }

    std::cout << "Listening on port " << port << std::endl;
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == -1) {
        std::cerr << "Error accepting connection" << std::endl;
    }
    char buffer[BUFFER_SIZE];
    // 接收数据块并重组成完整字节流
    std::vector<char> data_buffer;
    std::streamsize bytes_received;
    long long cnt = 0;
    while ((bytes_received = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        cnt += bytes_received;
    }
    std::cout << cnt / sizeof(OrderLog) << std::endl;


    close(serverSocket);
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(handleConnection, BASE_PORT + i);
    }
    for (auto &thread: threads) {
        thread.join();
    }
    return 0;
}