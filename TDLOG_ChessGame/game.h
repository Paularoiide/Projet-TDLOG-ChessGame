#pragma once
#include <vector>
#include "rules.h"

class Board; // temporary declaration
class Piece;
class Variant;
class Move;
class Player;

class Game{
    Board board;
    std::vector<Player> players;
    Variant variant;
    Color currentTurn;

    void StartGame();
    void PlayMove(Move move);
    bool isCheckmate();
};

