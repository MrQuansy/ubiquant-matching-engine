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
#include <cstring>

struct TradeEngine {

public:

    void getInstrumentId(std::string instrument);
    void insertPrevTradeInfo(PrevTradeInfo prevTradeInfo);
    void insertAlpha(Alpha alpha);
    void insertOrderLog(OrderLog orderLog);

private:

    // 0 is for myself
    unsigned char lastId = 1;
    std::map<std::string, unsigned char> instrumentToIdMap;
    std::map<unsigned char, ContractEngine> contractEngineMap;
};

#endif //UBIQUANTMATCHINGENGINE_TRADE_ENGINE_H
