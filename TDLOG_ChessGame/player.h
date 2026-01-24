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

