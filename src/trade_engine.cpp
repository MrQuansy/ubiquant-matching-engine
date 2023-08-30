//
// Created by Yongzao Dan on 2023/8/27.
//


#include "net/client.h"
#include "trade_engine.h"

void TradeEngine::initContract(const long instrument, const double &prevClosePrice, const int &prevPosition) {
    compact_prev_trade_info prevTradeInfo = {
            .prevClosePrice = prevClosePrice,
            .prevPosition = prevPosition
    };
    contractEngineMap[instrument] =
            new ContractEngine(session.first, session.second, prevTradeInfo, instrument);
}

void TradeEngine::insertAlpha(const long instrument, const long long &timestamp, const int &targetVolume) {

    if (timestampOffset == -1 && !ENABLE_DEBUG_TRADE_LOG) {
        timestampOffset = timestamp;
    }

    compact_alpha alpha = {
            .timestamp = static_cast<int>(timestamp - timestampOffset),
            .targetVolume = targetVolume
    };
    contractEngineMap[instrument]->insertAlpha(alpha);
}

void TradeEngine::insertOrderLog(
        const long instrument,
        const long long &timestamp,
        const int &type,
        const int &direction,
        const int &volume,
        const double &priceOff) {

    unsigned char d = direction == -1 ? Sale : Buy;
    compact_order_log orderLog = {
            .timestamp = static_cast<int>(timestamp - timestampOffset),
            .volume = volume,
            .price = priceOff,
            .directionAndType = (unsigned char) ((d << DIRECTION_OFFSET) | type)
    };
    contractEngineMap[instrument]->insertOrderLog(orderLog);
}

void TradeEngine::onComplete() {
    for (auto &contractEngine : contractEngineMap) {
        contractEngine.second->onComplete();
    }

    std::vector<twap_order> twapOrders = getTWAPOrders();
    std::vector<pnl_and_pos> pnlAndPos = getPNLAndPos();
    std::string fileName = path + "_" + std::to_string(session.first) + "_" + std::to_string(session.second);
//    sentResult(fileName, twapOrders, pnlAndPos);

    std::cout << "[Network] Finish send result: " << fileName << std::endl;
}

std::vector<twap_order> TradeEngine::getTWAPOrders() {
    std::vector<twap_order> twapOrders;
    for (auto &contractEngine : contractEngineMap) {
        long instrument_id = contractEngine.first;
        int twapSize = contractEngine.second->getTwapSize();
        compact_order_log* contractTwapOrders = contractEngine.second->getTwapOrders();
        for (int i = 0; i < twapSize; i++) {
            compact_order_log orderLog = contractTwapOrders[i];
            int direction = orderLog.directionAndType >> DIRECTION_OFFSET;
            direction = direction == Sale ? -1 : 1;
            twap_order twapOrder = {
                    .timestamp = orderLog.timestamp + timestampOffset,
                    .direction = direction,
                    .volume = orderLog.volume,
                    .price = orderLog.price,
            };
            twapOrder.instrumentId = instrument_id;
            twapOrders.push_back(twapOrder);
        }
    }
    std::sort(twapOrders.begin(), twapOrders.end());
    return twapOrders;
}

std::vector<pnl_and_pos> TradeEngine::getPNLAndPos() {
    std::vector<pnl_and_pos> pnlAndPos;
    for (auto &contractEngine : contractEngineMap) {
        long instrument_id = contractEngine.first;
        pnl_and_pos pnlAndPosItem = {
                .position = contractEngine.second->getPosition(),
                .pnl = contractEngine.second->getPNL()
        };
        pnlAndPosItem.instrumentId = instrument_id;
        pnlAndPos.push_back(pnlAndPosItem);
    }
    std::sort(pnlAndPos.begin(), pnlAndPos.end());
    return pnlAndPos;
}