#include "game.h"
#include <iostream>

using namespace std;

Game::Game() : board_(), currentTurn_(Color::White) {
}



void Game::startGame() {
    currentTurn_ = Color::White;
}

bool Game::playMove(const Move& moveReq) {
    // 1. Generate legal moves
    std::vector<Move> legalMoves = board_.generateLegalMoves(currentTurn_);

    // 2. Validate the move
    bool found = false;
    for (const auto& m : legalMoves) {
        if (m.from == moveReq.from && m.to == moveReq.to && m.promotion == moveReq.promotion) {
            found = true;
            break;
        }
    }
    
    if (!found) return false;

    // 3. Play the move
    board_.movePiece(moveReq.from, moveReq.to, moveReq.promotion);
    
    currentTurn_ = opposite(currentTurn_);

    // 4. Update state (Checkmate/Stalemate logic)
    std::vector<Move> nextMoves = board_.generateLegalMoves(currentTurn_);
    bool inCheck = board_.isInCheck(currentTurn_);

    if (nextMoves.empty()) {
        if (inCheck) {
            state_ = GameState::Checkmate; 
        } else {
            state_ = GameState::Stalemate; 
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
