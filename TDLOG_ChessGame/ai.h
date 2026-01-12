#pragma once
#include "board.h"
#include "move.h"
#include "piece.h"
#include "player.h"

// Scoring constants
const int INF = 50000;        // Infinity
const int MATE_VALUE = 49000; // Checkmate score (slightly less than infinity to favor quick mates)
class EvaluationFunctions {
    public:
        virtual ~EvaluationFunctions() = default;
        virtual int operator()(const Board& board) const = 0;
    };
    
class MaterialAndPositionEvaluation : public EvaluationFunctions {
    public:
        int operator()(const Board& board) const override;
    };

class AI : public Player {
    EvaluationFunctions* evaluate;
    int searchDepth;
public:
    AI(EvaluationFunctions* evalStrategy, int depth) 
        : evaluate(evalStrategy), searchDepth(depth) {}

    ~AI() {
        if (evaluate) delete evaluate;
    }
    Move getMove(Game& g) override; // to satisfy Player interface
    // Main function called by the main program
    // Note the addition of the 'turn' parameter to know who is playing
    Move getBestMove(const Board& board, Color turn);


private:
    // Recursive search engine (Negamax)
    int negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier);
};