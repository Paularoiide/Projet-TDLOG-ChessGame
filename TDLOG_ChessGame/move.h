#pragma once

class Piece;

struct Position {
    int x;
    int y;
    Position(int a, int b) : x(a),y(b) {}
    Position() : x(0), y(0) {}
};

struct Move { // temporary structure to represent a move
    Position from;
    Position to;
    Piece* movedPiece;
    Piece* capturedPiece;
    bool isPromotion;
};
