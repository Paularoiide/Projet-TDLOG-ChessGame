#include "ai.h"
#include <algorithm>
#include <vector>
#include <iostream>

// ==========================================
// 1. PIECE-SQUARE TABLES (Position Tables)
// ==========================================
// These tables define where it is good to place pieces (for White).
// We will mirror them for Black in the evaluation function.

// Pawns : Encourage advancement and control of the center
const int pawnTable[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

// Knights : Strongly encourage the center, penalize the edges
const int knightTable[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

// Bishops : Better to avoid corners and aim for long diagonals
const int bishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

// Rooks : Bonus for the 7th rank and central files
const int rookTable[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};

// Material values (P, N, B, R, Q, K)
// We assign a huge value to the King to never sacrifice it
const int pieceValues[] = { 100, 320, 330, 500, 900, 20000 };

// ==========================================
// 2. EVALUATION FUNCTION
// ==========================================
int AI::evaluate(const Board& board) {
    int score = 0;

    for (int sq = 0; sq < 64; ++sq) {
        Color c;
        PieceType pt = board.getPieceTypeAt(sq, c);
        
        if (pt == PieceType::None) continue;

        // 1. Material
        int val = pieceValues[static_cast<int>(pt)];

        // 2. Position (PST)
        // For White, read the table normally.
        // For Black, mirror the table vertically (index ^ 56).
        int tableScore = 0;
        int tableIdx = (c == Color::White) ? sq : (sq ^ 56);

        switch (pt) {
            case PieceType::Pawn:   tableScore = pawnTable[tableIdx]; break;
            case PieceType::Knight: tableScore = knightTable[tableIdx]; break;
            case PieceType::Bishop: tableScore = bishopTable[tableIdx]; break;
            case PieceType::Rook:   tableScore = rookTable[tableIdx]; break;
            default: break;
        }

        if (c == Color::White) score += (val + tableScore);
        else                   score -= (val + tableScore);
    }
    return score;
}

// ==========================================
// 3. NEGAMAX ALGORITHM (ALPHA-BETA)
// ==========================================
int AI::negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier) {
    // Base case: Depth reached
    if (depth == 0) {
        return colorMultiplier * evaluate(board);
    }

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateLegalMoves(turn);

    // Checkmate / Stalemate check
    if (moves.empty()) {
        if (board.isInCheck(turn)) {
            // Checkmate: Return a very low value, adjusted by depth
            // to prefer a mate in 1 move rather than in 5.
            return -MATE_VALUE + depth; 
        }
        return 0; // Stalemate
    }

    // Move sorting (optional but recommended for speed)
    // Here, we put captures first to cut the tree faster
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.isCapture > b.isCapture;
    });

    int maxScore = -INF;

    for (const auto& move : moves) {
        // Copy the board and simulate the move
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);

        // Recursive call (Note the minus sign and the inversion of alpha/beta)
        int score = -negamax(nextBoard, depth - 1, -beta, -alpha, -colorMultiplier);

        if (score > maxScore) {
            maxScore = score;
        }

        // Alpha-Beta Pruning
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break; // Beta cutoff: the opponent will not allow us to play this move
        }
    }

    return maxScore;
}

// ==========================================
// 4. ROOT OF THE SEARCH
// ==========================================
Move AI::getBestMove(const Board& board, int depth, Color turn) {
    std::vector<Move> moves = board.generateLegalMoves(turn);
    
    // Safety check
    if (moves.empty()) return Move(0, 0);

    // Sort to optimize
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.isCapture > b.isCapture;
    });

    Move bestMove = moves[0];
    int maxScore = -INF;
    int colorMultiplier = (turn == Color::White) ? 1 : -1;

    // Root of the search: loop over the first-level moves
    for (const auto& move : moves) {
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);

        // Launch Negamax at depth-1
        int score = -negamax(nextBoard, depth - 1, -INF, INF, -colorMultiplier);

        // Debug output (optional, to see what the AI thinks)
        std::cout << "Move: " << move.from << "->" << move.to << " Score: " << score << "\n";
        if (score > maxScore) {
            maxScore = score;
            bestMove = move;
        }
    }
    
    return bestMove;
}