#include "variant.h"
#include "board.h"
#include "piece.h"


void ClassicVariant::initialSetup(Board& board) const {
    // Pions
    for (int x = 0; x < board.size(); ++x) {
        board.setPiece(Position{x, 1}, new Pawn(Position{x, 1}, Color::Black));
        board.setPiece(Position{x, board.size()-2}, new Pawn(Position{x, board.size()-2}, Color::White));
    }
    // Tours
    board.setPiece({0,0}, new Rook({0,0}, Color::Black));
    board.setPiece({7,0}, new Rook({7,0}, Color::Black));
    board.setPiece({0,7}, new Rook({0,7}, Color::White));
    board.setPiece({7,7}, new Rook({7,7}, Color::White));
    // Cavaliers
    board.setPiece({1,0}, new Knight({1,0}, Color::Black));
    board.setPiece({6,0}, new Knight({6,0}, Color::Black));
    board.setPiece({1,7}, new Knight({1,7}, Color::White));
    board.setPiece({6,7}, new Knight({6,7}, Color::White));
    // Fous
    board.setPiece({2,0}, new Bishop({2,0}, Color::Black));
    board.setPiece({5,0}, new Bishop({5,0}, Color::Black));
    board.setPiece({2,7}, new Bishop({2,7}, Color::White));
    board.setPiece({5,7}, new Bishop({5,7}, Color::White));
    // Dames
    board.setPiece({3,0}, new Queen({3,0}, Color::Black));
    board.setPiece({3,7}, new Queen({3,7}, Color::White));
    // Rois
    board.setPiece({4,0}, new King({4,0}, Color::Black));
    board.setPiece({4,7}, new King({4,7}, Color::White));
}


bool ClassicVariant::isLegalMove(const Board& board, const Move& move, Color sideToMove) const {
    // Version simplifiée: on fait confiance à getLegalMoves de la pièce.
    // (A raffiner plus tard: vérifier que le roi n'est pas en échec après le coup.)
    (void)board; (void)move; (void)sideToMove; // silence unused warnings pour l'instant
    return true;
}
