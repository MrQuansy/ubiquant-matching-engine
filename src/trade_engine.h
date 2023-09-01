//
// Created by Yongzao Dan on 2023/8/26.
//

#ifndef UBIQUANTMATCHINGENGINE_TRADE_ENGINE_H
#define UBIQUANTMATCHINGENGINE_TRADE_ENGINE_H

#include "common.h"
#include "binary_heap.h"
#include "contract_engine.h"

// #include <unordered_map>
#include <queue>
#include <string>
#include <utility>
#include <vector>
#include <utility>
#include <algorithm>

const static unsigned long MOD = 20021;
const static int MAX_CONTRACT_NUM = 256;

struct TradeEngine {

public:


    TradeEngine(std::pair<int, int> session, std::string path) : session(std::move(session)), path(std::move(path)) {
        timestampOffset = ENABLE_DEBUG_TRADE_LOG ? 0 : -1;

        lastContract = 0;
        std::memset(contractPairs, 0, sizeof(contractPairs));
    }

    ~TradeEngine() {
        uint8_t t;
        for (int i = 0; i < lastContract; i++) {
            delete contractEngines[i];
        }

//        for (auto & contractEngine : contractEngineMap) {
//            delete contractEngine.second;
//        }
    }

    // Init a contract by its compact_prev_trade_info
    void initContract(const unsigned long &instrument, const double &prevClosePrice, const int &prevPosition);

    // Insert compact_alpha by time order
    void insertAlpha(const unsigned long &instrument, const long long &timestamp, const int &targetVolume);

    // Insert compact_order_log by time order
    void insertOrderLog(
            const unsigned long &instrument,
            const long long &timestamp,
            const int &type,
            const int &direction,
            const int &volume,
            const double &priceOff);


    void onComplete();

    int getOrInsertInstrument(const unsigned long &instrument);

    // TODO: optimize for output
    std::vector<twap_order> getTWAPOrders();
    std::vector<pnl_and_pos> getPNLAndPos();
    std::string path;

private:

    long long timestampOffset;
    std::pair<int, int> session;

    int lastContract;
    std::pair<unsigned long, int> contractPairs[MOD];
    ContractEngine *contractEngines[MAX_CONTRACT_NUM];

    // std::unordered_map<unsigned long, ContractEngine*> contractEngineMap;
};

#endif //UBIQUANTMATCHINGENGINE_TRADE_ENGINE_H
