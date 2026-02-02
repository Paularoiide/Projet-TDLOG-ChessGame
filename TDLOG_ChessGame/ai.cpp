#include "ai.h"
#include "game.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <future>
#include <random>

// 1) POSITION TABLES (Piece-Square Tables)

// Bonus/malus according to the occupied square. Index 0..63.

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
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
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

// Material values (extended array for “fairy” pieces)
const int pieceValues[] = {
    100, 320, 330, 500, 900, 20000,
    650, 850, 400, 300
};


// 2) IMPLEMENTATION OF THE "PLAYER" INTERFACE

Move AI::getMove(Game& g) {
    return getBestMove(g.board(), g.currentTurn());
}


// 3) EVALUATION (compatible fairy pieces)


// Returns the index of the least significant set bit (non-zero bitboard)
static inline int getLSB(uint64_t bb) {
#ifdef _MSC_VER
    unsigned long index;
    _BitScanForward64(&index, bb);
    return (int)index;
#else
    return __builtin_ctzll(bb);
#endif
}

int MaterialAndPositionEvaluation::operator()(const Board& board) const {
    int score = 0;

    // We loop up to 10 to include fairy pieces
    for (int p = 0; p < 10; ++p) {
        PieceType pt = static_cast<PieceType>(p);
        int val = pieceValues[p];

        // --- White ---
        Bitboard bbWhite = board.getBitboard(Color::White, pt);
        while (bbWhite) {
            int sq = getLSB(bbWhite);
            int posVal = 0;

            // Position bonus according to the piece type
            switch(pt) {
            case PieceType::Pawn:        posVal = pawnTable[sq]; break;
            case PieceType::Knight:      posVal = knightTable[sq]; break;
            case PieceType::Bishop:      posVal = bishopTable[sq]; break;
            case PieceType::Rook:        posVal = rookTable[sq]; break;
            case PieceType::Princess:    posVal = bishopTable[sq]; break;
            case PieceType::Empress:     posVal = rookTable[sq]; break;
            case PieceType::Nightrider:  posVal = knightTable[sq]; break;
            case PieceType::Grasshopper: posVal = knightTable[sq]; break;
            default: break;
            }

            score += (val + posVal);
            // remove the processed bit
            bbWhite &= (bbWhite - 1);
        }

        // --- Black ---
        Bitboard bbBlack = board.getBitboard(Color::Black, pt);
        while (bbBlack) {
            int sq = getLSB(bbBlack);
            // vertical mirror to use the same tables
            int tableIdx = sq ^ 56;
            int posVal = 0;

            switch(pt) {
            case PieceType::Pawn:        posVal = pawnTable[tableIdx]; break;
            case PieceType::Knight:      posVal = knightTable[tableIdx]; break;
            case PieceType::Bishop:      posVal = bishopTable[tableIdx]; break;
            case PieceType::Rook:        posVal = rookTable[tableIdx]; break;
            case PieceType::Princess:    posVal = bishopTable[tableIdx]; break;
            case PieceType::Empress:     posVal = rookTable[tableIdx]; break;
            case PieceType::Nightrider:  posVal = knightTable[tableIdx]; break;
            case PieceType::Grasshopper: posVal = knightTable[tableIdx]; break;
            default: break;
            }

            score -= (val + posVal);
            bbBlack &= (bbBlack - 1);
        }
    }

    return score;
}


// TOOLS: TRANSPOSITION TABLE (TT)


void AI::storeTT(uint64_t key, int score, int depth, int alpha, int beta, Move bestMove) {
    // We protect the TT if we do multi-threaded search
    std::lock_guard<std::mutex> lock(ttMutex);

    // Simple index in the table
    size_t index = key % ttSize;

    // Determines if the score is exact or a bound (fail-low / fail-high)
    TTFlag flag = TTFlag::EXACT;
    if (score <= alpha)      flag = TTFlag::ALPHA; // high bound
    else if (score >= beta)  flag = TTFlag::BETA;  // low bound

    transpositionTable[index] = { key, score, depth, bestMove, flag };
}

