#pragma once

#include <cstdint>
#include <iostream>
#include <format>
#include <array>
#include <immintrin.h>

namespace bb {
	static auto maskGen = [](const int idx) constexpr -> uint64_t {
		return (uint64_t)0x1 << idx;
	};

	static constexpr uint64_t mask[36] = {
		maskGen(0),  maskGen(1),  maskGen(2),  maskGen(3),  maskGen(4),  maskGen(5),
		maskGen(6),  maskGen(7),  maskGen(8),  maskGen(9),  maskGen(10), maskGen(11),
		maskGen(12), maskGen(13), maskGen(14), maskGen(15), maskGen(16), maskGen(17),
		maskGen(18), maskGen(19), maskGen(20), maskGen(21), maskGen(22), maskGen(23),
		maskGen(24), maskGen(25), maskGen(26), maskGen(27), maskGen(28), maskGen(29),
		maskGen(30), maskGen(31), maskGen(32), maskGen(33), maskGen(34), maskGen(35),
	};

	// inverted quadrant masks
	static constexpr uint64_t invqmask[4] = {
		0x1C71FFFFF, 0xE38E3FFFF,
		0xFFFFC71C7, 0xFFFFF8E38
	};

	// rotation mask
	static constexpr uint64_t rmask[36] = {
		0x800000000,       0x400000000,       0x200000000,       0x20000000,       0x10000000,       0x8000000,       0x800000,       0x400000,       0x200000,
		0x800000000 >>  3, 0x400000000 >>  3, 0x200000000 >>  3, 0x20000000 >>  3, 0x10000000 >>  3, 0x8000000  >> 3, 0x800000 >>  3, 0x400000 >>  3, 0x200000 >> 3,
		0x800000000 >> 18, 0x400000000 >> 18, 0x200000000 >> 18, 0x20000000 >> 18, 0x10000000 >> 18, 0x8000000 >> 18, 0x800000 >> 18, 0x400000 >> 18, 0x200000 >> 18,
		0x800000000 >> 21, 0x400000000 >> 21, 0x200000000 >> 21, 0x20000000 >> 21, 0x10000000 >> 21, 0x8000000 >> 21, 0x800000 >> 21, 0x400000 >> 21, 0x200000 >> 21
	};

	static constexpr auto packTable = [] {
		std::array<uint16_t, 512> pack{};

		for (int i = 0; i < 512; i++) {
			uint16_t pv = 0;

			for (int j = 0; j < 9; j++)
				if (i & 1 << j)
					pv += j * j * j;

			pack[i] = pv;
		}

		return pack;
	} ();

	static constexpr auto unpackTable = [] {
		std::array<std::array<uint16_t, 2>, 19683> unpack{};

		for (int v = 0; v < 19683; v++) {
			auto vv = v;
			uint16_t p0 = 0, p1 = 0;

			for (int i = 0; i < 9; i++) {
				const auto c = vv % 3;
				if (c == 1)
					p0 += 1 << i;
				else if (c == 2)
					p1 += 1 << i;
				vv /= 3;
			}

			unpack[v] = std::array<uint16_t, 2>{ p0, p1 };
		}

		return unpack;
	} ();

	void printBitboard(uint64_t bitboard);
	uint64_t set(uint64_t bitboard, uint8_t pos);
	uint64_t unset(uint64_t bitboard, uint8_t pos);
 	uint64_t get(uint64_t bitboard, uint8_t pos);
	uint64_t rotate90CW(uint64_t bitboard, uint8_t qidx);
	uint64_t rotate90CCW(uint64_t bitboard, uint8_t qidx);
	bool won(uint64_t bitboard);

	uint64_t quadrants(uint16_t q0, uint16_t q1, uint16_t q2, uint16_t q3);
	uint16_t quadrant(uint64_t bitboard, int q);
	uint16_t pack(uint16_t side0, uint16_t side1);
	uint64_t pack(uint64_t side0, uint64_t side1);
	uint16_t unpack(uint16_t state, int s);
	uint64_t unpack(uint64_t bitboard, int s);
	uint8_t count_stones(uint64_t bitboard);
}