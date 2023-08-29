//
// Created by Yongzao Dan on 2023/8/26.
//

#ifndef UBIQUANTMATCHINGENGINE_CONTRACT_ENGINE_H
#define UBIQUANTMATCHINGENGINE_CONTRACT_ENGINE_H

#include "common.h"
#include "binary_heap.h"

#include <queue>
#include <fstream>
#include <iostream>

// TODO: Optimize heap size
const static int HEAP_SIZE = 50 * 1E4;

struct ContractEngine {

public:

    ContractEngine(int sessionNum, int sessionLength, PrevTradeInfo prevTradeInfo) :
        sessionNum(sessionNum), sessionLength(sessionLength), prevTradeInfo(prevTradeInfo) {

        income = 0;
        lastPrice = prevTradeInfo.prevClosePrice;
        volume = targetVolume = prevTradeInfo.prevPosition;

        // Rule 3.3.13 0.9 * prevClosePrice <= price <= 1.1 * prevClosePrice
        upLimit = highPrecisionRound2(prevTradeInfo.prevClosePrice * 1.1);
        downLimit = highPrecisionRound2(prevTradeInfo.prevClosePrice * 0.9);

        saleHeap = new BinaryHeap(HEAP_SIZE, new MinBinaryHeapCmp());
        buyHeap = new BinaryHeap(HEAP_SIZE, new MaxBinaryHeapCmp());

        twapSize = twapHead = 0;
        twapOrders = new OrderLog[MAX_TWAP_LENGTH];

        if (ENABLE_DEBUG_TRADE_LOG) {
            tradeCount = 0;
            logFile = new std::ofstream(DEBUG_PREFIX + DEBUG_DATE + "_" + DEBUG_INSTRUMENT, std::ios::out);
            // (*logFile) << prevTradeInfo << std::endl;
        }
    }

    ~ContractEngine() {
        delete saleHeap;
        delete buyHeap;
        delete twapOrders;

        if (ENABLE_DEBUG_TRADE_LOG) {
            logFile->close();
            delete logFile;
        }
    }

    void insertAlpha(Alpha alpha);
    void insertOrderLog(OrderLog orderLog);
    void onComplete();

    int getTwapSize() const;
    OrderLog* getTwapOrders() const;

    int getPosition() const;
    double getPNL() const;

private:

    double income, lastPrice;
    double upLimit, downLimit;
    int volume, targetVolume;
    int sessionNum, sessionLength;
    PrevTradeInfo prevTradeInfo;

    BinaryHeap* saleHeap;
    BinaryHeap* buyHeap;

    int twapSize, twapHead;
    OrderLog* twapOrders;

    int tradeCount;
    std::ofstream* logFile;

    double getCurrentBasePrice(const unsigned char &direction);

    void processTwapOrdersIfNecessary(int currentTime);

    void processTrade();

    // Trade the specified orders and calculate PNL if necessary
    // return: trade volume
    int tradeOrder(const OrderLog &saleOrder, const OrderLog &buyOrder);

};

#endif //UBIQUANTMATCHINGENGINE_CONTRACT_ENGINE_H
