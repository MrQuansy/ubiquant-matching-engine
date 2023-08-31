//
// Created by Yongzao Dan on 2023/8/27.
//

#include "../src/common.h"
#include "../src/trade_engine.h"
#include "gtest/gtest.h"
#include "file_comparator.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

const static std::string BASELINE_DATA_PREFIX = DATA_PREFIX;
const static std::string BASELINE_OUTPUT_PREFIX = TEST_OUTPUT_PREFIX;
const static std::string BASELINE_STD_OUTPUT_PREFIX = STD_OUTPUT_PREFIX;

//const static std::string BASELINE_DATA_PREFIX = "/data2/ubtest/data/";
//const static std::string BASELINE_OUTPUT_PREFIX = "/data2/ubtest/test_data/";
//const static std::string BASELINE_STD_OUTPUT_PREFIX = "/data2/ubtest/output_adjuest/";

const static std::string DATES[] = {
        "20150101",
        "20160202",
        "20170303",
        "20180404",
        "20190505",
};

class BaseLineTest : public ::testing::Test {};

void readAllData(
        const std::string &date,
        std::vector<prev_trade_info> &prev_trade_infos,
        std::vector<alpha> &alphas,
        std::vector<order_log> &orderLogs);

void tradeToday(
        const std::string &date,
        const std::vector<prev_trade_info> &prev_trade_infos,
        const std::vector<alpha> &alphas,
        const std::vector<order_log> &orderLogs,
        std::vector<pnl_and_pos> pnlAndPositions[],
        std::vector<twap_order> twapOrders[]);

void writeAllData(
        const std::string &date,
        const std::vector<pnl_and_pos> pnlAndPositions[],
        const std::vector<twap_order> twapOrders[]);

TEST_F(BaseLineTest, base_line_test) {
    for (const auto& date : DATES) {

        std::vector<prev_trade_info> prevTradeInfos;
        std::vector<alpha> alphas;
        std::vector<order_log> orderLogs;

        std::vector<pnl_and_pos> pnlAndPositions[5];
        std::vector<twap_order> twapOrders[5];

        // Ready today data
        auto readStart = std::chrono::high_resolution_clock::now();
        readAllData(date, prevTradeInfos, alphas, orderLogs);
        auto readTime = std::chrono::high_resolution_clock::now() - readStart;
        std::cout << date << " read successfully, read time: ";
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(readTime).count() << " ms" << std::endl;

        // Trading today
        auto tradeStart = std::chrono::high_resolution_clock::now();
        tradeToday(date, prevTradeInfos, alphas, orderLogs, pnlAndPositions, twapOrders);
        auto tradeTime = std::chrono::high_resolution_clock::now() - tradeStart;
        std::cout << date << " trading successfully, trade time: ";
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(tradeTime).count() << " ms" << std::endl;

        // Write today data
        auto writeStart = std::chrono::high_resolution_clock::now();
        writeAllData(date, pnlAndPositions, twapOrders);
        auto writeTime = std::chrono::high_resolution_clock::now() - writeStart;
        std::cout << date << " write successfully, write time: ";
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(writeTime).count() << " ms" << std::endl;

        // Check today
        for (const auto & session : SESSIONS) {
            std::string suffix = "_" + std::to_string(session.first) + "_" + std::to_string(session.second);
            compareTWAPFiles(
                    BASELINE_OUTPUT_PREFIX + date + TWAP_ORDER + suffix,
                    BASELINE_STD_OUTPUT_PREFIX + TWAP_ORDER + "/" + date + suffix);
            comparePNLFiles(
                    BASELINE_OUTPUT_PREFIX + date + PNL_AND_POSITION + suffix,
                    BASELINE_STD_OUTPUT_PREFIX + PNL_AND_POSITION + "/" + date + suffix);
        }
        std::cout << date << " baseline pass" << std::endl;
    }

    std::cout << "all baseline pass" << std::endl;
}

void readAllData(
        const std::string &date,
        std::vector<prev_trade_info> &prev_trade_infos,
        std::vector<alpha> &alphas,
        std::vector<order_log> &orderLogs) {

    // Read prev_trade_info
    prev_trade_info prevTradeInfo{};
    std::ifstream prevTradeInfoFile(BASELINE_DATA_PREFIX + date + PREV_TRADE_INFO, std::ios::binary);
    while (prevTradeInfoFile.read(reinterpret_cast<char*>(&prevTradeInfo), sizeof(prev_trade_info))) {
        prev_trade_infos.push_back(prevTradeInfo);
    }
    prevTradeInfoFile.close();
    std::cout << date << " prev_trade_info read successfully" << std::endl;

    // Read alpha
    alpha a{};
    std::ifstream alphaFile(BASELINE_DATA_PREFIX + date + ALPHA, std::ios::binary);
    while (alphaFile.read(reinterpret_cast<char*>(&a), sizeof(alpha))) {
        alphas.push_back(a);
    }
    alphaFile.close();
    std::cout << date << " alpha read successfully" << std::endl;

    // Read order_log
    order_log orderLog{};
    std::ifstream orderLogFile(BASELINE_DATA_PREFIX + date + ORDER_LOG, std::ios::binary);
    while (orderLogFile.read(reinterpret_cast<char*>(&orderLog), sizeof(order_log))) {
        orderLogs.push_back(orderLog);
    }
    orderLogFile.close();
    std::cout << date << " order_log read successfully" << std::endl;
}

