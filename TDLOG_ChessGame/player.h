#pragma once
#include "move.h"
#include "game.h"

class Player {
public:
    virtual Move getMove(Game& g) = 0;
    virtual ~Player() = default;
    virtual Move getBestMove(const Board& board, Color turn);
};

// -------------------------
// Human player (via stdin)
// -------------------------
class HumanPlayer : public Player {
public:
    Move getMove(Game& g) override;
};

