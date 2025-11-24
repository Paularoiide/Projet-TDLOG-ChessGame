#pragma once
#include <memory>
#include <vector>
#include "rules.h"
#include "board.h"
#include "variant.h"
#include "move.h"


class Player;


class Game {
    Board board_;
    std::unique_ptr<Variant> variant_;
    Color currentTurn_{Color::White};
public:
    explicit Game(std::unique_ptr<Variant> variant)
        : board_(variant ? variant->boardSize() : 8), variant_(std::move(variant)) {}


    void startGame();
    bool playMove(const Move& move); // renvoie true si joué


    const Board& board() const { return board_; }
    Board& board() { return board_; }
    Color currentTurn() const { return currentTurn_; }
};
