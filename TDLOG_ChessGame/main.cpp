#include <iostream>
#include <string>
#include <vector>
#include <sstream> 
#include <cctype>  // pour std::tolower
#include "game.h"
#include "piece.h" 
#include "board.h"
#include "move.h"

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

    //std::cout << "\nCommands:\n";
    //std::cout << " - Standard : e2 e4\n";
    //std::cout << " - Promotion: a7 a8 q (or a7 a8q)\n";
    //std::cout << " - Quit     : q\n\n";

    // Lambda to convert "e2" -> index 0-63
    auto parse = [](const std::string& s) -> int { 
        if (s.length() < 2) return -1; 
        int file = s[0] - 'a';
        int rank = s[1] - '1';
        if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
        return rank * 8 + file; 
    };

    std::string line;
    while (true) {
        //std::cout << (g.currentTurn() == Color::White ? "[White]" : "[Black]") << " > ";
        
        // Read the full line to handle inputs with spaces like "a7 a8 q"
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string word1, word2, word3;
        
        ss >> word1;
        if (word1 == "q") break; // Quit command

        if (!(ss >> word2)) {
            //std::cout << "Invalid format. Usage: e2 e4\n";
            continue;
        }
        
        // Optional 3rd word for promotion (e.g. "q" in "a7 a8 q")
        ss >> word3; 

        // 1. Parse coordinates
        int from = parse(word1);
        int to = parse(word2);

        if (from == -1 || to == -1) {
             //std::cout << "Invalid coordinates.\n";
             continue;
        }

        PieceType promo = PieceType::None;

        // 2. Promotion detection
        // Case A: Attached (a7 a8q) -> Check end of word2
        if (word2.length() > 2) {
            char p = std::tolower(word2[2]);
            if (p == 'q') promo = PieceType::Queen;
            else if (p == 'r') promo = PieceType::Rook;
            else if (p == 'b') promo = PieceType::Bishop;
            else if (p == 'n') promo = PieceType::Knight;
        }
        // Case B: Separated (a7 a8 q) -> Check word3
        else if (!word3.empty()) {
            char p = std::tolower(word3[0]);
            if (p == 'q') promo = PieceType::Queen;
            else if (p == 'r') promo = PieceType::Rook;
            else if (p == 'b') promo = PieceType::Bishop;
            else if (p == 'n') promo = PieceType::Knight;
        }

        // 3. Play move
        Move m(from, to, promo);
        
        if (g.playMove(m)) {
            //std::cout << "Move played: " << word1 << " -> " << word2 << (promo != PieceType::None ? " (Promo)" : "") << "\n";
            print_board(g.board());

            // 4. Game State
            GameState state = g.gameState();
            if (state == GameState::Check) {
                //std::cout << "  CHECK!\n";
            }
            else if (state == GameState::Checkmate) {
                //std::string winner = (g.currentTurn() == Color::White) ? "Black" : "White";
                //std::cout << "\n CHECKMATE! " << winner << " wins the game!\n";
                break; 
            }
            else if (state == GameState::Stalemate) {
                //std::cout << "\n½ STALEMATE! Draw.\n";
                break; 
            }
        } else {
            //std::cout << " INVALID MOVE! (Check rules, promotion, or if in check)\n";
            print_board(g.board());
        }
    }
    
    return 0;
}
