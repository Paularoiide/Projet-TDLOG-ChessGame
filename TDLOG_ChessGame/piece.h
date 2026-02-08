#pragma once

#include <cstdint>

/**
 * @brief Player color.
 */
enum class Color {
    White,  ///< White side.
    Black,  ///< Black side.
    None    ///< No color (used as a sentinel value).
};

/**
 * @brief Type of chess piece.
 *
 * Includes both standard chess pieces and fairy chess pieces.
 */
enum class PieceType {
    Pawn,         ///< Pawn.
    Knight,       ///< Knight.
    Bishop,       ///< Bishop.
    Rook,         ///< Rook.
    Queen,        ///< Queen.
    King,         ///< King.
    Princess,     ///< Fairy piece: bishop + knight movement.
    Empress,      ///< Fairy piece: rook + knight movement.
    Nightrider,   ///< Fairy piece: repeated knight movement.
    Grasshopper,  ///< Fairy piece: hopper movement.
    None          ///< No piece.
};

/**
 * @brief Game variant.
 *
 * Defines which rule set and piece set are used.
 */
enum class Variant {
    Classic,      ///< Standard chess rules.
    FairyChess    ///< Chess variant with fairy pieces.
};

/**
 * @brief Get the opposite color.
 *
 * White becomes Black, Black becomes White.
 *
 * @param c Input color.
 * @return Opposite color.
 */
inline Color opposite(Color c) {
    return (c == Color::White) ? Color::Black : Color::White;
}
