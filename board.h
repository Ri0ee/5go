#pragma once

#include <iostream>
#include <assert.h>
#include <queue>
#include <array>
#include <unordered_set>

#include "bitboard.h"

struct Board {
	Board() {}
	Board(const Board& board) {
		bitboard = board.bitboard;
	}
	Board(const uint64_t packedBitboard) {
		bitboard = packedBitboard;
	}
	Board(const uint64_t p0, const uint64_t p1) {
		bitboard = bb::pack(p0, p1);
	}

	uint8_t currentMove() const {
		return bb::count(bitboard);
	}

	auto toArray() {
		std::array<std::array<int, 6>, 6> result{};
		for (int qx = 0; qx < 2; qx++) {
			for (int qy = 0; qy < 2; qy++) {
				uint16_t quadrant = bb::quadrant(bitboard, 2 * qx + qy);
				uint16_t p0 = bb::unpack(quadrant, 0);
				uint16_t p1 = bb::unpack(quadrant, 1);

				for (int col = 0; col < 3; col++)
					for (int row = 0; row < 3; row++)
						result[3 * qx + col][3 * qy + row] = ((p0 >> (3 * col + row)) & 0x1) + 2 * ((p1 >> (3 * col + row)) & 0x1);
			}
		}

		return result;
	}

	void fromArray(std::array<std::array<int, 6>, 6>& arr) {
		uint16_t q[4];
		for (int qx = 0; qx < 2; qx++) {
			for (int qy = 0; qy < 2; qy++) {
				uint16_t p0 = 0, p1 = 0;
				
				for (int col = 0; col < 3; col++) {
					for (int row = 0; row < 3; row++) {
						switch (arr[3 * qx + col][3 * qy + row]) {
						case 1: p0 |= 0x1 << (3 * col + row); break;
						case 2: p1 |= 0x1 << (3 * col + row); break;
						}
					}
				}

				q[2 * qx + qy] = bb::pack(p0, p1);
			}
		}

		bitboard = bb::quadrants(q[0], q[1], q[2], q[3]);
	}

	inline void rotate(int qidx, int dir) {
		uint64_t p0 = bb::unpack(bitboard, 0);
		uint64_t p1 = bb::unpack(bitboard, 1);

		uint16_t qp0 = bb::rotate(bb::quadrant(p0, qidx), dir);
		uint16_t qp1 = bb::rotate(bb::quadrant(p1, qidx), dir);

		switch (qidx) {
		case 0: p0 = bb::quadrants(qp0, bb::quadrant(p0, 1), bb::quadrant(p0, 2), bb::quadrant(p0, 3)); break;
		case 1: p0 = bb::quadrants(bb::quadrant(p0, 0), qp0, bb::quadrant(p0, 2), bb::quadrant(p0, 3)); break;
		case 2: p0 = bb::quadrants(bb::quadrant(p0, 0), bb::quadrant(p0, 1), qp0, bb::quadrant(p0, 3)); break;
		case 3: p0 = bb::quadrants(bb::quadrant(p0, 0), bb::quadrant(p0, 1), bb::quadrant(p0, 2), qp0); break;
		}

		switch (qidx) {
		case 0: p1 = bb::quadrants(qp1, bb::quadrant(p1, 1), bb::quadrant(p1, 2), bb::quadrant(p1, 3)); break;
		case 1: p1 = bb::quadrants(bb::quadrant(p1, 0), qp1, bb::quadrant(p1, 2), bb::quadrant(p1, 3)); break;
		case 2: p1 = bb::quadrants(bb::quadrant(p1, 0), bb::quadrant(p1, 1), qp1, bb::quadrant(p1, 3)); break;
		case 3: p1 = bb::quadrants(bb::quadrant(p1, 0), bb::quadrant(p1, 1), bb::quadrant(p1, 2), qp1); break;
		}

		bitboard = bb::pack(p0, p1);
	}

	void debugPrint() {
		debugPrint(bitboard);
	}

	bool stale() {
		return bb::count(bitboard) == 36;
	}

	int winner() {
		return bb::winner(bitboard);
	}

	void debugPrint(uint64_t packedBitboard) {
		std::cout << "----BOARD DUMP----\n";

		Board board(packedBitboard);
		std::cout << "normal view:\n";
		for (auto row : board.toArray()) {
			for (auto col : row)
				std::cout << col << " ";
			std::cout << '\n';
		}

		std::cout << "--BOARD DUMP END--\n";
	}

	uint64_t bitboard = 0;
};