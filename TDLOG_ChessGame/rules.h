#pragma once

enum class Color {
    White,
    Black
};

enum class PieceName {
    king,
    queen,
    rook,
    bishop,
    knight,
    pawn
};

inline Color opposite(Color c){return c==Color::White ? Color::Black : Color::White;}


