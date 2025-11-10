#pragma once
#include <vector>
#include "move.h"
#include "piece.h"
class Square{
    Position position;
    Piece* piece;
    public:
    Square(Position p): position(p),piece(nullptr) {}
    Piece* getPiece() const {return piece; } ;
    void setPiece(Piece* p ) {piece = p;};
};
class Board {
    protected:
    int size;
    std::vector<Square> squares;
    public:
    int getSize() const {return size;};
    Board(int s);
    Piece* getPiece(const Position pos) const;
    void movePiece(Move m);
    bool hasPiece(Position pos) const;
    bool inBounds(Position pos) const {
        return (pos.x >= 0 && pos.x < size && pos.y >= 0 && pos.y < size);
    }
};