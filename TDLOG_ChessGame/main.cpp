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

std::string indexToSquare(int sq) {
    std::string s = "";
    s += (char)('a' + (sq % 8));
    s += (char)('1' + (sq / 8));
    return s;
}

// --- UCI PROTOCOL LOOP ---

void uci_loop() {
    Game game;
    game.startGame(Variant::Classic);

    // Default UCI depth
    AI bot(new MaterialAndPositionEvaluation(), 5);

    std::string line, token;

    while (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        ss >> token;

        if (token == "uci") {
            std::cout << "id name TDLOG_ChessEngine" << std::endl;
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
                }
            }
        }
        else if (token == "go") {
            Move best = bot.getBestMove(game.board(), game.currentTurn());
            std::cout << "bestmove " << indexToSquare(best.from) << indexToSquare(best.to);
            if (best.promotion != PieceType::None) {
                char p = 'q';
                if (best.promotion == PieceType::Rook) p = 'r';
                else if (best.promotion == PieceType::Bishop) p = 'b';
                else if (best.promotion == PieceType::Knight) p = 'n';
                std::cout << p;
            }
            std::cout << std::endl;
        }
        else if (token == "quit") {
            break;
        }
    }
}

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
            selectedVariant = Variant::FairyChess;
        }
    }

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

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "quit" || line == "q") break;
        if (line.empty()) continue;

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
        if (state == GameState::Checkmate || state == GameState::Stalemate) {
            continue; // End of the game
        }

        // --- AI Turn ---
        if (!isPvP) {
            Move aiMove = bot.getBestMove(game.board(), game.currentTurn());
            game.playMove(aiMove);
            print_board_raw(game.board());
        }
    }

    return 0;
}
