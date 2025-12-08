#include "game.h"
#include <iostream>

using namespace std;

Game::Game() : board_(), currentTurn_(Color::White) {
}


static int evaluate(const Board& board, Color root) {
    // Valeur des pièces
    const int values[6] = {
        100,  // Pawn
        320,  // Knight
        330,  // Bishop
        500,  // Rook
        900,  // Queen
        20000 // King
    };

    int scoreWhite = 0;
    int scoreBlack = 0;

    for (int pt = 0; pt < 6; ++pt) {
        PieceType piece = static_cast<PieceType>(pt);
        // Comptage des pièces
        Bitboard wbb = board.getBitboard(Color::White, piece);
        Bitboard bbb = board.getBitboard(Color::Black, piece);

        int wCount = __builtin_popcountll(wbb);
        int bCount = __builtin_popcountll(bbb);

        scoreWhite += wCount * values[pt];
        scoreBlack += bCount * values[pt];
    }

    int diff = scoreWhite - scoreBlack;
    return (root == Color::White ? diff : -diff);
}

static int search(Board board,
                  Color toMove,
                  Color root,
                  int depth,
                  int alpha,
                  int beta
                  ){
    std::vector<Move> LegalMoves = board.generateLegalMoves(toMove);

    if (LegalMoves.empty()) {
        bool inCheck = board.isInCheck(toMove);
        if (inCheck) {
            int mateScore = 100000;
            return (toMove == root ? -mateScore : mateScore);
        }
        return 0;
    }

    if (depth == 0)
        return evaluate(board, root);

    bool maximizing = (toMove == root);
    int bestScore = maximizing ? -1000000 : 1000000;

    for (const Move& m : LegalMoves) {

        Board child = board;
        child.movePiece(m.from, m.to, m.promotion);   // FIX !!!

        int score = search(child, opposite(toMove), root, depth - 1, alpha, beta);

        if (maximizing) {
            if (score > bestScore) bestScore = score;
            if (score > alpha) alpha = score;
        } else {
            if (score < bestScore) bestScore = score;
            if (score < beta) beta = score;
        }

        if (beta <= alpha) break;
    }

    return bestScore;
}



void Game::startGame() {
    currentTurn_ = Color::White;
}

bool Game::playMove(const Move& moveReq) {
    // 1. Generate legal moves
    std::vector<Move> legalMoves = board_.generateLegalMoves(currentTurn_);

    // 2. Validate the move
    bool found = false;
    for (const auto& m : legalMoves) {
        // IMPORTANT 1 : On doit vérifier que la promotion correspond aussi !
        // (Sinon le jeu ne sait pas si vous voulez une Dame ou un Cavalier)
        if (m.from == moveReq.from && m.to == moveReq.to && m.promotion == moveReq.promotion) {
            found = true;
            break;
        }
    }
    
    if (!found) return false;

    // 3. Play the move
    // IMPORTANT 2 : Il faut PASSER l'argument promotion à movePiece !
    board_.movePiece(moveReq.from, moveReq.to, moveReq.promotion);
    
    currentTurn_ = opposite(currentTurn_);

    // 4. Update state (Checkmate/Stalemate logic)
    std::vector<Move> nextMoves = board_.generateLegalMoves(currentTurn_);
    bool inCheck = board_.isInCheck(currentTurn_);

    if (nextMoves.empty()) {
        if (inCheck) {
            state_ = GameState::Checkmate; 
        } else {
            state_ = GameState::Stalemate; 
        }
    } else {
        if (inCheck) {
            state_ = GameState::Check;
        } else {
            state_ = GameState::Playing;
        }
    }

    return true;
}

Move Game::findBestMove(int depth) const {
    Color side = currentTurn_;
    Color root = side;

    std::vector<Move> moves = board_.generateLegalMoves(side);
    if (moves.empty()) {
        // No legal moves: return a dummy move
        return Move(0, 0);
    }

    int bestScore = -1000000;
    Move bestMove = moves[0];

    int alpha = -1000000;
    int beta  =  1000000;

    for (const Move& m : moves) {
        Board child = board_;
        child.movePiece(m.from, m.to, m.promotion);  // FIX !!!

        int score = search(child, opposite(side), root, depth - 1, alpha, beta);

        if (score > bestScore) {
            bestScore = score;
            bestMove = m;
        }
        if (score > alpha) alpha = score;
    }


    return bestMove;
}
