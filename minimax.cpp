#include "minimax.h"

#define EXACT 1
#define ALPHA_BOUND 2
#define BETA_BOUND 3

constexpr int16_t maxScore =  30000;
constexpr int16_t minScore = -30000;

Board mm::bestMove(Board board, int p, uint8_t maxDepth) {
    for (uint8_t depth = 2; depth <= maxDepth; depth++) {
        negamax(board.bitboard, minScore, maxScore, depth);
        std::cout << std::format("depth: {}\n", depth);
    }

    uint64_t bestMove = 0;
    int16_t bestScore = minScore;
    int16_t worstScore = maxScore;

    auto moves = bb::advances(board.bitboard);
    for (auto move : moves) {
        auto entry = tt::get(move);

        if (entry.score > bestScore) {
            bestMove = move;
            bestScore = entry.score;
        }

        if (entry.score < worstScore) {
            worstScore = entry.score;
        }

        Board brd(move);
        std::cout << std::format("move: {:20} score: {:3}\n", move, entry.score);
        brd.debugPrint();
    }

    Board brd(bestMove);
    std::cout << std::format("move: {:20}\n\tbest score:\t{:3}\n\tworst score:\t{:3}\n", bestMove, bestScore, worstScore);
    brd.debugPrint();

    return bestMove;
}

int16_t mm::negamax(uint64_t root, int16_t alpha, int16_t beta, uint8_t depthleft) {
#ifdef _DEBUG
    Board brd(root);
    brd.debugPrint();
#endif // _DEBUG

    const auto p0 = bb::unpack(root, 0);
    const auto p1 = bb::unpack(root, 1);

    const auto moveCnt = bb::count(root);
    const auto winner = ev::winner(root);
    const auto currentPlayer = moveCnt % 2 + 1;

    if (winner != 0) {
        if (winner == 3) {
            stats::drawReturns++;
            return 0;
        }
        else if (winner == currentPlayer) {
            stats::winReturns++;
            return maxScore;
        }

        stats::lossReturns++;
        return minScore;
    }
    
    if (depthleft == 0)
        return -ev::eval(root);

    int16_t bestScore = minScore;
    uint64_t bestMove = 0;

    auto entry = tt::get(root);
    if (entry.key != 0 && entry.depth >= depthleft) {
        stats::ttHits++;

        switch (entry.type) {
        case EXACT: return entry.score;
        case ALPHA_BOUND: alpha = std::max(alpha, entry.score); break;
        case BETA_BOUND: beta = std::min(beta, entry.score); break;
        }

        if (alpha >= beta)
            return entry.score;

        //if (entry.bestMove != 0) {
        //    bestMove = entry.bestMove;
        //    bestScore = -negamax(bestMove, -beta, -alpha, depthleft - 1);

        //    stats::ttBestMovesChecked++;
        //}
    }

    if (bestScore < beta) {
        auto moves = bb::advances(root);
        for (auto move : moves) {
            auto score = -negamax(move, -beta, -std::max(alpha, bestScore), depthleft - 1);

            if (bestScore < score) {
                bestScore = score;
                bestMove = move;

                if (bestScore >= beta) {
                    break;
                }
            }
        }
    }

    const uint8_t type = (bestScore <= alpha ? BETA_BOUND :
                    bestScore >= beta ? ALPHA_BOUND : EXACT);

    tt::set(root, bestMove, bestScore, depthleft, type);

    return bestScore;
}