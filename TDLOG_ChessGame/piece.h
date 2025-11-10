#pragma once
#include "move.h"
#include "rules.h"
#include <vector>

class Board;

/// @class Piece
/// @brief Represents a general chess piece
/// @details This is the abstract base class for all chess pieces
class Piece {
protected:
    Position pos;       ///< current position
    PieceName name;     ///< type of the piece (King, Queen, etc.)
    Color color;        ///< color of the piece (White or Black)
    int number_of_moves = 0; ///< number of moves made (useful for pawns, castling...)

public:
    /// @brief Creates a Piece instance
    /// @param p position
    /// @param n name of the piece
    /// @param c color of the piece (White or Black)
    Piece(Position p, PieceName n, Color c);

    /// @brief Get the current position
    Position getPosition() const;

    /// @brief Get the piece color
    Color getColor() const;

    /// @brief Get the piece name/type
    PieceName getName() const;

    /// @brief Set the piece position
    void setPosition(Position p);

    /// @brief Increment the number of moves played (used for pawns or rooks)
    void incrementMoveCount() { number_of_moves++; }

    /// @brief Get all legal moves for the piece (must be implemented by derived classes)
    virtual std::vector<Move> getLegalMoves(const Board& board) const = 0;

    /// @brief Virtual destructor
    virtual ~Piece() = default;
};

// === Derivated piece classes === //

/// @class King
class King : public Piece {
public:
    King(Position p, Color c);
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};

/// @class Queen
class Queen : public Piece {
public:
    Queen(Position p, Color c);
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};

/// @class Rook
class Rook : public Piece {
public:
    Rook(Position p, Color c);
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};

/// @class Bishop
class Bishop : public Piece {
public:
    Bishop(Position p, Color c);
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};

/// @class Knight
class Knight : public Piece {
public:
    Knight(Position p, Color c);
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};

/// @class Pawn
class Pawn : public Piece {
public:
    Pawn(Position p, Color c);
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};
