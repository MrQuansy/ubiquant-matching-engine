//
// Created by Yongzao Dan on 2023/8/28.
//

#ifndef UBIQUANTMATCHINGENGINE_BINARY_FILE_COMPARE_H
#define UBIQUANTMATCHINGENGINE_BINARY_FILE_COMPARE_H

#include "../src/common.h"
#include "gtest/gtest.h"

#include <string>
#include <fstream>

void compareTWAPFiles(const std::string& file1, const std::string& file2);

void comparePNLFiles(const std::string& file1, const std::string& file2);

#endif //UBIQUANTMATCHINGENGINE_BINARY_FILE_COMPARE_H
