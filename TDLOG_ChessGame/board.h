#pragma once
#include <vector>
#include "move.h"
#include "piece.h"

struct Square {
    Position position{};
    Piece* piece{nullptr};
};

class Board {
    int size;
    std::vector<Square> squares; // 1D: index = y*size + x
public:
    explicit Board(int size_ = 8);
    int size() const { return size; }

    bool isInside(Position p) const { return p.x >= 0 && p.y >= 0 && p.x < size && p.y < size; }
    Square& at(Position p) { return squares[p.y * size + p.x]; }
    const Square& at(Position p) const { return squares[p.y * size + p.x]; }


    Piece* getPiece(Position p) const { return isInside(p) ? at(p).piece : nullptr; }
    void setPiece(Position p, Piece* piece);
    void movePiece(Position from, Position to);
};
