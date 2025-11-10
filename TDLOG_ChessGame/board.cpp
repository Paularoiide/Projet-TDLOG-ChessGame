#include "board.h"


Board::Board(int size) : size(size_), squares(size_ * size_) {
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            auto& sq = squares[y * size + x];
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
