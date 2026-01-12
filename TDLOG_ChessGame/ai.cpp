#include "ai.h"
#include "game.h" 
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

// Tableau étendu (10 valeurs) pour supporter Fairy Chess
const int pieceValues[] = { 
    100, 320, 330, 500, 900, 20000, 
    650, 850, 400, 300 
};

// ==========================================
// 2. PLAYER INTERFACE IMPL
// ==========================================
Move AI::getMove(Game& g) {
    return getBestMove(g.board(), g.currentTurn());
}

// ==========================================
// 3. EVALUATION FUNCTION (Fairy Compatible)
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

    // Boucle jusqu'à 10 pour inclure les pièces féeriques
    for (int p = 0; p < 10; ++p) {
        PieceType pt = static_cast<PieceType>(p);
        int val = pieceValues[p];

        // --- BLANCS ---
        Bitboard bbWhite = board.getBitboard(Color::White, pt);
        while (bbWhite) {
            int sq = getLSB(bbWhite);
            int posVal = 0;
            // Switch case complet (Fairy)
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
// HELPER : TRANSPOSITION TABLE
// ==========================================
void AI::storeTT(uint64_t key, int score, int depth, int alpha, int beta, Move bestMove) {
    // index calculation
    size_t index = key % ttSize;
    
    TTFlag flag = TTFlag::EXACT;
    if (score <= alpha) flag = TTFlag::ALPHA;      // Fail Low (Upper Bound)
    else if (score >= beta) flag = TTFlag::BETA;   // Fail High (Lower Bound)
    
    transpositionTable[index] = { key, score, depth, bestMove, flag };
}

bool AI::probeTT(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove) {
    size_t index = key % ttSize;
    const TTEntry& entry = transpositionTable[index];

    // Key verification (to avoid hash collisions)
    if (entry.key == key) {
        // Retrieve the best move if available (useful for move ordering later!)
        if (entry.bestMove.from != -1) bestMove = entry.bestMove;

        // We can only use the score if the stored search was 
        // at least as deep as what we're currently requesting.
        if (entry.depth >= depth) {
            if (entry.flag == TTFlag::EXACT) {
                score = entry.score;
                return true;
            }
            if (entry.flag == TTFlag::ALPHA && entry.score <= alpha) {
                score = alpha;
                return true;
            }
            if (entry.flag == TTFlag::BETA && entry.score >= beta) {
                score = beta;
                return true;
            }
        }
    }
    return false;
}
// ==========================================
// 4. NEGAMAX ALGORITHM (With Quiescence)
// ==========================================
int AI::negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier) {
    int alphaOrig = alpha;

    // 1. HASHING
    uint64_t hash = board.getHash();
    if (colorMultiplier == -1) { 
        // Hash for "Black" = HashBoard ^ SideKey. 
        hash = ~hash; 
    }

    // 2. PROBE TT
    int ttScore;
    Move ttMove(0,0); // Move suggested by the TT
    if (probeTT(hash, depth, alpha, beta, ttScore, ttMove)) {
        return ttScore; // Immediate cutoff!
    }

    if (depth == 0) return quiescence(board, alpha, beta, colorMultiplier);

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateLegalMoves(turn);

    if (moves.empty()) {
        if (board.isInCheck(turn)) return -MATE_VALUE + depth; 
        return 0; 
    }

    // 3. IMPROVED MOVE SORTING (Hash Move)
    // If the TT gave us a "Best move" from a previous search,
    // we must absolutely test it FIRST. That's what speeds everything up.
    auto moveSorter = [&](const Move& a, const Move& b) {
        // The TT move has absolute priority
        if (ttMove.from != 0) { // If valid move
            if (a.from == ttMove.from && a.to == ttMove.to) return true; 
            if (b.from == ttMove.from && b.to == ttMove.to) return false;
        }
        if (a.isCapture != b.isCapture) return a.isCapture;
        if (a.promotion != PieceType::None) return true;
        return false;
    };
    std::sort(moves.begin(), moves.end(), moveSorter);

    int maxScore = -INF;
    Move bestMoveFound(0,0);

    for (const auto& move : moves) {
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);
        
        int score = -negamax(nextBoard, depth - 1, -beta, -alpha, -colorMultiplier);
        
        if (score > maxScore) {
            maxScore = score;
            bestMoveFound = move;
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; 
    }

    // 4. STORE TT
    storeTT(hash, maxScore, depth, alphaOrig, beta, bestMoveFound);

    return maxScore;
}

// ==========================================
// 5. ROOT SEARCH
// ==========================================
Move AI::getBestMove(const Board& board, Color turn) {
    
    std::vector<Move> moves = board.generateLegalMoves(turn);
    if (moves.empty()) return Move(0, 0);

    // initial sort with TT move and captures first (if available)
    uint64_t hash = board.getHash();
    if (turn == Color::Black) hash = ~hash;
    
    int ttScore; Move ttMove(0,0);
    probeTT(hash, searchDepth, -INF, INF, ttScore, ttMove);

    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        if (ttMove.from != 0) {
            if (a.from == ttMove.from && a.to == ttMove.to) return true; 
            if (b.from == ttMove.from && b.to == ttMove.to) return false;
        }
        return a.isCapture > b.isCapture;
    });

    struct MoveResult { int score; Move move; };
    std::vector<std::future<MoveResult>> futures;
    int colorMultiplier = (turn == Color::White) ? 1 : -1;

    for (const auto& move : moves) {
        futures.push_back(std::async(std::launch::async, [=, &board]() -> MoveResult {
            Board threadBoard = board; 
            threadBoard.movePiece(move.from, move.to, move.promotion);
            
            // Negamax
            int score = -negamax(threadBoard, this->searchDepth - 1, -INF, INF, -colorMultiplier);
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
    
    // We also save the root result in the TT for future use
    storeTT(hash, maxScore, searchDepth, -INF, INF, bestMove);
    
    return bestMove;
}
// ==========================================
// 6. QUIESCENCE SEARCH (From Dev)
// ==========================================
int AI::quiescence(const Board& board, int alpha, int beta, int colorMultiplier) {
    // Stand Pat (Evaluation statique via notre fonction compatible Fairy)
    int stand_pat = colorMultiplier * (*evaluate)(board);

    if (stand_pat >= beta) return beta;
    const int DELTA = 975;
    if (stand_pat < alpha - DELTA) return alpha;
    if (stand_pat > alpha) alpha = stand_pat;

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateCaptures(turn); 

    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
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