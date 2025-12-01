#include "player.h"
#include <iostream>
#include <sstream>

static int parse(const std::string& s) {
    if (s.length() < 2) return -1;
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    return rank * 8 + file;
}

// ----------------------------------
// HumanPlayer: reads a line from stdin
// ----------------------------------
Move HumanPlayer::getMove(Game& g) {
    std::string line, a, b, promoWord;

    if (!std::getline(std::cin, line)) return Move(-1, -1);

    std::stringstream ss(line);
    ss >> a;
    if (!(ss >> b)) return Move(-1, -1);
    ss >> promoWord;

    int from = parse(a);
    int to   = parse(b);

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

// ----------------------------------
// AIPlayer: calls findBestMove()
// ----------------------------------
Move AIPlayer::getMove(Game& g) {
    return g.findBestMove(depth_);
}
