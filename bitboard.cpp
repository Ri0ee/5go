#include "bitboard.h"

uint64_t bb::quadrants(uint16_t q0, uint16_t q1, uint16_t q2, uint16_t q3) {
	return q0 | (uint64_t)q1 << 16 | (uint64_t)q2 << 32 | (uint64_t)q3 << 48;
}

uint16_t bb::pack(uint16_t p0, uint16_t p1) {
	return bb::packTable[p0] + 2 * bb::packTable[p1];
}

uint16_t bb::quadrant(uint64_t bitboard, int q) {
	return (bitboard >> 16 * q) & 0xffff;
}

uint64_t bb::pack(uint64_t p0, uint64_t p1) {
	return bb::quadrants(bb::pack(bb::quadrant(p0, 0), bb::quadrant(p1, 0)),
						 bb::pack(bb::quadrant(p0, 1), bb::quadrant(p1, 1)),
						 bb::pack(bb::quadrant(p0, 2), bb::quadrant(p1, 2)),
						 bb::pack(bb::quadrant(p0, 3), bb::quadrant(p1, 3)));
}

uint16_t bb::unpack(uint16_t bitboard, int s) {
	return bb::unpackTable[bitboard][s];
}

uint64_t bb::unpack(uint64_t bitboard, int s) {
	return bb::quadrants(bb::unpack(bb::quadrant(bitboard, 0), s),
						 bb::unpack(bb::quadrant(bitboard, 1), s),
						 bb::unpack(bb::quadrant(bitboard, 2), s),
						 bb::unpack(bb::quadrant(bitboard, 3), s));
}

uint8_t bb::count(uint64_t bitboard) {
	return (uint8_t)__popcnt64(bb::unpack(bitboard, 0) | bb::unpack(bitboard, 1));
}

uint8_t bb::count(uint64_t p0, uint64_t p1) {
	return (uint8_t)__popcnt64(p0 | p1);
}

uint64_t bb::flipSides(uint64_t bitboard) {
	return bb::pack(bb::unpack(bitboard, 1), bb::unpack(bitboard, 0));
}

uint16_t bb::rotate(uint16_t quadrant, int dir) {
	return bb::rotationTable[quadrant][dir];
}

std::unordered_set<uint64_t> bb::advances(uint64_t p0, uint64_t p1) {
	std::unordered_set<uint64_t> moves;

	uint64_t moveMask = (p0 | p1) ^ 0x01ff01ff01ff01ff;
	const uint64_t moveCnt = __popcnt64(moveMask);
	const auto side0 = (moveCnt % 2 ? p1 : p0);
	const auto side1 = (moveCnt % 2 ? p0 : p1);

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

			if (moveCnt % 2 == 0) {
				uint64_t packedRotationQ1 = bb::pack(advanceP0RotQ1, advanceP1RotQ1);
				uint64_t packedRotationQ2 = bb::pack(advanceP0RotQ2, advanceP1RotQ2);
				uint64_t packedRotationQ3 = bb::pack(advanceP0RotQ3, advanceP1RotQ3);
				uint64_t packedRotationQ4 = bb::pack(advanceP0RotQ4, advanceP1RotQ4);

				moves.insert(packedRotationQ1);
				moves.insert(packedRotationQ2);
				moves.insert(packedRotationQ3);
				moves.insert(packedRotationQ4);
			}
			else {
				uint64_t packedRotationQ1 = bb::pack(advanceP1RotQ1, advanceP0RotQ1);
				uint64_t packedRotationQ2 = bb::pack(advanceP1RotQ2, advanceP0RotQ2);
				uint64_t packedRotationQ3 = bb::pack(advanceP1RotQ3, advanceP0RotQ3);
				uint64_t packedRotationQ4 = bb::pack(advanceP1RotQ4, advanceP0RotQ4);

				moves.insert(packedRotationQ1);
				moves.insert(packedRotationQ2);
				moves.insert(packedRotationQ3);
				moves.insert(packedRotationQ4);
			}
		}

		moveMask ^= move;
	}

	return moves;
}

std::unordered_set<uint64_t> bb::advances(uint64_t packedBitboard) {
	return bb::advances(bb::unpack(packedBitboard, 0), bb::unpack(packedBitboard, 1));
}

