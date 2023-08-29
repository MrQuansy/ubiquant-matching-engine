//
// Created by Yongzao Dan on 2023/8/26.
//

#include "contract_engine.h"

void ContractEngine::insertAlpha(Alpha alpha) {
    int diff = alpha.targetVolume - targetVolume;
    if (!diff) {
        // Skip if diff == 0
        return;
    }

    targetVolume = alpha.targetVolume;
    unsigned char direction = (diff > 0 ? Buy : Sale) << DIRECTION_OFFSET;
    diff = _abs(diff);
    for (int i = 0; i < sessionNum; i++) {
        // Timestamp, volume and direction for all TWAP orders could be calculated in advance
        twapOrders[twapSize++] = {
                .timestamp = alpha.timestamp + i * sessionLength * 1000,
                // The TWAP formula
                .volume = (diff * (i + 1) / sessionNum) - (diff * i / sessionNum),
                .instrument = alpha.instrument,
                .directionAndType = static_cast<unsigned char>(direction | MyContract)
        };
    }
}

void ContractEngine::insertOrderLog(OrderLog orderLog) {
    // Ensure all TWAP orders with timestamp < orderLog.timestamp are processed
    processTwapOrdersIfNecessary(orderLog.timestamp);

    int type = orderLog.directionAndType & TYPE_MASK;
    int direction = (orderLog.directionAndType & DIRECTION_MASK) >> DIRECTION_OFFSET;
    double basePrice, _upLimit, _downLimit;

//    if (orderLog.timestamp == 26593) {
//        double z = getCurrentBasePrice(direction);
//        std::cout << "DEBUG! " << z << std::endl;
//    }

    BinaryHeap *heap;
    if (type <= OurBestPrice) {
        switch (type) {

            case PriceLimit: {
                heap = direction == Sale ? saleHeap : buyHeap;
                // price = basePrice + priceOff for PriceLimitContract
                basePrice = getCurrentBasePrice(direction);
                orderLog.price = basePrice + orderLog.price;
                //orderLog.price = round2(basePrice + orderLog.price);

                if (direction == Buy) {
                    // Rule 3.3.16 buyPrice <= max(basePrice * 1.02, basePrice + 0.01 * 10)
                    _upLimit = _max(basePrice * 1.02, basePrice + 0.1);
                    _upLimit = _min(_upLimit, upLimit);
                    _upLimit = highPrecisionRound2(_upLimit);

                    _downLimit = downLimit;
                } else {
                    // Rule 3.3.16 salePrice >= min(basePrice * 0.98, basePrice - 0.01 * 10)
                    _downLimit = _min(basePrice * 0.98, basePrice - 0.1);
                    _downLimit = _max(_downLimit, downLimit);
                    _downLimit = highPrecisionRound2(_downLimit);

                    _upLimit = upLimit;
                }
                if (_le(orderLog.price, _downLimit) || _le(_upLimit, orderLog.price)) {
                    break;
                }

                heap->insert(orderLog);
                break;
            }

            case OppositeBestPrice:
                heap = direction == Sale ? buyHeap : saleHeap;
                if (!heap->isEmpty()) {
                    // OppositeBestPrice takes effect only when there exists opposite orders
                    orderLog.price = heap->top().price;
                    if (direction == Sale) {
                        saleHeap->insert(orderLog);
                    } else {
                        buyHeap->insert(orderLog);
                    }
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

        if (ENABLE_DEBUG_TRADE_LOG) {
            (*logFile) << orderLog;
            if (type == PriceLimit) {
                (*logFile) << ", price=" << std::fixed << std::setprecision(6) << highPrecisionRound2(orderLog.price)
                           << ", base_price=" << std::fixed << std::setprecision(6) << highPrecisionRound2(basePrice)
                           << ", up_limit=" << std::fixed << std::setprecision(6) << highPrecisionRound2(_upLimit)
                           << ", down_limit=" << std::fixed << std::setprecision(6) << highPrecisionRound2(_downLimit);
            }
            (*logFile) << std::endl;
        }

        processTrade();

    } else {
        if (ENABLE_DEBUG_TRADE_LOG) {
            (*logFile) << orderLog << std::endl;
        }

        heap = direction == Sale ? buyHeap : saleHeap;
        switch (type) {

            case BestFivePriceNoLeft: {
                int levelNum = 0;
                double lastPriceLevel = -1;
                while (orderLog.volume > 0 && !heap->isEmpty()) {
                    OrderLog topOrder = heap->top();
                    if (_neq(topOrder.price, lastPriceLevel)) {
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

void ContractEngine::onComplete() {
    // Trade all remain TWAP orders
    processTwapOrdersIfNecessary(MAX_INT);
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

void ContractEngine::processTwapOrdersIfNecessary(int currentTime) {
    while (twapHead < twapSize && twapOrders[twapHead].timestamp < currentTime) {
        OrderLog *orderLog = &twapOrders[twapHead++];
        int direction = orderLog->directionAndType >> DIRECTION_OFFSET;
        orderLog->price = getCurrentBasePrice(direction);
        if (orderLog->volume) {
            // Insert orderLog if volume != 0
            switch (direction) {
                case Buy:
                    buyHeap->insert(*orderLog);
                    break;
                case Sale:
                    saleHeap->insert(*orderLog);
                    break;
            }
        }

        if (ENABLE_DEBUG_TRADE_LOG) {
            (*logFile) << "twap order: "
                       << "timestamp=" << orderLog->timestamp << ", "
                       << "direction=" << ((int) (orderLog->directionAndType >> DIRECTION_OFFSET) ? 1 : -1) << ", "
                       << "volume=" << orderLog->volume << ", "
                       << "price=" << std::fixed << std::setprecision(6) << orderLog->price << std::endl;
        }

        processTrade();
    }
}

// Update lastPrice, return tradeVolume and update income if necessary
int ContractEngine::tradeOrder(const OrderLog &saleOrder, const OrderLog &buyOrder) {
    double tradePrice;
    if (_eq(saleOrder.price, buyOrder.price)) {
        tradePrice = saleOrder.price;
    } else {
        // Rule 3.4.4
        tradePrice = saleOrder.timestamp > buyOrder.timestamp ? buyOrder.price : saleOrder.price;
    }
    lastPrice = tradePrice;

    int tradeVolume = _min(saleOrder.volume, buyOrder.volume);
    if ((saleOrder.directionAndType & TYPE_MASK) == MyContract) {
        income += highPrecisionRound2(tradePrice * tradeVolume);
        volume -= tradeVolume;
    } else if ((buyOrder.directionAndType & TYPE_MASK) == MyContract) {
        income -= highPrecisionRound2(tradePrice * tradeVolume);
        volume += tradeVolume;
    }

    if (ENABLE_DEBUG_TRADE_LOG) {
        (*logFile) << "[" << tradeCount++ << "]" << " trade: "
                   << "price=" << std::fixed << std::setprecision(6) << tradePrice
                   << ", volume=" << tradeVolume << std::endl;
    }

    return tradeVolume;
}

void ContractEngine::processTrade() {
    while (!saleHeap->isEmpty() && !buyHeap->isEmpty()) {
        OrderLog* saleOrder = saleHeap->topPtr();
        OrderLog* buyOrder = buyHeap->topPtr();
        if (_gt(saleOrder->price, buyOrder->price)) {
            break;
        }

        int tradeVolume = tradeOrder(*saleOrder, *buyOrder);
        saleOrder->volume -= tradeVolume;
        buyOrder->volume -= tradeVolume;
        if (saleOrder->volume == 0) {
            saleHeap->pop();
        }
        if (buyOrder->volume == 0) {
            buyHeap->pop();
        }
    }
}

int ContractEngine::getTwapSize() const {
    return twapSize;
}

OrderLog* ContractEngine::getTwapOrders() const {
    return twapOrders;
}

int ContractEngine::getPosition() const {
    return volume;
}

double ContractEngine::getPNL() const {
    return highPrecisionRound2(lastPrice * volume - prevTradeInfo.prevClosePrice * prevTradeInfo.prevPosition + income);
}