bool AI::probeTT(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove) {
    // We protect the TT if we do multi-threaded search
    std::lock_guard<std::mutex> lock(ttMutex);

    size_t index = key % ttSize;
    const TTEntry& entry = transpositionTable[index];

    // Check the key (otherwise collision)
    if (entry.key == key) {
        // If we have a better move stored, we retrieve it
        if (entry.bestMove.from != -1) bestMove = entry.bestMove;

        // We only use the result if the stored depth is sufficient
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


// 4) NEGAMAX + ALPHA-BETA + QUIESCENCE

int AI::negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier) {
    int alphaOrig = alpha;

    // 1) Hash of the position
    uint64_t hash = board.getHash();
    if (colorMultiplier == -1) {
        // Simple trick to distinguish sides if necessary
        hash = ~hash;
    }

    // 2) Attempt in the transposition table
    int ttScore;
    Move ttMove(0,0);
    if (probeTT(hash, depth, alpha, beta, ttScore, ttMove)) {
        return ttScore;
    }

    // Stop: switch to quiescence to avoid the horizon effect
    if (depth == 0) return quiescence(board, alpha, beta, colorMultiplier);

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateLegalMoves(turn);

    // No moves: checkmate or stalemate
    if (moves.empty()) {
        if (board.isInCheck(turn)) return -MATE_VALUE - depth; // prefers quick mates
        return 0;
    }

    // 3) Sorting moves: we try the TT move first, then captures/promotions
    auto moveSorter = [&](const Move& a, const Move& b) {
        if (ttMove.from != 0) {
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
        if (alpha >= beta) break; // alpha-beta cutoff
    }

    // 4) Saving in the TT
    storeTT(hash, maxScore, depth, alphaOrig, beta, bestMoveFound);

    return maxScore;
}


// 5) Search from roots (Lazy SMP)

Move AI::getBestMove(const Board& board, Color turn) {
   //1. Configuration
   int colorMultiplier = (turn == Color::White) ? 1 : -1;
   uint64_t rootHash = board.getHash();
   if (turn  == Color::Black) {rootHash = ~rootHash;}
   // Limitation on the number of threads
   int numThreads = std::thread::hardware_concurrency();
   if (numThreads < 1) numThreads = 1;

   // vector to store tasks
   std::vector<std::future<void>> futures;
   //2. DDefinition of a thread's task
   auto searchWorker = [&](int threadID) {
    // Each thread has its own copy of the board
    Board threadBoard = board;
    // Iterative deepening
    // Allows the thread to perform a partial search to fill the TT
    // which allows other threads to prune more effectively
    for (int depth = 1; depth <= searchDepth; ++depth) {
        negamax(threadBoard, depth, -INF, INF, colorMultiplier);
    }
    };
   //3. Launching secondary threads
   // We launch (N-1) threads, the main thread also performs a search
   for(int i = 1; i < numThreads;i++){
        futures.push_back(std::async(std::launch::async, searchWorker, i));
   }
   //4. Main search in the main thread
    searchWorker(0);
   //5 Synchronization of threads
   for(auto& f : futures){
        f.get();
   }
   //6. Retrieving move scores from the TT
   std::vector<Move> moves = board.generateLegalMoves(turn);
   if (moves.empty()) {
       return Move(0,0); // No legal moves
   }
   struct ScoredMove {
         Move move;
         int score;
   };
   std::vector<std::future<ScoredMove>> scoreFutures;
   for (const auto& move : moves) {
    scoreFutures.push_back(std::async(std::launch::async, [=, &board]() -> ScoredMove {
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);
        int score = -negamax(nextBoard, searchDepth -1, -INF, INF, -colorMultiplier);
        return {move, score};
    }));
    }
    std::vector<ScoredMove> scoredMoves;
    for (auto& sf : scoreFutures) {
        scoredMoves.push_back(sf.get());
    }
   //7. Sort moves by descending score
   std::sort(scoredMoves.begin(), scoredMoves.end(), [](const ScoredMove& a, const ScoredMove& b)
    {
          return a.score > b.score;
    });
    //8. Random choice among the best moves
    if (scoredMoves.size() > 1) {
        int bestScore = scoredMoves[0].score;
        int secondScore = scoredMoves[1].score;
        // Security: if the first move is a mate, we choose it directly
        // or if the second is significantly lower
        bool bestIsMate = (bestScore >= 48000);
        bool hugeGap = (bestScore - secondScore) > 200;
        if(!bestIsMate && !hugeGap){
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 1);
            if (dis(gen) == 1) {
                return scoredMoves[1].move;
            }
        }

    }
    return scoredMoves[0].move;
}




// ==========================================
// 6. QUIESCENCE SEARCH (From Dev)
// ==========================================
int AI::quiescence(const Board& board, int alpha, int beta, int colorMultiplier) {
    // 1. Stand Pat
    int stand_pat = colorMultiplier * (*evaluate)(board);

    if (stand_pat >= beta) return beta;

    const int DELTA = 975; // security margin
    if (stand_pat < alpha - DELTA) {
        return alpha;
    }

    if (stand_pat > alpha) alpha = stand_pat;

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    // We generate only capture moves for quiescence search
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
