#pragma once

#include <string>
#include "piece.h"

/**
 * @brief 2D board coordinate (x, y).
 *
 * Coordinates are 0-based on an 8x8 board:
 * - x in [0..7] is the file (column)
 * - y in [0..7] is the rank (row)
 */
struct Position {
    int x; ///< File (0..7).
    int y; ///< Rank (0..7).

    /**
     * @brief Equality comparison.
     * @param o Other position.
     * @return True if both coordinates match.
     */
    bool operator==(const Position& o) const { return x == o.x && y == o.y; }
};

/**
 * @brief Chess move represented by source and destination squares.
 *
 * Squares are stored as linear indices in [0..63] using:
 * index = y * 8 + x.
 *
 * The move also stores:
 * - whether it is a capture
 * - an optional promotion piece type
 */
struct Move {
    int from; ///< Source square index (0..63).
    int to;   ///< Destination square index (0..63).

    bool isCapture = false; ///< True if the move captures an opponent piece.
    PieceType promotion = PieceType::None; ///< Promotion piece type (None if no promotion).

    /**
     * @brief Construct a move.
     * @param f Source square index.
     * @param t Destination square index.
     * @param p Promotion piece type (PieceType::None for no promotion).
     */
    Move(int f, int t, PieceType p = PieceType::None)
        : from(f), to(t), promotion(p) {}
};

/**
 * @brief Convert a 2D position to a 0..63 board index.
 * @param p Board coordinate.
 * @return Linear square index.
 */
inline int toIndex(Position p) { return p.y * 8 + p.x; }

/**
 * @brief Convert a 0..63 board index to a 2D position.
 * @param idx Linear square index.
 * @return Board coordinate (x, y).
 */
inline Position toPosition(int idx) { return {idx % 8, idx / 8}; }
