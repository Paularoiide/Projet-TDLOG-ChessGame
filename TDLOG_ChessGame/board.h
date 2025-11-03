#pragma once
#include <vector>
#include "move.h"
#include "piece.h"
class Square{
    Position position;
    Piece* piece;
};
class Board {
    int size;
    std::vector<Square> squares;
    public:
    Board(int s);
    Piece* getPiece(Position pos);
    void movePiece(Move m);
    bool hasPiece(Position pos);
};