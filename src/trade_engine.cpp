//
// Created by Yongzao Dan on 2023/8/27.
//

#include "trade_engine.h"

void TradeEngine::initContract(const std::string &instrument, const double &prevClosePrice, const int &prevPosition) {
    int instrumentId = lastId++;
    instrumentToIdMap[instrument] = instrumentId;
    idToInstrumentMap[instrumentId] = instrument;

    PrevTradeInfo prevTradeInfo = {
            .prevClosePrice = prevClosePrice,
            .prevPosition = prevPosition,
            .instrument = instrumentId
    };
    contractEngineMap[instrumentId] =
            new ContractEngine(session.first, session.second, prevTradeInfo);
}

void TradeEngine::insertOrderLog(
        const std::string &instrument,
        const long long &timestamp,
        const int &type,
        const int &direction,
        const int &volume,
        const double &priceOff) {

    if (!timestampOffset) {
        timestampOffset = timestamp;
    }

    unsigned char d = direction == -1 ? Sale : Buy;
    OrderLog orderLog = {
            .timestamp = timestamp - timestampOffset,
            .volume = volume,
            .price = priceOff,
            .instrument = instrumentToIdMap[instrument],
            .directionAndType = (unsigned char) (d | type)
    };
    contractEngineMap[orderLog.instrument]->insertOrderLog(orderLog);
}

void TradeEngine::insertAlpha(const std::string &instrument, const long long &timestamp, const int &targetVolume) {
    Alpha alpha = {
            .timestamp = timestamp - timestampOffset,
            .targetVolume = targetVolume,
            .instrument = instrumentToIdMap[instrument]
    };
    contractEngineMap[alpha.instrument]->insertAlpha(alpha);
}

std::vector<twap_order> TradeEngine::getTWAPOrders() {
    std::vector<twap_order> twapOrders;
    for (auto &contractEngine : contractEngineMap) {
        std::vector<OrderLog> contractTwapOrders = contractEngine.second->getTwapOrders();
        for (auto &orderLog : contractTwapOrders) {
            int direction = orderLog.directionAndType >> DIRECTION_OFFSET;
            direction = direction == Sale ? -1 : 1;
            twap_order twapOrder = {
                    .timestamp = orderLog.timestamp + timestampOffset,
                    .direction = direction,
                    .volume = orderLog.volume,
                    .price = orderLog.price,
            };
            std::strcpy(twapOrder.instrumentId, idToInstrumentMap[orderLog.instrument].c_str());
            twapOrders.push_back(twapOrder);
        }
    }
    std::sort(twapOrders.begin(), twapOrders.end());
    return twapOrders;
}

std::vector<pnl_and_pos> TradeEngine::getPNLAndPos() {
    std::vector<pnl_and_pos> pnlAndPos;
    for (auto &contractEngine : contractEngineMap) {
        pnl_and_pos pnlAndPosItem = {
                .position = contractEngine.second->getPosition(),
                .pnl = contractEngine.second->getPNL()
        };
        std::strcpy(pnlAndPosItem.instrumentId, idToInstrumentMap[contractEngine.first].c_str());
        pnlAndPos.push_back(pnlAndPosItem);
    }
    std::sort(pnlAndPos.begin(), pnlAndPos.end());
    return pnlAndPos;
}