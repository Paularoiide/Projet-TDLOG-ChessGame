#include "piece.h"

Piece::Piece(Position p, PieceName n, Color c){
    pos = p;
    name = n;
    color = c;
}
Position Piece::getPosition() {
    return pos;
}
Color Piece::getColor() {
    return color;
}
PieceName Piece::getName() {
    return name;
}

King::King(Position p, Color c) : Piece(p, king, c){};

Queen::Queen(Position p, Color c) : Piece(p, queen, c){};