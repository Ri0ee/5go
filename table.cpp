#include "table.h"

static uint64_t* table;

static constexpr int tableSize = 28;

static constexpr int scoreSize = 16;
static constexpr int scoreMask = (1 << scoreSize) - 1;

static constexpr int depthSize = 8;
static constexpr int depthMask = ((1 << depthSize) - 1) << scoreSize;

void tt::init() {
	table = new uint64_t[1 << tableSize]();
}

void tt::set(uint64_t bitboard, int16_t score, int8_t depth) {
	uint64_t h = tt::hash(bitboard);
	table[h & ((1 << tableSize) - 1)] = (depth << 16) | score;
}

// return pair<score, depth>
std::pair<int16_t, uint8_t> tt::get(uint64_t bitboard) {
	uint64_t h = hash(bitboard);
	uint64_t entry = table[h & ((1 << tableSize) - 1)];
	return std::make_pair((int16_t)(entry & scoreMask), (uint8_t)(entry & depthMask));
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