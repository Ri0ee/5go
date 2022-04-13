#pragma once

#include <iostream>
#include <vector>
#include <assert.h>
#include <list>
#include <concurrent_priority_queue.h>
#include <queue>
#include <map>
#include <unordered_map>
#include <array>
#include "bitboard.h"

// 35 34 33 32 31 30
// 29 28 27 26 25 24
// 23 22 21 20 19 18
// 17 16 15 14 13 12
// 11 10  9  8  7  6
//  5  4  3  2  1  0

static auto maskGen = [](const int idx0, const int idx1, const int idx2, const int idx3, const int idx4) constexpr -> uint64_t {
	return (uint64_t)0x1 << idx0 | (uint64_t)0x1 << idx1 | (uint64_t)0x1 << idx2 | (uint64_t)0x1 << idx3 | (uint64_t)0x1 << idx4;
};

static constexpr uint64_t winMasks[32] = {
	// horizontal
	maskGen(35, 34, 33, 32, 31), maskGen(34, 33, 32, 31, 30),
	maskGen(29, 28, 27, 26, 25), maskGen(28, 27, 26, 25, 24),
	maskGen(23, 22, 21, 20, 19), maskGen(22, 21, 20, 19, 18),
	maskGen(17, 16, 15, 14, 13), maskGen(16, 15, 14, 13, 12),
	maskGen(11, 10,  9,  8,  7), maskGen(10,  9,  8,  7,  6),
	maskGen( 5,  4,  3,  2,  1), maskGen( 4,  3,  2,  1,  0),

	// vertical
	maskGen(35, 29, 23, 17, 11), maskGen(29, 23, 17, 11,  5),
	maskGen(34, 28, 22, 16, 10), maskGen(28, 22, 16, 10,  4),
	maskGen(33, 27, 21, 15,  9), maskGen(27, 21, 15,  9,  3),
	maskGen(32, 26, 20, 14,  8), maskGen(26, 20, 14,  8,  2),
	maskGen(31, 25, 19, 13,  7), maskGen(25, 19, 13,  7,  1),
	maskGen(30, 24, 18, 12,  6), maskGen(24, 18, 12,  6,  0),

	// diagonal
	maskGen(35, 28, 21, 14,  7), maskGen(28, 21, 14,  7,  0),
	maskGen(34, 27, 20, 13,  6), maskGen(29, 22, 15,  8,  1),

	// counter-diagonal
	maskGen(30, 25, 20, 15, 10), maskGen(25, 20, 15, 10,  5),
	maskGen(31, 26, 21, 16, 11), maskGen(24, 19, 14,  9,  4)
};

static constexpr int16_t scoreDistribution[6] = {
	0,
	1,
	3,
	9,
	27,
	127
};

struct Board {
	Board() {}
	Board(const Board& board) {
		move = board.move;
		bitboard[0] = board.bitboard[0];
		bitboard[1] = board.bitboard[1];
	}

	auto toArray() {
		std::array<int, 36> result{};

		for (const int qx : range(2)) {
			for (const int qy : range(2)) {
				const uint16_t q = bb::quadrant(board, 2 * qx + qy);
				const side_t s0 = unpack(q, 0),
					s1 = unpack(q, 1);
				for (const int x : range(3))
					for (const int y : range(3))
						table(3 * qx + x, 3 * qy + y) = ((s0 >> (3 * x + y)) & 1) + 2 * ((s1 >> (3 * x + y)) & 1);
			}
		}

		return result;
	}

	void debugPrint() {
		std::cout << std::format("move: {}\n", move);
		bb::printBitboard(bitboard[0]); std::cout << '\n';
		bb::printBitboard(bitboard[1]); std::cout << '\n';
		bb::printBitboard(bitboard[0] | bitboard[1]); std::cout << '\n';
	}

	int8_t get(const uint8_t pos) const {
		if (bb::get(bitboard[0], pos))
			return -1;

		if (bb::get(bitboard[1], pos))
			return 1;

		return 0;
	}

	void put(uint8_t pos) {
		bitboard[move % 2] = bb::set(bitboard[move % 2], pos);
		move++;
	}

	void rotate(uint8_t rotation) {
		if (rotation % 2 == 0) {
			bitboard[0] = bb::rotate90CCW(bitboard[0], rotation / 2);
			bitboard[1] = bb::rotate90CCW(bitboard[1], rotation / 2);
		}
		else {
			bitboard[0] = bb::rotate90CW(bitboard[0], (rotation - 1) / 2);
			bitboard[1] = bb::rotate90CW(bitboard[1], (rotation - 1) / 2);
		}
	}

	int8_t winner() {
		if (bb::won(bitboard[0]))
			return -1;

		if (bb::won(bitboard[1]))
			return 1;

		return 0;
	}

	bool stale() {
		return false;
	}

	Board advance(uint8_t pos, uint8_t rotation) {
		Board tempBoard(*this);
		tempBoard.put(pos);
		tempBoard.rotate(rotation);
		return tempBoard;
	}

	int16_t eval() {
		int16_t score = 0;
		uint64_t firstPlayer = bitboard[0];
		uint64_t secondPlayer = bitboard[1];

		for (uint8_t i = 0; i < 32; i++) {
			uint64_t mask = winMasks[i];
			uint64_t first = firstPlayer & mask;
			uint64_t second = secondPlayer & mask;
			
			if (first && !second) 
				score += scoreDistribution[__popcnt64(first)];
			else if (!first && second) 
				score -= scoreDistribution[__popcnt64(second)];
		}

		return score;
	}

	std::priority_queue<std::pair<int16_t, std::pair<uint8_t, uint8_t>>, std::vector<std::pair<int16_t, std::pair<uint8_t, uint8_t>>>> getMoves() {
		std::priority_queue<std::pair<int16_t, std::pair<uint8_t, uint8_t>>, std::vector<std::pair<int16_t, std::pair<uint8_t, uint8_t>>>> moves;
		int64_t united = bitboard[0] | bitboard[1];

		for (uint8_t i = 0; i < 36; i++) {
			if (bb::get(united, i) == 0) {
				for (uint8_t r = 0; r < 8; r++) {
					auto board = advance(i, r);
					const int16_t evaluation = board.eval();
					moves.push(std::make_pair(evaluation, std::make_pair(i, r)));
				}
			}
		}

		return moves;
	}

	uint16_t move = 0;
	uint64_t bitboard[2] = {0, 0};
};