#include "board.h"

using namespace std;

Board::Board(int s){
    size = s;
    squares.resize(s*s);
    for(int i=0; i < size*size;i++){
        squares[i] = Square(Position(i/size,i%size));
    }
}
Piece* Board::getPiece(const Position pos) const {
    return squares[pos.x*size+pos.y].getPiece();
}
void Board::movePiece(Move m){
    squares[m.to.x*size + m.to.y].setPiece(getPiece(m.from));
    squares[m.from.x*size + m.to.y].setPiece(nullptr);
}
bool Board::hasPiece(Position pos) const {
    return not (getPiece(pos) == nullptr);
}