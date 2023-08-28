//
// Created by Yongzao Dan on 2023/8/28.
//

#include "binary_file_compare.h"

void compareTWAPFiles(const std::string& file1, const std::string& file2) {
    std::ifstream binaryFile1(file1, std::ios::binary);
    std::ifstream binaryFile2(file2, std::ios::binary);

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
}

void comparePNLFiles(const std::string& file1, const std::string& file2) {
    std::ifstream binaryFile1(file1, std::ios::binary);
    std::ifstream binaryFile2(file2, std::ios::binary);

    pnl_and_pos p1{}, p2{};
    while (binaryFile1.read(reinterpret_cast<char *>(&p1), sizeof(pnl_and_pos)) &&
           binaryFile2.read(reinterpret_cast<char *>(&p2), sizeof(pnl_and_pos))) {

        EXPECT_TRUE(p1 == p2);
        if (!(p1 == p2)) {
            std::cout << "Expected: " << p2 << std::endl;
            std::cout << "Actual: " << p1 << std::endl;
            exit(-1);
        }
    }
}