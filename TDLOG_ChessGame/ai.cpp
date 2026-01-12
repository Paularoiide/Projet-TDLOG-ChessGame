#include "ai.h"
#include "game.h" // Nécessaire pour AI::getMove(Game& g)
#include <algorithm>
#include <vector>
#include <iostream>
#include <future>

// ==========================================
// 1. PIECE-SQUARE TABLES
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

const int pieceValues[] = {
    100, 320, 330, 500, 900, 20000,
    650,
    850,
    400,
    300
};

// ==========================================
// 2. EVALUATION FUNCTION IMPLEMENTATION
// ==========================================
static inline int getLSB(uint64_t bb) {
#ifdef _MSC_VER
    unsigned long index;
    _BitScanForward64(&index, bb);
    return index;
#else
    return __builtin_ctzll(bb);
#endif
}

int MaterialAndPositionEvaluation::operator()(const Board& board) const {
    int score = 0;

    for (int p = 0; p < 10; ++p) {
        PieceType pt = static_cast<PieceType>(p);
        int val = pieceValues[p];

        // --- BLANCS ---
        Bitboard bbWhite = board.getBitboard(Color::White, pt);
        while (bbWhite) {
            int sq = getLSB(bbWhite);
            int posVal = 0;
            switch(pt) {
            case PieceType::Pawn: posVal = pawnTable[sq]; break;
            case PieceType::Knight: posVal = knightTable[sq]; break;
            case PieceType::Bishop: posVal = bishopTable[sq]; break;
            case PieceType::Rook: posVal = rookTable[sq]; break;
            case PieceType::Princess: posVal = bishopTable[sq]; break;
            case PieceType::Empress: posVal = rookTable[sq]; break;
            case PieceType::Nightrider: posVal = knightTable[sq]; break;
            case PieceType::Grasshopper: posVal = knightTable[sq]; break;
            default: break;
            }
            score += (val + posVal);
            bbWhite &= (bbWhite - 1);
        }

        // --- NOIRS ---
        Bitboard bbBlack = board.getBitboard(Color::Black, pt);
        while (bbBlack) {
            int sq = getLSB(bbBlack);
            int tableIdx = sq ^ 56;
            int posVal = 0;
            switch(pt) {
            case PieceType::Pawn: posVal = pawnTable[tableIdx]; break;
            case PieceType::Knight: posVal = knightTable[tableIdx]; break;
            case PieceType::Bishop: posVal = bishopTable[tableIdx]; break;
            case PieceType::Rook: posVal = rookTable[tableIdx]; break;
            case PieceType::Princess: posVal = bishopTable[tableIdx]; break;
            case PieceType::Empress: posVal = rookTable[tableIdx]; break;
            case PieceType::Nightrider: posVal = knightTable[tableIdx]; break;
            case PieceType::Grasshopper: posVal = knightTable[tableIdx]; break;
            default: break;
            }
            score -= (val + posVal);
            bbBlack &= (bbBlack - 1);
        }
    }
    return score;
}

// ==========================================
// 3. NEGAMAX ALGORITHM
// ==========================================
int AI::negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier) {
    if (depth == 0) {
        // On utilise l'opérateur () du pointeur d'évaluation
        return colorMultiplier * (*evaluate)(board);
    }

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateLegalMoves(turn);

    if (moves.empty()) {
        if (board.isInCheck(turn)) return -MATE_VALUE + depth;
        return 0; // Pat
    }

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
// 4. SEARCH METHODS
// ==========================================

// Implémentation requise par l'interface Player
Move AI::getMove(Game& g) {
    return getBestMove(g.board(), g.currentTurn());
}

Move AI::getBestMove(const Board& board, Color turn) {
    std::vector<Move> moves = board.generateLegalMoves(turn);
    if (moves.empty()) return Move(0, 0);

    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.isCapture > b.isCapture;
    });

    struct MoveResult {
        int score;
        Move move;
    };

    std::vector<std::future<MoveResult>> futures;
    int colorMultiplier = (turn == Color::White) ? 1 : -1;

    // On utilise la profondeur stockée dans l'objet AI
    int currentDepth = this->searchDepth;

    for (const auto& move : moves) {
        futures.push_back(std::async(std::launch::async, [=]() -> MoveResult {
            Board threadBoard = board;
            threadBoard.movePiece(move.from, move.to, move.promotion);

            int score = -negamax(threadBoard, currentDepth - 1, -INF, INF, -colorMultiplier);
            return {score, move};
        }));
    }

    Move bestMove = moves[0];
    int maxScore = -INF;

    for (auto& f : futures) {
        MoveResult res = f.get();
        if (res.score > maxScore) {
            maxScore = res.score;
            bestMove = res.move;
        }
    }

    return bestMove;
}
