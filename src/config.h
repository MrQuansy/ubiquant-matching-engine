//
// Created by Quansiyi on 2023/8/31.
//

#ifndef UBIQUANTMATCHINGENGINE_CONFIG_H
#define UBIQUANTMATCHINGENGINE_CONFIG_H

#define WORKER_THREAD_NUM 5
#define ALIGN_ORDER_BUFFER_SIZE 1024*1024*512
#define ALIGN_ALPHA_BUFFER_SIZE 1024*1024*2
#define ALIGN_PREV_BUFFER_SIZE 1024*4
#define READ_BUFFER_SIZE 1024*1024*360
#define BUFFER_NUM 8

const static int MAX_TWAP_LENGTH = 480 * 5;

// Input path: DATA_PREFIX + DATE + FILE_NAME
const static std::string DATA_PREFIX = "/mnt/test_data/";
//const static std::string DATA_PREFIX = "/Users/mrquan/Desktop/quant/data/";
const static std::string ALPHA = "/alpha";
const static std::string ORDER_LOG = "/order_log";
const static std::string PREV_TRADE_INFO = "/prev_trade_info";

// Output Path: OUTPUT_PREFIX + DATE + FILE_NAME + SESSION
const static std::string TEST_OUTPUT_PREFIX = "/mnt/test_data/";
const static std::string STD_OUTPUT_PREFIX = "/mnt/output_adjuest";
const static std::string TWAP_ORDER = "/twap_order";
const static std::string PNL_AND_POSITION = "/pnl_and_position";

// Debug Fields
const static bool ENABLE_DEBUG_TRADE_LOG = false;
const static std::string DEBUG_PREFIX = "/mnt/logs/";
const static std::string STD_LOG_PREFIX = "/mnt/log_adjust/";

#endif //UBIQUANTMATCHINGENGINE_CONFIG_H
