//
// Created by 陈彦泽 on 2023/8/29.
//

#include "../common.h"
#include <thread>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <arpa/inet.h>
#include <sys/stat.h>

const char *const PNL_DIR_PATH = "/mnt/output/pnl_and_position/";
const char *const TWAP_DIR_PATH = "/mnt/output/twap_order/";
const long BUFFER_SIZE = 1 * 1024 * 1024; // 1MB buffer

void handleClient(int clientSocket) {
    std::cout << BUFFER_SIZE << std::endl;
    char buffer[BUFFER_SIZE];
    char nameBuffer[13];
    if (recv(clientSocket, nameBuffer, 12, 0) <= 0) {
        // 处理接收错误
        close(clientSocket);
        return;
    }
    nameBuffer[12] = '\0';
    std::string fileName(nameBuffer);
    // 从 clientSocket 读取数据并发送响应
    // 接受 twap
    long dataSize;
    long totalRecSize = 0;
    int tmpRecSize = 0;
    if (recv(clientSocket, &dataSize, sizeof(dataSize), 0) <= 0) {
        // 处理接收错误
        std::cerr << "Failed to rec twapOrderSize" << std::endl;
        close(clientSocket);
        return;
    }
    std::cout << "Start receive file..." << fileName << std::endl;
    std::ofstream twapOutputFile(TWAP_DIR_PATH + fileName, std::ios::binary);
    if (!twapOutputFile.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }
    while (totalRecSize < dataSize) {
        if ((tmpRecSize = recv(clientSocket, buffer, std::min(BUFFER_SIZE, dataSize - totalRecSize), 0)) <= 0) {
            // 处理接收错误
            std::cerr << "Failed to rec twap" << std::endl;
            close(clientSocket);
            return;
        }
        twapOutputFile.write(buffer, tmpRecSize);
        totalRecSize += tmpRecSize;
    }
    twapOutputFile.close();

    std::cout << "complete twap" << std::endl;

    // 接受 pnl
    totalRecSize = 0;
    if (recv(clientSocket, &dataSize, sizeof(dataSize), 0) <= 0) {
        // 处理接收错误
        std::cerr << "Failed to rec pnlSize" << std::endl;
        close(clientSocket);
        return;
    }
    std::ofstream pnlOutputFile(PNL_DIR_PATH + fileName, std::ios::binary);
    if (!pnlOutputFile.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }
    while (totalRecSize < dataSize) {
        if ((tmpRecSize = recv(clientSocket, buffer, std::min(BUFFER_SIZE, dataSize - totalRecSize), 0)) <= 0) {
            // 处理接收错误
            std::cerr << "Failed to rec pnl" << std::endl;
            close(clientSocket);
            return;
        }
        pnlOutputFile.write(buffer, tmpRecSize);
        totalRecSize += tmpRecSize;
    }
    pnlOutputFile.close();

    std::cout << "complete pnl" << std::endl;
    // 关闭客户端连接
    close(clientSocket);
}

int main() {
    mkdir("/mnt/output/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir(TWAP_DIR_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir(PNL_DIR_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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
    serverAddr.sin_addr.s_addr = inet_addr("172.28.142.141");

    // 绑定地址和端口
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
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
        int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &clientAddrSize);
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
