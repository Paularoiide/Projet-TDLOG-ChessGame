#include <iostream>
#include "game.h"

// Fonction d'aide pour afficher le plateau Bitboard
void print_board(const Board& b) {
    for (int y = 7; y >= 0; --y) { // On affiche de la ligne 8 (index y=7) vers 1
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
        std::cout << "\n";
    }
}

int main() {
    Game g(nullptr); // Plus besoin de variant complexe pour l'instant
    g.startGame();
    
    print_board(g.board());

    std::cout << "\nEntrez un coup (ex: e2 e3), q pour quitter:\n";
    std::string a, b;
    
    auto parse = [](const std::string& s) -> int { 
        return (s[1] - '1') * 8 + (s[0] - 'a'); 
    };

    while (std::cin >> a) {
        if (a == "q") break; 
        std::cin >> b;
        
        Move m(parse(a), parse(b));
        g.playMove(m);
        
        print_board(g.board());
    }
    return 0;
}