#pragma once

#include <cinttypes>
#include <utility>

namespace tt {
	struct Entry {
		uint64_t key;
		uint64_t bestMove;
		int16_t score;
		uint8_t depth;
		uint8_t type;
	};

	void init();
	void set(uint64_t bitboard, uint64_t bestMove, int16_t score, uint8_t depth, uint8_t type);
	Entry& get(uint64_t bitboard);
	uint64_t hash(uint64_t bitboard);
}