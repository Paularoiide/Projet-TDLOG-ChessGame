#include <iostream>
#include <string>
#include <cctype>
#include "game.h"
#include "piece.h" // Needed for PieceType and Color
#include "board.h"
#include "move.h"

// Helper function to print the Bitboard board
void print_board(const Board& b) {
    std::cout << "\n  a b c d e f g h\n";
    for (int y = 7; y >= 0; --y) {
        std::cout << y + 1 << " ";
        for (int x = 0; x < 8; ++x) {
            int sq = y * 8 + x;
            Color c;
            PieceType pt = b.getPieceTypeAt(sq, c);
            char symb = '.';
            switch(pt) {
            case PieceType::Pawn: symb = 'P'; break;
            case PieceType::Knight: symb = 'N'; break;
            case PieceType::Bishop: symb = 'B'; break;
            case PieceType::Rook: symb = 'R'; break;
            case PieceType::Queen: symb = 'Q'; break;
            case PieceType::King: symb = 'K'; break;
            default: break;
            }
            if (c == Color::Black) symb = std::tolower(symb);
            std::cout << symb << " ";
        }
        std::cout << y + 1 << "\n";
    }
    std::cout << "  a b c d e f g h\n";
}

int main() {
    Game g;
    g.startGame();

    print_board(g.board());

    std::cout << "\nEnter a move (ex: e2 e4), q to quit:\n";
    std::string a, b;

    auto parse = [](const std::string& s) -> int {
        if (s.length() < 2) return 0;
        int file = s[0] - 'a';
        int rank = s[1] - '1';
        return rank * 8 + file;
    };

    while (true) {
        // Display whose turn it is
        std::cout << (g.currentTurn() == Color::White ? "[White]" : "[Black]") << " > ";

        if (!(std::cin >> a)) break;
        if (a == "q") break;

        std::cin >> b;

        Move m(parse(a), parse(b));

        if (g.playMove(m)) {
            std::cout << "Move played: " << a << " -> " << b << "\n";
            print_board(g.board());

            // --- NEW BLOCK: Endgame verification ---
            GameState state = g.gameState();

            if (state == GameState::Check) {
                std::cout << " CHECK!\n";
            }
            else if (state == GameState::Checkmate) {
                // If it is White's move and they are checkmated, Black wins.
                std::string winner = (g.currentTurn() == Color::White) ? "Black" : "White";
                std::cout << "\n CHECKMATE! " << winner << " win the game!\n";
                break; // Exit the loop, game over
            }
            else if (state == GameState::Stalemate) {
                std::cout << "\n½ STALEMATE! Draw (No legal moves but not in check).\n";
                break; // Game over
            }
            // -------------------------------------------------------

        } else {
            std::cout << " INVALID MOVE! (Check the rules or if you are in check)\n";
        }
    }

    return 0;
}
