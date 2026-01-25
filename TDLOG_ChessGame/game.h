#pragma once
#include <vector>
#include "board.h"
#include "piece.h" // Pour l'enum Color
#include "move.h"
enum class GameState { Playing, Check, Checkmate, Stalemate };
class Game {
    Board board_;
    Color currentTurn_;
    GameState state_{GameState::Playing}; // Nouvel état

public:
    Game();
    void startGame(Variant v = Variant::Classic);
    bool playMove(const Move& move);
    const Board& board() const { return board_; }
    Color currentTurn() const { return currentTurn_; }
    GameState gameState() const { return state_; } // Getter
};
