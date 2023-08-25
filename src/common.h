//
// Created by Yongzao Dan on 2023/8/25.
//

#ifndef UBIQUANTMATCHINGENGINE_COMMON_H
#define UBIQUANTMATCHINGENGINE_COMMON_H

const double EPS = 1E-4;

#define _abs(x) ((x) < 0 ? -(x) : (x))

struct OrderLog {
    int timestamp;
    int volume;
    double priceOff;
    unsigned char instrument;
    unsigned char typeAndDirection;

//    bool operator > (const OrderLog &o) const {
//        if (_abs(priceOff - o.priceOff) > EPS) {
//            return priceOff > o.priceOff;
//        }
//        return timestamp < o.timestamp;
//    }

    // Test Only
//    bool operator < (const OrderLog &o) const {
//        if (_abs(priceOff - o.priceOff) > EPS) {
//            return priceOff < o.priceOff;
//        }
//        return timestamp < o.timestamp;
//    }
} __attribute__((packed));

struct minHeapCmp {
    // return true if o1.priceOff < o2.priceOff || (o1.priceOff == o2.priceOff && o1.timestamp < o2.timestamp)
    bool operator () (const OrderLog &o1, const OrderLog &o2) const {
        if (_abs(o1.priceOff - o2.priceOff) > EPS) {
            return o1.priceOff < o2.priceOff;
        }
        return o1.timestamp < o2.timestamp;
    }
};

struct maxHeapCmp {
    // return true if o1.priceOff > o2.priceOff || (o1.priceOff == o2.priceOff && o1.timestamp < o2.timestamp)
    bool operator () (const OrderLog &o1, const OrderLog &o2) const {
        if (_abs(o1.priceOff - o2.priceOff) > EPS) {
            return o1.priceOff > o2.priceOff;
        }
        return o1.timestamp < o2.timestamp;
    }
};

#endif //UBIQUANTMATCHINGENGINE_COMMON_H
