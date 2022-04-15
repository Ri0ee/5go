#pragma once

#include <array>
#include <iostream>
#include <cstdint>
#include <immintrin.h>

#include "bitboard.h"
#include "table.h"
#include "stats.h"

namespace ev {
	static auto maskGen = [](const int idx0, const int idx1, const int idx2, const int idx3, const int idx4) constexpr -> uint64_t {
		return (uint64_t)0x1 << idx0 | (uint64_t)0x1 << idx1 | (uint64_t)0x1 << idx2 | (uint64_t)0x1 << idx3 | (uint64_t)0x1 << idx4;
	};

	static constexpr uint64_t winMasks[32] = {
		// horizontal
		maskGen(0, 1, 2, 16, 17), maskGen(1, 2, 16, 17, 18),
		maskGen(3, 4, 5, 19, 20), maskGen(4, 5, 19, 20, 21),
		maskGen(6, 7, 8, 22, 23), maskGen(7, 8, 22, 23, 24),
		maskGen(32, 33, 34, 48, 49), maskGen(33, 34, 48, 49, 50),
		maskGen(35, 36, 37, 51, 52), maskGen(36, 37, 51, 52, 53),
		maskGen(38, 39, 40, 54, 55), maskGen(39, 40, 54, 55, 56),

		// vertical
		maskGen(0, 3, 6, 32, 35), maskGen(3, 6, 32, 35, 38),
		maskGen(1, 4, 7, 33, 36), maskGen(4, 7, 33, 36, 39),
		maskGen(2, 5, 8, 34, 37), maskGen(5, 6, 34, 37, 40),
		maskGen(16, 19, 22, 48, 51), maskGen(19, 22, 48, 51, 54),
		maskGen(17, 20, 23, 49, 52), maskGen(20, 23, 49, 52, 55),
		maskGen(18, 21, 24, 50, 53), maskGen(21, 24, 50, 53, 56),

		// diagonal
		maskGen(0, 4, 8, 48, 52), maskGen(4, 8, 48, 52, 56), // main diagonal
		maskGen(0, 5, 22, 49, 53), // top diagonal
		maskGen(3, 7, 34, 51, 55), // bottom diagonal

		// counter-diagonal
		maskGen(18, 20, 22, 34, 36), maskGen(20, 22, 34, 36, 38), // main diagonal
		maskGen(8, 17, 19, 33, 35), // top diagonal
		maskGen(21, 23, 37, 39, 48), // bottom diagonal
	};

	static constexpr int16_t scoreDistribution[6] = {
		0, 1, 3, 9, 27, 127
	};

	int16_t eval(uint64_t packedBitboard);
	int16_t eval(uint64_t p0, uint64_t p1);
	int winner(uint64_t packedBitboard);
	int winner(uint64_t p0, uint64_t p1);
}