//
// Created by Yongzao Dan on 2023/8/28.
//

#include "file_comparator.h"

void compareTWAPFiles(const std::string& file1, const std::string& file2) {
    std::ifstream binaryFile1(file1, std::ios::binary);
    std::ifstream binaryFile2(file2, std::ios::binary);

    std::cout << "checking file: "<< file1 << std::endl;
    checkFileSize(file1, file2);
    twap_order o1{}, o2{};
    while (binaryFile1.read(reinterpret_cast<char*>(&o1), sizeof(twap_order)) &&
           binaryFile2.read(reinterpret_cast<char*>(&o2), sizeof(twap_order))) {

        EXPECT_TRUE(o1 == o2);
        if (!(o1 == o2)) {
            std::cout << "Expected: " << o2 << std::endl;
            std::cout << "Actual: " << o1 << std::endl;
            exit(-1);
        }
    }
    binaryFile1.close();
    binaryFile2.close();
}

void comparePNLFiles(const std::string& file1, const std::string& file2) {
    std::ifstream binaryFile1(file1, std::ios::binary);
    std::ifstream binaryFile2(file2, std::ios::binary);

    std::cout << "checking file: "<< file1 << std::endl;
    checkFileSize(file1, file2);
    pnl_and_pos p1{}, p2{};
    while (binaryFile1.read(reinterpret_cast<char *>(&p1), sizeof(pnl_and_pos)) &&
           binaryFile2.read(reinterpret_cast<char *>(&p2), sizeof(pnl_and_pos))) {

        EXPECT_TRUE(p1 == p2);
        if (!(p1 == p2)) {
            std::cout << file2 << " Expected: " << p2 << std::endl;
            std::cout << file1 << " Actual: " << p1 << std::endl;
            exit(-1);
        }
    }
    binaryFile1.close();
    binaryFile2.close();
}

void compareLogFiles(const std::string& actual, const std::string& expect) {
    std::ifstream ai(actual, std::ios::in);
    std::ifstream ei(expect, std::ios::in);

    std::string as, es;
    checkFileSize(actual, expect);
    while (std::getline(ai, as) && std::getline(ei, es)) {
        EXPECT_TRUE(as == es);
        if (as != es) {
            std::cout << "Expected: " << es << std::endl;
            std::cout << "Actual: " << as << std::endl;
            exit(-1);
        }
    }
    ai.close();
    ei.close();
}

void checkFileSize(const std::string& actual, const std::string& expect){
    int fd1 = open(actual.c_str(), O_RDONLY);
    if (fd1 == -1) {
        std::cerr << "Error opening file" << std::endl;
    }
    struct stat stat_buf1;
    fstat(fd1, &stat_buf1);
    int fd2 = open(expect.c_str(), O_RDONLY);
    if (fd1 == -1) {
        std::cerr << "Error opening file" << std::endl;
    }
    struct stat stat_buf2;
    fstat(fd2, &stat_buf2);
    if(stat_buf1.st_size!=stat_buf2.st_size){
        std::cout << " Expected file size: " << stat_buf2.st_size << std::endl;
        std::cout << " Actual file size: " << stat_buf1.st_size << std::endl;
    }
    close(fd1);
    close(fd2);
}