#pragma once
#include <memory>
#include <vector>
#include "board.h"

#include "move.h"

class Game {
    Board board_;
    Color currentTurn_{Color::White};
    
public:
    explicit Game(void* variant = nullptr) : board_() {} 

    void startGame();
    bool playMove(const Move& move); 

    const Board& board() const { return board_; }
    Board& board() { return board_; }
    Color currentTurn() const { return currentTurn_; }
};