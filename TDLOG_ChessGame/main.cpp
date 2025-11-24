#include <iostream>
#include <memory>
#include "game.h"
#include "variant.h"


static void print_board(const Board& b) {
    for (int y = 0; y < b.size(); ++y) {
        for (int x = 0; x < b.size(); ++x) {
            auto* p = b.getPiece({x,y});
            char c = '.';
            if (p) {
                switch (p->getName()) {
                case PieceName::king: c = 'K'; break;
                case PieceName::queen: c = 'Q'; break;
                case PieceName::rook: c = 'R'; break;
                case PieceName::bishop: c = 'B'; break;
                case PieceName::knight: c = 'N'; break;
                case PieceName::pawn: c = 'P'; break;
                }
                if (p->getColor() == Color::Black) c = (char)tolower(c);
            }
            std::cout << c << ' ';
        }
        std::cout << "\n";
    }
}


int main() {
    Game g(std::make_unique<ClassicVariant>());
    g.startGame();
    print_board(g.board());
    std::cout << "\nEntrez un coup (ex: e2 e4), q pour quitter:\n";
    std::string a,b;
    auto parse = [](const std::string& s)->Position{ return Position{ s[0]-'a', 7 - (s[1]-'1') }; };
    while (std::cin >> a) {
        if (a == "q") break; std::cin >> b;
        Move m(parse(a), parse(b));
        m.movedPiece = g.board().getPiece(m.from);

        bool ok = g.playMove(m);
        std::cout << (ok ? "OK" : "ILL") << "\n";
        print_board(g.board());
    }
}
