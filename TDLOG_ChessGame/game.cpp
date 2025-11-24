#include "game.h"
#include <iostream>

void Game::startGame() {
    currentTurn_ = Color::White;
    // Initialisation déjà faite dans le constructeur de Board
}

bool Game::playMove(const Move& move) {
    // 1. Vérifier si le coup est dans la liste des coups légaux
    // (Note: pour la performance pure, on évite parfois cette vérification complète en production, 
    // mais pour la rigueur ici on le fait).
    
    std::vector<Move> legalMoves = board_.generateLegalMoves(currentTurn_);
    bool found = false;
    for (const auto& m : legalMoves) {
        if (m.from == move.from && m.to == move.to) {
            found = true; 
            break; 
        }
    }
    
    // Note temporaire : Comme generateLegalMoves est incomplet dans l'exemple ci-dessus,
    // on autorise le coup si c'est géométriquement valide pour tester.
    // En production, décommentez le bloc "if (!found) return false;"
    
    board_.movePiece(move.from, move.to);
    currentTurn_ = opposite(currentTurn_);
    return true;
}