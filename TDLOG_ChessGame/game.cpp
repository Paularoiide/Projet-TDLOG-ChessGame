#include "game.h"
#include <iostream>

Game::Game() : board_(), currentTurn_(Color::White) {
    // Le constructeur de Board initialise déjà les pièces par défaut
}

void Game::startGame() {
    // Si vous aviez une méthode board_.reset(), ce serait ici.
    // Pour l'instant, on s'assure juste que les Blancs commencent.
    currentTurn_ = Color::White;
}

bool Game::playMove(const Move& moveReq) {
    // 1. Générer TOUS les coups légaux pour la couleur qui doit jouer
    std::vector<Move> legalMoves = board_.generateLegalMoves(currentTurn_);

    // 2. Chercher si le coup demandé (moveReq) existe dans la liste des coups légaux
    bool isLegal = false;
    for (const auto& m : legalMoves) {
        // On compare uniquement l'origine et la destination
        // (Le joueur ne précise pas "isCapture", c'est le moteur qui le sait)
        if (m.from == moveReq.from && m.to == moveReq.to) {
            isLegal = true;
            break;
        }
    }

    // 3. Si le coup n'est pas trouvé, c'est un coup illégal
    if (!isLegal) {
        return false;
    }

    // 4. Si c'est bon, on applique le coup sur le plateau
    board_.movePiece(moveReq.from, moveReq.to);

    // 5. On change le tour
    // (Utilisation de la fonction opposite définie dans piece.h)
    currentTurn_ = opposite(currentTurn_);

    return true;
}