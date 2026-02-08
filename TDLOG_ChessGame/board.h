#pragma once

#include <vector>
#include <cstdint>
#include <iostream>

#include "piece.h"
#include "move.h"

/**
 * @brief A bitboard is a 64-bit mask representing a set of squares.
 *
 * Bit i (0..63) corresponds to square i.
 */
using Bitboard = uint64_t;

/**
 * @brief Chess board representation based on bitboards.
 *
 * The board stores one bitboard per (color, piece type), plus three occupancy
 * bitboards (white, black, both). It also stores castling rights, en passant state,
 * and a Zobrist hash key for fast position identification.
 *
 * This implementation supports up to 10 piece types (including fairy pieces),
 * depending on the values provided by @ref PieceType.
 */
class Board {
    /**
     * @brief Per-color, per-piece bitboards.
     *
     * Layout:
     * - bitboards_[0][0] = White pawns
     * - bitboards_[1][5] = Black king
     * - etc
     *
     * Note: second dimension is sized for 10 piece types (classic + fairy).
     */
    Bitboard bitboards_[2][10];

    /**
     * @brief Occupancy bitboards (updated after each move).
     *
     * - occupancies_[0] = all white pieces
     * - occupancies_[1] = all black pieces
     * - occupancies_[2] = all pieces
     */
    Bitboard occupancies_[3];

    /**
     * @brief Castling rights flags.
     *
     * Order: [White king-side, White queen-side, Black king-side, Black queen-side]
     */
    bool castleRights_[4] = {true, true, true, true};

    /**
     * @brief En passant target square index (0..63), or -1 if none.
     *
     * This is the square that can be captured onto via en passant on the next ply.
     */
    int enPassantTarget_ = -1;

    /**
     * @brief Zobrist hash key of the current position.
     *
     * Updated after each move (currently by recomputing).
     */
    uint64_t zobristKey_ = 0;

public:
    /**
     * @brief Construct a board and initialize it to the starting position.
     * @param v Variant to initialize (e.g., classic or fairy chess).
     */
    Board(Variant v = Variant::Classic);

    /**
     * @brief Get the bitboard for a given color and piece type.
     * @param c Color of the pieces.
     * @param pt Piece type.
     * @return Bitboard containing all pieces of type @p pt for color @p c.
     */
    Bitboard getBitboard(Color c, PieceType pt) const {
        return bitboards_[static_cast<int>(c)][static_cast<int>(pt)];
    }

    /**
     * @brief Get the piece type located at a square.
     * @param square Square index (0..63).
     * @param color Output: the color found on that square (or Color::None).
     * @return The piece type on the square (or PieceType::None).
     */
    PieceType getPieceTypeAt(int square, Color& color) const;

    /**
     * @brief Check whether a square is occupied by any piece.
     * @param square Square index (0..63).
     * @return True if occupied, false otherwise.
     */
    bool isSquareOccupied(int square) const;

    // ----------------------------
    // Bit manipulation helpers
    // ----------------------------

    /**
     * @brief Set a bit in a bitboard (mark a square as occupied).
     * @param bb Bitboard to modify.
     * @param square Square index (0..63).
     */
    static void setBit(Bitboard& bb, int square) { bb |= (1ULL << square); }

    /**
     * @brief Clear a bit in a bitboard (mark a square as empty).
     * @param bb Bitboard to modify.
     * @param square Square index (0..63).
     */
    static void popBit(Bitboard& bb, int square) { bb &= ~(1ULL << square); }

    /**
     * @brief Read a bit in a bitboard.
     * @param bb Bitboard to query.
     * @param square Square index (0..63).
     * @return True if the bit is set, false otherwise.
     */
    static bool getBit(Bitboard bb, int square) { return (bb & (1ULL << square)); }

    // ----------------------------
    // Game logic / state updates
    // ----------------------------

    /**
     * @brief Apply a move on the board.
     *
     * Handles:
     * - regular moves and captures
     * - en passant captures
     * - castling rook movement and castling rights updates
     * - pawn double push (en passant target)
     * - optional promotion
     *
     * @param from Source square index.
     * @param to Destination square index.
     * @param promotion Promotion piece type (PieceType::None if no promotion).
     */
    void movePiece(int from, int to, PieceType promotion = PieceType::None);

    /**
     * @brief Generate all legal moves for the given side to play.
     *
     * The method generates pseudo-legal moves and filters out moves that leave
     * the king in check.
     *
     * @param turn Side to play.
     * @return Vector of fully legal moves.
     */
    std::vector<Move> generateLegalMoves(Color turn) const;

    /**
     * @brief Generate capture moves only (used for quiescence search).
     *
     * This function returns only capturing moves (and possibly capture-promotions)
     *
     *
     * @param turn Side to play.
     * @return Vector of capture moves.
     */
    std::vector<Move> generateCaptures(Color turn) const;

    /**
     * @brief Recompute occupancy bitboards from per-piece bitboards.
     *
     * Should be called after any change to @ref bitboards_.
     */
    void updateOccupancies();

    /**
     * @brief Print the board to stdout.
     */
    void print() const;

    // ----------------------------
    // Check / attack utilities
    // ----------------------------

    /**
     * @brief Test if a square is attacked by a given color.
     * @param square Target square index (0..63).
     * @param attacker Attacking side.
     * @return True if @p attacker attacks @p square.
     */
    bool isSquareAttacked(int square, Color attacker) const;

    /**
     * @brief Get the king square for a given color.
     * @param c Side whose king should be located.
     * @return Square index of the king, or -1 if not found.
     */
    int getKingSquare(Color c) const;

    /**
     * @brief Check whether a side is currently in check.
     * @param c Side to test.
     * @return True if side @p c is in check.
     */
    bool isInCheck(Color c) const;

    /**
     * @brief Check whether castling is currently allowed for a side.
     * @param c Side to test.
     * @param kingSide True for king-side castling, false for queen-side.
     * @return True if castling right is still available (rights flag only).
     *
     * @note This function only checks the stored rights flag. Legality conditions
     * (empty squares, attacked squares, king in check) are checked during
     * move generation.
     */
    bool canCastle(Color c, bool kingSide) const;

    /**
     * @brief Disable castling for a side on one side (king-side or queen-side).
     * @param c Side to modify.
     * @param kingSide True for king-side, false for queen-side.
     */
    void disableCastle(Color c, bool kingSide);

    /**
     * @brief Get the en passant target square.
     * @return Square index (0..63) or -1 if none.
     */
    int getEnPassantTarget() const { return enPassantTarget_; }

    // ----------------------------
    // Zobrist hashing
    // ----------------------------

    /**
     * @brief Get the current Zobrist hash key of the position.
     * @return 64-bit hash key.
     */
    uint64_t getHash() const { return zobristKey_; }

    /**
     * @brief Initialize Zobrist random keys (static).
     *
     * Must be called at least once before hashing is used.
     *
     */
    static void initZobristKeys();

    /**
     * @brief Recompute the full Zobrist hash from scratch.
     *
     * This is safer but slower than incremental hashing, and is often used
     * after moves to ensure consistency.
     *
     * @return 64-bit hash key for the current position.
     */
    uint64_t calculateHash() const;
};
