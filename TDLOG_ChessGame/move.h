#pragma once

class Piece;

struct Position {
    int x;
    int y;
};

struct Move { // temporary structure to represent a move
    Position from;
    Position to;
    Piece* movedPiece;
    Piece* capturedPiece;
    bool isPromotion;
};
