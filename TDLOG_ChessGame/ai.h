#pragma once
#include "board.h"
#include "move.h"
#include "piece.h"

const int INF = 50000;

class AI {
public:
    
    static Move getBestMove(const Board& board, int depth, Color turn);

private:
    static int negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier);
    static int evaluate(const Board& board);
};