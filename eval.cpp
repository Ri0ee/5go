#include "eval.h"

int16_t ev::eval(uint64_t packedBitboard) {
    return ev::eval(bb::unpack(packedBitboard, 0), bb::unpack(packedBitboard, 1));
}

int ev::winner(uint64_t packedBitboard) {
    return ev::winner(bb::unpack(packedBitboard, 0), bb::unpack(packedBitboard, 1));
}

int16_t ev::eval(uint64_t p0, uint64_t p1) {
    int16_t score = 0;

    for (uint8_t i = 0; i < 32; i++) {
        uint64_t mask = ev::winMasks[i];
        uint64_t p0streaks = p0 & mask;
        uint64_t p1streaks = p1 & mask;

        if (p0streaks && !p1streaks)
            score += ev::scoreDistribution[__popcnt64(p0streaks)];
        else if (!p0streaks && p1streaks)
            score -= ev::scoreDistribution[__popcnt64(p1streaks)];
    }
    
    const auto moves = bb::count(p0, p1);
    return (moves % 2 == 0 ? -score : score);
}

int ev::winner(uint64_t p0, uint64_t p1) {
    bool p0win = false, p1win = false;

    for (uint8_t i = 0; i < 32; i++) {
        uint64_t mask = ev::winMasks[i];
        uint64_t p0streaks = p0 & mask;
        uint64_t p1streaks = p1 & mask;

        p0win = p0win || p0streaks == mask;
        p1win = p1win || p1streaks == mask;
    }

    uint8_t moves = bb::count(p0, p1);

    if (p0win && p1win || (!p0win && !p1win && moves == 36)) return 3; // draw
    if (p0win) return 1; // first player win
    if (p1win) return 2; // second player win

    return 0; // game still going
}
