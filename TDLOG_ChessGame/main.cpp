#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "game.h"
#include "board.h"
#include "move.h"
#include "ai.h"
#include "player.h"

// Affiche le plateau brut pour Python (8 lignes)
void print_board_raw(const Board& b) {
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
    std::cout << std::flush;
}

Move parse_move_string(std::string line) {
    std::stringstream ss(line);
    std::string a, b, promoWord;
    ss >> a >> b >> promoWord;

    auto parseSq = [](const std::string& s) -> int {
        if (s.size() < 2) return -1;
        return (s[1] - '1') * 8 + (s[0] - 'a');
    };

    int from = parseSq(a);
    int to = parseSq(b);

    PieceType promo = PieceType::None;
    if (!promoWord.empty()) {
        char p = std::tolower(promoWord[0]);
        if (p == 'q') promo = PieceType::Queen;
        else if (p == 'r') promo = PieceType::Rook;
        else if (p == 'b') promo = PieceType::Bishop;
        else if (p == 'n') promo = PieceType::Knight;
    }
    return Move(from, to, promo);
}

int main() {
    Game game;
    game.startGame();

    // 1. Envoi état initial
    print_board_raw(game.board());

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit" || line == "q") break;
        if (line.empty()) continue;

        // --- TOUR HUMAIN ---
        Move humanMove = parse_move_string(line);
        if (!game.playMove(humanMove)) {
            // Coup illégal : on renvoie le plateau inchangé pour que Python reste synchro
            print_board_raw(game.board());
            continue; 
        }

        // On envoie le plateau après le coup humain
        print_board_raw(game.board());

        // --- CORRECTION MAJEURE ICI ---
        // On ne s'arrête QUE si c'est Mat ou Pat. 
        // Si c'est "Check" (Echec simple), on CONTINUE pour que l'IA se défende.
        GameState state = game.gameState();
        if (state == GameState::Checkmate || state == GameState::Stalemate) {
            continue; // Fin de partie, l'IA ne joue plus
        }

        // --- TOUR IA ---
        Move aiMove = AI::getBestMove(game.board(), 4, game.currentTurn());
        game.playMove(aiMove);

        // On envoie le plateau après le coup IA
        print_board_raw(game.board());
    }

    return 0;
}