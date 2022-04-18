#include "table.h"

tt::Entry* tt::table = nullptr;

void tt::init() {
	tt::table = new Entry[1 << tt::tableSize]();

	stats::stats["tt size (bytes)"] = ((1 << tt::tableSize) * sizeof(tt::Entry));
	stats::stats["tt entry size"] = sizeof(tt::Entry);
}

void tt::reset() {
	delete[] tt::table;
	tt::table = nullptr;
}

bool tt::valid() {
	return tt::table != nullptr;
}

void tt::set(uint64_t bitboard, uint64_t bestMove, int16_t score, uint8_t depth, tt::Entry::Type type) {
	uint64_t hash = tt::hash(bitboard);
	auto& entry = tt::table[hash & tt::keyMask];

	entry.key = hash;
	entry.bestMove = bestMove;
	entry.score = score;
	entry.depth = depth;
	entry.type = type;
}

tt::Entry& tt::get(uint64_t bitboard) {
	uint64_t hash = tt::hash(bitboard);
	tt::Entry& entry = tt::table[hash & tt::keyMask];

	if (entry.key != hash && entry.key != 0) {
		entry.type = tt::Entry::Type::undefined;
		stats::stats["tt collisions"]++;
	}

	return entry;
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