//
// Created by Yongzao Dan on 2023/8/30.
//

#include "gtest/gtest.h"
#include "../src/common.h"
#include "../src/trade_engine.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class SingleContractTest : public ::testing::Test {};

void compareLogFiles(const std::string& actual, const std::string& expect);

const static std::string DEBUG_DATE = "20150101";
const static std::string DEBUG_INSTRUMENT = "000.UBE";
const static std::pair<int, int> DEBUG_SESSION = SESSIONS[0];

/** Test trade log for a single contract and a single Session output */
TEST_F(SingleContractTest, single_contract_test) {
    // Init engines
    TradeEngine tradeEngine(DEBUG_SESSION);

    // Read and init prev_trade_info
    prev_trade_info prevTradeInfo{};
    std::ifstream prevTradeInfoFile(DATA_PREFIX + DEBUG_DATE + PREV_TRADE_INFO, std::ios::binary);
    while (prevTradeInfoFile.read(reinterpret_cast<char*>(&prevTradeInfo), sizeof(prev_trade_info))) {
        if (std::strcmp(prevTradeInfo.instrument_id, DEBUG_INSTRUMENT.c_str()) == 0) {
            tradeEngine.initContract(
                    std::string(prevTradeInfo.instrument_id),
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
        if (std::strcmp(a.instrument_id, DEBUG_INSTRUMENT.c_str()) == 0) {
            tradeEngine.insertAlpha(
                    std::string(a.instrument_id),
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
        if (std::strcmp(orderLog.instrument_id, DEBUG_INSTRUMENT.c_str()) == 0) {
            tradeEngine.insertOrderLog(
                    std::string(orderLog.instrument_id),
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

void compareLogFiles(const std::string& actual, const std::string& expect) {
    std::ifstream ai(actual, std::ios::in);
    std::ifstream ei(expect, std::ios::in);

    std::string as, es;
    while (std::getline(ai, as) && std::getline(ei, es)) {

        EXPECT_TRUE(as == es);
        if (as != es) {
            std::cout << "Expected: " << es << std::endl;
            std::cout << "Actual: " << as << std::endl;
            exit(-1);
        }
    }

    std::cout << "All logs are the same!" << std::endl;
}