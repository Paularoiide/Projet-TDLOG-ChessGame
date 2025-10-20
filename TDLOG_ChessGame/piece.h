#pragma once
#include "move.h"
#include "rules.h"
#include "game.h"
#include <vector>
/// @class Piece
/// @brief Represents a general piece
/// @details This is the base class for all pieces (classical chess or not)
class Piece {
    Position pos;
    PieceName name;
    Color color;
    public:
    /// @brief Creates a Piece instance
    /// @param p position
    /// @param n name of the piece
    /// @param c color of the piece (White or Black)
    Piece(Position p, PieceName n, Color c);
    /// @brief get the position of the piece
    /// @return position pos
    Position getPosition() const;
    /// @brief get the color of the piece
    /// @return Color color
    Color getColor() const;
    /// @brief get the name of the piece
    /// @return name 
    PieceName getName() const;
    /// @brief get the legal moves of the piece
    /// @details This is a pure virtual method that must be implemented by all derivated classes
    /// @param board
    /// @return a vector of legal moves
    virtual std::vector<Move> getLegalMoves(const Board& board) const;
    /// @brief set pos to the Position p
    /// @param p new position
    void setPosition(Position p);
    /// @brief Virtual destructor (possibly useless)
    virtual ~Piece() = default;
};
/// @class King
/// @brief Derivated classes from Piece
/// @details Represents a king an the board
class King : public Piece {
    public:
    /// @brief Creates a King instance
    /// @param p Starting position
    /// @param c color (White or Black)
    King(Position p , Color c);
    /// @brief get the legal moves of the king
    /// @param board 
    /// @return a vector of legal moves
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};
class Queen : public Piece {
    public:
    Queen(Position p, Color c);
};
class Rook : public Piece {
    public:
    Rook(Position p, Color c);
};