#include "piece.h"
using namespace std;
Piece::Piece(const Position p, const PieceName n,const Color c): pos(p), name(n),color(c) {}

Position Piece::getPosition() const {
    return pos;
}
Color Piece::getColor() const {
    return color;
}
PieceName Piece::getName() const{
    return name;
}


King::King(Position p, Color c) : Piece(p, king, c){};

Queen::Queen(Position p, Color c) : Piece(p, queen, c){};

Rook::Rook(Position p, Color c) : Piece(p, rook, c){};

Bishop::Bishop(Position p, Color c) : Piece(p, bishop, c){};

Knight::Knight(Position p, Color c) : Piece(p, knight, c){};

Pawn::Pawn(Position p, Color c) : Piece(p, pawn, c){};

vector<Move> Pawn::getLegalMoves(const Board& board) const {
    if (color == Color(Black)){
        vector<Move> moves = {Move(pos,pos-Position(1,0))};
        if (number_of_move == 0){
            moves.push_back(Move(pos,pos - Position(2,0)));
        }

    }
    else{
        vector<Move> moves = {Move(pos,pos+Position(1,0))};
        if (number_of_move == 0){
            moves.push_back(Move(pos,pos+Position(2,0)));
        }

    }
}