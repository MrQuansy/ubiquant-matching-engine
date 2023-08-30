//
// Created by Yongzao Dan on 2023/8/30.
//

#include "gtest/gtest.h"
#include "../src/common.h"
#include "../src/trade_engine.h"
#include "file_comparator.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class SingleContractTest : public ::testing::Test {};

const static std::string DEBUG_DATE = "20150101";
const static char* DEBUG_INSTRUMENT = "000.UBE";
const static std::pair<int, int> DEBUG_SESSION = SESSIONS[0];

/** Test trade log for a single contract and a single Session output */
TEST_F(SingleContractTest, single_contract_test) {
    // Init engines
    TradeEngine tradeEngine(DEBUG_SESSION,DEBUG_DATE);

    // Read and init prev_trade_info
    prev_trade_info prevTradeInfo{};
    std::ifstream prevTradeInfoFile(DATA_PREFIX + DEBUG_DATE + PREV_TRADE_INFO, std::ios::binary);
    while (prevTradeInfoFile.read(reinterpret_cast<char*>(&prevTradeInfo), sizeof(prev_trade_info))) {
        if (prevTradeInfo.instrument_id == *(long*)DEBUG_INSTRUMENT) {
            tradeEngine.initContract(
                    prevTradeInfo.instrument_id,
                    prevTradeInfo.prev_close_price,
                    prevTradeInfo.prev_position
            );
        }
    }
    prevTradeInfoFile.close();
    std::cout << DEBUG_DATE << " prev_trade_info read and init successfully" << std::endl;

    // Read and insert alpha
    alpha a{};
    std::ifstream alphaFile(DATA_PREFIX + DEBUG_DATE + ALPHA, std::ios::binary);
    while (alphaFile.read(reinterpret_cast<char*>(&a), sizeof(alpha))) {
        if (a.instrument_id == *(long*)DEBUG_INSTRUMENT) {
            tradeEngine.insertAlpha(
                    a.instrument_id,
                    a.timestamp,
                    a.target_volume
            );
        }
    }
    alphaFile.close();
    std::cout << DEBUG_DATE << " alpha read and insert successfully" << std::endl;

    // Read and insert order_log
    order_log orderLog{};
    std::ifstream orderLogFile(DATA_PREFIX + DEBUG_DATE + ORDER_LOG, std::ios::binary);
    while (orderLogFile.read(reinterpret_cast<char*>(&orderLog), sizeof(order_log))) {
        if (orderLog.instrument_id == *(long*)DEBUG_INSTRUMENT) {
            tradeEngine.insertOrderLog(
                    orderLog.instrument_id,
                    orderLog.timestamp,
                    orderLog.type,
                    orderLog.direction,
                    orderLog.volume,
                    orderLog.price_off
            );
        }
    }
    orderLogFile.close();
    std::cout << DEBUG_DATE << " order_log read and insert successfully" << std::endl;

    // Complete today
    tradeEngine.onComplete();
    std::cout << DEBUG_DATE << " trading successfully" << std::endl;

    compareLogFiles(
            DEBUG_PREFIX + DEBUG_DATE + "_" + DEBUG_INSTRUMENT,
            STD_LOG_PREFIX + DEBUG_DATE + "/" +
            "num" + std::to_string(DEBUG_SESSION.first) + "_" +
            "length" + std::to_string(DEBUG_SESSION.second) + "_" +
            DEBUG_INSTRUMENT);
}