#include <iostream>
#include <string>
#include <cctype> // for std::tolower
#include "game.h"
#include "piece.h" 
#include "board.h"
#include "move.h"

// Helper function to print the board
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
            // Black pieces in lowercase
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

    std::cout << "\nCommands:\n";
    std::cout << " - Standard : e2 e4\n";
    std::cout << " - Promotion: a7 a8q (q=Queen, r=Rook, b=Bishop, n=Knight)\n";
    std::cout << " - Quit     : q\n\n";

    std::string a, b;
    
    // Lambda to convert "e2" -> index 0-63
    auto parse = [](const std::string& s) -> int { 
        if (s.length() < 2) return 0; // Basic protection
        int file = s[0] - 'a';
        int rank = s[1] - '1';
        return rank * 8 + file; 
    };

    while (true) {
        // Display whose turn it is
        std::cout << (g.currentTurn() == Color::White ? "[White]" : "[Black]") << " > ";
        
        if (!(std::cin >> a)) break;
        if (a == "q") break; 
        
        std::cin >> b; // b can be "e8" or "e8q"
        
        // 1. Parse coordinates
        int from = parse(a);
        int to = parse(b);
        PieceType promo = PieceType::None;

        // 2. Parse promotion (e.g., "e8q")
        if (b.length() > 2) {
            char p = std::tolower(b[2]);
            if (p == 'q') promo = PieceType::Queen;
            else if (p == 'r') promo = PieceType::Rook;
            else if (p == 'b') promo = PieceType::Bishop;
            else if (p == 'n') promo = PieceType::Knight;
        }

        // 3. Create the move
        Move m(from, to, promo);
        
        // 4. Attempt to play the move
        if (g.playMove(m)) {
            std::cout << "Move played: " << a << " -> " << b << "\n";
            print_board(g.board());

            // 5. Check game state (Check / Mate / Stalemate)
            GameState state = g.gameState();

            if (state == GameState::Check) {
                std::cout << " CHECK!\n";
            }
            else if (state == GameState::Checkmate) {
                std::string winner = (g.currentTurn() == Color::White) ? "Black" : "White";
                std::cout << "\n CHECKMATE! " << winner << " wins the game!\n";
                break; // Game over
            }
            else if (state == GameState::Stalemate) {
                std::cout << "\n½ STALEMATE! Draw (No legal moves but not in check).\n";
                break; // Game over
            }

        } else {
            std::cout << " INVALID MOVE! (Check rules, check status, or format)\n";
        }
    }
    
    return 0;
}