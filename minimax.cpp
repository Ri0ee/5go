#include "minimax.h"

int aiPlayer = 0;

Board mm::bestMove(Board board, int p, uint8_t maxDepth) {
    aiPlayer = p;

    using std::chrono::high_resolution_clock;
    using std::chrono::duration;

    auto t1 = high_resolution_clock::now();
    for (uint8_t depth = 2; depth <= maxDepth; depth++) {
        negamax(board.bitboard, mm::minScore, mm::maxScore, depth);

        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms = t2 - t1;
        std::cout << std::format("depth: {} {}\n", depth, ms);

        if (ms > std::chrono::seconds{ 10 })
            break;
    }

    auto entry = tt::get(board.bitboard);
    return entry.bestMove;
}

int16_t mm::negamax(uint64_t root, int16_t alpha, int16_t beta, uint8_t depthleft) {
    const auto p0 = bb::unpack(root, 0);
    const auto p1 = bb::unpack(root, 1);
    const auto winner = bb::winner(p0, p1);
    const auto player = bb::count(p0, p1) % 2 + 1;
    const auto entry = tt::get(root);

    int16_t bestScore = mm::minScore;
    uint64_t bestMove = 0;

    if (winner != 0) {
        if (winner == 3) return 0;
        else if (winner == player) return mm::maxScore;
        return mm::minScore;
    }
    
    if (depthleft == 0)
        return -bb::eval(p0, p1);

    if (entry.key == tt::hash(root) && entry.depth >= depthleft) {
        switch (entry.type) {
        case tt::Entry::Type::exact: return entry.score;
        case tt::Entry::Type::alphaBound: alpha = std::max(alpha, entry.score); break;
        case tt::Entry::Type::betaBound: beta = std::min(beta, entry.score); break;
        }

        if (alpha >= beta)
            return entry.score;

        if (entry.bestMove != 0) {
            bestMove = entry.bestMove;
            bestScore = -negamax(bestMove, -beta, -alpha, depthleft - 1);
        }
    }

    if (bestScore < beta) {
        if (player == 1) {
            for (const auto& [static_eval, move] : bb::advances(p0, p1)) {
                auto score = -negamax(move, -beta, -std::max(alpha, bestScore), depthleft - 1);

                if (bestScore < score) {
                    bestScore = score;
                    bestMove = move;
                    if (bestScore >= beta) break;
                }
            }
        }
        else {
            auto moves = bb::advances(p0, p1);
            for (auto rit = moves.rbegin(); rit != moves.rend(); ++rit) { // reverse move ordering for the second player
                auto move = (*rit).second;
                auto score = -negamax(move, -beta, -std::max(alpha, bestScore), depthleft - 1);

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
    tt::set(root, bestMove, bestScore, depthleft, type);

    return bestScore;
}