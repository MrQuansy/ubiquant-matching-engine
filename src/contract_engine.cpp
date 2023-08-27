//
// Created by Yongzao Dan on 2023/8/26.
//

#include "contract_engine.h"

void ContractEngine::insertOrderLog(OrderLog orderLog) {
    int type = orderLog.directionAndType & TYPE_MASK;
    int direction = (orderLog.directionAndType & DIRECTION_MASK) >> DIRECTION_OFFSET;

    BinaryHeap *heap;
    if (type <= OurBestPrice) {
        switch (type) {

            case PriceLimit:
                heap = direction == Sale ? saleHeap : buyHeap;
                // price = basePrice + priceOff for PriceLimitContract
                orderLog.price += getCurrentBasePrice(direction);
                heap->insert(orderLog);
                break;

            case OppositeBestPrice:
                heap = direction == Sale ? buyHeap : saleHeap;
                if (!heap->isEmpty()) {
                    // OppositeBestPrice takes effect only when there exists opposite orders
                    orderLog.price = heap->top().price;
                    heap->insert(orderLog);
                }
                break;

            case OurBestPrice:
                heap = direction == Sale ? saleHeap : buyHeap;
                if (!heap->isEmpty()) {
                    // OurBestPrice takes effect only when there exists our orders
                    orderLog.price = heap->top().price;
                    heap->insert(orderLog);
                }
                break;
        }

        processTrade();

    } else {
        heap = direction == Sale ? buyHeap : saleHeap;
        switch (type) {

            case BestFivePriceNoLeft: {
                int levelNum = 0;
                double lastPriceLevel = -1;
                while (orderLog.volume > 0 && !heap->isEmpty()) {
                    OrderLog topOrder = heap->top();
                    if (_abs(topOrder.price - lastPriceLevel) > EPS) {
                        // Five best limit
                        ++levelNum;
                        lastPriceLevel = topOrder.price;
                        if (levelNum > 5) {
                            break;
                        }
                    }

                    int tradeVolume = direction == Sale ?
                                      tradeOrder(orderLog, topOrder) :
                                      tradeOrder(topOrder, orderLog);

                    orderLog.volume -= tradeVolume;
                    if (tradeVolume == topOrder.volume) {
                        heap->pop();
                    } else {
                        heap->topPtr()->volume -= tradeVolume;
                    }
                }
                break;
            }

            case ImmediateDealNoLeft:
                while (orderLog.volume > 0 && !heap->isEmpty()) {
                    OrderLog topOrder = heap->top();

                    int tradeVolume = direction == Sale ?
                                      tradeOrder(orderLog, topOrder) :
                                      tradeOrder(topOrder, orderLog);

                    orderLog.volume -= tradeVolume;
                    if (tradeVolume == topOrder.volume) {
                        heap->pop();
                    } else {
                        heap->topPtr()->volume -= tradeVolume;
                    }
                }
                break;

            case AllOrNothing: {
                int volumeSum = 0;
                std::queue<OrderLog> pendingOrders;
                while (volumeSum < orderLog.volume && !heap->isEmpty()) {
                    OrderLog topOrder = heap->pop();
                    volumeSum += topOrder.volume;
                    pendingOrders.push(topOrder);
                }

                if (volumeSum >= orderLog.volume) {
                    // Trade all pending orders
                    while (!pendingOrders.empty()) {
                        OrderLog topOrder = pendingOrders.front();
                        pendingOrders.pop();
                        int tradeVolume = direction == Sale ?
                                          tradeOrder(orderLog, topOrder) :
                                          tradeOrder(topOrder, orderLog);
                        orderLog.volume -= tradeVolume;
                        topOrder.volume -= tradeVolume;
                        if (topOrder.volume) {
                            heap->insert(topOrder);
                        }
                    }
                } else {
                    // Cancel all pending orders
                    while (!pendingOrders.empty()) {
                        heap->insert(pendingOrders.front());
                        pendingOrders.pop();
                    }
                }

                break;
            }
        }
    }
}

void ContractEngine::insertAlpha(Alpha alpha) {
    int diff = alpha.targetVolume - targetVolume;
    if (!diff) {
        // Skip if diff == 0
        return;
    }

    targetVolume = alpha.targetVolume;
    unsigned char direction = (diff > 0 ? Buy : Sale) << DIRECTION_OFFSET;
    double basePrice = getCurrentBasePrice(direction);

    for (int i = 0; i < sessionNum; i++) {
        OrderLog orderLog = {
                .timestamp = alpha.timestamp + i * sessionLength,
                // The TWAP formula
                .volume = (diff * (i + 1) / sessionNum) - (diff * i / sessionNum),
                .price = basePrice,
                .instrument = alpha.instrument,
                .directionAndType = direction | MyContract
        };
        twapOrders.push_back(orderLog);
        if (orderLog.volume) {
            // Insert orderLog if volume > 0
            switch (direction) {
                case Buy:
                    buyHeap->insert(orderLog);
                    break;
                case Sale:
                    saleHeap->insert(orderLog);
                    break;
            }
        }
    }

    processTrade();
}

double ContractEngine::getCurrentBasePrice(const unsigned char &direction) {
    switch (direction) {
        case Sale:
            if (!buyHeap->isEmpty()) {
                return buyHeap->top().price;
            }
            if (!saleHeap->isEmpty()) {
                return saleHeap->top().price;
            }
            break;
        case Buy:
            if (!saleHeap->isEmpty()) {
                return saleHeap->top().price;
            }
            if (!buyHeap->isEmpty()) {
                return buyHeap->top().price;
            }
            break;
    }
    return lastPrice;
}

int ContractEngine::tradeOrder(const OrderLog &saleOrder, const OrderLog &buyOrder) {
    double tradePrice;
    if (_abs(saleOrder.price - buyOrder.price) < EPS) {
        tradePrice = saleOrder.price;
    } else {
        // Rule 3.4.4
        tradePrice = saleOrder.timestamp > buyOrder.timestamp ? buyOrder.price : saleOrder.price;
    }
    lastPrice = tradePrice;

    int tradeVolume = _min(saleOrder.volume, buyOrder.volume);
    if ((saleOrder.directionAndType & TYPE_MASK) == MyContract) {
        income += tradePrice * tradeVolume;
    } else if ((buyOrder.directionAndType & TYPE_MASK) == MyContract) {
        income -= tradePrice * tradeVolume;
    }
    return tradeVolume;
}

void ContractEngine::processTrade() {
    while (!saleHeap->isEmpty() && !buyHeap->isEmpty()) {
        OrderLog saleOrder = saleHeap->top();
        OrderLog buyOrder = buyHeap->top();
        if (saleOrder.price > buyOrder.price && _abs(saleOrder.price - buyOrder.price) > EPS) {
            break;
        }

        int tradeVolume = tradeOrder(saleOrder, buyOrder);
        saleOrder.volume -= tradeVolume;
        buyOrder.volume -= tradeVolume;
        if (saleOrder.volume == 0) {
            saleHeap->pop();
        }
        if (buyOrder.volume == 0) {
            buyHeap->pop();
        }
    }
}

int ContractEngine::getPosition() const {
    return volume;
}

double ContractEngine::getPNL() const {
    return lastPrice * volume - prevTradeInfo.prevClosePrice * prevTradeInfo.prevPosition + income;
}

std::vector<OrderLog> ContractEngine::getTwapOrders() const {
    return twapOrders;
}