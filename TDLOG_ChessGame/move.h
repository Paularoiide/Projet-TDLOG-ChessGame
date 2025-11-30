#pragma once
#include <string>
#include "piece.h"
struct Position {
    int x, y;
    bool operator==(const Position& o) const { return x == o.x && y == o.y; }
};

struct Move {
    int from;
    int to;
    bool isCapture = false;
    PieceType promotion = PieceType::None; // Added for pawn promotion
    Move(int f, int t, PieceType p = PieceType::None) 
        : from(f), to(t), promotion(p) {}
};
// Helpers for conversion
inline int toIndex(Position p) { return p.y * 8 + p.x; }
inline Position toPosition(int idx) { return {idx % 8, idx / 8}; }
