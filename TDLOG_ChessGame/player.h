#pragma once

#include "move.h"
#include "game.h"

/**
 * @brief Abstract interface for a game player.
 *
 * A Player is responsible for choosing a move given the current game state.
 * Implementations can be human-controlled, AI-controlled, network-controlled
 */
class Player {
public:
    /**
     * @brief Compute the next move to play.
     * @param g Current game state.
     * @return Chosen move.
     */
    virtual Move getMove(Game& g) = 0;

    /**
     * @brief Virtual destructor for safe polymorphic deletion.
     */
    virtual ~Player() = default;
};

/**
 * @brief Human-controlled player reading moves from standard input.
 *
 * Input format:
 * - "e2 e4" for a normal move
 * - "e7 e8 q" for a promotion (q/r/b/n)
 */
class HumanPlayer : public Player {
public:
    /**
     * @brief Read and parse a move from stdin.
     * @param g Current game state (not modified).
     * @return Parsed move, or Move(-1, -1) on invalid input.
     */
    Move getMove(Game& g) override;
};
