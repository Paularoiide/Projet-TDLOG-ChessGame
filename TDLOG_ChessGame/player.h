#pragma once
#include "move.h"
#include "game.h"

class Player {
public:
    virtual Move getMove(Game& g) = 0;
    virtual ~Player() = default;
};

// -------------------------
// Human player (via stdin)
// -------------------------
class HumanPlayer : public Player {
public:
    Move getMove(Game& g) override;
};

// -------------------------
// AI player (minimax)
// -------------------------
class AIPlayer : public Player {
    int depth_;
public:
    AIPlayer(int depth = 8) : depth_(depth) {}
    Move getMove(Game& g) override;
};
