#pragma once
#include "move.h"
enum PieceName {
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};
enum Color {
    White,
    Black
};
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