void tradeToday(
        const std::string &date,
        const std::vector<prev_trade_info> &prev_trade_infos,
        const std::vector<alpha> &alphas,
        const std::vector<order_log> &orderLogs,
        std::vector<pnl_and_pos> pnlAndPositions[],
        std::vector<twap_order> twapOrders[]) {

    // Init engines
    TradeEngine tradeEngines[] = {
            TradeEngine(SESSIONS[0],date),
            TradeEngine(SESSIONS[1],date),
            TradeEngine(SESSIONS[2],date),
            TradeEngine(SESSIONS[3],date),
            TradeEngine(SESSIONS[4],date)
    };

    // Init prev_trade_info
    for (auto &prevTradeInfo : prev_trade_infos) {
        for (auto &tradeEngine : tradeEngines) {
            tradeEngine.initContract(
                    prevTradeInfo.instrument_id,
                    prevTradeInfo.prev_close_price,
                    prevTradeInfo.prev_position
            );
        }
    }
    std::cout << date << " prev_trade_info init successfully" << std::endl;

    // Insert alpha
    for (auto &alpha : alphas) {
        for (auto &tradeEngine : tradeEngines) {
            tradeEngine.insertAlpha(
                    alpha.instrument_id,
                    alpha.timestamp,
                    alpha.target_volume
            );
        }
    }
    std::cout << date << " alpha insert successfully" << std::endl;

    // Insert order_log
    for (auto &orderLog : orderLogs) {
//        for (auto &tradeEngine : tradeEngines) {
//            tradeEngine.insertOrderLog(
//                    orderLog.instrument_id,
//                    orderLog.timestamp,
//                    orderLog.type,
//                    orderLog.direction,
//                    orderLog.volume,
//                    orderLog.price_off
//            );
//        }
        tradeEngines[0].insertOrderLog(
                orderLog.instrument_id,
                orderLog.timestamp,
                orderLog.type,
                orderLog.direction,
                orderLog.volume,
                orderLog.price_off
        );
        tradeEngines[1].insertOrderLog(
                orderLog.instrument_id,
                orderLog.timestamp,
                orderLog.type,
                orderLog.direction,
                orderLog.volume,
                orderLog.price_off
        );
        tradeEngines[2].insertOrderLog(
                orderLog.instrument_id,
                orderLog.timestamp,
                orderLog.type,
                orderLog.direction,
                orderLog.volume,
                orderLog.price_off
        );
        tradeEngines[3].insertOrderLog(
                orderLog.instrument_id,
                orderLog.timestamp,
                orderLog.type,
                orderLog.direction,
                orderLog.volume,
                orderLog.price_off
        );
        tradeEngines[4].insertOrderLog(
                orderLog.instrument_id,
                orderLog.timestamp,
                orderLog.type,
                orderLog.direction,
                orderLog.volume,
                orderLog.price_off
        );
    }
    std::cout << date << " order_log insert successfully" << std::endl;

    // Complete today
    for (auto &tradeEngine : tradeEngines) {
        tradeEngine.onComplete();
    }

    // Get all pnl_and_position
    for (int i = 0; i < 5; i++) {
        pnlAndPositions[i] = tradeEngines[i].getPNLAndPos();
    }

    // Get all twap_order
    for (int i = 0; i < 5; i++) {
        twapOrders[i] = tradeEngines[i].getTWAPOrders();
    }
}

void writeAllData(
        const std::string &date,
        const std::vector<pnl_and_pos> pnlAndPositions[],
        const std::vector<twap_order> twapOrders[]) {

    // Write pnl_and_position
    for (int i = 0; i < 5; i++) {
        std::string suffix = "_" + std::to_string(SESSIONS[i].first) + "_" + std::to_string(SESSIONS[i].second);
        std::ofstream pnlAndPositionFile(BASELINE_OUTPUT_PREFIX + date + PNL_AND_POSITION + suffix, std::ios::binary);
        for (auto pnlAndPosition : pnlAndPositions[i]) {
            pnlAndPositionFile.write(reinterpret_cast<char*>(&pnlAndPosition), sizeof(pnl_and_pos));
        }
        pnlAndPositionFile.close();
    }
    std::cout << date << " pnl_and_position write successfully" << std::endl;

    // Write twap_order
    for (int i = 0; i < 5; i++) {
        std::string suffix = "_" + std::to_string(SESSIONS[i].first) + "_" + std::to_string(SESSIONS[i].second);
        std::ofstream twapOrderFile(BASELINE_OUTPUT_PREFIX + date + TWAP_ORDER + suffix, std::ios::binary);
        for (auto twapOrder : twapOrders[i]) {
            twapOrderFile.write(reinterpret_cast<char*>(&twapOrder), sizeof(twap_order));
        }
        twapOrderFile.close();
    }
    std::cout << date << " twap_order write successfully" << std::endl;
}