//
// Created by Yongzao Dan on 2023/8/28.
//

#include "gtest/gtest.h"
#include "../src/common.h"
#include "../src/trade_engine.h"
#include "file_comparator.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class TradeLogTest : public ::testing::Test {};

const static std::string DATES[] = {
        "20150101",
        "20160202",
        "20170303",
        "20180404",
        "20190505",
};

/** Test all trade log output */
TEST_F(TradeLogTest, trade_log_test) {
    for (auto &DATE : DATES) {
        for (auto &SESSION : SESSIONS) {
            // Init engine
            TradeEngine tradeEngine(SESSION,DATE);

            std::vector<unsigned long> instruments;

            // Read and init prev_trade_info
            prev_trade_info prevTradeInfo{};
            std::ifstream prevTradeInfoFile(DATA_PREFIX + DATE + PREV_TRADE_INFO, std::ios::binary);
            while (prevTradeInfoFile.read(reinterpret_cast<char*>(&prevTradeInfo), sizeof(prev_trade_info))) {
                prevTradeInfo.instrument_id;
                tradeEngine.initContract(
                        prevTradeInfo.instrument_id,
                        prevTradeInfo.prev_close_price,
                        prevTradeInfo.prev_position
                );
                instruments.push_back(prevTradeInfo.instrument_id);
            }
            prevTradeInfoFile.close();
            std::cout << DATE << "_" + std::to_string(SESSION.first) + "_" + std::to_string(SESSION.second)
                      << " prev_trade_info read and init successfully" << std::endl;

            // Read and insert alpha
            alpha a{};
            std::ifstream alphaFile(DATA_PREFIX + DATE + ALPHA, std::ios::binary);
            while (alphaFile.read(reinterpret_cast<char*>(&a), sizeof(alpha))) {
                tradeEngine.insertAlpha(
                        a.instrument_id,
                        a.timestamp,
                        a.target_volume
                );
            }
            alphaFile.close();
            std::cout << DATE << "_" + std::to_string(SESSION.first) + "_" + std::to_string(SESSION.second)
                      << " alpha read and insert successfully" << std::endl;

            // Read and insert order_log
            order_log orderLog{};
            std::ifstream orderLogFile(DATA_PREFIX + DATE + ORDER_LOG, std::ios::binary);
            while (orderLogFile.read(reinterpret_cast<char*>(&orderLog), sizeof(order_log))) {
                tradeEngine.insertOrderLog(
                        orderLog.instrument_id,
                        orderLog.timestamp,
                        orderLog.type,
                        orderLog.direction,
                        orderLog.volume,
                        orderLog.price_off
                );
            }
            orderLogFile.close();
            std::cout << DATE << "_" + std::to_string(SESSION.first) + "_" + std::to_string(SESSION.second)
                      << " order_log read and insert successfully" << std::endl;

            // Complete today
            tradeEngine.onComplete();
            std::cout << DATE << "_" + std::to_string(SESSION.first) + "_" + std::to_string(SESSION.second)
                      << " trading successfully" << std::endl;

            for (auto &instrument : instruments) {
                compareLogFiles(
                        DEBUG_PREFIX + (char*)&instrument,
                        STD_LOG_PREFIX + DATE + "/" +
                        "num" + std::to_string(SESSION.first) + "_" +
                        "length" + std::to_string(SESSION.second) + "_" +
                                (char*)&instrument);
                std::cout << DATE << "_" + std::to_string(SESSION.first) + "_" + std::to_string(SESSION.second)
                          << "_" +  std::string((char*)&instrument) + " log compare successfully" << std::endl;
            }
        }
    }
}