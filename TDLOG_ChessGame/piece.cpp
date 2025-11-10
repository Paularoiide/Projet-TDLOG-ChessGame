#include "piece.h"
#include "board.h"
using namespace std;

Piece::Piece(Position p, PieceName n, Color c)
    : pos(p), name(n), color(c), number_of_moves(0) {}

Position Piece::getPosition() const { return pos; }
Color Piece::getColor() const { return color; }
PieceName Piece::getName() const { return name; }

void Piece::setPosition(Position p) { pos = p; }

// ==== Constructors for each subclass ====

King::King(Position p, Color c) : Piece(p, PieceName::king, c) {}
Queen::Queen(Position p, Color c) : Piece(p, PieceName::queen, c) {}
Rook::Rook(Position p, Color c) : Piece(p, PieceName::rook, c) {}
Bishop::Bishop(Position p, Color c) : Piece(p, PieceName::bishop, c) {}
Knight::Knight(Position p, Color c) : Piece(p, PieceName::knight, c) {}
Pawn::Pawn(Position p, Color c) : Piece(p, PieceName::pawn, c) {}

// ==== Pawn ====
vector<Move> Pawn::getLegalMoves(const Board& board) const {
    vector<Move> moves;

    int dir = (color == Color::White) ? -1 : 1;
    Position one_step = pos + Position(0, dir);

    // Forward movement
    if (board.isInside(one_step) && !(board.getPiece(one_step)==nullptr)) {
        moves.emplace_back(pos, one_step);
        // Two-step at start
        int start_rank = (color == Color::White) ? board.size() - 2 : 1;
        Position two_step = pos + Position(0, 2 * dir);
        if (pos.y == start_rank && board.isInside(two_step) && board.getPiece(one_step)==nullptr) {
            moves.emplace_back(pos, two_step);
        }
    }

    // Captures
    vector<Position> captures = { Position(-1, dir), Position(1, dir) };
    for (const auto& cap : captures) {
        Position target_pos = pos + cap;
        if (board.isInside(target_pos)) {
            Piece* target = board.getPiece(target_pos);
            if (target && target->getColor() != color) {
                moves.emplace_back(pos, target_pos);
            }
        }
    }

    return moves;
}

// ==== Knight ====
vector<Move> Knight::getLegalMoves(const Board& board) const {
    vector<Move> moves;
    vector<Position> offsets = {
        {1,2}, {2,1}, {-1,2}, {-2,1},
        {1,-2}, {2,-1}, {-1,-2}, {-2,-1}
    };
    for (const auto& o : offsets) {
        Position new_pos = pos + o;
        if (board.isInside(new_pos)) {
            Piece* target = board.getPiece(new_pos);
            if (!target || target->getColor() != color)
                moves.emplace_back(pos, new_pos);
        }
    }
    return moves;
}

// ==== Bishop ====
vector<Move> Bishop::getLegalMoves(const Board& board) const {
    vector<Move> moves;
    vector<Position> directions = { {1,1}, {1,-1}, {-1,1}, {-1,-1} };

    for (const auto& dir : directions) {
        Position current = pos + dir;
        while (board.isInside(current)) {
            Piece* target = board.getPiece(current);
            if (target) {
                if (target->getColor() != color)
                    moves.emplace_back(pos, current);
                break;
            }
            moves.emplace_back(pos, current);
            current = current + dir;
        }
    }
    return moves;
}

// ==== Rook ====
vector<Move> Rook::getLegalMoves(const Board& board) const {
    vector<Move> moves;
    vector<Position> directions = { {1,0}, {-1,0}, {0,1}, {0,-1} };

    for (const auto& dir : directions) {
        Position current = pos + dir;
        while (board.isInside(current)) {
            Piece* target = board.getPiece(current);
            if (target) {
                if (target->getColor() != color)
                    moves.emplace_back(pos, current);
                break;
            }
            moves.emplace_back(pos, current);
            current = current + dir;
        }
    }
    return moves;
}

// ==== Queen ====
vector<Move> Queen::getLegalMoves(const Board& board) const {
    vector<Move> moves;
    vector<Position> directions = {
        {1,0}, {-1,0}, {0,1}, {0,-1},
        {1,1}, {1,-1}, {-1,1}, {-1,-1}
    };

    for (const auto& dir : directions) {
        Position current = pos + dir;
        while (board.isInside(current)) {
            Piece* target = board.getPiece(current);
            if (target) {
                if (target->getColor() != color)
                    moves.emplace_back(pos, current);
                break;
            }
            moves.emplace_back(pos, current);
            current = current + dir;
        }
    }
    return moves;
}

// ==== King ====
vector<Move> King::getLegalMoves(const Board& board) const {
    vector<Move> moves;
    vector<Position> offsets = {
        {1,0}, {-1,0}, {0,1}, {0,-1},
        {1,1}, {1,-1}, {-1,1}, {-1,-1}
    };

    for (const auto& o : offsets) {
        Position new_pos = pos + o;
        if (board.isInside(new_pos)) {
            Piece* target = board.getPiece(new_pos);
            if (!target || target->getColor() != color)
                moves.emplace_back(pos, new_pos);
        }
    }
    return moves;
}
