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

const static double EPS = 1E-6;
const static int MAX_INT = ~0U >> 1;
const static unsigned char TYPE_MASK = 7;
const static unsigned char DIRECTION_MASK = 8;
const static unsigned char DIRECTION_OFFSET = 3;

#define _abs(x) ((x) < 0 ? -(x) : (x))
#define _min(x, y) ((x) < (y) ? (x) : (y))
#define _max(x, y) ((x) > (y) ? (x) : (y))

#define _eq(x, y) (_abs((x) - (y)) <= EPS)
#define _neq(x, y) (_abs((x) - (y)) > EPS)
#define _le(x, y) ((x) < (y) && _neq(x, y))
#define _gt(x, y) ((x) > (y) && _neq(x, y))

// Use this function carefully since it's time-consuming
const static double ROUND_MULTI = 1E6;
const static int ROUND_DIV = 1E3;
const static int ROUND_MOD = 10;
const static int ROUND_CMP = 4;
inline double highPrecisionRound2(const double &x) {
    int y = std::round(x * ROUND_MULTI) / ROUND_DIV;
    y = (y % ROUND_MOD > ROUND_CMP) ? y / 10 + 1 : y / 10;
    return y / 100.0;
}

const static int MAX_TWAP_LENGTH = 480 * 5;

const static std::pair<int, int> SESSIONS[5] = {
    std::make_pair(3, 1),
    std::make_pair(3, 3),
    std::make_pair(3, 5),
    std::make_pair(5, 2),
    std::make_pair(5, 3)
};

// Input path: DATA_PREFIX + DATE + FILE_NAME
const static std::string DATA_PREFIX = "/mnt/data/";
const static std::string ALPHA = "/alpha";
const static std::string ORDER_LOG = "/order_log";
const static std::string PREV_TRADE_INFO = "/prev_trade_info";

// Output Path: OUTPUT_PREFIX + DATE + FILE_NAME + SESSION
const static std::string TEST_OUTPUT_PREFIX = "/mnt/test_data/";
const static std::string STD_OUTPUT_PREFIX = "/mnt/output_adjuest";
const static std::string TWAP_ORDER = "/twap_order";
const static std::string PNL_AND_POSITION = "/pnl_and_position";

// Debug Fields
const static bool ENABLE_DEBUG_TRADE_LOG = false;
const static std::string DEBUG_PREFIX = "/mnt/logs/";
const static std::string DEBUG_DATE = "20150101";
const static std::string DEBUG_INSTRUMENT = "100.UBE";
const static std::pair<int, int> DEBUG_SESSION = SESSIONS[4];

const static std::string STD_LOG_PREFIX = "/mnt/log_adjust/";

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

    friend std::ostream& operator<<(std::ostream& os, const OrderLog& o) {
        os << "history order: ";
        os << "timestamp=" << o.timestamp << ", ";
        os << "direction=" << ((int) (o.directionAndType >> DIRECTION_OFFSET) ? 1 : -1) << ", ";
        os << "order_type=" << ((int) (o.directionAndType & TYPE_MASK)) << ", ";
        os << "volume=" << o.volume;
        return os;
    }

} __attribute__((packed));

struct Compare {
    virtual bool operator () (const OrderLog &o1, const OrderLog &o2) const = 0;
};

struct MinBinaryHeapCmp : Compare {
    // return true if o1.price < o2.price || (o1.price == o2.price && o1.timestamp < o2.timestamp)
    bool operator () (const OrderLog &o1, const OrderLog &o2) const override {
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
    bool operator () (const OrderLog &o1, const OrderLog &o2) const override {
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

struct PrevTradeInfo {
    double prevClosePrice;
    int prevPosition;
    unsigned char instrument;

    friend std::ostream& operator<<(std::ostream& os, const PrevTradeInfo& o) {
        os << "prev trade info: ";
        os << "prev_close_price=" << o.prevClosePrice << ", ";
        os << "prev_position=" << o.prevPosition;
        return os;
    }
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
        return std::strcmp(instrumentId, o.instrumentId) < 0;
    }

    bool operator == (const twap_order &o) const {
        return std::strcmp(instrumentId, o.instrumentId) == 0 && timestamp == o.timestamp &&
               direction == o.direction && volume == o.volume && _eq(price, o.price);
    }

    friend std::ostream& operator<<(std::ostream& os, const twap_order& o) {
        os << "twap_order: {.instrumentId: " << o.instrumentId;
        os << " .timestamp: " << o.timestamp;
        os << " .direction: " << o.direction;
        os << " .volume: " << o.volume;
        os << " .price: " << o.price << "}";
        return os;
    }

} __attribute__((packed));

struct pnl_and_pos {
    char instrumentId[8];
    int position;
    double pnl;

    bool operator < (const pnl_and_pos &o) const {
        return std::strcmp(instrumentId, o.instrumentId) < 0;
    }

    bool operator == (const pnl_and_pos &o) const {
        return std::strcmp(instrumentId, o.instrumentId) == 0 && position == o.position && _eq(pnl, o.pnl);
    }

    friend std::ostream& operator<<(std::ostream& os, const pnl_and_pos& o) {
        os << "pnl_and_pos: {.instrumentId: " << o.instrumentId;
        os << " .position: " << o.position;
        os << " .pnl: " << o.pnl << "}";
        return os;
    }

} __attribute__((packed));

#endif //UBIQUANTMATCHINGENGINE_COMMON_H
