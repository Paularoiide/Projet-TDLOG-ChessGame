#include "game.h"
#include "piece.h"

void Game::startGame() {
    if (variant_) variant_->initialSetup(board_);
    currentTurn_ = Color::White;
}


bool Game::playMove(const Move& move) {
    // Sanity checks
    if (!board_.isInside(move.from) || !board_.isInside(move.to)) return false;
    Piece* p = board_.getPiece(move.from);
    if (!p) return false;
    if (p->getColor() != currentTurn_) return false;


    // Générer les coups légaux de la pièce et vérifier que "move" en fait partie
    bool found = false;
    for (const auto& m : p->getLegalMoves(board_)) {
        if (m.to == move.to) { found = true; break; }
    }
    if (!found) return false;


    if (variant_ && !variant_->isLegalMove(board_, move, currentTurn_)) return false;


    // Appliquer le coup
    board_.movePiece(move.from, move.to);
    p->setPosition(move.to);
    // TODO: gérer capture/mémoire, promotion, roque, en-passant


    currentTurn_ = opposite(currentTurn_);
    return true;
}
