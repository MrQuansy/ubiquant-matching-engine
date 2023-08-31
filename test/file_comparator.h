//
// Created by Yongzao Dan on 2023/8/28.
//

#ifndef UBIQUANTMATCHINGENGINE_FILE_COMPARATOR_H
#define UBIQUANTMATCHINGENGINE_FILE_COMPARATOR_H

#include "../src/common.h"
#include "gtest/gtest.h"

#include <string>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void compareTWAPFiles(const std::string& actual, const std::string& expect);

void comparePNLFiles(const std::string& actual, const std::string& expect);

void compareLogFiles(const std::string& actual, const std::string& expect);

void checkFileSize(const std::string& actual, const std::string& expect);

#endif //UBIQUANTMATCHINGENGINE_FILE_COMPARATOR_H
