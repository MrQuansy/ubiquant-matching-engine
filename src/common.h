//
// Created by Yongzao Dan on 2023/8/25.
//

#ifndef UBIQUANTMATCHINGENGINE_COMMON_H
#define UBIQUANTMATCHINGENGINE_COMMON_H

#include <cstring>
#include <utility>

const static double EPS = 1E-4;
const static int MAX_INT = ~0U >> 1;
const static unsigned char MY_INSTRUMENT_ID = 0;
const static unsigned char TYPE_MASK = 7;
const static unsigned char DIRECTION_MASK = 8;
const static unsigned char DIRECTION_OFFSET = 3;

const static std::pair<int, int> SESSIONS[5] = {
    std::make_pair(3, 1),
    std::make_pair(3, 3),
    std::make_pair(3, 5),
    std::make_pair(5, 2),
    std::make_pair(5, 3)
};

#define _abs(x) ((x) < 0 ? -(x) : (x))
#define _min(x, y) ((x) < (y) ? (x) : (y))

enum ContractType : unsigned char {
    PriceLimit = (unsigned char) 0,
    OppositeBestPrice = (unsigned char) 1,
    OurBestPrice = (unsigned char) 2,
    BestFivePriceNoLeft = (unsigned char) 3,
    ImmediateDealNoLeft = (unsigned char) 4,
    AllOrNothing = (unsigned char) 5,
    MyContract = (unsigned char) 6
};

enum Direction : unsigned char {
    Sale = (unsigned char) 0,
    Buy = (unsigned char) 1
};

struct OrderLog {
    int timestamp;
    int volume;
    double price;
    unsigned char instrument;
    unsigned char directionAndType;
} __attribute__((packed));

struct Compare {
    virtual bool operator () (const OrderLog &o1, const OrderLog &o2) const = 0;
};

struct MinBinaryHeapCmp : Compare {
    // return true if o1.price < o2.price || (o1.price == o2.price && o1.timestamp < o2.timestamp)
    bool operator () (const OrderLog &o1, const OrderLog &o2) const {
        if (_abs(o1.price - o2.price) > EPS) {
            // 1. Trade minimal price
            return o1.price < o2.price;
        }
        return o1.timestamp != o2.timestamp ?
               // 2. Trade earlier order
               o1.timestamp < o2.timestamp :
               // 3. Trade MyContract at last
               (o1.directionAndType & TYPE_MASK) < (o2.directionAndType & TYPE_MASK);
    }
};

struct MaxBinaryHeapCmp : Compare {
    // return true if o1.price > o2.price || (o1.price == o2.price && o1.timestamp < o2.timestamp)
    bool operator () (const OrderLog &o1, const OrderLog &o2) const {
        if (_abs(o1.price - o2.price) > EPS) {
            // 1. Trade maximal price
            return o1.price > o2.price;
        }
        return o1.timestamp != o2.timestamp ?
               // 2. Trade earlier order
               o1.timestamp < o2.timestamp :
               // 3. Trade MyContract at last
               (o1.directionAndType & TYPE_MASK) < (o2.directionAndType & TYPE_MASK);
    }
};

struct PrevTradeInfo {
    double prevClosePrice;
    int prevPosition;
    unsigned char instrument;
};

struct Alpha {
    int timestamp;
    int targetVolume;
    unsigned char instrument;
};

// Input Only
struct prev_trade_info {
    char instrument_id[8];
    double prev_close_price;
    int prev_position;
} __attribute__((packed));

struct order_log {
    char instrument_id[8];
    long timestamp;
    int type;
    int direction;
    int volume;
    double price_off;
} __attribute__((packed));

struct alpha {
    char instrument_id[8];
    long timestamp;
    int target_volume;
} __attribute__((packed));

// Output Only
struct twap_order {
    char instrumentId[8];
    long timestamp;
    int direction;
    int volume;
    double price;

    bool operator < (const twap_order &o) const {
        if (timestamp != o.timestamp) {
            return timestamp < o.timestamp;
        }
        return std::strcmp(instrumentId, o.instrumentId);
    }
} __attribute__((packed));

struct pnl_and_pos {
    char instrumentId[8];
    int position;
    double pnl;

    bool operator < (const pnl_and_pos &o) const {
        return std::strcmp(instrumentId, o.instrumentId);
    }
} __attribute__((packed));

#endif //UBIQUANTMATCHINGENGINE_COMMON_H
