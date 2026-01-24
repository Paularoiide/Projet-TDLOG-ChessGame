#include "ai.h"
#include "game.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <future>


// 1) TABLES DE POSITION (Piece-Square Tables)

// Bonus/malus suivant la case occupée. Index 0..63.

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

// Valeurs matérielles (tableau étendu pour les pièces “féeriques”)
const int pieceValues[] = {
    100, 320, 330, 500, 900, 20000,
    650, 850, 400, 300
};


// 2) IMPLÉMENTATION DE L'INTERFACE "PLAYER"

Move AI::getMove(Game& g) {
    return getBestMove(g.board(), g.currentTurn());
}


// 3) ÉVALUATION (compatible pièces féeriques)


// Renvoie l'indice du bit à 1 le plus à droite (bitboard non nul)
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

    // On boucle jusqu'à 10 pour inclure les pièces féeriques
    for (int p = 0; p < 10; ++p) {
        PieceType pt = static_cast<PieceType>(p);
        int val = pieceValues[p];

        // --- BLANCS ---
        Bitboard bbWhite = board.getBitboard(Color::White, pt);
        while (bbWhite) {
            int sq = getLSB(bbWhite);
            int posVal = 0;

            // Bonus de position selon le type de pièce
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
            // supprime le bit traité
            bbWhite &= (bbWhite - 1);
        }

        // --- NOIRS ---
        Bitboard bbBlack = board.getBitboard(Color::Black, pt);
        while (bbBlack) {
            int sq = getLSB(bbBlack);
            // miroir vertical pour utiliser les mêmes tables
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


// OUTILS : TABLE DE TRANSPOSITION (TT)


void AI::storeTT(uint64_t key, int score, int depth, int alpha, int beta, Move bestMove) {
    // On protège la TT si on fait de la recherche multi-thread
    std::lock_guard<std::mutex> lock(ttMutex);

    // Index simple dans la table
    size_t index = key % ttSize;

    // Détermine si le score est exact ou une borne (fail-low / fail-high)
    TTFlag flag = TTFlag::EXACT;
    if (score <= alpha)      flag = TTFlag::ALPHA; // borne haute
    else if (score >= beta)  flag = TTFlag::BETA;  // borne basse

    transpositionTable[index] = { key, score, depth, bestMove, flag };
}

bool AI::probeTT(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove) {
    // On protège la TT si on fait de la recherche multi-thread
    std::lock_guard<std::mutex> lock(ttMutex);

    size_t index = key % ttSize;
    const TTEntry& entry = transpositionTable[index];

    // Vérifie la clé (sinon collision)
    if (entry.key == key) {
        // Si on a un meilleur coup stocké, on le récupère
        if (entry.bestMove.from != -1) bestMove = entry.bestMove;

        // On n'utilise le résultat que si la profondeur stockée est suffisante
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


// 4) NEGAMAX + ALPHA-BÊTA + QUIESCENCE

int AI::negamax(const Board& board, int depth, int alpha, int beta, int colorMultiplier) {
    int alphaOrig = alpha;

    // 1) Hash de la position
    uint64_t hash = board.getHash();
    if (colorMultiplier == -1) {
        // Astuce simple pour distinguer les camps si nécessaire
        hash = ~hash;
    }

    // 2) Tentative dans la table de transposition
    int ttScore;
    Move ttMove(0,0);
    if (probeTT(hash, depth, alpha, beta, ttScore, ttMove)) {
        return ttScore;
    }

    // Arrêt : on passe en quiescence pour éviter l'effet horizon
    if (depth == 0) return quiescence(board, alpha, beta, colorMultiplier);

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateLegalMoves(turn);

    // Pas de coups : mat ou pat
    if (moves.empty()) {
        if (board.isInCheck(turn)) return -MATE_VALUE - depth; // préfère les mats rapides
        return 0;
    }

    // 3) Tri des coups : on essaie d'abord le coup TT, puis captures/promotions
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
        if (alpha >= beta) break; // coupure alpha-bêta
    }

    // 4) Sauvegarde dans la TT
    storeTT(hash, maxScore, depth, alphaOrig, beta, bestMoveFound);

    return maxScore;
}


// 5) RECHERCHE À LA RACINE (LAZY SMP)

Move AI::getBestMove(const Board& board, Color turn) {
   //1. Configuration
   int colorMultiplier = (turn == Color::White) ? 1 : -1;
   uint64_t rootHash = board.getHash();
   if (turn  == Color::Black) {rootHash = ~rootHash;}
   // Limitation sur le nombre de threads
   int numThreads = std::thread::hardware_concurrency();
   if (numThreads < 1) numThreads = 1;

   // vecteur pour stocker les taches
   std::vector<std::future<void>> futures;
   //2. Définition de la tache d'un thread
   auto searchWorker = [&](int threadID) {
    // Chaque thread a sa propre copie de la board
    Board threadBoard = board;
    // Iterative deepening
    // Permet au thread de faire une recherche partielle pour remplir la TT
    // ce qui permet aux autres threads d'elager plus efficacement
    for (int depth = 1; depth <= searchDepth; ++depth) {
        negamax(threadBoard, depth, -INF, INF, colorMultiplier);
    }
    };
   //3. Lancement des threads secondaires
   // On lance (N-1) threads, le main thread fait aussi une recherche
   for(int i = 1; i < numThreads;i++){
        futures.push_back(std::async(std::launch::async, searchWorker, i));
   }
   //4. Recherche principale dans le thread principal
    searchWorker(0);
   //5 Synchronisation des threads
   for(auto& f : futures){
        f.get();
   }
   //6. Récupération du meilleur coup depuis la TT
   int BestScore;
   Move BestMove(0,0);
   if (probeTT(rootHash, this -> searchDepth, -INF, INF, BestScore, BestMove)){
        return BestMove;
    }
    // Si pas trouvé, on retourne un coup aléatoire (devrait pas arriver)
    std::vector<Move> legalMoves = board.generateLegalMoves(turn);
    if (!legalMoves.empty()) {
        return legalMoves[0];
    }
}


// 6) QUIESCENCE (recherche sur les captures)

int AI::quiescence(const Board& board, int alpha, int beta, int colorMultiplier) {
    // Évaluation statique
    int stand_pat = colorMultiplier * (*evaluate)(board);

    if (stand_pat >= beta) return beta;

    // Delta pruning : si trop bas, on ne creuse pas
    const int DELTA = 975;
    if (stand_pat < alpha - DELTA) return alpha;

    if (stand_pat > alpha) alpha = stand_pat;

    Color turn = (colorMultiplier == 1) ? Color::White : Color::Black;
    std::vector<Move> moves = board.generateCaptures(turn);

    // Promotions avant le reste
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        if (a.promotion != PieceType::None && b.promotion == PieceType::None) return true;
        if (a.promotion == PieceType::None && b.promotion != PieceType::None) return false;
        return false;
    });

    for (const auto& move : moves) {
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);

        // On évite les captures illégales (ex : on se met en échec)
        if (nextBoard.isInCheck(turn)) {
            continue;
        }

        int score = -quiescence(nextBoard, -beta, -alpha, -colorMultiplier);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}
