//
// Created by Quansiyi on 2023/8/29.
//

#ifndef UBIQUANTMATCHINGENGINE_IO_H
#define UBIQUANTMATCHINGENGINE_IO_H

#include "common.h"
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

#define INCR(INDEX) ((INDEX+1) & (BUFFER_NUM -1))
#define ALL_WORKER_BIT  (1 << WORKER_THREAD_NUM) - 1
#define WORKER_BIT(id)  1 << id

enum buffer_flag{FILE_HEAD, FILE_END, IN_PROCESS};
struct order_buffer {
    void *order_data;
    void *alpha_data;
    void *prev_info_data;
    std::atomic<uint8_t> finish_bit;
    volatile uint32_t prev_count;
    volatile uint32_t alpha_count;
    volatile uint32_t order_count;
    volatile buffer_flag flag;
    std::string path;

    order_buffer() : prev_count(0), alpha_count(0), order_count(0), finish_bit(ALL_WORKER_BIT), flag(IN_PROCESS) {
        if (posix_memalign(&order_data, ALIGN_ORDER_BUFFER_SIZE, ALIGN_ORDER_BUFFER_SIZE) != 0) {
            std::cerr << "Memory allocation failed" << std::endl;
        }
        if (posix_memalign(&alpha_data, ALIGN_ALPHA_BUFFER_SIZE, ALIGN_ALPHA_BUFFER_SIZE) != 0) {
            std::cerr << "Memory allocation failed" << std::endl;
        }
        if (posix_memalign(&prev_info_data, ALIGN_PREV_BUFFER_SIZE, ALIGN_PREV_BUFFER_SIZE) != 0) {
            std::cerr << "Memory allocation failed" << std::endl;
        }
    }
};

order_buffer buffers[BUFFER_NUM];

void load_path_list(const std::string &dir_path, std::vector<std::string> &path_list, const char* start_date) {
    DIR *pDir;
    struct dirent *ptr;
    if (!(pDir = opendir(dir_path.c_str()))) {
        std::cerr << "Folder doesn't Exist!" << std::endl;
        return;
    }

    while ((ptr = readdir(pDir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
            if(start_date!= nullptr && strcmp(ptr->d_name,start_date)<0) continue;
            path_list.push_back(dir_path + ptr->d_name);
        }
    }
    sort(path_list.begin(), path_list.end());
    closedir(pDir);
}

int32_t direct_io_load(const std::string &path, int buffer_index) {
    std::cout << "[I/O Thread] Start loading file: " << path << std::endl;
    time_t start_time = now();
    std::string order_file_name = path + ORDER_LOG;
    std::string alpha_file_name = path + ALPHA;
    std::string prev_info_file_name = path + PREV_TRADE_INFO;
    int fd[3];
    if ((fd[0] = open(prev_info_file_name.c_str(), O_RDONLY | O_DIRECT)) < 0) {
        std::cerr << "open file error! " << order_file_name << std::endl;
    }
    if ((fd[1] = open(alpha_file_name.c_str(), O_RDONLY | O_DIRECT)) < 0) {
        std::cerr << "open file error! " << order_file_name << std::endl;
    }
    if ((fd[2] = open(order_file_name.c_str(), O_RDONLY | O_DIRECT)) < 0) {
        std::cerr << "open file error! " << order_file_name << std::endl;
    }

//    if((fd[0] = open(prev_info_file_name.c_str(), O_RDONLY))<0){
//        std::cerr<<"open file error! "<<order_file_name<<std::endl;
//    }
//    if((fd[1] = open(alpha_file_name.c_str(), O_RDONLY))<0){
//        std::cerr<<"open file error! "<<order_file_name<<std::endl;
//    }
//    if((fd[2] = open(order_file_name.c_str(), O_RDONLY))<0){
//        std::cerr<<"open file error! "<<order_file_name<<std::endl;
//    }

    time_t total_waiting_time = 0;
    time_t start_waiting_time = now();
    while (buffers[buffer_index].finish_bit != ALL_WORKER_BIT);
    total_waiting_time += now() - start_waiting_time;

    order_buffer * b = &buffers[buffer_index];
    b->flag = FILE_HEAD;
    b->path = path.substr(path.size()-DATE_LENGTH);

    ssize_t bytesRead = 0;
    uint32_t offset = 0;
    while ((bytesRead = pread(fd[0], b->prev_info_data, ALIGN_PREV_BUFFER_SIZE, offset)) > 0) {
        offset += bytesRead;
    }
    close(fd[0]);

    b->prev_count = offset / sizeof(prev_trade_info);

    offset = 0;
    while ((bytesRead = pread(fd[1], b->alpha_data, ALIGN_ALPHA_BUFFER_SIZE, offset)) > 0) {
        offset += bytesRead;
    }
    close(fd[1]);

    b->alpha_count = offset / sizeof(alpha);

    offset = 0;
    while ((bytesRead = pread(fd[2], b->order_data, READ_BUFFER_SIZE, offset)) > 0) {
        offset += bytesRead;
        b->order_count = bytesRead / sizeof(order_log);
        //std::cout<< "[I/O Thread] Finish loading order log segment: "<< buffer_index <<std::endl;
        b->finish_bit = 0;
        buffer_index = INCR(buffer_index);
        start_waiting_time = now();
        while (buffers[buffer_index].finish_bit != ALL_WORKER_BIT);
        total_waiting_time += now() - start_waiting_time;
        b = &buffers[buffer_index];
        b->flag = IN_PROCESS;
        b->order_count = 0;
    }

    b->flag = FILE_END;
//    std::cout<< "[I/O Thread] Finish loading order log segment: "<< buffer_index <<std::endl;
    b->finish_bit=0;


    close(fd[2]);
    std::cout << "[I/O Thread] Throughput: " << offset / 1024.0 / 1024 / (now() - start_time) * SECOND_TO_NANO
              << " mb/s" << std::endl;
    std::cout << "[I/O Thread] Total waiting time: " << total_waiting_time / MILLI_TO_NANO << " ms" << std::endl;
    std::cout << "[I/O Thread] End loading file: " << path << ". Time cost: " << (now() - start_time) / MILLI_TO_NANO
              << "ms" << std::endl;
    return INCR(buffer_index);
}

#endif //UBIQUANTMATCHINGENGINE_IO_H
