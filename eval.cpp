#include "eval.h"

static int ttHits = 0;

int16_t ev::eval(uint64_t packedBitboard) {
    uint64_t p0 = bb::unpack(packedBitboard, 0);
    uint64_t p1 = bb::unpack(packedBitboard, 1);
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

    return score;
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
    
    uint8_t moves = bb::count(p0, p1);
    if (moves % 2 == 0)
        return score;
    else
        return -score;
}

int ev::winner(uint64_t packedBitboard) {
    return ev::winner(bb::unpack(packedBitboard, 0), bb::unpack(packedBitboard, 1));
}

int ev::winner(uint64_t p0, uint64_t p1) {
    bool firstPlayerWin = false, secondPlayerWin = false;

    for (uint8_t i = 0; i < 32; i++) {
        uint64_t mask = ev::winMasks[i];
        uint64_t p0streaks = p0 & mask;
        uint64_t p1streaks = p1 & mask;

        firstPlayerWin = firstPlayerWin || p0streaks == mask;
        secondPlayerWin = secondPlayerWin || p1streaks == mask;
    }

    if (!firstPlayerWin && !secondPlayerWin)
        return 0;

    if (firstPlayerWin)
        return 1;

    if (secondPlayerWin)
        return 2;

    return 3;
}