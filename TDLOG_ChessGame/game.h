#pragma once
#include <vector>
#include "board.h"
#include "piece.h" // For the Color enum
#include "move.h"
enum class GameState { Playing, Check, Checkmate, Stalemate, Prom };
class Game {
    Board board_;
    Color currentTurn_;
    GameState state_{GameState::Playing}; // New state
    int promPos; // Position of the pawn being promoted

public:
    Game();
    void startGame(Variant v = Variant::Classic);
    bool playMove(const Move& move);
    bool prom(int pt);
    const Board& board() const { return board_; }
    Color currentTurn() const { return currentTurn_; }
    GameState gameState() const { return state_; } // Getter
};
