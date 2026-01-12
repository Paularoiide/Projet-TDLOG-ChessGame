#include "ai.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <future>
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
// to satisfy linker
Move AI::getMove(Game& g) {
    return getBestMove(g.board(), g.currentTurn());
}
// ==========================================
// 2. EVALUATION FUNCTION (Optimized)
// ==========================================
static inline int getLSB(uint64_t bb) {
    return __builtin_ctzll(bb);
}

int MaterialAndPositionEvaluation::operator()(const Board& board) const {
    int score = 0;

    // Material and positional evaluation
    for (int p = 0; p < 6; ++p) {
        PieceType pt = static_cast<PieceType>(p);
        int val = pieceValues[p];

        // --- BLANCS ---
        Bitboard bbWhite = board.getBitboard(Color::White, pt);
        while (bbWhite) {
            int sq = getLSB(bbWhite);
            
            // add material and positional value
            score += (val + (pt == PieceType::Pawn   ? pawnTable[sq] :
                             pt == PieceType::Knight ? knightTable[sq] :
                             pt == PieceType::Bishop ? bishopTable[sq] :
                             pt == PieceType::Rook   ? rookTable[sq] : 0));
            
            bbWhite &= (bbWhite - 1);
        }

        // --- BLACK ---
        Bitboard bbBlack = board.getBitboard(Color::Black, pt);
        while (bbBlack) {
            int sq = getLSB(bbBlack);
            
            // For black pieces, we need to mirror the square for positional value
            int tableIdx = sq ^ 56;

            score -= (val + (pt == PieceType::Pawn   ? pawnTable[tableIdx] :
                             pt == PieceType::Knight ? knightTable[tableIdx] :
                             pt == PieceType::Bishop ? bishopTable[tableIdx] :
                             pt == PieceType::Rook   ? rookTable[tableIdx] : 0));
            
            bbBlack &= (bbBlack - 1);
        }
    }
    return score;
}

// ==========================================
// 3. NEGAMAX ALGORITHM
// ==========================================
int AI::negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier) {
    if (depth == 0) return quiescence(board, alpha, beta, colorMultiplier);

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateLegalMoves(turn);

    if (moves.empty()) {
        if (board.isInCheck(turn)) return -MATE_VALUE + depth; 
        return 0; 
    }
    // Move ordering: prioritize captures and promotions
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        if (a.isCapture != b.isCapture) return a.isCapture;
        if (a.promotion != PieceType::None) return true;
        return false;
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
Move AI::getBestMove(const Board& board, Color turn) {
    std::vector<Move> moves = board.generateLegalMoves(turn);
    if (moves.empty()) return Move(0, 0);

    // Move ordering: prioritize captures
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.isCapture > b.isCapture;
    });
    // Structure to hold move and its score
    struct MoveResult {
        int score;
        Move move;
    };

    std::vector<std::future<MoveResult>> futures;
    int colorMultiplier = (turn == Color::White) ? 1 : -1;
    for (const auto& move : moves) {
        
        futures.push_back(std::async(std::launch::async, [=, &board]() -> MoveResult {
            // 1. Copy the board
            Board threadBoard = board; 
            // 2. apply move
            threadBoard.movePiece(move.from, move.to, move.promotion);

            // 3. Calculate the score (depth - 1)
            int score = -negamax(threadBoard, this->searchDepth - 1, -INF, INF, -colorMultiplier);

            return {score, move};
        }));
    }

    Move bestMove = moves[0];
    int maxScore = -INF;

    for (auto& f : futures) {
        // .get() waits for the thread to finish and retrieves the result
        MoveResult res = f.get();
        
        if (res.score > maxScore) {
            maxScore = res.score;
            bestMove = res.move;
        }
    }
    
    return bestMove;
}
// ==========================================
// 5. QUIESCENCE SEARCH (Calming Search)
// ==========================================
// This function extends the search at leaf nodes to include only "noisy" moves like captures.
int AI::quiescence(const Board& board, int alpha, int beta, int colorMultiplier) {
    // 1. Stand Pat
    int stand_pat = colorMultiplier * (*evaluate)(board);

    if (stand_pat >= beta) return beta;

    // --- OPTIMIZATION : DELTA PRUNING ---
    // If the stand pat is so low that even adding the maximum possible gain from captures
    // we don't even search captures.
    // Safety margin = 975 (Queen + a bit more)
    const int DELTA = 975;
    if (stand_pat < alpha - DELTA) {
        return alpha;
    }

    if (stand_pat > alpha) alpha = stand_pat;

    // 2. Optimized generation (captures only)
    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    
    // --- CALL THE NEW FUNCTION ---
    std::vector<Move> moves = board.generateCaptures(turn); 

    // 3. MVV-LVA sorting (Always useful)
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        //  Here, we know that every move is a capture
        // so we sort by "probable value" (approximation)
        // Promotion first
        if (a.promotion != PieceType::None && b.promotion == PieceType::None) return true;
        if (a.promotion == PieceType::None && b.promotion != PieceType::None) return false;
        return false;
    });

    for (const auto& move : moves) {
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);

        int score = -quiescence(nextBoard, -beta, -alpha, -colorMultiplier);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}