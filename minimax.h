#pragma once

#include <limits>
#include <format>
#include "bitboard.h"
#include "board.h"

namespace mm {
	std::pair<uint8_t, uint8_t> bestMove(Board board, uint8_t depth = 36);
	int16_t alphaBetaMax(Board& board, int16_t alpha, int16_t beta, uint8_t depthleft);
	int16_t alphaBetaMin(Board& board, int16_t alpha, int16_t beta, uint8_t depthleft);
}