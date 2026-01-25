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

<<<<<<< HEAD

int parseSquare(const std::string& s) {
    if (s.size() < 2) return -1;
    return (s[1] - '1') * 8 + (s[0] - 'a');
}

// --- BOUCLE UCI (Pour Arena) ---
=======
std::string indexToSquare(int sq) {
    std::string s = "";
    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));
    return s;
}

// --- UCI PROTOCOL LOOP ---

>>>>>>> dev
void uci_loop() {
    Game game;
    game.startGame(Variant::Classic);

<<<<<<< HEAD
    AI bot(new MaterialAndPositionEvaluation(), 6);
=======
    // Default UCI depth
    AI bot(new MaterialAndPositionEvaluation(), 5);
>>>>>>> dev

    std::string line, token;

    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        ss >> token;

        if (token == "uci") {
<<<<<<< HEAD
            std::cout << "id name TDLOG_Engine" << std::endl;
=======
            std::cout << "id name TDLOG_ChessEngine" << std::endl;
>>>>>>> dev
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
<<<<<<< HEAD
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
=======
            std::string posType;
            ss >> posType;
            if (posType == "startpos") {
                game.startGame(Variant::Classic);
            }

            std::string movesKeyword;
            while (ss >> movesKeyword) {
                if (movesKeyword == "moves") {
                    std::string moveStr;
                    while (ss >> moveStr) {
                        // Parse UCI move (e.g., "e2e4" or "a7a8q")
                        if (moveStr.length() < 4) continue;

                        int f = (moveStr[0] - 'a') + (moveStr[1] - '1') * 8;
                        int t = (moveStr[2] - 'a') + (moveStr[3] - '1') * 8;
                        PieceType p = PieceType::None;

                        if (moveStr.length() > 4) {
                            char promoChar = moveStr[4];
                            if (promoChar == 'q') p = PieceType::Queen;
                            else if (promoChar == 'r') p = PieceType::Rook;
                            else if (promoChar == 'b') p = PieceType::Bishop;
                            else if (promoChar == 'n') p = PieceType::Knight;
                        }

                        game.playMove(Move(f, t, p));
                    }
                    break;
>>>>>>> dev
                }
            }
        }
        else if (token == "go") {
            Move best = bot.getBestMove(game.board(), game.currentTurn());
            std::cout << "bestmove " << indexToSquare(best.from) << indexToSquare(best.to);
<<<<<<< HEAD
            if (best.promotion != PieceType::None) std::cout << "q";
=======
            if (best.promotion != PieceType::None) {
                char p = 'q';
                if (best.promotion == PieceType::Rook) p = 'r';
                else if (best.promotion == PieceType::Bishop) p = 'b';
                else if (best.promotion == PieceType::Knight) p = 'n';
                std::cout << p;
            }
>>>>>>> dev
            std::cout << std::endl;
        }
        else if (token == "quit") {
            break;
        }
    }
}

<<<<<<< HEAD

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
=======
// --- MAIN ---

int main(int argc, char* argv[]) {
    // 0. CHECK FOR UCI MODE
    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "uci") {
            uci_loop();
            return 0;
        }
    }

    Variant selectedVariant = Variant::Classic;
    bool isPvP = false;
    int searchDepth = 5;

    // 1. VARIANT
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "fairy") {
>>>>>>> dev
            selectedVariant = Variant::FairyChess;
        }
    }

<<<<<<< HEAD
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
=======
    // 2. MODE
    if (argc > 2) {
        std::string mode = argv[2];
        if (mode == "pvp") {
            isPvP = true;
        }
    }

    // 3. DEPTH
    if (argc > 3) {
        try {
            searchDepth = std::stoi(argv[3]);
            if (searchDepth < 1) searchDepth = 1;
            if (searchDepth > 10) searchDepth = 10;
        } catch (...) {
            searchDepth = 4;
        }
    }

    Game game;
    game.startGame(selectedVariant);

    AI bot(new MaterialAndPositionEvaluation(), searchDepth);

    print_board_raw(game.board());
>>>>>>> dev

    std::string line;
    int player = 0; 
    bool played = false;

<<<<<<< HEAD
    while (true) {
        GameState state = g.gameState();
=======
        if (line.rfind("possible", 0) == 0) {
            std::stringstream ss(line);
            std::string cmd, sqStr;
            ss >> cmd >> sqStr;
            int reqSq = -1;
            if (sqStr.size() >= 2) {
                int f = sqStr[0] - 'a';
                int r = sqStr[1] - '1';
                if (f >= 0 && f < 8 && r >= 0 && r < 8) reqSq = r * 8 + f;
            }
            if (reqSq != -1) {
                std::vector<Move> moves = game.board().generateLegalMoves(game.currentTurn());
                bool first = true;
                for (const auto& m : moves) {
                    if (m.from == reqSq) {
                        if (!first) std::cout << " ";
                        std::cout << indexToSquare(m.to);
                        first = false;
                    }
                }
            }
            std::cout << std::endl;
            continue;
        }

        // --- Human Turn ---
        Move humanMove = parse_move_string(line);
        if (!game.playMove(humanMove)) {
            print_board_raw(game.board());
            continue;
        }

        print_board_raw(game.board());

        GameState state = game.gameState();
>>>>>>> dev
        if (state == GameState::Checkmate || state == GameState::Stalemate) {
            std::cout << "END" << std::endl;
            break; 
        }

<<<<<<< HEAD
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
=======
        // --- AI Turn ---
        if (!isPvP) {
            Move aiMove = bot.getBestMove(game.board(), game.currentTurn());
            game.playMove(aiMove);
            print_board_raw(game.board());
>>>>>>> dev
        }
    }

    delete players[0];
    delete players[1];

    return 0;
}
