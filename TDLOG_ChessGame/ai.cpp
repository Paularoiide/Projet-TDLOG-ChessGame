#include "ai.h"
#include <algorithm>
#include <vector>
#include <iostream>

// ... (Le début du fichier avec les Tables reste inchangé) ...
// Copiez-collez tout le début du fichier jusqu'à la fonction negamax incluse.
// Je remets ici juste la fin avec la correction dans getBestMove.

// ==========================================
// 1. PIECE-SQUARE TABLES (Position Tables)
// ==========================================
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
        int val = pieceValues[static_cast<int>(pt)];
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
// 3. NEGAMAX ALGORITHM
// ==========================================
int AI::negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier) {
    if (depth == 0) return colorMultiplier * evaluate(board);

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateLegalMoves(turn);

    if (moves.empty()) {
        if (board.isInCheck(turn)) return -MATE_VALUE + depth; 
        return 0; 
    }

    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.isCapture > b.isCapture;
    });

    int maxScore = -INF;
    for (const auto& move : moves) {
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);
        int score = -negamax(nextBoard, depth - 1, -beta, -alpha, -colorMultiplier);
        if (score > maxScore) maxScore = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; 
    }
    return maxScore;
}

// ==========================================
// 4. ROOT OF THE SEARCH
// ==========================================
Move AI::getBestMove(const Board& board, int depth, Color turn) {
    std::vector<Move> moves = board.generateLegalMoves(turn);
    if (moves.empty()) return Move(0, 0);

    // Sort to optimize
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.isCapture > b.isCapture;
    });

    Move bestMove = moves[0];
    int maxScore = -INF;
    int colorMultiplier = (turn == Color::White) ? 1 : -1;

    for (const auto& move : moves) {
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);

        // Negamax recursion
        int score = -negamax(nextBoard, depth - 1, -INF, INF, -colorMultiplier);

        // --- CORRECTION ICI : ON COMMENTE LE PRINT ---
        // std::cout << "Move: " << move.from << "->" << move.to << " Score: " << score << "\n";
        // ---------------------------------------------

        if (score > maxScore) {
            maxScore = score;
            bestMove = move;
        }
    }
    
    return bestMove;
}