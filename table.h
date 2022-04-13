#pragma once

#include <array>
#include <board.h>

namespace tt {
	void init();
	void set(uint64_t bitboard, int16_t score, int8_t depth);
	std::pair<int16_t, uint8_t> get(uint64_t bitboard);
	uint64_t hash(uint64_t bitboard);
}