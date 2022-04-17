#include "minimax.h"

Board mm::bestMove(Board board, int p, uint8_t maxDepth) {
    //for (uint8_t depth = 2; depth <= maxDepth; depth++) {
    //    negamax(board.bitboard, mm::minScore, mm::maxScore, depth);
    //    std::cout << std::format("depth: {}\n", depth);
    //}

    negamax(board.bitboard, mm::minScore, mm::maxScore, 8);

    uint64_t bestMove = 0;
    uint64_t worstMove = 0;
    int16_t highScore = mm::minScore;
    int16_t lowScore = mm::maxScore;

    bb::scoreAndMoveSet goodMoves, badMoves;

    for (const auto& [static_eval, move] : bb::advances(board.bitboard)) {
        auto entry = tt::get(move);
        if (entry.key == 0 || entry.type == tt::Entry::Type::undefined) continue;

        if (entry.score >= highScore) {
            if (entry.score > highScore)
                goodMoves.clear();

            bestMove = move;
            highScore = entry.score;

            goodMoves.insert(std::make_pair(highScore, bestMove));
        }

        if (entry.score <= lowScore) {
            if (entry.score < lowScore)
                badMoves.clear();

            worstMove = move;
            lowScore = entry.score;

            badMoves.insert(std::make_pair(lowScore, move));
        }

#ifdef _DEBUG
        Board brd(move);
        std::cout << std::format("move: {:20} score: {:3}\n", move, entry.score);
        brd.debugPrint();
#endif // _DEBUG
    }

#ifdef _DEBUG
    std::cout << "\n\nworst moves: \n";
    for (const auto& [static_eval, move] : badMoves) {
        Board brd(move);
        brd.debugPrint();
    }

    std::cout << "\n\nbest moves: \n";
    for (const auto& [static_eval, move] : goodMoves) {
        Board brd(move);
        brd.debugPrint();
    }
#endif // _DEBUG

    std::cout << "\n\nchosen move: \n";
    std::cout << std::format("move uin64_t: {:20}\nhigh deep score:\t{:3}\nlow deep score:\t{:3}\n", bestMove, highScore, lowScore);
    Board brd(bestMove);
    brd.debugPrint();

    return (p == 2 ? worstMove : bestMove);
}

int16_t mm::negamax(uint64_t root, int16_t alpha, int16_t beta, uint8_t depthleft) {
#ifdef _DEBUG
    Board brd(root);
    brd.debugPrint();
#endif // _DEBUG

    const auto p0 = bb::unpack(root, 0);
    const auto p1 = bb::unpack(root, 1);
    const auto winner = bb::winner(p0, p1);
    const auto entry = tt::get(root);

    int16_t bestScore = mm::minScore;
    uint64_t bestMove = 0;

    if (winner != 0) {
        if (winner == 3) {
            stats::stats["drawReturns"]++;
            return 0;
        }
        else if (winner == bb::count(p0, p1) % 2 + 1) { // if winner is a current player (1 or 2)
            stats::stats["winReturns"]++;
            return mm::maxScore;
        }

        stats::stats["lossReturns"]++;
        return mm::minScore;
    }
    
    if (depthleft == 0)
        return -bb::eval(p0, p1);

    if (entry.key != 0 && entry.type != tt::Entry::Type::undefined && entry.depth >= depthleft) {
        stats::stats["ttHits"]++;

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

            stats::stats["ttBestMovesChecked"]++;
        }
    }

    if (bestScore < beta) {
        for (const auto& [static_eval, move] : bb::advances(p0, p1)) {
            auto score = -negamax(move, -beta, -std::max(alpha, bestScore), depthleft - 1);

            if (bestScore < score) {
                bestScore = score;
                bestMove = move;
                if (bestScore >= beta) break;
            }
        }
    }

    tt::Entry::Type type = tt::Entry::Type::exact;
    if (bestScore <= alpha) type = tt::Entry::Type::betaBound;
    else if (bestScore >= beta) type = tt::Entry::Type::alphaBound;
    tt::set(root, bestMove, bestScore, depthleft, type);

    return bestScore;
}