//
// Created by 陈彦泽 on 2023/8/29.
//

#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void handleClient(int clientSocket) {
    // 从 clientSocket 读取数据并发送响应
    // 先接收数据长度
    size_t dataSize;
    if (recv(clientSocket, &dataSize, sizeof(dataSize), 0) <= 0) {
        // 处理接收错误
        close(clientSocket);
        return;
    }

    // 根据长度接收实际数据
    std::vector<char> buffer(dataSize);
    if (recv(clientSocket, buffer.data(), dataSize, 0) <= 0) {
        // 处理接收错误
        close(clientSocket);
        return;
    }
    // 关闭客户端连接
    close(clientSocket);
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;

    // 创建 socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    // 配置服务器地址
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(20222); // 监听端口
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 绑定地址和端口
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding." << std::endl;
        close(serverSocket);
        return 1;
    }

    // 开始监听，允许最多10个等待连接
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server listening on port 20222..." << std::endl;

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);

        // 等待客户端连接
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (clientSocket == -1) {
            std::cerr << "Error accepting client." << std::endl;
            continue;
        }

        // 创建一个新线程处理客户端
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach(); // 分离线程，让它自行回收资源
    }

    // 关闭服务器 socket
    close(serverSocket);

    return 0;
}
