#pragma once

#include <limits>
#include <format>
#include <array>

#include "bitboard.h"
#include "eval.h"
#include "board.h"
#include "table.h"

namespace mm {
	Board bestMove(Board board, int p, uint8_t depth = 5);
	int16_t negamax(Board& board, int16_t alpha, int16_t beta, uint8_t depthleft);
}