#pragma once

class Piece;

struct Position {
    int x;
    int y;
    Position(int a, int b) : x(a),y(b) {}
    Position() : x(0), y(0) {}
    bool operator==(const Position& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Position& o) const { return !(*this == o); }
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
