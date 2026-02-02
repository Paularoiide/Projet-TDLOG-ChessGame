#pragma once
#include <cstdint>

enum class Color { White, Black, None };
enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King, Princess, 
    Empress, Nightrider, Grasshopper, None };

// Enum for game variants
enum class Variant { Classic, FairyChess };

// Helper to invert color
inline Color opposite(Color c) {
    return (c == Color::White) ? Color::Black : Color::White;
}
