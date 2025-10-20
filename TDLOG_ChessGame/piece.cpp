#include "piece.h"

Piece::Piece(Position p, PieceName n, Color c){
    pos = p;
    name = n;
    color = c;
}
Position Piece::getPosition() const {
    return pos;
}
Color Piece::getColor() const {
    return color;
}
PieceName Piece::getName() const{
    return name;
}


King::King(Position p, Color c) : Piece(p, king, c){};

Queen::Queen(Position p, Color c) : Piece(p, queen, c){};

Rook::Rook(Position p, Color c) : Piece(p, rook, c){};