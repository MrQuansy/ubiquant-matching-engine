//
// Created by Yongzao Dan on 2023/8/27.
//


#include "net/client.h"
#include "trade_engine.h"

void TradeEngine::initContract(const unsigned long &instrument, const double &prevClosePrice, const int &prevPosition) {
    compact_prev_trade_info prevTradeInfo = {
            .prevClosePrice = prevClosePrice,
            .prevPosition = prevPosition
    };
    contractEngines[getOrInsertInstrument(instrument)] =
            new ContractEngine(session.first, session.second, prevTradeInfo, instrument);
}

void TradeEngine::insertAlpha(const unsigned long &instrument, const long long &timestamp, const int &targetVolume) {

    if (timestampOffset == -1 && !ENABLE_DEBUG_TRADE_LOG) {
        timestampOffset = timestamp;
    }

    compact_alpha alpha = {
            .timestamp = static_cast<int>(timestamp - timestampOffset),
            .targetVolume = targetVolume
    };
    contractEngines[getOrInsertInstrument(instrument)]->insertAlpha(alpha);
}

void TradeEngine::insertOrderLog(
        const unsigned long &instrument,
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
    contractEngines[getOrInsertInstrument(instrument)]->insertOrderLog(orderLog);
}

void TradeEngine::onComplete() {
    for (int i = 0; i < lastContract; i++) {
        contractEngines[i]->onComplete();
    }

    std::vector<twap_order> twapOrders = getTWAPOrders();
    std::vector<pnl_and_pos> pnlAndPos = getPNLAndPos();
    std::string fileName = path + "_" + std::to_string(session.first) + "_" + std::to_string(session.second);
    //sentResult(fileName, twapOrders, pnlAndPos);

    std::cout << "[Network] Finish send result: " << fileName << std::endl;
}

inline unsigned long reverseInstrumentId(unsigned long instrumentId) {
    unsigned long result = 0;
    for (int i = 0; i < 8; i++) {
        result = (result << 8) | (instrumentId & 0xff);
        instrumentId >>= 8;
    }
    return result;
}

std::vector<twap_order> TradeEngine::getTWAPOrders() {
    std::vector<twap_order> twapOrders;
    for (int i = 0; i < lastContract; i++) {
        ContractEngine *contractEngine = contractEngines[i];
        unsigned long instrumentId = reverseInstrumentId(contractEngine->getInstrument());
        int twapSize = contractEngine->getTwapSize();
        compact_order_log* contractTwapOrders = contractEngine->getTwapOrders();
        for (int j = 0; j < twapSize; j++) {
            compact_order_log orderLog = contractTwapOrders[j];
            int direction = orderLog.directionAndType >> DIRECTION_OFFSET;
            direction = direction == Sale ? -1 : 1;
            twap_order twapOrder = {
                    .timestamp = orderLog.timestamp + timestampOffset,
                    .direction = direction,
                    .volume = orderLog.volume,
                    .price = orderLog.price,
            };
            twapOrder.instrumentId = instrumentId;
            twapOrders.push_back(twapOrder);
        }
    }
    std::sort(twapOrders.begin(), twapOrders.end());
    for (auto &twapOrder : twapOrders) {
        twapOrder.instrumentId = reverseInstrumentId(twapOrder.instrumentId);
    }
    return twapOrders;
}

std::vector<pnl_and_pos> TradeEngine::getPNLAndPos() {
    std::vector<pnl_and_pos> pnlAndPos;
    for (int i = 0; i < lastContract; i++) {
        ContractEngine *contractEngine = contractEngines[i];
        unsigned long instrumentId = reverseInstrumentId(contractEngine->getInstrument());
        pnl_and_pos pnlAndPosItem = {
                .position = contractEngine->getPosition(),
                .pnl = contractEngine->getPNL()
        };
        pnlAndPosItem.instrumentId = instrumentId;
        pnlAndPos.push_back(pnlAndPosItem);
    }
    std::sort(pnlAndPos.begin(), pnlAndPos.end());
    for (auto &pnlAndPo : pnlAndPos) {
        pnlAndPo.instrumentId = reverseInstrumentId(pnlAndPo.instrumentId);
    }
    return pnlAndPos;
}

int TradeEngine::getOrInsertInstrument(const unsigned long &instrument) {
    int index = instrument % MOD;
    if (contractPairs[index].first == instrument) {
        return contractPairs[index].second;
    }

    while (contractPairs[index].first && contractPairs[index].first != instrument) {
        index = index < MOD ? index + 1 : 0;
    }

    contractPairs[index] = std::make_pair(instrument, lastContract);
    return lastContract++;
}