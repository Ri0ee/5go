#pragma once

#include <cstdint>
#include <iostream>
#include <format>
#include <array>
#include <set>
#include <immintrin.h>

#include "table.h"
#include "stats.h"

namespace bb {
	typedef std::pair<int16_t, uint64_t> scoreAndMove;
	typedef std::set<scoreAndMove, std::greater<scoreAndMove>> scoreAndMoveSet;

	constexpr auto pow = [](uint64_t a, uint64_t b) {
		uint64_t p = 1;
		while (b) {
			if (b & 1) p *= a;
			b >>= 1;
			a *= a;
		}
		return p;
	};

	constexpr auto qbit = [](int x, int y) -> int {
		return 1 << (3 * x + y);
	};

	constexpr auto maskGen = [](int idx0, int idx1, int idx2, int idx3, int idx4) -> uint64_t {
		return (uint64_t)0x1 << idx0 | (uint64_t)0x1 << idx1 | (uint64_t)0x1 << idx2 | (uint64_t)0x1 << idx3 | (uint64_t)0x1 << idx4;
	};

	static constexpr auto packTable = [] {
		std::array<uint16_t, 512> pack{};

		for (int i = 0; i < 512; i++) {
			uint16_t pv = 0;

			for (int j = 0; j < 9; j++)
				if (i & 1 << j)
					pv += (uint16_t)pow(3, j);

			pack[i] = pv;
		}

		return pack;
	} ();

	static constexpr auto unpackTable = [] {
		std::array<std::array<uint16_t, 2>, pow(3, 9)> unpack{};

		for (int v = 0; v < pow(3, 9); v++) {
			auto vv = v;
			uint16_t p0 = 0, p1 = 0;

			for (int i = 0; i < 9; i++) {
				switch (vv % 3) {
				case 1: p0 += 1 << i; break;
				case 2: p1 += 1 << i; break;
				}
				vv /= 3;
			}

			unpack[v] = { p0, p1 };
		}

		return unpack;
	} ();

	static constexpr auto rotationTable = [] {
		std::array<std::array<uint16_t, 2>, 512> rotations{};

		for (int v = 0; v < 512; v++) {
			uint16_t left = 0, right = 0;
			for (int x = 0; x < 3; x++) {
				for (int y = 0; y < 3; y++) {
					if (v & qbit(x, y)) {
						left |= qbit(2 - y, x);
						right |= qbit(y, 2 - x);
					}
				}
			}

			rotations[v] = { left, right };
		}
		
		return rotations;
	} ();

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
		maskGen(2, 5, 8, 34, 37), maskGen(5, 8, 34, 37, 40),
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
		0, 1, 3, 9, 27, 81
	};

	inline uint64_t quadrants(uint16_t q0, uint16_t q1, uint16_t q2, uint16_t q3);
	inline uint16_t quadrant(uint64_t bitboard, int q);
	inline uint16_t pack(uint16_t p0, uint16_t p1);
	inline uint64_t pack(uint64_t p0, uint64_t p1);
	inline uint16_t unpack(uint16_t packedQuadrant, int s);
	inline uint64_t unpack(uint64_t packedBitboard, int s);
	inline uint16_t rotate(uint16_t quadrant, int dir);
	
	uint64_t flipSides(uint64_t bitboard);
	inline uint64_t flipSides(uint64_t p0, uint64_t p1);

	uint8_t count(uint64_t bitboard);
	inline uint8_t count(uint64_t p0, uint64_t p1);

	scoreAndMoveSet advances(uint64_t packedBitboard);
	inline scoreAndMoveSet advances(uint64_t p0, uint64_t p1);

	int16_t eval(uint64_t packedBitboard);
	inline int16_t eval(uint64_t p0, uint64_t p1);

	int winner(uint64_t packedBitboard);
	inline int winner(uint64_t p0, uint64_t p1);
}