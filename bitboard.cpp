#include "bitboard.h"

void bb::printBitboard(uint64_t bitboard) {
	std::cout << std::format("{:06b}\n", (bitboard & 0xFC0000000) >> 30);
	std::cout << std::format("{:06b}\n", (bitboard & 0x3F000000) >> 24);
	std::cout << std::format("{:06b}\n", (bitboard & 0xFC0000) >> 18);
	std::cout << std::format("{:06b}\n", (bitboard & 0x3F000) >> 12);
	std::cout << std::format("{:06b}\n", (bitboard & 0xFC0) >> 6);
	std::cout << std::format("{:06b}\n", (bitboard & 0x3F));
}

uint64_t bb::set(uint64_t bitboard, uint8_t pos) {
	return bitboard | bb::mask[pos];
}

uint64_t bb::unset(uint64_t bitboard, uint8_t pos) {
	return bitboard & ~bb::mask[pos];
}

uint64_t bb::get(uint64_t bitboard, uint8_t pos) {
	return bitboard & bb::mask[pos];
}

uint64_t bb::rotate90CW(uint64_t bitboard, uint8_t qidx) {
	uint64_t tBitboard = bitboard & bb::invqmask[qidx];
	uint64_t k = qidx * 9;

	tBitboard |= (bitboard & bb::rmask[k + 0]) >> 2;
	tBitboard |= (bitboard & bb::rmask[k + 1]) >> 7;
	tBitboard |= (bitboard & bb::rmask[k + 2]) >> 12;

	tBitboard |= (bitboard & bb::rmask[k + 3]) << 5;
	tBitboard |= (bitboard & bb::rmask[k + 4]);
	tBitboard |= (bitboard & bb::rmask[k + 5]) >> 5;

	tBitboard |= (bitboard & bb::rmask[k + 6]) << 12;
	tBitboard |= (bitboard & bb::rmask[k + 7]) << 7;
	tBitboard |= (bitboard & bb::rmask[k + 8]) << 2;

	return tBitboard;
}

uint64_t bb::rotate90CCW(uint64_t bitboard, uint8_t qidx) {
	uint64_t tBitboard = bitboard & bb::invqmask[qidx];
	uint64_t k = qidx * 9;

	tBitboard |= (bitboard & bb::rmask[k + 0]) >> 12;
	tBitboard |= (bitboard & bb::rmask[k + 1]) >> 5;
	tBitboard |= (bitboard & bb::rmask[k + 2]) << 2;

	tBitboard |= (bitboard & bb::rmask[k + 3]) >> 7;
	tBitboard |= (bitboard & bb::rmask[k + 4]);
	tBitboard |= (bitboard & bb::rmask[k + 5]) << 7;

	tBitboard |= (bitboard & bb::rmask[k + 6]) >> 2;
	tBitboard |= (bitboard & bb::rmask[k + 7]) << 5;
	tBitboard |= (bitboard & bb::rmask[k + 8]) << 12;

	return tBitboard;
}

bool bb::won(uint64_t bitboard) {
	uint8_t leftHorizontal =
		((bitboard & 0xFC0000000) >> 30) &
		((bitboard & 0x03F000000) >> 24) &
		((bitboard & 0x000FC0000) >> 18) &
		((bitboard & 0x00003F000) >> 12) &
		((bitboard & 0x000000FC0) >>  6);
	if (leftHorizontal > 0) return true;

	uint8_t rightHorizontal = 
		((bitboard & 0x03F000000) >> 24) &
		((bitboard & 0x000FC0000) >> 18) &
		((bitboard & 0x00003F000) >> 12) &
		((bitboard & 0x000000FC0) >>  6) &
		 (bitboard & 0x00000003F);
	if (rightHorizontal > 0) return true;

	uint64_t highVertical =
		 (bitboard & 0x820820820) &
		((bitboard & 0x410410410) << 1) &
		((bitboard & 0x208208208) << 2) &
		((bitboard & 0x104104104) << 3) &
		((bitboard & 0x082082082) << 4);
	if (highVertical > 0) return true;

	uint64_t lowVertical =
		 (bitboard & 0x410410410) &
		((bitboard & 0x208208208) << 1) &
		((bitboard & 0x104104104) << 2) &
		((bitboard & 0x082082082) << 3) &
		((bitboard & 0x041041041) << 4);
	if (lowVertical > 0) return true;

	if ((bitboard & 0x810204080) == 0x810204080) return true;
	if ((bitboard & 0x408102040) == 0x408102040) return true;
	if ((bitboard & 0x020408102) == 0x020408102) return true;
	if ((bitboard & 0x010204081) == 0x010204081) return true;
	if ((bitboard & 0x042108400) == 0x042108400) return true;
	if ((bitboard & 0x084210800) == 0x084210800) return true;
	if ((bitboard & 0x001084210) == 0x001084210) return true;
	if ((bitboard & 0x002108420) == 0x002108420) return true;

	return false;
}

static inline uint64_t bb::quadrants(uint16_t q0, uint16_t q1, uint16_t q2, uint16_t q3) {
	return q0 | (uint64_t)q1 << 16 | (uint64_t)q2 << 32 | (uint64_t)q3 << 48;
}

static inline uint16_t bb::pack(uint16_t side0, uint16_t side1) {
	return bb::packTable[side0] + 2 * bb::packTable[side1];
}

static inline uint16_t bb::quadrant(uint64_t bitboard, int q) {
	return (bitboard >> 16 * q) & 0xffff;
}

static inline uint64_t bb::pack(uint64_t side0, uint64_t side1) {
	return bb::quadrants(bb::pack(bb::quadrant(side0, 0), bb::quadrant(side1, 0)),
		bb::pack(bb::quadrant(side0, 1), bb::quadrant(side1, 1)),
		bb::pack(bb::quadrant(side0, 2), bb::quadrant(side1, 2)),
		bb::pack(bb::quadrant(side0, 3), bb::quadrant(side1, 3)));
}

static inline uint16_t bb::unpack(uint16_t state, int s) {
	return bb::unpackTable[state][s];
}

static inline uint64_t bb::unpack(uint64_t bitboard, int s) {
	return bb::quadrants(bb::unpack(bb::quadrant(bitboard, 0), s),
		bb::unpack(bb::quadrant(bitboard, 1), s),
		bb::unpack(bb::quadrant(bitboard, 2), s),
		bb::unpack(bb::quadrant(bitboard, 3), s));
}

static inline uint8_t bb::count_stones(uint64_t bitboard) {
	return (uint8_t)__popcnt64(bb::unpack(bitboard, 0) | bb::unpack(bitboard, 1));
}