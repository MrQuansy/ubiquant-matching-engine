//
// Created by Yongzao Dan on 2023/8/25.
//

#ifndef UBIQUANTMATCHINGENGINE_COMMON_H
#define UBIQUANTMATCHINGENGINE_COMMON_H

#include <string>
#include <iomanip>
#include <cstring>
#include <utility>
#include <iostream>
#include <cmath>
#include <atomic>
#include "config.h"

const static double EPS = 1E-6;
const static int MAX_INT = ~0U >> 1;
const static unsigned char TYPE_MASK = 7;
const static unsigned char DIRECTION_MASK = 8;
const static unsigned char DIRECTION_OFFSET = 3;

constexpr time_t MICRO_TO_NANO = 1000;
constexpr time_t MILLI_TO_NANO = 1000000;
constexpr time_t SECOND_TO_NANO = 1000000000;
inline time_t now()
{
    static time_t t0 = time(NULL);
    static timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return (t.tv_sec - t0) * SECOND_TO_NANO + t.tv_nsec;
}

#define _abs(x) ((x) < 0 ? -(x) : (x))
#define _min(x, y) ((x) < (y) ? (x) : (y))
#define _max(x, y) ((x) > (y) ? (x) : (y))

#define _eq(x, y) (_abs((x) - (y)) <= EPS)
#define _neq(x, y) (_abs((x) - (y)) > EPS)
#define _le(x, y) ((x) < (y) && _neq(x, y))
#define _gt(x, y) ((x) > (y) && _neq(x, y))

// Use this function carefully since it's time-consuming
const static double FLOOR = 0.5 - EPS;
inline double highPrecisionRound2(const double &x) {
//    double sign = x < 0 ? -1.0 : 1.0;
//    double y = x * 100.0 * sign;
//    double _y = (long long) y;
//    _y += y - _y > FLOOR ? 1.0 : 0.0;
//    return _y / 100.0 * sign;

//    double y = x * 100.0;
//    double _y = (long long) y;
//    _y += y - _y > FLOOR ? 1.0 : 0.0;
//    return _y / 100.0;

    return (double) ((long long) (x * 100.0 + (0.5 + EPS))) / 100.0;
}

inline double highPrecisionRound2CoverNegative(const double &x) {
    double sign = x < 0 ? -1.0 : 1.0;
    double y = x * 100.0 * sign;
    double _y = (long long) y;
    _y += y - _y > FLOOR ? 1.0 : 0.0;
    return _y / 100.0 * sign;

//    double sign = x < 0 ? -1.0 : 1.0;
//    double y = x * 100.0;
//    return (double) ((long long) (y + (0.5 + EPS) * sign)) / 100.0;
}

const static std::pair<int, int> SESSIONS[5] = {
    std::make_pair(3, 1),
    std::make_pair(3, 3),
    std::make_pair(3, 5),
    std::make_pair(5, 2),
    std::make_pair(5, 3)
};

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

struct compact_order_log {
    int timestamp;
    int volume;
    double price;
    unsigned char directionAndType;

    friend std::ostream& operator<<(std::ostream& os, const compact_order_log& o) {
        os << "history order: ";
        os << "timestamp=" << o.timestamp << ", ";
        os << "direction=" << ((int) (o.directionAndType >> DIRECTION_OFFSET) ? 1 : -1) << ", ";
        os << "order_type=" << ((int) (o.directionAndType & TYPE_MASK)) << ", ";
        os << "volume=" << o.volume;
        return os;
    }

} __attribute__((packed));

struct Compare {
    virtual bool operator () (const compact_order_log &o1, const compact_order_log &o2) const = 0;
};

struct MinBinaryHeapCmp : Compare {
    // return true if o1.price < o2.price || (o1.price == o2.price && o1.timestamp < o2.timestamp)
    bool operator () (const compact_order_log &o1, const compact_order_log &o2) const override {
        if (_neq(o1.price, o2.price)) {
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
    bool operator () (const compact_order_log &o1, const compact_order_log &o2) const override {
        if (_neq(o1.price, o2.price)) {
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

struct compact_prev_trade_info {
    double prevClosePrice;
    int prevPosition;

    friend std::ostream& operator<<(std::ostream& os, const compact_prev_trade_info& o) {
        os << "prev trade info: ";
        os << "prev_close_price=" << o.prevClosePrice << ", ";
        os << "prev_position=" << o.prevPosition;
        return os;
    }
};

struct compact_alpha {
    int timestamp;
    int targetVolume;
};

// Input Only
struct prev_trade_info {
    unsigned long instrument_id;
    double prev_close_price;
    int prev_position;
} __attribute__((packed));

struct order_log {
    unsigned long instrument_id;
    long timestamp;
    int type;
    int direction;
    int volume;
    double price_off;
} __attribute__((packed));

struct alpha {
    unsigned long instrument_id;
    long timestamp;
    int target_volume;
} __attribute__((packed));

// Output Only
struct twap_order {
    unsigned long instrumentId;
    long timestamp;
    int direction;
    int volume;
    double price;

    bool operator < (const twap_order &o) const {
        if (timestamp != o.timestamp) {
            return timestamp < o.timestamp;
        }
        return instrumentId < o.instrumentId;
    }

    bool operator == (const twap_order &o) const {
        return instrumentId == o.instrumentId && timestamp == o.timestamp &&
               direction == o.direction && volume == o.volume && _eq(price, o.price);
    }

    friend std::ostream& operator<<(std::ostream& os, const twap_order& o) {
        os << "twap_order: {.instrumentId: " << (char *)&o.instrumentId;
        os << " .timestamp: " << o.timestamp;
        os << " .direction: " << o.direction;
        os << " .volume: " << o.volume;
        os << " .price: " << o.price << "}";
        return os;
    }

} __attribute__((packed));

struct pnl_and_pos {
    unsigned long instrumentId;
    int position;
    double pnl;

    bool operator < (const pnl_and_pos &o) const {
        return instrumentId < o.instrumentId;
    }

    bool operator == (const pnl_and_pos &o) const {
        return instrumentId == o.instrumentId && position == o.position && _eq(pnl, o.pnl);
    }

    friend std::ostream& operator<<(std::ostream& os, const pnl_and_pos& o) {
        os << "pnl_and_pos: {.instrumentId: " << (char*)&o.instrumentId;
        os << " .position: " << o.position;
        os << " .pnl: " << o.pnl << "}";
        return os;
    }

} __attribute__((packed));

#endif //UBIQUANTMATCHINGENGINE_COMMON_H
