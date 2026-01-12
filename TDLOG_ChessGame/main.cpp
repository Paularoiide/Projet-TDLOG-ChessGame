#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "game.h"
#include "board.h"
#include "move.h"
#include "ai.h"
#include "player.h"

// Display the board
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


//Inutilise
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
    Game g;
    g.startGame();

    print_board_raw(g.board());

    std::string line;
    Player* player1;
    Player* player2;
    Player* players[2] = {player1, player2};

    //initialization
    while (std::getline(std::cin, line)){
        if (line.length() >= 4 && line.substr(0, 4) == "quit") {
            std::cout << "END" << std::endl;
            return 0;
        } 
        if (line.empty()) continue;
        if(line == "PvP"){
            *player1 = HumanPlayer();
            *player2 = HumanPlayer();
        }
        else{
            if(line == "PvAI"){
                *player1 = HumanPlayer();
                *player2 = AI(new MaterialAndPositionEvaluation(), 6);
            }
            if(line == "AIvAI"){
                *player1 = AI(new MaterialAndPositionEvaluation(), 6);
                *player2 = AI(new MaterialAndPositionEvaluation(), 6);
            }
        }
        break;
    }

    int player = 0; 
    bool break_iteration = true; //mis a false si move illegal
    bool command_read = false; //indique si la commande a deja ete exploitee


    while ((dynamic_cast<AI*>(players[player]))||(std::getline(std::cin, line))) {
        Move* move;

        if (dynamic_cast<AI*>(players[player])){
            *move = players[player]->getBestMove(g.board(), g.currentTurn());
        }
        
        else{

            if (line.length() >= 4 && line.substr(0, 4) == "quit") {
                std::cout << "END" << std::endl;
                return 0;
            } 
            if (line.empty()) continue;

            if (line.length() >= 4 && line.substr(0, 4) == "move") {
                //joueur humain

                std::stringstream ss(line.substr(5));
                std::string a, b;
                ss >> a >> b;

                auto parseSquare = [](const std::string& s) -> int {
                    if (s.size() < 2) return -1;
                    return (s[1] - '1') * 8 + (s[0] - 'a');
                };

                int from = parseSquare(a);
                int to   = parseSquare(b);

                *move = Move(from, to);
            }
                
            bool answer = g.playMove(*move);
            GameState state = g.gameState();

            //Promotion
            if (answer && state==GameState::Prom){
                std::cout << "PROM" << std::endl;
                print_board_raw(g.board());
                while (std::getline(std::cin, line)){
                    if (line.length() >= 4 && line.substr(0, 4) == "quit") {
                        std::cout << "END" << std::endl;
                        return 0;
                    }
                    if (line.empty()) continue;
                    if (line.substr(0, 4) == "PROM"){
                        answer = g.prom(stoi(line.substr(5)));
                        if (!answer) continue;
                    }
                }
            }
            std::cout << answer << std::endl;
            print_board_raw(g.board());

            if (state == GameState::Checkmate || state == GameState::Stalemate) {
                continue; // End of the game
            }
        }
        player = (player+1)%2;
    }
    return 0;
}
