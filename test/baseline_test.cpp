//
// Created by Yongzao Dan on 2023/8/27.
//

#include "../src/common.h"
#include "../src/trade_engine.h"
#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

const static std::string DATA_PREFIX = "~/Downloads/data/";
const static std::string DATES[] = {
        "20150101",
        "20160202",
        "20170303",
        "20180404",
        "20190505",
};
const static std::string ALPHA = "/alpha";
const static std::string ORDER_LOG = "/order_log";
const static std::string PREV_TRADE_INFO = "/prev_trade_info";
const static std::string TWAP_ORDER = "/twap_order";
const static std::string PNL_AND_POSITION = "/pnl_and_position";
const static std::string OUTPUT_PREFIX = "~/Downloads/output";

bool compareBinaryFiles(const std::string& file1, const std::string& file2);

class BaseLineTest : public ::testing::Test {};

TEST_F(BaseLineTest, base_line_test) {
    for (const auto& date : DATES) {
        // Init engines
        TradeEngine tradeEngines[5] = {
                TradeEngine(SESSIONS[0]),
                TradeEngine(SESSIONS[1]),
                TradeEngine(SESSIONS[2]),
                TradeEngine(SESSIONS[3]),
                TradeEngine(SESSIONS[4])
        };

        // Read prev_trade_info
        prev_trade_info prevTradeInfo{};
        std::ifstream prevTradeInfoFile(DATA_PREFIX + date + PREV_TRADE_INFO, std::ios::binary);
        while (prevTradeInfoFile.read(reinterpret_cast<char*>(&prevTradeInfo), sizeof(prev_trade_info))) {
            for (auto & tradeEngine : tradeEngines) {
                tradeEngine.initContract(
                        std::string(prevTradeInfo.instrument_id),
                        prevTradeInfo.prev_close_price,
                        prevTradeInfo.prev_position
                );
            }
        }
        prevTradeInfoFile.close();

        // Read order_log
        order_log orderLog{};
        std::vector<order_log> orderLogs;
        std::ifstream orderLogFile(DATA_PREFIX + date + ORDER_LOG, std::ios::binary);
        while (orderLogFile.read(reinterpret_cast<char*>(&orderLog), sizeof(order_log))) {
            orderLogs.push_back(orderLog);
        }
        orderLogFile.close();

        // Read alpha
        alpha a{};
        std::vector<alpha> alphas;
        std::ifstream alphaFile(DATA_PREFIX + date + ALPHA, std::ios::binary);
        while (alphaFile.read(reinterpret_cast<char*>(&a), sizeof(alpha))) {
            alphas.push_back(a);
        }
        alphaFile.close();

        // Trading
        int oPos = 0, aPos = 0;
        while (oPos < orderLogs.size() || aPos < alphas.size()) {
            if (aPos == alphas.size() - 1 || orderLogs[oPos].timestamp <= alphas[aPos].timestamp) {
                for (auto & tradeEngine : tradeEngines) {
                    tradeEngine.insertOrderLog(
                            std::string(orderLogs[oPos].instrument_id),
                            orderLogs[oPos].timestamp,
                            orderLogs[oPos].type,
                            orderLogs[oPos].direction,
                            orderLogs[oPos].volume,
                            orderLogs[oPos].price_off
                    );
                }
                oPos++;
            } else {
                for (auto & tradeEngine : tradeEngines) {
                    tradeEngine.insertAlpha(
                            std::string(alphas[aPos].instrument_id),
                            alphas[aPos].timestamp,
                            alphas[aPos].target_volume
                    );
                }
                aPos++;
            }
        }

        // Write twap_order
        for (int i = 0; i < 5; i++) {
            std::string suffix = "_" + std::to_string(SESSIONS[i].first) + "_" + std::to_string(SESSIONS[i].second);
            std::ofstream twapOrderFile(DATA_PREFIX + date + TWAP_ORDER + suffix, std::ios::binary);
            std::vector<twap_order> twapOrders = tradeEngines[i].getTWAPOrders();
            for (auto twapOrder : twapOrders) {
                twapOrderFile.write(reinterpret_cast<char*>(&twapOrder), sizeof(twap_order));
            }
            twapOrderFile.close();
        }

        // Write pnl_and_position
        for (int i = 0; i < 5; i++) {
            std::string suffix = "_" + std::to_string(SESSIONS[i].first) + "_" + std::to_string(SESSIONS[i].second);
            std::ofstream pnlAndPositionFile(DATA_PREFIX + date + PNL_AND_POSITION + suffix, std::ios::binary);
            std::vector<pnl_and_pos> pnlAndPositions = tradeEngines[i].getPNLAndPos();
            for (auto pnlAndPosition : pnlAndPositions) {
                pnlAndPositionFile.write(reinterpret_cast<char*>(&pnlAndPosition), sizeof(pnl_and_pos));
            }
            pnlAndPositionFile.close();
        }
    }

    // Compare twap_order and pnl_and_position
    for (const auto& date : DATES) {
        for (const auto & i : SESSIONS) {
            std::string suffix = "_" + std::to_string(i.first) + "_" + std::to_string(i.second);
            EXPECT_TRUE(compareBinaryFiles(
                    DATA_PREFIX + date + TWAP_ORDER + suffix,
                    OUTPUT_PREFIX + TWAP_ORDER + "/" + date + suffix));
            EXPECT_TRUE(compareBinaryFiles(
                    DATA_PREFIX + date + PNL_AND_POSITION + suffix,
                    OUTPUT_PREFIX + PNL_AND_POSITION + "/" + date + suffix));
        }
    }
}

bool compareBinaryFiles(const std::string& file1, const std::string& file2) {
    std::ifstream binaryFile1(file1, std::ios::binary);
    std::ifstream binaryFile2(file2, std::ios::binary);

    char char1, char2;
    bool filesEqual = true;
    while (binaryFile1.get(char1) && binaryFile2.get(char2)) {
        if (char1 != char2) {
            filesEqual = false;
            break;
        }
    }

    if (binaryFile1.eof() != binaryFile2.eof()) {
        filesEqual = false;
    }

    binaryFile1.close();
    binaryFile2.close();

    return filesEqual;
}