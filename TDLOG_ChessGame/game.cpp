#include "game.h"
#include <iostream>

using namespace std;

Game::Game() : board_(), currentTurn_(Color::White) {
}



void Game::startGame() {
    currentTurn_ = Color::White;
    promPos = -1;
}

void Game::endTurn() {
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
    if (board_.movePiece(moveReq.from, moveReq.to, moveReq.promotion)){
        state_ = GameState::Prom;
        promPos = moveReq.to;
    }
    else{
        endTurn();
    }

    return true;
}

bool Game::prom(int choice){
    if ((0<=choice)&&(choice<=3)){
        PieceType pt;
        switch(choice){
            case 0: pt=PieceType::Knight;
            case 1: pt=PieceType::Bishop;
            case 2: pt=PieceType::Rook;
            case 3: pt=PieceType::Queen;
            default: pt=PieceType::None;
        }
        if (pt!=PieceType::None && promPos!=(-1) && board_.doProm(promPos, pt)){
            promPos = -1;
            state_ = GameState::Playing; //probablement inutil
            endTurn();
            return true;
        }
    }
    return false;
}