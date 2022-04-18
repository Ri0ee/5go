#include "bitboard.h"

uint64_t bb::quadrants(uint16_t q0, uint16_t q1, uint16_t q2, uint16_t q3) {
	return q0 | (uint64_t)q1 << 16 | (uint64_t)q2 << 32 | (uint64_t)q3 << 48;
}

uint16_t bb::quadrant(uint64_t bitboard, int q) {
	return (bitboard >> 16 * q) & 0xffff;
}

uint16_t bb::pack(uint16_t p0, uint16_t p1) {
	return bb::packTable[p0] + 2 * bb::packTable[p1];
}

uint64_t bb::pack(uint64_t p0, uint64_t p1) {
	return bb::quadrants(bb::pack(bb::quadrant(p0, 0), bb::quadrant(p1, 0)),
						 bb::pack(bb::quadrant(p0, 1), bb::quadrant(p1, 1)),
						 bb::pack(bb::quadrant(p0, 2), bb::quadrant(p1, 2)),
						 bb::pack(bb::quadrant(p0, 3), bb::quadrant(p1, 3)));
}

uint16_t bb::unpack(uint16_t packedQuadrant, int s) {
	return bb::unpackTable[packedQuadrant][s];
}

uint64_t bb::unpack(uint64_t packedBitboard, int s) {
	return bb::quadrants(bb::unpack(bb::quadrant(packedBitboard, 0), s),
						 bb::unpack(bb::quadrant(packedBitboard, 1), s),
						 bb::unpack(bb::quadrant(packedBitboard, 2), s),
						 bb::unpack(bb::quadrant(packedBitboard, 3), s));
}

uint8_t bb::count(uint64_t packedBitboard) {
	return bb::count(bb::unpack(packedBitboard, 0), bb::unpack(packedBitboard, 1));
}

uint8_t bb::count(uint64_t p0, uint64_t p1) {
	return (uint8_t)__popcnt64(p0 | p1);
}

uint16_t bb::rotate(uint16_t quadrant, int dir) {
	return bb::rotationTable[quadrant][dir];
}

