//
// Created by 陈彦泽 on 2023/8/30.
//
#include "file_comparator.h"
#include <iostream>
#include <string>
#include <cstring>
#include <dirent.h>
#include <fstream>

void filesHaveSameContent(const std::string& pathA, const std::string& pathB) {
    std::ifstream fileA(pathA, std::ios::binary);
    std::ifstream fileB(pathB, std::ios::binary);

    if (!fileA.is_open() || !fileB.is_open()) {
        std::cout << "Files with different content: " << pathA << std::endl;
        return;
    }

    char chA, chB;
    int cnt = 0;
    while (fileA.get(chA) && fileB.get(chB)) {
        cnt ++;
        if (chA != chB) {
            std::cout << "Files with different content: " << pathA <<" at pos: " << cnt << std::endl;

            return;
        }
    }
}


void findDifferentTWAPFiles(const std::string& dirA, const std::string& dirB) {
    DIR *dir = opendir(dirB.c_str());
    if (dir == nullptr) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) { // 判断是否是普通文件
            std::string fileName = entry->d_name;
            std::string filePathA = dirA + "/" + fileName;
            std::string filePathB = dirB + "/" + fileName;
//            filesHaveSameContent(filePathB, filePathA);
            compareTWAPFiles(filePathB, filePathA);
        }
    }

    closedir(dir);
}

void findDifferentPNLFiles(const std::string& dirA, const std::string& dirB) {
    DIR *dir = opendir(dirB.c_str());
    if (dir == nullptr) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) { // 判断是否是普通文件
            std::string fileName = entry->d_name;
            std::string filePathA = dirA + "/" + fileName;
            std::string filePathB = dirB + "/" + fileName;
//            filesHaveSameContent(filePathB, filePathA);
            comparePNLFiles(filePathB, filePathA);
        }
    }

    closedir(dir);
}


int main() {
    std::string dirA = "/mnt/answer/";
    std::string dirB = "/mnt/output/";

    findDifferentPNLFiles(dirA + "pnl_and_position", dirB + "pnl_and_position");
    findDifferentTWAPFiles(dirA + "twap_order", dirB + "twap_order");

    return 0;
}
