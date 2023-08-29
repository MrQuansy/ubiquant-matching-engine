//
// Created by Yongzao Dan on 2023/8/27.
//

#include "../src/common.h"
#include "../src/trade_engine.h"
#include "gtest/gtest.h"
#include "binary_file_compare.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

const static std::string DATES[] = {
        "20150101",
        "20160202",
        "20170303",
        "20180404",
        "20190505",
};

class BaseLineTest : public ::testing::Test {};

TEST_F(BaseLineTest, base_line_test) {
    for (const auto& date : DATES) {
        // Init engines
        TradeEngine tradeEngines[] = {
                //TradeEngine(SESSIONS[0]),
                //TradeEngine(SESSIONS[1]),
                //TradeEngine(SESSIONS[2]),
                //TradeEngine(SESSIONS[3]),
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
        std::cout << date << " prev_trade_info read and init successfully" << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        // Read alpha
        alpha a{};
        std::ifstream alphaFile(DATA_PREFIX + date + ALPHA, std::ios::binary);
        while (alphaFile.read(reinterpret_cast<char*>(&a), sizeof(alpha))) {
            for (auto & tradeEngine : tradeEngines) {
                tradeEngine.insertAlpha(
                        std::string(a.instrument_id),
                        a.timestamp,
                        a.target_volume
                );
            }
        }
        alphaFile.close();
        std::cout << date << " alpha read and insert successfully" << std::endl;

        // Read order_log
        order_log orderLog{};
        std::ifstream orderLogFile(DATA_PREFIX + date + ORDER_LOG, std::ios::binary);
        while (orderLogFile.read(reinterpret_cast<char*>(&orderLog), sizeof(order_log))) {
            for (auto & tradeEngine : tradeEngines) {
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
        std::cout << date << " order_log read and insert successfully" << std::endl;

        auto tradeTime = std::chrono::high_resolution_clock::now() - start;
        std::cout << date << " trading successfully, trade time: ";
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(tradeTime).count() << " ms" << std::endl;

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
        std::cout << date << " twap_order write successfully" << std::endl;

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
        std::cout << date << " pnl_and_position write successfully" << std::endl;

        for (const auto & session : SESSIONS) {
            std::string suffix = "_" + std::to_string(session.first) + "_" + std::to_string(session.second);
            compareTWAPFiles(
                    DATA_PREFIX + date + TWAP_ORDER + suffix,
                    STD_OUTPUT_PREFIX + TWAP_ORDER + "/" + date + suffix);
            comparePNLFiles(
                    DATA_PREFIX + date + PNL_AND_POSITION + suffix,
                    STD_OUTPUT_PREFIX + PNL_AND_POSITION + "/" + date + suffix);
        }
    }

    std::cout << "Baseline pass" << std::endl;
}
