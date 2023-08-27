//
// Created by Yongzao Dan on 2023/8/26.
//

#ifndef UBIQUANTMATCHINGENGINE_CONTRACT_ENGINE_H
#define UBIQUANTMATCHINGENGINE_CONTRACT_ENGINE_H

#include "common.h"
#include "binary_heap.h"

#include <queue>

// TODO: Optimize heap size
const static int HEAP_SIZE = 50 * 1E4;

struct ContractEngine {

public:

    ContractEngine(int sessionNum, int sessionLength, PrevTradeInfo prevTradeInfo) :
        sessionNum(sessionNum), sessionLength(sessionLength), prevTradeInfo(prevTradeInfo) {

        income = 0;
        lastPrice = prevTradeInfo.prevClosePrice;
        volume = targetVolume = prevTradeInfo.prevPosition;

        saleHeap = new BinaryHeap(HEAP_SIZE, new MinBinaryHeapCmp());
        buyHeap = new BinaryHeap(HEAP_SIZE, new MaxBinaryHeapCmp());
    }

    ~ContractEngine() {
        delete saleHeap;
        delete buyHeap;
    }

    void insertOrderLog(OrderLog orderLog);
    void insertAlpha(Alpha alpha);

    double getPNL() const;
    std::queue<OrderLog> getTwapQueue() const; // TODO: optimize output

private:

    double income, lastPrice;
    int volume, targetVolume;
    int sessionNum, sessionLength;
    PrevTradeInfo prevTradeInfo;

    BinaryHeap* saleHeap;
    BinaryHeap* buyHeap;
    std::queue<OrderLog> twapQueue;

    double getCurrentBasePrice(const unsigned char &direction);

    // Trade the specified orders and calculate PNL if necessary
    // return: trade volume
    int tradeOrder(const OrderLog &saleOrder, const OrderLog &buyOrder);

    void processTrade();
};

#endif //UBIQUANTMATCHINGENGINE_CONTRACT_ENGINE_H
