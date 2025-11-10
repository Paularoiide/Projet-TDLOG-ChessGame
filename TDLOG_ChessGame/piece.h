#pragma once
#include "move.h"
#include "rules.h"
#include "board.h"
#include <vector>
/// @class Piece
/// @brief Represents a general piece
/// @details This is the base class for all pieces (classical chess or not)
class Piece {
    protected:
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
    virtual std::vector<Move> getLegalMoves(const Board& board) const = 0; // const = 0 is pure virtual method
    /// @brief set pos to the Position p
    /// @param p new position
    void setPosition(Position p) {pos = p;};
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
/// @class Queen
/// @brief Derivated classes from Piece
/// @details Represents a queen on the board
class Queen : public Piece {
    public:
    /// @brief Creates a Queen instance
    /// @param p starting position
    /// @param c color (White or Black)
    Queen(Position p, Color c);
    /// @brief get the legal moves of the queen
    /// @param board 
    /// @return a vector of legal moves
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};
/// @class Rook
/// @brief Derivated classes from Piece
/// @details Represents a rook on the board
class Rook : public Piece {
    public:
    /// @brief Creates a Rook instance
    /// @param p starting position
    /// @param c color (White or Black)
    Rook(Position p, Color c);
    /// @brief get the legal moves of the rook
    /// @param board 
    /// @return a vector of legal moves
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};
/// @class Bishop
/// @brief Derivated classes from Piece
/// @details Represents a bishop on the board
class Bishop : public Piece {
    public:
    /// @brief Creates a Bishop instance
    /// @param p starting position
    /// @param c color (White or Black)
    Bishop(Position p, Color c);
    /// @brief get the legal moves of the bishop
    /// @param board 
    /// @return an vector of legal moves
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};
/// @class Knight
/// @brief Derivated classes from Piece
/// @details Represents a knight on the board
class Knight : public Piece {
    public:
    /// @brief Creates a Knight instance
    /// @param p starting position
    /// @param c color (White or Black)
    Knight(Position p, Color c);
    /// @brief get the legal moves of the knight
    /// @param board
    /// @return a vector of legal moves
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};
/// @class Pawn
/// @brief Derivated classes from Piece
/// @details Represents a pawn on the board, added number of move to track if it can move two squares.
class Pawn : public Piece {
    int number_of_move = 0;
    public:
    /// @brief Creates a Pawn instance
    /// @param p starting position
    /// @param c color (White or Black)
    Pawn(Position p, Color c);
    /// @brief get the legal moves of the pawn
    /// @param board
    /// @return a vector of legal moves
    std::vector<Move> getLegalMoves(const Board& board) const override final;
};