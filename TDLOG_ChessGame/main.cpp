#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "game.h"
#include "board.h"
#include "move.h"
#include "ai.h"
#include "player.h"

// --- UTILS ---

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


int parseSquare(const std::string& s) {
    if (s.size() < 2) return -1;
    return (s[1] - '1') * 8 + (s[0] - 'a');
}

// --- BOUCLE UCI (Pour Arena) ---
void uci_loop() {
    Game game;
    game.startGame(Variant::Classic);

    AI bot(new MaterialAndPositionEvaluation(), 6);

    std::string line, token;

    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        ss >> token;

        if (token == "uci") {
            std::cout << "id name TDLOG_Engine" << std::endl;
            std::cout << "id author You" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (token == "ucinewgame") {
            game.startGame(Variant::Classic);
        }
        else if (token == "position") {
            std::string sub;
            ss >> sub;

            if (sub == "startpos") {
                game.startGame(Variant::Classic);
                ss >> sub;
            }

            if (sub == "moves") {
                std::string moveStr;
                while (ss >> moveStr) {
                    std::string fromStr = moveStr.substr(0, 2);
                    std::string toStr = moveStr.substr(2, 2);

                    int from = parseSquare(fromStr);
                    int to = parseSquare(toStr);

                    PieceType promo = PieceType::None;
                    if (moveStr.length() > 4) {
                        char p = moveStr[4];
                        if (p == 'q') promo = PieceType::Queen;
                        else if (p == 'r') promo = PieceType::Rook;
                        else if (p == 'b') promo = PieceType::Bishop;
                        else if (p == 'n') promo = PieceType::Knight;
                    }

                    game.playMove(Move(from, to, promo));
                }
            }
        }
        else if (token == "go") {
            Move best = bot.getBestMove(game.board(), game.currentTurn());
            std::cout << "bestmove " << indexToSquare(best.from) << indexToSquare(best.to);
            if (best.promotion != PieceType::None) std::cout << "q";
            std::cout << std::endl;
        }
        else if (token == "quit") {
            break;
        }
    }
}


int main(int argc, char* argv[]) {


    if (argc > 1 && std::string(argv[1]) == "uci") {
        uci_loop();
        return 0;
    }

    Variant selectedVariant = Variant::Classic;
    std::string gamemode = "PvP";
    int depth[2] = {5, 5};

    if(argc>1){
        if (std::string(argv[1]) == "fairy"){
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

    Player* players[2];

    if(gamemode == "PvP"){
        players[0] = new HumanPlayer();
        players[1] = new HumanPlayer();
    }
    else if (gamemode == "PvAI") {
        players[0] = new HumanPlayer();
        players[1] = new AI(new MaterialAndPositionEvaluation(), depth[0]);
    }
    else if (gamemode == "AIvP") {
        players[0] = new AI(new MaterialAndPositionEvaluation(), depth[0]);
        players[1] = new HumanPlayer();
    }
    else {
        players[0] = new AI(new MaterialAndPositionEvaluation(), depth[0]);
        players[1] = new AI(new MaterialAndPositionEvaluation(), depth[1]);
    }

    print_board_raw(g.board());

    std::string line;
    int player = 0; 
    bool played = false;

    while (true) {
        GameState state = g.gameState();
        if (state == GameState::Checkmate || state == GameState::Stalemate) {
            std::cout << "END" << std::endl;
            break; 
        }

        AI* bot = dynamic_cast<AI*>(players[player]);

        if (bot) {
            Move bestMove = bot->getBestMove(g.board(), g.currentTurn());
            g.playMove(bestMove);
            std::cout << "VAL" << std::endl;
            print_board_raw(g.board());
            player = (player + 1) % 2;
        }
        else {
            if (!std::getline(std::cin, line)) break;
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string cmd;
            ss >> cmd;

            if (cmd == "QUI") { 
                std::cout << "END" << std::endl;
                break;
            }
            else if (cmd == "POS") {
                std::string sqStr;
                ss >> sqStr;
                int reqSq = parseSquare(sqStr);
                
                std::cout << "POS";
                if (reqSq != -1) {
                    std::vector<Move> moves = g.board().generateLegalMoves(g.currentTurn());
                    for (const auto& m : moves) {
                        if (m.from == reqSq) {
                            std::cout << " " << indexToSquare(m.to);
                        }
                    }
                }
                std::cout << std::endl;
            }
            else if (cmd == "MOV") {
                std::string sFrom, sTo, sPromo;
                ss >> sFrom >> sTo >> sPromo;
                
                int f = parseSquare(sFrom);
                int t = parseSquare(sTo);
                PieceType p = PieceType::None;
                
                if (!sPromo.empty()) {
                    char c = tolower(sPromo[0]);
                    if (c == 'q') p = PieceType::Queen;
                    else if (c == 'r') p = PieceType::Rook;
                    else if (c == 'b') p = PieceType::Bishop;
                    else if (c == 'n') p = PieceType::Knight;
                }

                Move m(f, t, p);
                
                if (g.playMove(m)) {
                    std::cout << "VAL" << std::endl;
                    print_board_raw(g.board());
                    player = (player + 1) % 2;
                } else {
                    std::cout << "ILL" << std::endl;
                    print_board_raw(g.board());
                }
            }
        }
    }

    delete players[0];
    delete players[1];

    return 0;
}
