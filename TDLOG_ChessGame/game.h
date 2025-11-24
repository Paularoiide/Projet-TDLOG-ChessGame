#pragma once
#include <vector>
#include "board.h"
#include "piece.h" // Pour l'enum Color
#include "move.h"

class Game {
    Board board_;
    Color currentTurn_;

public:
    // Constructeur simple
    Game();

    // Lance ou relance la partie
    void startGame();

    // Tente de jouer un coup. Retourne true si le coup était valide et joué.
    bool playMove(const Move& move);

    // Accesseurs
    const Board& board() const { return board_; }
    Color currentTurn() const { return currentTurn_; }
};