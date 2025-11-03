#pragma once

class Piece;

struct Position {
    int x;
    int y;
    Position(int a, int b) : x(a),y(b) {}
};
Position operator+(Position p1, Position p2);
Position operator-(Position p1, Position p2);
struct Move { // temporary structure to represent a move
    Position from;
    Position to;
    Piece* movedPiece;
    Piece* capturedPiece;
    bool isPromotion;
    Move(Position p1,Position p2) : from(p1),to(p2) {}
};
