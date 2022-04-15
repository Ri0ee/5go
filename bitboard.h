#pragma once

#include <cstdint>
#include <iostream>
#include <format>
#include <array>
#include <unordered_set>
#include <queue>
#include <immintrin.h>

namespace bb {
	constexpr auto pow = [](uint64_t a, uint64_t b) {
		uint64_t p = 1;
		while (b) {
			if (b & 1) p *= a;
			b >>= 1;
			a *= a;
		}
		return p;
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
				auto c = vv % 3;
				if (c == 1)
					p0 += 1 << i;
				else if (c == 2)
					p1 += 1 << i;
				vv /= 3;
			}

			unpack[v] = { p0, p1 };
		}

		return unpack;
	} ();

	constexpr auto qbit = [](int x, int y) -> int {
		return 1 << (3 * x + y);
	};

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

	uint64_t quadrants(uint16_t q0, uint16_t q1, uint16_t q2, uint16_t q3);
	uint16_t quadrant(uint64_t bitboard, int q);
	uint16_t pack(uint16_t side0, uint16_t side1);
	uint64_t pack(uint64_t side0, uint64_t side1);
	uint16_t unpack(uint16_t state, int s);
	uint64_t unpack(uint64_t bitboard, int s);
	uint16_t rotate(uint16_t quadrant, int dir);
	
	uint64_t flipSides(uint64_t bitboard);

	uint8_t count(uint64_t bitboard);
	uint8_t count(uint64_t p0, uint64_t p1);

	std::unordered_set<uint64_t> advances(uint64_t packedBitboard);
	std::unordered_set<uint64_t> advances(uint64_t p0, uint64_t p1);
}