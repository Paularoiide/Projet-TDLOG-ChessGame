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
    // 1. Générer coups légaux
    std::vector<Move> legalMoves = board_.generateLegalMoves(currentTurn_);

    // 2. Valider le coup
    bool found = false;
    for (const auto& m : legalMoves) {
        if (m.from == moveReq.from && m.to == moveReq.to) {
            found = true;
            break;
        }
    }
    if (!found) return false;

    // 3. Jouer le coup
    board_.movePiece(moveReq.from, moveReq.to);
    currentTurn_ = opposite(currentTurn_);

    // 4. Mettre à jour l'état pour le NOUVEAU joueur
    std::vector<Move> nextMoves = board_.generateLegalMoves(currentTurn_);
    bool inCheck = board_.isInCheck(currentTurn_);

    if (nextMoves.empty()) {
        if (inCheck) {
            state_ = GameState::Checkmate; // Plus de coups + Échec = MAT
        } else {
            state_ = GameState::Stalemate; // Plus de coups + Pas échec = PAT
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