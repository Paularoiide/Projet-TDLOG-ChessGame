#include <iostream>
#include <sstream>
#include "game.h"
#include "board.h"
#include "move.h"

static void print_board(const Board& b) {
    // Print ranks 8 -> 1, files a -> h
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;

            Color c;
            PieceType pt = b.getPieceTypeAt(sq, c);

            char ch = '-';
            if (pt != PieceType::None) {
                switch (pt) {
                case PieceType::Pawn:   ch = 'P'; break;
                case PieceType::Knight: ch = 'N'; break;
                case PieceType::Bishop: ch = 'B'; break;
                case PieceType::Rook:   ch = 'R'; break;
                case PieceType::Queen:  ch = 'Q'; break;
                case PieceType::King:   ch = 'K'; break;
                default: break;
                }
                if (c == Color::Black) ch = (char)tolower(ch);
            }

            std::cout << ch;
            if (file < 7) std::cout << ' ';
        }
        std::cout << '\n';
    }
}

static int parseSquare(const std::string& s) {
    if (s.size() < 2) return -1;
    int file = s[0] - 'a';   // a..h -> 0..7
    int rank = s[1] - '1';   // '1'..'8' -> 0..7 (rank 1 at bottom)
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    return rank * 8 + file;
}

int main() {
    Game g;
    g.startGame();

    // Initial board
    print_board(g.board());
    std::cout.flush();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "q" || line == "quit")
            break;
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string a, b, promoWord;
        ss >> a >> b >> promoWord;

        int from = parseSquare(a);
        int to   = parseSquare(b);

        PieceType promo = PieceType::None;
        if (!promoWord.empty()) {
            char p = std::tolower(promoWord[0]);
            if (p == 'q') promo = PieceType::Queen;
            else if (p == 'r') promo = PieceType::Rook;
            else if (p == 'b') promo = PieceType::Bishop;
            else if (p == 'n') promo = PieceType::Knight;
        }

        Move humanMove(from, to, promo);

        // 1) On tente le coup humain
        bool ok = g.playMove(humanMove);

        // 2) On affiche le board après le coup humain
        //    (même s'il était illégal => board inchangé)
        print_board(g.board());
        std::cout.flush();

        // 3) Si le coup est illégal ou la partie est finie -> pas de coup IA
        if (!ok || g.gameState() == GameState::Checkmate || g.gameState() == GameState::Stalemate) {
            // IMPORTANT : on renvoie quand même un DEUXIÈME plateau
            // pour satisfaire le second read_board() côté Python.
            print_board(g.board());
            std::cout.flush();
            if (!ok) {
                // coup illégal : on continue la partie normalement
                continue;
            } else {
                // partie finie : on peut éventuellement break ici si tu veux
                break;
            }
        }

        // 4) Coup de l'IA (camp opposé)
        Move ai = g.findBestMove(3);   // profondeur 3 par exemple
        g.playMove(ai);

        // 5) On affiche le board après le coup IA
        print_board(g.board());
        std::cout.flush();
    }

    return 0;
}
