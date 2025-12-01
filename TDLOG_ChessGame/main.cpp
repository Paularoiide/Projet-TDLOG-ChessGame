#include <iostream>
#include <string>
#include <vector>
#include <sstream> 
#include <cctype>  // pour std::tolower
#include "game.h"
#include "piece.h" 
#include "board.h"
#include "move.h"
#include <memory>
#include "player.h"

// Helper function to print the board
void print_board(const Board& b) {
    for (int y = 7; y >= 0; --y) { 
        for (int x = 0; x < 8; ++x) {
            int sq = y * 8 + x;
            Color c;
            PieceType pt = b.getPieceTypeAt(sq, c);
            char symb = '-';
            switch(pt) {
                case PieceType::Pawn: symb = 'P'; break;
                case PieceType::Knight: symb = 'N'; break;
                case PieceType::Bishop: symb = 'B'; break;
                case PieceType::Rook: symb = 'R'; break;
                case PieceType::Queen: symb = 'Q'; break;
                case PieceType::King: symb = 'K'; break;
                default: break;
            }
            // Black pieces in lowercase
            if (c == Color::Black) symb = std::tolower(symb);
            std::cout << symb << " ";
        }
        std::cout << std::endl;
    }
}

int main() {
    Game g;
    g.startGame();

    print_board(g.board());

    // White = human, Black = AI
    std::unique_ptr<Player> white = std::make_unique<HumanPlayer>();
    std::unique_ptr<Player> black = std::make_unique<AIPlayer>(5);

    while (true) {
        Player& current = (g.currentTurn() == Color::White ? *white : *black);

        Move m = current.getMove(g);

        if (m.from == -1 || m.to == -1)
            break; // Quit or bad input

        if (g.playMove(m)) {
            print_board(g.board());
            GameState state = g.gameState();

            if (state == GameState::Checkmate || state == GameState::Stalemate)
                break;
        }
        else {
            // INVALID → reprint board (for Python interface)
            print_board(g.board());
        }
    }

    return 0;
}

