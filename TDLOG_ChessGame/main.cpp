#include <iostream>
#include <string>
#include <cctype> 
#include "game.h"
#include "piece.h" // Nécessaire pour PieceType et Color
#include "board.h"
#include "move.h"

// Fonction d'aide pour afficher le plateau Bitboard
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

    std::cout << "\nEntrez un coup (ex: e2 e4), q pour quitter:\n";
    std::string a, b;
    
    auto parse = [](const std::string& s) -> int { 
        if (s.length() < 2) return 0;
        int file = s[0] - 'a';
        int rank = s[1] - '1';
        return rank * 8 + file; 
    };

    while (true) {
        // Affichage du trait (qui doit jouer)
        std::cout << (g.currentTurn() == Color::White ? "[Blancs]" : "[Noirs]") << " > ";
        
        if (!(std::cin >> a)) break;
        if (a == "q") break; 
        
        std::cin >> b;
        
        Move m(parse(a), parse(b));
        
        if (g.playMove(m)) {
            std::cout << "Coup joue : " << a << " -> " << b << "\n";
            print_board(g.board());

            // --- NOUVEAU BLOC : Vérification de la fin de partie ---
            GameState state = g.gameState();

            if (state == GameState::Check) {
                std::cout << " ECHEC !\n";
            }
            else if (state == GameState::Checkmate) {
                // Si c'est aux Blancs de jouer et qu'ils sont mat, les Noirs ont gagné.
                std::string winner = (g.currentTurn() == Color::White) ? "Les Noirs" : "Les Blancs";
                std::cout << "\n ECHEC ET MAT ! " << winner << " gagnent la partie !\n";
                break; // On sort de la boucle, fin du jeu
            }
            else if (state == GameState::Stalemate) {
                std::cout << "\n½ PAT ! Match Nul (Plus de coups legaux mais pas d'echec).\n";
                break; // Fin du jeu
            }
            // -------------------------------------------------------

        } else {
            std::cout << " Coup INVALIDE ! (Verifiez les regles, ou si vous etes en echec)\n";
        }
    }
    
    return 0;
}