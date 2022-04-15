#include "minimax.h"

#define EXACT 1
#define ALPHA_BOUND 2
#define BETA_BOUND 3

constexpr int16_t maxScore =  std::numeric_limits<int16_t>::max();
constexpr int16_t minScore = -std::numeric_limits<int16_t>::max();

Board mm::bestMove(Board board, int p, uint8_t maxDepth) {
    std::cout << minScore << "\n";
    for (uint8_t depth = 2; depth <= maxDepth; depth++) {
        negamax(board, minScore, maxScore, depth);
        std::cout << std::format("depth: {}\n", depth);
    }

    uint64_t bestMove = 0;
    int16_t bestScore = minScore;

    auto moves = bb::advances(board.bitboard);
    for (auto move : moves) {
        auto entry = tt::get(move);

        if (entry.score > bestScore) {
            bestMove = move;
            bestScore = entry.score;
        }

        Board brd(move);
        std::cout << std::format("move: {:20} score: {:3}\n", move, entry.score);
        brd.debugPrint();
    }

    return bestMove;
}

int16_t mm::negamax(Board& root, int16_t alpha, int16_t beta, uint8_t depthleft) {
    root.debugPrint();

    uint64_t p0 = bb::unpack(root.bitboard, 0);
    uint64_t p1 = bb::unpack(root.bitboard, 1);

    auto moveCnt = bb::count(p0, p1);
    auto winner = ev::winner(p0, p1);

    if (winner == 3) return 0; // draw
    else if (winner == moveCnt % 2 + 1) return maxScore;
    else if (winner == moveCnt % 2 + 2) return minScore;

    if (depthleft == 0)
        return -ev::eval(p0, p1);

    int16_t bestScore = minScore;
    uint64_t bestMove = 0;

    auto entry = tt::get(root.bitboard);
    if (entry.key != 0 && entry.depth >= depthleft) {
        stats::ttHits++;
        switch (entry.type) {
        case EXACT: return entry.score;
        case ALPHA_BOUND: alpha = std::max(alpha, entry.score); break;
        case BETA_BOUND: beta = std::min(beta, entry.score); break;
        }

        if (alpha >= beta)
            return entry.score;

        if (entry.bestMove != 0) {
            bestMove = entry.bestMove;
            Board board(bestMove);
            board.bitboard = bb::flipSides(board.bitboard);
            bestScore = -negamax(board, -beta, -alpha, depthleft - 1);
        }
    }

    if (bestScore < beta) {
        auto moves = bb::advances(p1, p0);
        for (auto move : moves) {
            Board board(move);
            auto score = -negamax(board, -beta, -std::max(alpha, bestScore), depthleft - 1);

            if (bestScore < score) {
                bestScore = score;
                bestMove = move;

                if (bestScore >= beta) {
                    stats::betaCutoffs++;
                    break;
                }
            }
        }
    }

    uint8_t entryType = EXACT;
    if (bestScore <= alpha) entryType = BETA_BOUND;
    else if (bestScore >= beta) entryType = ALPHA_BOUND;

    tt::set(root.bitboard, bestMove, bestScore, depthleft, entryType);

    return bestScore;
}