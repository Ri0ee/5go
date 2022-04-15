#include "table.h"

static tt::Entry* table;

static constexpr int tableSize = 27;
static constexpr int keyMask = (1 << tableSize) - 1;

void tt::init() {
	table = new Entry[1 << tableSize]();
}

void tt::set(uint64_t bitboard, uint64_t bestMove, int16_t score, uint8_t depth, uint8_t type) {
	uint64_t h = tt::hash(bitboard);
	auto& entry = table[h & keyMask];

	entry.key = h;
	entry.bestMove = bestMove;
	entry.score = score;
	entry.depth = depth;
	entry.type = type;
}

tt::Entry& tt::get(uint64_t bitboard) {
	return table[hash(bitboard) & keyMask];
}

uint64_t tt::hash(uint64_t bitboard) {
	// Thomas Wang, Jan	1997, inrevertible integer hash function
	bitboard = (~bitboard) + (bitboard << 21);
	bitboard = bitboard ^ (bitboard >> 24);
	bitboard = (bitboard + (bitboard << 3)) + (bitboard << 8);
	bitboard = bitboard ^ (bitboard >> 14);
	bitboard = (bitboard + (bitboard << 2)) + (bitboard << 4);
	bitboard = bitboard ^ (bitboard >> 28);
	bitboard = bitboard + (bitboard << 31);
	return bitboard;
}