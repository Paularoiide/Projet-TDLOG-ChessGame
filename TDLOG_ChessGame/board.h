#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include "piece.h"
#include "move.h"

// A Bitboard is just an unsigned 64-bit integer
using Bitboard = uint64_t;

class Board {
    // 2 colors, 6 piece types
    // bitboards[0][0] = White Pawns, bitboards[1][5] = Black King, etc.
    Bitboard bitboards_[2][10];

    // Utility bitboards (updated after each move)
    Bitboard occupancies_[3]; // 0: White, 1: Black, 2: Both

    // Castling rights: [White_K, White_Q, Black_K, Black_Q]
    bool castleRights_[4] = {true, true, true, true};
    // En passant target square (-1 if none)
    int enPassantTarget_ = -1;

public:
    Board(Variant v = Variant::Classic); // Initializes the standard starting position

    // Fast accessors
    Bitboard getBitboard(Color c, PieceType pt) const {
        return bitboards_[static_cast<int>(c)][static_cast<int>(pt)];
    }

    PieceType getPieceTypeAt(int square, Color& color) const;
    bool isSquareOccupied(int square) const;

    // Bit manipulation
    static void setBit(Bitboard& bb, int square) { bb |= (1ULL << square); }
    static void popBit(Bitboard& bb, int square) { bb &= ~(1ULL << square); }
    static bool getBit(Bitboard bb, int square) { return (bb & (1ULL << square)); }

    // Game logic
    void movePiece(int from, int to, PieceType promotion = PieceType::None);

    // Move generation (simplified example)
    std::vector<Move> generateLegalMoves(Color turn) const;

    // Updates global occupancies
    void updateOccupancies();

    // For display / printing
    void print() const;

    // Checks if a given square is attacked by a specific color
    bool isSquareAttacked(int square, Color attacker) const;

    // Finds the king's position for a given color
    int getKingSquare(Color c) const;

    // Indicates whether the player of color 'c' is in check
    bool isInCheck(Color c) const;

    bool canCastle(Color c, bool kingSide) const;
    void disableCastle(Color c, bool kingSide);

    int getEnPassantTarget() const { return enPassantTarget_; }
};
