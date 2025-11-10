#include "piece.h"

Piece::Piece(Position p, PieceName n, Color c)
    : pos(p), name(n), color(c)
{}

Position Piece::getPosition() const {
    return pos;
}
Color Piece::getColor() const {
    return color;
}
PieceName Piece::getName() const{
    return name;
}


King::King(Position p, Color c) : Piece(p, PieceName::king, c){};
Queen::Queen(Position p, Color c) : Piece(p, PieceName::queen, c){};
Rook::Rook(Position p, Color c) : Piece(p, PieceName::rook, c){};
Bishop::Bishop(Position p, Color c) : Piece(p, PieceName::bishop, c){};
Knight::Knight(Position p, Color c) : Piece(p, PieceName::knight, c){};
Pawn::Pawn(Position p, Color c) : Piece(p, PieceName::pawn, c){};

std::vector<Move> King::getLegalMoves(const Board&) const { return {}; }
std::vector<Move> Queen::getLegalMoves(const Board&) const { return {}; }
std::vector<Move> Rook::getLegalMoves(const Board&) const { return {}; }
std::vector<Move> Bishop::getLegalMoves(const Board&) const { return {}; }
std::vector<Move> Knight::getLegalMoves(const Board&) const { return {}; }
std::vector<Move> Pawn::getLegalMoves(const Board&) const { return {}; }
