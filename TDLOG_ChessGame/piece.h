#pragma once
#include "move.h"
#include "rules.h"
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
    /// @brief Move the piece to position p and change pos
    /// @param p positon
    virtual void MoveTo(Position p);
    /// @brief Virtual destructor (possibly useless)
    virtual ~Piece() = default;
};

class King : public Piece {
    public:
    King(Position p , Color c);
    void MoveTo(Position p) override;
};
class Queen : public Piece {
    public:
    Queen(Position p, Color c);
    void MoveTo(Position p) override;
};
class Rook : public Piece {
    public:
    Rook(Position p, Color c);
    void MoveTo(Position p) override;
};