bb::scoreAndMoveSet bb::advances(uint64_t p0, uint64_t p1) {
	bb::scoreAndMoveSet moves;

	uint64_t moveMask = (p0 | p1) ^ 0x01ff01ff01ff01ff;
	const uint64_t moveCnt = __popcnt64(moveMask);
	const auto side0 = (moveCnt % 2) ? p1 : p0;
	const auto side1 = (moveCnt % 2) ? p0 : p1;

	for (int i = 0; i < moveCnt; i++) {
		uint64_t move = (uint64_t)0x1 << _tzcnt_u64(moveMask);
		uint64_t pp = side0 | move;

		for (int r = 0; r < 2; r++) {
			uint64_t advanceP0RotQ1 = bb::quadrants(
				bb::rotate(bb::quadrant(pp, 0), r),
				bb::quadrant(pp, 1),
				bb::quadrant(pp, 2),
				bb::quadrant(pp, 3));

			uint64_t advanceP0RotQ2 = bb::quadrants(
				bb::quadrant(pp, 0),
				bb::rotate(bb::quadrant(pp, 1), r),
				bb::quadrant(pp, 2),
				bb::quadrant(pp, 3));

			uint64_t advanceP0RotQ3 = bb::quadrants(
				bb::quadrant(pp, 0),
				bb::quadrant(pp, 1),
				bb::rotate(bb::quadrant(pp, 2), r),
				bb::quadrant(pp, 3));

			uint64_t advanceP0RotQ4 = bb::quadrants(
				bb::quadrant(pp, 0),
				bb::quadrant(pp, 1),
				bb::quadrant(pp, 2),
				bb::rotate(bb::quadrant(pp, 3), r));

			uint64_t advanceP1RotQ1 = bb::quadrants(
				bb::rotate(bb::quadrant(side1, 0), r),
				bb::quadrant(side1, 1),
				bb::quadrant(side1, 2),
				bb::quadrant(side1, 3));

			uint64_t advanceP1RotQ2 = bb::quadrants(
				bb::quadrant(side1, 0),
				bb::rotate(bb::quadrant(side1, 1), r),
				bb::quadrant(side1, 2),
				bb::quadrant(side1, 3));

			uint64_t advanceP1RotQ3 = bb::quadrants(
				bb::quadrant(side1, 0),
				bb::quadrant(side1, 1),
				bb::rotate(bb::quadrant(side1, 2), r),
				bb::quadrant(side1, 3));

			uint64_t advanceP1RotQ4 = bb::quadrants(
				bb::quadrant(side1, 0),
				bb::quadrant(side1, 1),
				bb::quadrant(side1, 2),
				bb::rotate(bb::quadrant(side1, 3), r));

			uint64_t packedRotationQ1 = 0, packedRotationQ2 = 0, packedRotationQ3 = 0, packedRotationQ4 = 0;
			if (moveCnt % 2 == 0) {
				packedRotationQ1 = bb::pack(advanceP0RotQ1, advanceP1RotQ1);
				packedRotationQ2 = bb::pack(advanceP0RotQ2, advanceP1RotQ2);
				packedRotationQ3 = bb::pack(advanceP0RotQ3, advanceP1RotQ3);
				packedRotationQ4 = bb::pack(advanceP0RotQ4, advanceP1RotQ4);
			}
			else {
				packedRotationQ1 = bb::pack(advanceP1RotQ1, advanceP0RotQ1);
				packedRotationQ2 = bb::pack(advanceP1RotQ2, advanceP0RotQ2);
				packedRotationQ3 = bb::pack(advanceP1RotQ3, advanceP0RotQ3);
				packedRotationQ4 = bb::pack(advanceP1RotQ4, advanceP0RotQ4);
			}

			moves.insert(std::make_pair(bb::eval(packedRotationQ1), packedRotationQ1));
			moves.insert(std::make_pair(bb::eval(packedRotationQ2), packedRotationQ2));
			moves.insert(std::make_pair(bb::eval(packedRotationQ3), packedRotationQ3));
			moves.insert(std::make_pair(bb::eval(packedRotationQ4), packedRotationQ4));
		}

		moveMask ^= move;
	}

	return moves;
}

bb::scoreAndMoveSet bb::advances(uint64_t packedBitboard) {
	return bb::advances(bb::unpack(packedBitboard, 0), bb::unpack(packedBitboard, 1));
}

int16_t bb::eval(uint64_t packedBitboard) {
	return bb::eval(bb::unpack(packedBitboard, 0), bb::unpack(packedBitboard, 1));
}

int bb::winner(uint64_t packedBitboard) {
	return bb::winner(bb::unpack(packedBitboard, 0), bb::unpack(packedBitboard, 1));
}

int16_t bb::eval(uint64_t p0, uint64_t p1) {
	int16_t score = 0;

	for (uint8_t i = 0; i < 32; i++) {
		uint64_t mask = bb::winMasks[i];
		uint64_t p0streaks = p0 & mask;
		uint64_t p1streaks = p1 & mask;

		if (p0streaks && !p1streaks)
			score += bb::scoreDistribution[__popcnt64(p0streaks)];
		else if (!p0streaks && p1streaks)
			score -= bb::scoreDistribution[__popcnt64(p1streaks)];
	}

	const auto moves = bb::count(p0, p1);
	return (moves % 2 == 0 ? -score : score);
}

int bb::winner(uint64_t p0, uint64_t p1) {
	bool p0win = false, p1win = false;

	for (uint8_t i = 0; i < 32; i++) {
		uint64_t mask = bb::winMasks[i];
		uint64_t p0streaks = p0 & mask;
		uint64_t p1streaks = p1 & mask;

		p0win = p0win || p0streaks == mask;
		p1win = p1win || p1streaks == mask;
	}

	uint8_t moves = bb::count(p0, p1);

	if (p0win && p1win || (!p0win && !p1win && moves == 36)) return 3; // draw
	if (p0win) return 1; // first player win
	if (p1win) return 2; // second player win

	return 0; // game still going
}