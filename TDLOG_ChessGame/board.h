#pragma once
#include <vector>
#include "move.h"
#include "piece.h"

class Square{
    Position position;
    Piece* piece;
    public:
    Square() : position(Position()), piece(nullptr) {}
    Square(Position p): position(p),piece(nullptr) {}
    Piece* getPiece() const {return piece; } ;
    void setPiece(Piece* p ) {piece = p;};
};
class Board {
    int size;
    std::vector<Square> squares;
    public:
    Board(int s);
    Piece* getPiece(const Position pos) const;
    void movePiece(Move m);
    bool hasPiece(Position pos) const;
};
