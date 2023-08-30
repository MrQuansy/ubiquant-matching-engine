//
// Created by Yongzao Dan on 2023/8/26.
//

#ifndef UBIQUANTMATCHINGENGINE_TRADE_ENGINE_H
#define UBIQUANTMATCHINGENGINE_TRADE_ENGINE_H

#include "common.h"
#include "binary_heap.h"
#include "contract_engine.h"

#include <map>
#include <queue>
#include <string>
#include <utility>
#include <vector>
#include <utility>
#include <algorithm>

struct TradeEngine {

public:

    TradeEngine(std::pair<int, int> session, std::string path) : session(std::move(session)), path(path) {
        lastId = 0;
        timestampOffset = ENABLE_DEBUG_TRADE_LOG ? 0 : -1;
    }

    ~TradeEngine() {
        for (auto & contractEngine : contractEngineMap) {
            delete contractEngine.second;
        }
    }

    // Init a contract by its compact_prev_trade_info
    void initContract(const long instrument, const double &prevClosePrice, const int &prevPosition);

    // Insert compact_alpha by time order
    void insertAlpha(const long instrument, const long long &timestamp, const int &targetVolume);

    // Insert compact_order_log by time order
    void insertOrderLog(
            const long instrument,
            const long long &timestamp,
            const int &type,
            const int &direction,
            const int &volume,
            const double &priceOff);


    void onComplete();

    // TODO: optimize for output
    inline std::vector<twap_order> getTWAPOrders();
    inline std::vector<pnl_and_pos> getPNLAndPos();

private:

    int lastId;
    long long timestampOffset;
    std::pair<int, int> session;
    std::string path;

    std::map<long, ContractEngine*> contractEngineMap;
};

#endif //UBIQUANTMATCHINGENGINE_TRADE_ENGINE_H
