#pragma once

#include <iostream>
#include <cinttypes>
#include <utility>

#include "stats.h"

namespace tt {
	struct Entry {
		uint64_t key;
		uint64_t bestMove;
		int16_t score;
		uint8_t depth;

		enum class Type : uint8_t {
			undefined = 0,
			exact = 1,
			alphaBound = 2,
			betaBound = 3
		} type;
	};

	static constexpr int tableSize = 27;
	static constexpr int keyMask = (1 << tableSize) - 1;
	extern tt::Entry* table;

	void init();
	void reset();
	bool valid();
	void set(uint64_t bitboard, uint64_t bestMove, int16_t score, uint8_t depth, Entry::Type type);
	Entry& get(uint64_t bitboard);
	uint64_t hash(uint64_t bitboard);
}