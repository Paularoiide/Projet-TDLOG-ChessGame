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
                case PieceType::Princess:   ch = 'A'; break;
                case PieceType::Empress:    ch = 'E'; break;
                case PieceType::Nightrider: ch = 'H'; break;
                case PieceType::Grasshopper: ch = 'G'; break;
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


std::string indexToSquare(int sq) {
    std::string s = "";
    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));
    return s;
}


int main(int argc, char* argv[]) {

    Variant selectedVariant = Variant::Classic;
    std::string gamemode = "PvP";
    int depth[2] = {5, 5};

    if(argc>1){
        if (argv[1]=="fairy"){
            selectedVariant = Variant::FairyChess;
        }
    }

    if(argc>2){
        gamemode = argv[2];
    }

    if(argc>3){
        depth[0] = std::stoi(argv[3]);
    }

    if(argc>4){
        depth[1] = std::stoi(argv[4]);
    }

    Game g;
    g.startGame(selectedVariant);

    print_board_raw(g.board());

    std::string line;
    Player* player1;
    Player* player2;
    Player* players[2] = {player1, player2};

    if(gamemode == "PvP"){
        *player1 = HumanPlayer();
        *player2 = HumanPlayer();
    }
    else{
        if(gamemode == "PvAI"){
            *player1 = HumanPlayer();
        }
        if(line == "AIvAI"){
            *player1 = AI(new MaterialAndPositionEvaluation(), depth[0]);
        }
        *player2 = AI(new MaterialAndPositionEvaluation(), depth[1]);

    }


    int player = 0; 
    bool played = false;

    while ((dynamic_cast<AI*>(players[player]))||(std::getline(std::cin, line))) {
        Move* move;

        if (dynamic_cast<AI*>(players[player])){
            *move = players[player]->getBestMove(g.board(), g.currentTurn());
            bool answer = g.playMove(*move);
            GameState state = g.gameState();

            if (answer){
                played = true;
                std::cout << "VAL" << std::endl;
            }
            else{
                std::cout << "ILL" << std::endl;
            }

            print_board_raw(g.board());

            if (state == GameState::Checkmate || state == GameState::Stalemate) {
                std::cout << "END" << std::endl;
                return 0;
            }
        }
        
        else{

            if (line.length() >= 3 && line.substr(0, 3) == "QUI") {
                std::cout << "END" << std::endl;
                return 0;
            } 
            if (line.empty()) continue;

            auto parseSquare = [](const std::string& s) -> int {
                if (s.size() < 2) return -1;
                return (s[1] - '1') * 8 + (s[0] - 'a');
            };

            if (line.length() >= 3 && line.substr(0, 3) == "POS") {
                std::stringstream ss(line.substr(4));
                std::string a;
                ss >> a;
                int from = parseSquare(a);
                //Retourner les cases possibles TODO
            }

            if (line.length() >= 3 && line.substr(0, 3) == "MOV") {
                //joueur humain

                std::stringstream ss(line.substr(4));
                std::string a, b;
                ss >> a >> b;

                int from = parseSquare(a);
                int to   = parseSquare(b);

                *move = Move(from, to);
            }
                
            bool answer = g.playMove(*move);
            GameState state = g.gameState();

            //Promotion
            if (answer && state==GameState::Prom){
                std::cout << "PRO" << std::endl;
                print_board_raw(g.board());
                while (std::getline(std::cin, line)){
                    if (line.length() >= 3 && line.substr(0, 3) == "QUI") {
                        std::cout << "END" << std::endl;
                        return 0;
                    }
                    if (line.empty()) continue;
                    if (line.substr(0, 3) == "PRO"){
                        answer = g.prom(stoi(line.substr(4)));
                        if (!answer) continue;
                    }
                }
            }
            if (answer){
                played = true;
                std::cout << "VAL" << std::endl;
            }
            else{
                std::cout << "ILL" << std::endl;
            }
            print_board_raw(g.board());

            if (state == GameState::Checkmate || state == GameState::Stalemate) {
                std::cout << "END" << std::endl;
                return 0;
            }
        }
        if (played){
            player = (player+1)%2;
        }
    }
    return 0;
}