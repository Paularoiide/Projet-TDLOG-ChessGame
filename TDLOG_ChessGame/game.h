#pragma once

#include <vector>
#include "board.h"
#include "piece.h"   ///< Defines the Color enum.
#include "move.h"

/**
 * @brief Global state of the game.
 *
 * Describes the situation after the last played move.
 */
enum class GameState {
    Playing,    ///< Game is ongoing and the side to move is not in check.
    Check,      ///< The side to move is currently in check.
    Checkmate,  ///< The side to move has no legal moves and is in check.
    Stalemate,  ///< The side to move has no legal moves and is not in check.
    Prom        ///< A pawn promotion is pending.
};

/**
 * @brief High-level game controller.
 *
 * This class manages:
 * - The chess board.
 * - The side to move.
 * - Move validation and execution.
 * - Detection of check, checkmate and stalemate.
 * - Pawn promotion handling.
 */
class Game {
    Board board_;                        ///< Current board position.
    Color currentTurn_;                  ///< Side to move.
    GameState state_{GameState::Playing};///< Current game state.
    int promPos;                         ///< Board square of the pawn to promote.

public:
    /**
     * @brief Construct a new game.
     *
     * Initializes a default board and sets White as the first player.
     */
    Game();

    /**
     * @brief Start a new game with the given variant.
     *
     * Reinitializes the board and resets the turn to White.
     *
     * @param v Variant used to initialize the board.
     */
    void startGame(Variant v = Variant::Classic);

    /**
     * @brief Play a move for the current player.
     *
     * The move is checked against the list of legal moves generated
     * from the current position. If the move is legal, it is applied,
     * the turn is switched, and the game state is updated accordingly.
     *
     * @param move Requested move.
     * @return True if the move is legal and has been played, false otherwise.
     */
    bool playMove(const Move& move);

    /**
     * @brief Resolve a pawn promotion.
     *
     * Applies the chosen promotion piece to the pawn that reached
     * the last rank and clears the promotion state.
     *
     * @param pt Piece type selected for promotion.
     * @return True if the promotion was successfully applied.
     */
    bool prom(int pt);

    /**
     * @brief Access the current board.
     * @return Const reference to the board.
     */
    const Board& board() const { return board_; }

    /**
     * @brief Get the side to move.
     * @return Current player color.
     */
    Color currentTurn() const { return currentTurn_; }

    /**
     * @brief Get the current game state.
     * @return Current GameState.
     */
    GameState gameState() const { return state_; }
};
