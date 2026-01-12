#pragma once
#include "board.h"
#include "move.h"
#include "piece.h"
#include "player.h"
// Type of entries in the TT (Transition Table)
enum class TTFlag { EXACT, ALPHA, BETA };

struct TTEntry {
    uint64_t key;   // To verify it's the same position (collision)
    int score;      // The stored score
    int depth;      // The search depth that produced this score
    Move bestMove;  // The best move found
    TTFlag flag;    // Type of score (Lower bound, upper bound, or exact)
};
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
    std::vector<TTEntry> transpositionTable;
    int ttSize = 0;
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
    int quiescence(const Board& board, int alpha, int beta, int colorMultiplier);
    // Helpers for Transposition Table
    void storeTT(uint64_t key, int score, int depth, int alpha, int beta, Move bestMove);
    bool probeTT(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove);
};