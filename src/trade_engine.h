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
#include <vector>
#include <utility>

struct TradeEngine {

public:

    TradeEngine(std::pair<int, int> session) : session(session) {
        lastId = 0;
        timestampOffset = 0;
    }

    // Init a contract by its PrevTradeInfo
    void initContract(const std::string &instrument, const double &prevClosePrice, const int &prevPosition);

    // Insert OrderLog by time order
    void insertOrderLog(
            const std::string &instrument,
            const long long &timestamp,
            const int &type,
            const int &direction,
            const int &volume,
            const double &priceOff);

    // Insert Alpha by time order
    // Notice that an alpha should always been inserted after the OrderLog if they have the same timestamp
    void insertAlpha(const std::string &instrument, const long long &timestamp, const int &targetVolume);

    // TODO: optimize for output
    std::vector<twap_order> getTWAPOrders();
    std::vector<pnl_and_pos> getPNLAndPos();

private:

    int lastId;
    long long timestampOffset;
    std::pair<int, int> session;

    std::map<std::string, unsigned char> instrumentToIdMap;
    std::map<unsigned char, std::string> idToInstrumentMap;
    std::map<unsigned char, ContractEngine*> contractEngineMap;
};

#endif //UBIQUANTMATCHINGENGINE_TRADE_ENGINE_H
