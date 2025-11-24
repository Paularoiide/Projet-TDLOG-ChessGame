#pragma once
#include <string>

struct Position {
    int x, y;
    bool operator==(const Position& o) const { return x == o.x && y == o.y; }
};

struct Move {
    int from; // Index 0-63
    int to;   // Index 0-63
    // On stocke les infos minimales
    bool isCapture = false;
    
    Move(int f, int t) : from(f), to(t) {}
};

// Helpers pour conversion
inline int toIndex(Position p) { return p.y * 8 + p.x; }
inline Position toPosition(int idx) { return {idx % 8, idx / 8}; }