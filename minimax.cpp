#include "minimax.h"

std::pair<uint8_t, uint8_t> mm::bestMove(Board board, uint8_t depth) {
    auto moves = board.getMoves();
    std::pair<uint8_t, uint8_t> bestMove;
    int16_t bestScore = 0;

    bool firstSearch = true;

    int width = 0;
    while (!moves.empty() && width < 10) {
        width++;
        auto move = moves.top().second;
        moves.pop();

        auto nextBoard = board.advance(move.first, move.second);
        auto score = mm::alphaBetaMax(board, -std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::max(), depth);

        std::cout << std::format("move: {:02d} rotation: {:02d} score: {:06d}\n", move.first, move.second, score);

        if (score > bestScore || firstSearch) {
            bestMove = move;
            bestScore = score;
        }
    }

    return bestMove;
}

int16_t mm::alphaBetaMax(Board& board, int16_t alpha, int16_t beta, uint8_t depthleft) {
    if (depthleft == 0) 
        return board.eval();

    auto moves = board.getMoves();
    while (!moves.empty()) {
        auto move = moves.top();
        moves.pop();

        auto nextBoard = board.advance(move.second.first, move.second.second);
        auto score = mm::alphaBetaMin(nextBoard, alpha, beta, depthleft - 1);

        if (score >= beta)
            return beta;

        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

int16_t mm::alphaBetaMin(Board& board, int16_t alpha, int16_t beta, uint8_t depthleft) {
    if (depthleft == 0) 
        return -board.eval();

    auto moves = board.getMoves();
    while (!moves.empty()) {
        auto move = moves.top();
        moves.pop();

        auto nextBoard = board.advance(move.second.first, move.second.second);
        auto score = mm::alphaBetaMax(nextBoard, alpha, beta, depthleft - 1);

        if (score <= alpha)
            return alpha;

        if (score < beta)
            beta = score;
    }

    return beta;
}