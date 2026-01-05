#include "ai.h"
#include <algorithm>
#include <vector>
#include <iostream>

// ==========================================
// 1. PIECE-SQUARE TABLES (Position Tables)
// ==========================================
// (On garde les tables classiques pour l'instant et on les réutilisera pour les pièces féeriques)

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

// --- MODIFICATION 1 : AJOUT DES VALEURS FÉERIQUES ---
// Indices : 0:Pawn, 1:Knight, 2:Bishop, 3:Rook, 4:Queen, 5:King,
//           6:Princess, 7:Empress, 8:Nightrider, 9:Grasshopper
const int pieceValues[] = {
    100, 320, 330, 500, 900, 20000,
    650, // Princess (Bishop+Knight)
    850, // Empress (Rook+Knight)
    400, // Nightrider (Strong Knight)
    300  // Grasshopper (Roughly Knight)
};

// ==========================================
// 2. EVALUATION FUNCTION
// ==========================================
int AI::evaluate(const Board& board) {
    int score = 0;
    for (int sq = 0; sq < 64; ++sq) {
        Color c;
        PieceType pt = board.getPieceTypeAt(sq, c);
        if (pt == PieceType::None) continue;

        // Récupération de la valeur matérielle
        int val = pieceValues[static_cast<int>(pt)];

        // Calcul du bonus de position
        int tableScore = 0;
        int tableIdx = (c == Color::White) ? sq : (sq ^ 56); // Miroir vertical pour les noirs

        switch (pt) {
        case PieceType::Pawn:        tableScore = pawnTable[tableIdx]; break;
        case PieceType::Knight:      tableScore = knightTable[tableIdx]; break;
        case PieceType::Bishop:      tableScore = bishopTable[tableIdx]; break;
        case PieceType::Rook:        tableScore = rookTable[tableIdx]; break;

        // --- MODIFICATION 2 : LOGIQUE POUR PIÈCES FÉERIQUES ---
        // On approxime avec les tables existantes
        case PieceType::Princess:    tableScore = bishopTable[tableIdx]; break; // Aime les diagonales
        case PieceType::Empress:     tableScore = rookTable[tableIdx]; break;   // Aime les colonnes ouvertes
        case PieceType::Nightrider:  tableScore = knightTable[tableIdx]; break; // Aime le centre
        case PieceType::Grasshopper: tableScore = knightTable[tableIdx]; break; // Aime le centre

        // Queen et King utilisent généralement des tables spécifiques,
            // ici on laisse 0 ou on pourrait ajouter une table simple.
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
        return 0; // Pat
    }

    // Tri simple pour optimiser l'élagage Alpha-Beta (les captures d'abord)
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

    // Tri des coups à la racine également
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
        return a.isCapture > b.isCapture;
    });

    Move bestMove = moves[0];
    int maxScore = -INF;
    int colorMultiplier = (turn == Color::White) ? 1 : -1;

    for (const auto& move : moves) {
        Board nextBoard = board;
        nextBoard.movePiece(move.from, move.to, move.promotion);

        int score = -negamax(nextBoard, depth - 1, -INF, INF, -colorMultiplier);

        // --- CORRECTION : SUPPRESSION DU DEBUG ---
        // Les lignes suivantes doivent être commentées ou supprimées
        // pour ne pas perturber l'interface Python.
        // std::cout << "Move: " << move.from << "->" << move.to << " Score: " << score << "\n";
        // -----------------------------------------

        if (score > maxScore) {
            maxScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}
