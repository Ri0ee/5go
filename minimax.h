#pragma once

#include <limits>
#include <format>
#include <array>
#include <ranges>
#include <chrono>

#include "bitboard.h"
#include "board.h"
#include "table.h"

namespace mm {
	constexpr int16_t maxScore = 30000;
	constexpr int16_t minScore = -30000;

	Board bestMove(Board board, int p, uint8_t depth = 5);
	int16_t negamax(uint64_t root, int16_t alpha, int16_t beta, uint8_t depthleft);
}