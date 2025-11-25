#include "game.h"
#include <iostream>

Game::Game() : board_(), currentTurn_(Color::White) {
    // The Board constructor already initializes the default starting position
}

void Game::startGame() {
    // If you had a board_.reset() method, it would be called here.
    // For now, we simply ensure that White always starts.
    currentTurn_ = Color::White;
}

bool Game::playMove(const Move& moveReq) {
    // 1. Generate legal moves
    std::vector<Move> legalMoves = board_.generateLegalMoves(currentTurn_);

    // 2. Validate the move
    bool found = false;
    for (const auto& m : legalMoves) {
        if (m.from == moveReq.from && m.to == moveReq.to) {
            found = true;
            break;
        }
    }
    if (!found) return false;

    // 3. Play the move
    board_.movePiece(moveReq.from, moveReq.to);
    currentTurn_ = opposite(currentTurn_);

    // 4. Update the game state for the NEW player to move
    std::vector<Move> nextMoves = board_.generateLegalMoves(currentTurn_);
    bool inCheck = board_.isInCheck(currentTurn_);

    if (nextMoves.empty()) {
        if (inCheck) {
            state_ = GameState::Checkmate; // No moves + Check = Checkmate
        } else {
            state_ = GameState::Stalemate; // No moves + No check = Stalemate
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
