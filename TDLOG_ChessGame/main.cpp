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
    std::cout << " - Promotion: a7 a8 q (or a7 a8q)\n";
    std::cout << " - Quit     : q\n\n";

    // Convert "e2" -> 0..63
    auto parse = [](const std::string& s) -> int {
        if (s.length() < 2) return -1;
        int file = s[0] - 'a';
        int rank = s[1] - '1';
        if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
        return rank * 8 + file;
    };

    std::string line;

    while (true) {

        // =============================
        // 1) HUMAN = WHITE
        // =============================
        if (g.currentTurn() == Color::White) {

            std::cout << "[White] > ";

            if (!std::getline(std::cin, line)) break;
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string word1, word2, word3;

            ss >> word1;
            if (word1 == "q") break;

            if (!(ss >> word2)) {
                std::cout << "Invalid format. Usage: e2 e4\n";
                continue;
            }
            ss >> word3; // optional for promotion

            int from = parse(word1);
            int to   = parse(word2);

            if (from == -1 || to == -1) {
                std::cout << "Invalid coordinates.\n";
                continue;
            }

            // PROMOTION
            PieceType promo = PieceType::None;

            // Attached: e7 e8q
            if (word2.length() > 2) {
                char p = std::tolower(word2[2]);
                if (p == 'q') promo = PieceType::Queen;
                else if (p == 'r') promo = PieceType::Rook;
                else if (p == 'b') promo = PieceType::Bishop;
                else if (p == 'n') promo = PieceType::Knight;
            }
            // Separated: e7 e8 q
            else if (!word3.empty()) {
                char p = std::tolower(word3[0]);
                if (p == 'q') promo = PieceType::Queen;
                else if (p == 'r') promo = PieceType::Rook;
                else if (p == 'b') promo = PieceType::Bishop;
                else if (p == 'n') promo = PieceType::Knight;
            }

            Move m(from, to, promo);

            if (g.playMove(m)) {
                std::cout << "White plays: " << word1 << " -> " << word2
                          << (promo != PieceType::None ? " (Promo)" : "")
                          << "\n";

                print_board(g.board());

                GameState state = g.gameState();
                if (state == GameState::Check)
                    std::cout << "CHECK!\n";
                else if (state == GameState::Checkmate) {
                    std::cout << "CHECKMATE! White wins.\n";
                    break;
                }
                else if (state == GameState::Stalemate) {
                    std::cout << "STALEMATE. Draw.\n";
                    break;
                }
            }
            else {
                std::cout << "INVALID MOVE!\n";
            }
        }

        // =============================
        // 2) AI = BLACK
        // =============================
        else {
            std::cout << "[Black AI thinking...]\n";

            Move aiMove = g.findBestMove(5);  // depth = 5

            // Convert to string like "e2"
            auto toStr = [](int sq) {
                std::string s;
                s += char('a' + (sq % 8));
                s += char('1' + (sq / 8));
                return s;
            };

            std::string a = toStr(aiMove.from);
            std::string b = toStr(aiMove.to);

            g.playMove(aiMove);

            std::cout << "Black plays: " << a << " -> " << b << "\n";
            print_board(g.board());

            GameState state = g.gameState();

            if (state == GameState::Check) {
                std::cout << "CHECK on White!\n";
            }
            else if (state == GameState::Checkmate) {
                std::cout << "CHECKMATE! Black (AI) wins.\n";
                break;
            }
            else if (state == GameState::Stalemate) {
                std::cout << "STALEMATE. Draw.\n";
                break;
            }
        }
    }

    return 0;
}