//std::unordered_set<uint64_t> bb::advances(uint64_t p0, uint64_t p1) {
//	std::unordered_set<uint64_t> moves;
//
//	uint64_t moveMask = (p0 | p1) ^ 0x01ff01ff01ff01ff;
//	uint64_t moveCnt = __popcnt64(moveMask);
//
//	for (int i = 0; i < moveCnt; i++) {
//		uint64_t move = (uint64_t)0x1 << _tzcnt_u64(moveMask);
//		uint64_t p0advance = p0 | move;
//		for (int r = 0; r < 2; r++) {
//			uint64_t advanceP0RotQ1 = bb::quadrants(
//				bb::rotate(bb::quadrant(p0advance, 0), r),
//				bb::quadrant(p0advance, 1),
//				bb::quadrant(p0advance, 2),
//				bb::quadrant(p0advance, 3));
//
//			uint64_t advanceP0RotQ2 = bb::quadrants(
//				bb::quadrant(p0advance, 0),
//				bb::rotate(bb::quadrant(p0advance, 1), r),
//				bb::quadrant(p0advance, 2),
//				bb::quadrant(p0advance, 3));
//
//			uint64_t advanceP0RotQ3 = bb::quadrants(
//				bb::quadrant(p0advance, 0),
//				bb::quadrant(p0advance, 1),
//				bb::rotate(bb::quadrant(p0advance, 2), r),
//				bb::quadrant(p0advance, 3));
//
//			uint64_t advanceP0RotQ4 = bb::quadrants(
//				bb::quadrant(p0advance, 0),
//				bb::quadrant(p0advance, 1),
//				bb::quadrant(p0advance, 2),
//				bb::rotate(bb::quadrant(p0advance, 3), r));
//
//			uint64_t advanceP1RotQ1 = bb::quadrants(
//				bb::rotate(bb::quadrant(p1, 0), r),
//				bb::quadrant(p1, 1),
//				bb::quadrant(p1, 2),
//				bb::quadrant(p1, 3));
//
//			uint64_t advanceP1RotQ2 = bb::quadrants(
//				bb::quadrant(p1, 0),
//				bb::rotate(bb::quadrant(p1, 1), r),
//				bb::quadrant(p1, 2),
//				bb::quadrant(p1, 3));
//
//			uint64_t advanceP1RotQ3 = bb::quadrants(
//				bb::quadrant(p1, 0),
//				bb::quadrant(p1, 1),
//				bb::rotate(bb::quadrant(p1, 2), r),
//				bb::quadrant(p1, 3));
//
//			uint64_t advanceP1RotQ4 = bb::quadrants(
//				bb::quadrant(p1, 0),
//				bb::quadrant(p1, 1),
//				bb::quadrant(p1, 2),
//				bb::rotate(bb::quadrant(p1, 3), r));
//
//			uint64_t packedRotationQ1 = bb::pack(advanceP0RotQ1, advanceP1RotQ1);
//			uint64_t packedRotationQ2 = bb::pack(advanceP0RotQ2, advanceP1RotQ2);
//			uint64_t packedRotationQ3 = bb::pack(advanceP0RotQ3, advanceP1RotQ3);
//			uint64_t packedRotationQ4 = bb::pack(advanceP0RotQ4, advanceP1RotQ4);
//
//			moves.insert(packedRotationQ1);
//			moves.insert(packedRotationQ2);
//			moves.insert(packedRotationQ3);
//			moves.insert(packedRotationQ4);
//		}
//
//		moveMask ^= move;
//	}
//
//	return moves;
//}
//
//std::priority_queue<std::pair<int16_t,  bb::advances(uint64_t p0, uint64_t p1) {
//	std::unordered_set<uint64_t> moves;
//
//	uint64_t moveMask = (p0 | p1) ^ 0x01ff01ff01ff01ff;
//	uint64_t moveCnt = __popcnt64(moveMask);
//
//	for (int i = 0; i < moveCnt; i++) {
//		uint64_t move = (uint64_t)0x1 << _tzcnt_u64(moveMask);
//		uint64_t p0advance = p0 | move;
//		for (int r = 0; r < 2; r++) {
//			uint64_t advanceP0RotQ1 = bb::quadrants(
//				bb::rotate(bb::quadrant(p0advance, 0), r),
//				bb::quadrant(p0advance, 1),
//				bb::quadrant(p0advance, 2),
//				bb::quadrant(p0advance, 3));
//
//			uint64_t advanceP0RotQ2 = bb::quadrants(
//				bb::quadrant(p0advance, 0),
//				bb::rotate(bb::quadrant(p0advance, 1), r),
//				bb::quadrant(p0advance, 2),
//				bb::quadrant(p0advance, 3));
//
//			uint64_t advanceP0RotQ3 = bb::quadrants(
//				bb::quadrant(p0advance, 0),
//				bb::quadrant(p0advance, 1),
//				bb::rotate(bb::quadrant(p0advance, 2), r),
//				bb::quadrant(p0advance, 3));
//
//			uint64_t advanceP0RotQ4 = bb::quadrants(
//				bb::quadrant(p0advance, 0),
//				bb::quadrant(p0advance, 1),
//				bb::quadrant(p0advance, 2),
//				bb::rotate(bb::quadrant(p0advance, 3), r));
//
//			uint64_t advanceP1RotQ1 = bb::quadrants(
//				bb::rotate(bb::quadrant(p1, 0), r),
//				bb::quadrant(p1, 1),
//				bb::quadrant(p1, 2),
//				bb::quadrant(p1, 3));
//
//			uint64_t advanceP1RotQ2 = bb::quadrants(
//				bb::quadrant(p1, 0),
//				bb::rotate(bb::quadrant(p1, 1), r),
//				bb::quadrant(p1, 2),
//				bb::quadrant(p1, 3));
//
//			uint64_t advanceP1RotQ3 = bb::quadrants(
//				bb::quadrant(p1, 0),
//				bb::quadrant(p1, 1),
//				bb::rotate(bb::quadrant(p1, 2), r),
//				bb::quadrant(p1, 3));
//
//			uint64_t advanceP1RotQ4 = bb::quadrants(
//				bb::quadrant(p1, 0),
//				bb::quadrant(p1, 1),
//				bb::quadrant(p1, 2),
//				bb::rotate(bb::quadrant(p1, 3), r));
//
//			uint64_t packedRotationQ1 = bb::pack(advanceP0RotQ1, advanceP1RotQ1);
//			uint64_t packedRotationQ2 = bb::pack(advanceP0RotQ2, advanceP1RotQ2);
//			uint64_t packedRotationQ3 = bb::pack(advanceP0RotQ3, advanceP1RotQ3);
//			uint64_t packedRotationQ4 = bb::pack(advanceP0RotQ4, advanceP1RotQ4);
//
//			moves.insert(packedRotationQ1);
//			moves.insert(packedRotationQ2);
//			moves.insert(packedRotationQ3);
//			moves.insert(packedRotationQ4);
//		}
//
//		moveMask ^= move;
//	}
//
//	return moves;
//}