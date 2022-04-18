#include "minimax.h"

Board mm::bestMove(Board board, uint8_t maxDepth) {
    auto t1 = std::chrono::high_resolution_clock::now();
    for (uint8_t depth = 2; depth <= maxDepth; depth++) {
        negamax(board.bitboard, mm::minScore, mm::maxScore, depth);

        auto t = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> ms = t - t1;
        std::cout << std::format("depth: {} {}\n", depth, ms);

        //if (ms > std::chrono::seconds{ 10 })
            //break;
    }

    return tt::get(board.bitboard).bestMove;
}

int16_t mm::negamax(uint64_t root, int16_t alpha, int16_t beta, uint8_t depthLeft) {
    stats::stats["states checked"]++;

    const auto p0 = bb::unpack(root, 0);
    const auto p1 = bb::unpack(root, 1);

    int16_t bestScore = mm::minScore;
    uint64_t bestMove = 0;

    const auto winner = bb::winner(p0, p1);
    const auto player = bb::count(p0, p1) % 2 + 1;
    if (winner != 0) {
        if (winner == 3) return 0;
        else if (winner == player) return mm::maxScore;
        return mm::minScore;
    }
    
    if (depthLeft == 0)
        return -bb::eval(p0, p1);

    const auto entry = tt::get(root);
    if (entry.type != tt::Entry::Type::undefined && entry.depth >= depthLeft) {
        stats::stats["tt hits"]++;

        switch (entry.type) {
        case tt::Entry::Type::exact: return entry.score;
        case tt::Entry::Type::alphaBound: alpha = std::max(alpha, entry.score); break;
        case tt::Entry::Type::betaBound: beta = std::min(beta, entry.score); break;
        }

        if (alpha >= beta)
            return entry.score;

        if (entry.bestMove != 0) {
            bestMove = entry.bestMove;
            bestScore = -negamax(bestMove, -beta, -alpha, depthLeft - 1);
            stats::stats["tt moves"]++;
        }
    }

    if (bestScore < beta) {
        if (player == 1) {
            for (const auto& [static_eval, move] : bb::advances(p0, p1)) {
                const int16_t score = -negamax(move, -beta, -std::max(alpha, bestScore), depthLeft - 1);

                if (bestScore < score) {
                    bestScore = score;
                    bestMove = move;
                    if (bestScore >= beta) break;
                }
            }
        }
        else {
            auto moves = bb::advances(p0, p1);
            for (auto rit = moves.rbegin(); rit != moves.rend(); ++rit) {
                auto move = (*rit).second;
                const int16_t score = -negamax(move, -beta, -std::max(alpha, bestScore), depthLeft - 1);

                if (bestScore < score) {
                    bestScore = score;
                    bestMove = move;
                    if (bestScore >= beta) break;
                }
            }
        }
    }

    tt::Entry::Type type = tt::Entry::Type::exact;
    if (bestScore <= alpha) type = tt::Entry::Type::betaBound;
    else if (bestScore >= beta) type = tt::Entry::Type::alphaBound;
    tt::set(root, bestMove, bestScore, depthLeft, type);

    return bestScore;
}