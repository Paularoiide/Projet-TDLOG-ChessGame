#include <iostream>
#include <sstream>
#include "game.h"
#include "board.h"
#include "move.h"

static void print_board(const Board& b) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;

            Color c;
            PieceType pt = b.getPieceTypeAt(sq, c);

            char ch = '-';
            if (pt != PieceType::None) {
                switch (pt) {
                case PieceType::Pawn: ch = 'P'; break;
                case PieceType::Knight: ch = 'N'; break;
                case PieceType::Bishop: ch = 'B'; break;
                case PieceType::Rook: ch = 'R'; break;
                case PieceType::Queen: ch = 'Q'; break;
                case PieceType::King: ch = 'K'; break;
                default: break;
                }
                if (c == Color::Black) ch = (char)tolower(ch);
            }

            std::cout << ch;
            if (file < 7) std::cout << ' ';
        }
        std::cout << '\n';
    }
    std::cout.flush();
}

static int parseSquare(const std::string& s) {
    if (s.size() < 2) return -1;
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    return rank * 8 + file;
}

int main() {
    Game g;
    g.startGame();

    // Print initial board
    print_board(g.board());

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "q") break;
        if (line.empty()) continue;

        // Parse request
        std::stringstream ss(line);
        std::string a, b, promoWord;
        ss >> a >> b >> promoWord;

        int from = parseSquare(a);
        int to   = parseSquare(b);

        PieceType promo = PieceType::None;
        if (!promoWord.empty()) {
            char p = std::tolower(promoWord[0]);
            if (p=='q') promo = PieceType::Queen;
            else if (p=='r') promo = PieceType::Rook;
            else if (p=='b') promo = PieceType::Bishop;
            else if (p=='n') promo = PieceType::Knight;
        }

        Move humanMove(from, to, promo);

        // ================
        // 1) HUMAN MOVE
        // ================
        bool ok = g.playMove(humanMove);

        // Always print board after human try
        print_board(g.board());

        // If illegal → print AGAIN (Python expects 2 boards)
        if (!ok) {
            print_board(g.board());
            continue;
        }

        // If game ended → print again for Python & exit
        if (g.gameState() != GameState::Playing) {
            print_board(g.board());
            break;
        }

        // =====================
        // 2) AI MOVE
        // =====================
        Move ai = g.findBestMove(5);
        g.playMove(ai);

        // Print board after AI move
        print_board(g.board());
    }

    return 0;
}
