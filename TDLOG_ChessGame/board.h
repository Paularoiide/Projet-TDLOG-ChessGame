#pragma once
#include <vector>
#include "move.h"
#include "piece.h"

struct Square {
    Position position{};
    Piece* piece{nullptr};
};

class Board {
    int size_;
    std::vector<Square> squares_; // 1D: index = y*size + x
public:
    explicit Board(int size = 8);
    int size() const { return size_; }
    bool isInside(Position p) const { return p.x >= 0 && p.y >= 0 && p.x < size_ && p.y < size_; }
    Square& at(Position p) { return squares_[p.y * size_ + p.x]; }
    const Square& at(Position p) const { return squares_[p.y * size_ + p.x]; }


    Piece* getPiece(Position p) const { return isInside(p) ? at(p).piece : nullptr; }
    void setPiece(Position p, Piece* piece);
    void movePiece(Position from, Position to);
};
