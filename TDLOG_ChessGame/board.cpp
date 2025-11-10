#include "board.h"

Board::Board(int size) : size_(size), squares_(size * size) {
    for (int y = 0; y < size_; ++y) {
        for (int x = 0; x < size_; ++x) {
            auto& sq = squares_[y * size_ + x];
            sq.position = Position{x, y};
            sq.piece = nullptr;
        }
    }
}


void Board::setPiece(Position p, Piece* piece) {
    if (!isInside(p)) return;
    at(p).piece = piece;
}


void Board::movePiece(Position from, Position to) {
    if (!isInside(from) || !isInside(to)) return;
    Piece* p = at(from).piece;
    at(to).piece = p;
    at(from).piece = nullptr;
}
