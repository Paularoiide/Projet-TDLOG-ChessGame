#pragma once
#include "move.h"
#include "rules.h"
class Piece {
    Position pos;
    PieceName name;
    Color color;
    public:
    Piece(Position p, PieceName n, Color c);
    Position getPosition();
    Color getColor();
    PieceName getName();
    virtual void MoveTo(Position p);
};

class King : public Piece {
    public:
    King(Position p , Color c);
    void MoveTo(Position p) override;
};