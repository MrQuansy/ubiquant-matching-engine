//
// Created by 陈彦泽 on 2023/8/29.
//

#ifndef UBIQUANTMATCHINGENGINE_CLIENT_H
#define UBIQUANTMATCHINGENGINE_CLIENT_H

#include "../common.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

inline void sentResult(std::string &fileName, twap_order *twaps, int twapOrderSize, pnl_and_pos *pnls, int pnlSize) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("172.28.142.141"); // Change to the receiver's IP address
    serverAddr.sin_port = htons(20222);
    if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server on port " << 20222 << std::endl;
        close(clientSocket);
        return;
    }
    // send filename
    if (fileName.size() != 12) {
        std::cerr << "String length must be " << 12 << " characters." << std::endl;
        return;
    }
    if (send(clientSocket, fileName.c_str(), 12, 0) == -1) {
        std::cerr << "Failed to send filename" << std::endl;
        close(clientSocket);
        return;
    }

    // send twap
    long totalBytes = twapOrderSize * sizeof(twap_order);
    if (send(clientSocket, reinterpret_cast<char *>(&totalBytes), sizeof(totalBytes), 0) == -1) {
        std::cerr << "Failed to send twapOrderSize" << std::endl;
        close(clientSocket);
        return;
    }
    if (send(clientSocket, reinterpret_cast<char *>(twaps), totalBytes, 0) == -1) {
        std::cerr << "Failed to send twaps" << std::endl;
        close(clientSocket);
        return;
    }
    // send pnl
    totalBytes = pnlSize * sizeof(pnl_and_pos);
    if (send(clientSocket, reinterpret_cast<char *>(&totalBytes), sizeof(totalBytes), 0) == -1) {
        std::cerr << "Failed to send pnlSize" << std::endl;
        return;
    }
    if (send(clientSocket, reinterpret_cast<char *>(pnls), totalBytes, 0) == -1) {
        std::cerr << "Failed to send pnls" << std::endl;
        return;
    }
}

#endif //UBIQUANTMATCHINGENGINE_CLIENT_H
