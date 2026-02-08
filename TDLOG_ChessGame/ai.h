#pragma once

#include "board.h"
#include "move.h"
#include "piece.h"
#include "player.h"

#include <mutex>   ///< Thread-safety for the transposition table.
#include <vector>
#include <cstdint>

/**
 * @brief Type of bound stored in a transposition table entry.
 *
 * The flag indicates how the stored score should be interpreted:
 * - EXACT: the score is exact for the searched depth.
 * - ALPHA: the score is an upper bound (fail-low).
 * - BETA : the score is a lower bound (fail-high).
 */
enum class TTFlag { EXACT, ALPHA, BETA };

/**
 * @brief One entry in the transposition table (TT).
 *
 * The TT caches results of previously evaluated positions (identified by a hash key),
 * to speed up the search by reusing scores and best moves.
 */
struct TTEntry {
    /** @brief Position hash key used to detect collisions. */
    uint64_t key;

    /** @brief Stored evaluation score (interpreted according to @ref flag). */
    int score;

    /** @brief Search depth at which this entry was computed. */
    int depth;

    /** @brief Best move found for this position at the stored depth. */
    Move bestMove;

    /** @brief Indicates whether @ref score is exact, an upper bound, or a lower bound. */
    TTFlag flag;

    /**
     * @brief Default constructor creating an "empty" entry.
     *
     * Uses a null key and an invalid best move (-1, -1) as sentinel values.
     */
    TTEntry()
        : key(0), score(0), depth(0), bestMove(-1, -1), flag(TTFlag::EXACT) {}

    /**
     * @brief Construct a fully specified TT entry.
     * @param k Position hash key.
     * @param s Stored score.
     * @param d Search depth used to compute the score.
     * @param m Best move found.
     * @param f Bound type of the stored score.
     */
    TTEntry(uint64_t k, int s, int d, Move m, TTFlag f)
        : key(k), score(s), depth(d), bestMove(m), flag(f) {}
};

/** @brief "Infinity" score used in alpha-beta search. */
const int INF = 50000;

/**
 * @brief Checkmate score.
 *
 * Slightly smaller than @ref INF to allow preferring quicker mates ( mate in 3
 * is scored higher than mate in 5 when depth is included).
 */
const int MATE_VALUE = 49000;

/**
 * @brief Interface for evaluation strategies.
 *
 * This class acts as a strategy object: different evaluation functions can be injected
 * into the AI to change its playing style/strength.
 */
class EvaluationFunctions {
public:
    virtual ~EvaluationFunctions() = default;

    /**
     * @brief Evaluate a position.
     * @param board Current board position.
     * @return Evaluation score from White's perspective (positive = White is better).
     */
    virtual int operator()(const Board& board) const = 0;
};

/**
 * @brief Default evaluation based on material and piece-square tables.
 *
 * Combines material values and positional bonuses (piece-square tables).
 * Designed to support both standard and fairy pieces (depending on your PieceType set).
 */
class MaterialAndPositionEvaluation : public EvaluationFunctions {
public:
    /**
     * @brief Evaluate a position using material + piece-square tables.
     * @param board Current board position.
     * @return Evaluation score from White's perspective.
     */
    int operator()(const Board& board) const override;
};

/**
 * @brief Chess AI player using Negamax + Alpha-Beta + Quiescence and a Transposition Table.
 *
 * This AI searches for the best move from a given position using a depth-limited
 * negamax alpha-beta search. A quiescence search is used at leaf nodes to reduce
 * the horizon effect. A transposition table (TT) is used to cache results.
 *
 * The implementation is intended to support multi-threaded search, hence the mutex
 * protecting the TT.
 */
class AI : public Player {
    /** @brief Evaluation strategy used by the AI (owned by AI). */
    EvaluationFunctions* evaluate;

    /** @brief Maximum search depth (plies). */
    int searchDepth;

    /** @brief Fixed-size transposition table storage. */
    std::vector<TTEntry> transpositionTable;

    /** @brief Number of entries in @ref transpositionTable. */
    int ttSize = 0;

    /** @brief Mutex to protect TT accesses during multi-threaded search. */
    std::mutex ttMutex;

public:
    /**
     * @brief Construct an AI with a given evaluation strategy and search depth.
     * @param evalStrategy Pointer to an evaluation strategy (AI takes ownership).
     * @param depth Search depth in plies.
     */
    AI(EvaluationFunctions* evalStrategy, int depth)
        : evaluate(evalStrategy), searchDepth(depth) {
        ttSize = 2000000;
        transpositionTable.resize(ttSize);
    }

    /**
     * @brief Destructor.
     *
     * Deletes the owned evaluation strategy.
     */
    ~AI() {
        if (evaluate) delete evaluate;
    }

    /**
     * @brief Get a move for the current game state (Player interface).
     * @param g Current game instance.
     * @return Chosen move.
     */
    Move getMove(Game& g) override;

    /**
     * @brief Compute the best move from a given board position.
     * @param board Current board position.
     * @param turn Side to play.
     * @return Best move found by the search.
     */
    Move getBestMove(const Board& board, Color turn);

private:
    /**
     * @brief Negamax search with alpha-beta pruning.
     *
     * @param board Current position.
     * @param depth Remaining depth (plies).
     * @param alpha Alpha bound.
     * @param beta Beta bound.
     * @param colorMultiplier +1 for White to move, -1 for Black to move.
     * @return Best score for the side to move (after applying @p colorMultiplier).
     */
    int negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier);

    /**
     * @brief Quiescence search to reduce the horizon effect.
     *
     * Typically explores only tactical moves (captures, sometimes checks/promotions),
     * starting from a "stand pat" evaluation.
     *
     * @param board Current position.
     * @param alpha Alpha bound.
     * @param beta Beta bound.
     * @param colorMultiplier +1 for White to move, -1 for Black to move.
     * @return Refined evaluation score.
     */
    int quiescence(const Board& board, int alpha, int beta, int colorMultiplier);

    /**
     * @brief Store a result in the transposition table.
     *
     * The entry flag (EXACT/ALPHA/BETA) is determined based on the returned score
     * relative to the original alpha/beta window.
     *
     * @param key Position hash key.
     * @param score Score to store.
     * @param depth Search depth used to compute the score.
     * @param alpha Original alpha bound (at node entry).
     * @param beta Beta bound.
     * @param bestMove Best move found at this node.
     */
    void storeTT(uint64_t key, int score, int depth, int alpha, int beta, Move bestMove);

    /**
     * @brief Probe the transposition table for a given position.
     *
     * If a valid entry exists with sufficient depth, and its bound is compatible with
     * the current alpha/beta window, the function can return a usable score to cut
     * the search early.
     *
     * @param key Position hash key.
     * @param depth Required search depth.
     * @param alpha Alpha bound.
     * @param beta Beta bound.
     * @param score Output: retrieved score if found/usable.
     * @param bestMove Output: retrieved best move if available.
     * @return True if the entry provides a usable score for the current window.
     */
    bool probeTT(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove);
};
