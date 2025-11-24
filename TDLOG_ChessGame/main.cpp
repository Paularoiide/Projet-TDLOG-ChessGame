#include <iostream>
#include <string>
#include <cctype> // pour std::tolower
#include "game.h"
#include "piece.h" // Nécessaire pour PieceType et Color
#include "board.h"
#include "move.h"

// Fonction d'aide pour afficher le plateau Bitboard
void print_board(const Board& b) {
    std::cout << "\n  a b c d e f g h\n"; // Aide visuelle pour les colonnes
    for (int y = 7; y >= 0; --y) { 
        std::cout << y + 1 << " "; // Aide visuelle pour les rangées
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
            // Les pièces noires en minuscules
            if (c == Color::Black) symb = std::tolower(symb);
            std::cout << symb << " ";
        }
        std::cout << y + 1 << "\n";
    }
    std::cout << "  a b c d e f g h\n";
}

int main() {
    // --- CHANGEMENT ICI : Constructeur par défaut ---
    Game g; 
    g.startGame();
    
    print_board(g.board());

    std::cout << "\nEntrez un coup (ex: e2 e4), q pour quitter:\n";
    std::string a, b;
    
    // Lambda pour convertir "e2" -> index 0-63
    auto parse = [](const std::string& s) -> int { 
        // Vérification basique pour éviter les crashs si l'input est mauvais
        if (s.length() < 2) return 0;
        int file = s[0] - 'a';
        int rank = s[1] - '1';
        return rank * 8 + file; 
    };

    while (true) {
        std::cout << "> "; // Invite de commande
        if (!(std::cin >> a)) break;
        if (a == "q") break; 
        
        std::cin >> b;
        
        // Création du coup
        Move m(parse(a), parse(b));
        
        // --- CHANGEMENT ICI : Gestion du retour de playMove ---
        if (g.playMove(m)) {
            std::cout << "Coup joue : " << a << " -> " << b << "\n";
            print_board(g.board());
        } else {
            std::cout << "Coup INVALIDE ! (Verifiez les regles ou le tour)\n";
        }
    }
    
    return 0;
}