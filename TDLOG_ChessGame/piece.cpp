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
    if (color == Color(Black) && pos.x != 0 && !(board.hasPiece(pos - Position(1,0)))){
        vector<Move> moves = {Move(pos,pos-Position(1,0))};
        if (number_of_move == 0 && !(board.hasPiece(pos - Position(2,0)))){
            moves.push_back(Move(pos,pos - Position(2,0)));
        }
        vector<Position> captures = {Position(-1,-1),Position(-1,1)};
        for (const auto& cap : captures){
            Position new_pos = pos + cap;
            if (board.inBounds(new_pos)) {
                Piece* target_piece = board.getPiece(new_pos);
                if(target_piece && target_piece -> getColor() != this -> getColor()){
                    moves.push_back(Move(pos,new_pos));
                }
            }
        }
        return moves;
    }
    else if (pos.x != board.getSize()-1 && !(board.hasPiece(pos + Position(1,0)))){
        vector<Move> moves = {Move(pos,pos+Position(1,0))};
        if (number_of_move == 0 && !(board.hasPiece(pos + Position(2,0)))){
            moves.push_back(Move(pos,pos+Position(2,0)));
        }
        vector<Position> captures = {Position(1,-1),Position(1,1)};
        for (const auto& cap : captures){
            Position new_pos = pos + cap;
            if (board.inBounds(new_pos)) {
                Piece* target_piece = board.getPiece(new_pos);
                if(target_piece && target_piece -> getColor() != this -> getColor()){
                    moves.push_back(Move(pos,new_pos));
                }
            }
        }
        return moves;
    }
    return {};
}

vector<Move> Knight::getLegalMoves(const Board& board) const {
    vector<Move> moves = {};
    vector<Position> potential_moves = {
        Position(2,1), Position(2,-1), Position(-2,1), Position(-1,2),
        Position(-2,-1), Position(-1,-2), Position(1,2), Position(1,-2)
    };
    for (const auto& move : potential_moves) {
        Position new_pos = pos + move;
        if (board.inBounds(new_pos)) {
            Piece* target_piece = board.getPiece(new_pos);
            if (target_piece == nullptr || target_piece -> getColor() != this -> getColor()) {
                moves.push_back(Move(pos, new_pos));
            }
        }
    }
    return moves;
}

vector<Move> Bishop::getLegalMoves(const Board& board) const {
    vector<Move> moves = {};
    vector<Position> directions = {Position(1,1), Position(1, -1), Position(-1,1), Position(-1,-1)};
    for(const auto& dir : directions){
        Position current_pos = pos +dir;
        while (board.inBounds(current_pos)){
            Piece* target_piece = board.getPiece(current_pos);
            if (target_piece){
                if(target_piece -> getColor() != this -> getColor()){
                    moves.push_back(Move(pos,current_pos));
                }
                break;
            } else {
                moves.push_back(Move(pos,current_pos));
                current_pos = current_pos + dir;
            }
            }
        }
        return moves;
}

vector<Move> Rook::getLegalMoves(const Board& board) const {
    vector<Move> moves = {};
    vector<Position> directions = {Position(1,0), Position(-1,0), Position(0,1), Position(0,-1)};
    for(const auto& dir : directions){
        Position current_pos = pos+ dir;
        while (board.inBounds(current_pos)){
            Piece* target_piece = board.getPiece(current_pos);
            if (target_piece){
                if(target_piece -> getColor() != this -> getColor()){
                    moves.push_back(Move(pos,current_pos));
                }
                break;
            } else {
                moves.push_back(Move(pos,current_pos));
                current_pos = current_pos + dir;
            }
        }
    }
    return moves;
}

vector<Move> Queen::getLegalMoves(const Board& board) const {
    vector<Move> moves = {};
    vector<Position> directions = {Position(1,0), Position(-1,0), Position(0,1), Position(0,-1),
        Position(1,1), Position(1, -1), Position(-1,1), Position(-1,-1)};
    for(const auto& dir : directions){
        Position current_pos = pos+ dir;            
        while (board.inBounds(current_pos)){
            Piece* target_piece = board.getPiece(current_pos);
            if (target_piece){
                if(target_piece -> getColor() != this -> getColor()){
                    moves.push_back(Move(pos,current_pos));
                }
                break;
                } else {
                    moves.push_back(Move(pos,current_pos));
                    current_pos = current_pos + dir;
                }
            }
    }
    return moves;    
}

vector<Move> King::getLegalMoves(const Board& board) const {
    vector<Move> moves ={};
    vector<Position> potential_moves = {
        Position(1,0), Position(-1,0), Position(0,1), Position(0,-1),
        Position(1,1),Position(-1,1), Position(1,-1), Position(-1,-1)
    };
    for(const auto& move : potential_moves){
        Position current_pos = pos+ move;
        while (board.inBounds(current_pos)){
            Piece* target_piece = board.getPiece(current_pos);
            if (target_piece){
                if(target_piece -> getColor() != this -> getColor()){
                    moves.push_back(Move(pos,current_pos));
                }
                break;
            } else {
                moves.push_back(Move(pos,current_pos));
                current_pos = current_pos + move;
            }
        }
    }
    return moves;
}

