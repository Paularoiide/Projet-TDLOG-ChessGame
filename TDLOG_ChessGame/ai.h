#pragma once
#include "board.h"
#include "move.h"
#include "piece.h"
#include "player.h"

// Scoring constants
const int INF = 50000;        // Infinity
const int MATE_VALUE = 49000; // Checkmate score (slightly less than infinity to favor quick mates)

class AI : public Player {
public:
    // Main function called by the main program
    // Note the addition of the 'turn' parameter to know who is playing
    static Move getBestMove(const Board& board, int depth, Color turn);

private:
    // Recursive search engine (Negamax)
    static int negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier);

    // Static evaluation function (Material + Position)
    static int evaluate(const Board& board);
};