#include "board.h"
#include <cstring> // for std::memset
#include <cmath>   // for std::abs
#include <vector>
#include <random> // for Zobrist hashing

// --- ZOBRIST KEYS (Static) ---
// We store random numbers for [Color][Piece][Square]
static uint64_t zPieceKeys[2][6][64];
static uint64_t zEnPassantKeys[65]; // 64 squares + 1 (none)
static uint64_t zCastleKeys[16];    // 4 rights (bitmask 0-15)
static uint64_t zSideKey;           // For the turn (Black)
// Flag to check if initialized
static bool zInitialized = false;

Board::Board(Variant v) {
    initZobristKeys();
    // 1. Reset everything to 0
    std::memset(bitboards_, 0, sizeof(bitboards_));
    std::memset(occupancies_, 0, sizeof(occupancies_));

    // index definition
    const int PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5;
    const int PRINCESS = 6, EMPRESS = 7, NIGHTRIDER = 8, GRASSHOPPER = 9;

    // ==========================================
    // 1. COMMON SETUP (Pawns, Rooks, Queen, King)
    // ==========================================

    // --- White ---
    for (int i = 8; i < 16; ++i) setBit(bitboards_[0][PAWN], i); // Pawns rank 2
    setBit(bitboards_[0][ROOK], 0); setBit(bitboards_[0][ROOK], 7); // Rooks a1, h1
    setBit(bitboards_[0][QUEEN], 3); // Queen d1
    setBit(bitboards_[0][KING], 4);  // King e1

    // --- Black ---
    for (int i = 48; i < 56; ++i) setBit(bitboards_[1][PAWN], i); // Pawns rank 7
    setBit(bitboards_[1][ROOK], 56); setBit(bitboards_[1][ROOK], 63); // Rooks a8, h8
    setBit(bitboards_[1][QUEEN], 59); // Queen d8
    setBit(bitboards_[1][KING], 60);  // King e8

    // ==========================================
    // 2. BRANCHING ACCORDING TO THE VARIANT
    // ==========================================

    if (v == Variant::FairyChess) {
        // --- FAIRY MODE ---
        // In this mode, we REPLACE Knights/Bishops with Nightrider/Princess/Empress
        // and add Grasshoppers.

        // WHITE
        setBit(bitboards_[0][NIGHTRIDER], 1); // b1 (replaces Knight)
        setBit(bitboards_[0][PRINCESS], 2);   // c1 (replaces Bishop)
        setBit(bitboards_[0][EMPRESS], 5);    // f1 (replaces Bishop)
        setBit(bitboards_[0][NIGHTRIDER], 6); // g1 (replaces Knight)


        // BLACK
        setBit(bitboards_[1][NIGHTRIDER], 57); // b8
        setBit(bitboards_[1][PRINCESS], 58);   // c8
        setBit(bitboards_[1][EMPRESS], 61);    // f8
        setBit(bitboards_[1][NIGHTRIDER], 62); // g8


    } else {
        // --- CLASSIC MODE ---
        // Standard configuration

        // WHITE
        setBit(bitboards_[0][KNIGHT], 1); // b1
        setBit(bitboards_[0][KNIGHT], 6); // g1
        setBit(bitboards_[0][BISHOP], 2); // c1
        setBit(bitboards_[0][BISHOP], 5); // f1

        // BLACK
        setBit(bitboards_[1][KNIGHT], 57); // b8
        setBit(bitboards_[1][KNIGHT], 62); // g8
        setBit(bitboards_[1][BISHOP], 58); // c8
        setBit(bitboards_[1][BISHOP], 61); // f8
    }

    // --- FINALIZATION ---
    updateOccupancies();

    // Castling rights (True par défaut)
    castleRights_[0] = castleRights_[1] = true;
    castleRights_[2] = castleRights_[3] = true;

    enPassantTarget_ = -1;
    zobristKey_ = calculateHash();
}

// =======================
//        HELPERS
// =======================

void Board::updateOccupancies() {
    occupancies_[0] = 0; // White
    occupancies_[1] = 0; // Black
    occupancies_[2] = 0; // Both

    for (int p = 0; p < 10; ++p) {
        occupancies_[0] |= bitboards_[0][p];
        occupancies_[1] |= bitboards_[1][p];
    }
    occupancies_[2] = occupancies_[0] | occupancies_[1];
}

PieceType Board::getPieceTypeAt(int square, Color& color) const {
    for (int c = 0; c < 2; ++c) {
        for (int p = 0; p < 10; ++p) {
            if (getBit(bitboards_[c][p], square)) {
                color = static_cast<Color>(c);
                return static_cast<PieceType>(p);
            }
        }
    }
    color = Color::None;
    return PieceType::None;
}

void Board::movePiece(int from, int to, PieceType promotion) {
    Color color;
    PieceType pt = getPieceTypeAt(from, color);
    if (pt == PieceType::None) return;

    // 1. Detect En Passant
    bool isEnPassant = (pt == PieceType::Pawn && to == enPassantTarget_);

    // 2. Handle En Passant capture (remove the pawn behind)
    if (isEnPassant) {
        int capturedSq = (color == Color::White) ? to - 8 : to + 8;
        popBit(bitboards_[static_cast<int>(opposite(color))][static_cast<int>(PieceType::Pawn)], capturedSq);
    }

    // 3. Handle castling rights on king/rook moves
    if (pt == PieceType::King) {
        disableCastle(color, true);
        disableCastle(color, false);
    }
    if (pt == PieceType::Rook) {
        if (color == Color::White && from == 0)  disableCastle(Color::White, false);
        if (color == Color::White && from == 7)  disableCastle(Color::White, true);
        if (color == Color::Black && from == 56) disableCastle(Color::Black, false);
        if (color == Color::Black && from == 63) disableCastle(Color::Black, true);
    }

    // 4. Handle normal capture (cannot capture own piece)
    Color targetColor;
    PieceType targetPt = getPieceTypeAt(to, targetColor);
    if (targetPt != PieceType::None) {
        if (targetColor == color && !isEnPassant) {
            // Friendly capture should never happen on a legal move; fail-safe early return.
            return;
        }
        popBit(bitboards_[static_cast<int>(targetColor)][static_cast<int>(targetPt)], to);

        // Capturing a rook may remove castling rights
        if (targetPt == PieceType::Rook) {
            if (targetColor == Color::White && to == 0)  disableCastle(Color::White, false);
            if (targetColor == Color::White && to == 7)  disableCastle(Color::White, true);
            if (targetColor == Color::Black && to == 56) disableCastle(Color::Black, false);
            if (targetColor == Color::Black && to == 63) disableCastle(Color::Black, true);
        }
    }

    // 5. Handle rook move in castling (king move of two squares)
    if (pt == PieceType::King && std::abs(to - from) == 2) {
        if (color == Color::White && to == 6) { popBit(bitboards_[0][3], 7);  setBit(bitboards_[0][3], 5); }
        if (color == Color::White && to == 2) { popBit(bitboards_[0][3], 0);  setBit(bitboards_[0][3], 3); }
        if (color == Color::Black && to == 62){ popBit(bitboards_[1][3], 63); setBit(bitboards_[1][3], 61); }
        if (color == Color::Black && to == 58){ popBit(bitboards_[1][3], 56); setBit(bitboards_[1][3], 59); }
    }

    // 6. Update en-passant target (only valid one ply)
    int nextEnPassantTarget = -1;
    if (pt == PieceType::Pawn && std::abs(to - from) == 16) {
        nextEnPassantTarget = (from + to) / 2;
    }
    enPassantTarget_ = nextEnPassantTarget;

    // 7. Move the piece (with optional promotion)
    popBit(bitboards_[static_cast<int>(color)][static_cast<int>(pt)], from);
    if (promotion != PieceType::None) {
        setBit(bitboards_[static_cast<int>(color)][static_cast<int>(promotion)], to);
    } else {
        setBit(bitboards_[static_cast<int>(color)][static_cast<int>(pt)], to);
    }

    updateOccupancies();
    zobristKey_ = calculateHash();
}

// =======================
//   GENERATE LEGAL MOVES
// =======================

// Offset tables
const int knightOffsets[] = {-17, -15, -10, -6, 6, 10, 15, 17};
const int kingOffsets[]   = {-9, -8, -7, -1, 1, 7, 8, 9};
const int rookDirs[]   = {-8, 8, -1, 1};
const int bishopDirs[] = {-9, -7, 7, 9};

std::vector<Move> Board::generateLegalMoves(Color turn) const {
    std::vector<Move> moves;
    moves.reserve(35);

    int c = static_cast<int>(turn);
    int opp = c ^ 1;

    Bitboard us   = occupancies_[c];
    Bitboard them = occupancies_[opp];
    Bitboard occ  = occupancies_[2];

    // --- 1. PAWNS ---
    Bitboard pawns = bitboards_[c][static_cast<int>(PieceType::Pawn)];
    int up = (turn == Color::White) ? 8 : -8;
    int startRank     = (turn == Color::White) ? 1 : 6;
    int promotionRank = (turn == Color::White) ? 7 : 0;

    auto addPawnMove = [&](int f, int t) {
        int r = t / 8;
        if (r == promotionRank) {
            moves.emplace_back(f, t, PieceType::Queen);
            moves.emplace_back(f, t, PieceType::Rook);
            moves.emplace_back(f, t, PieceType::Bishop);
            moves.emplace_back(f, t, PieceType::Knight);
        } else {
            moves.emplace_back(f, t);
        }
    };

    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(pawns, sq)) continue;

        int x = sq % 8;
        int y = sq / 8;

        // A. Single push
        int target = sq + up;
        if (target >= 0 && target < 64 && !getBit(occ, target)) {
            addPawnMove(sq, target);

            // B. Double push
            if (y == startRank) {
                int doubleTarget = sq + (up * 2);
                if (!getBit(occ, doubleTarget)) {
                    moves.emplace_back(sq, doubleTarget);
                }
            }
        }

        // C. Captures (standard + en passant)
        int captureOffsets[] = {up - 1, up + 1};
        for (int offset : captureOffsets) {
            int capSq = sq + offset;

            if (capSq < 0 || capSq >= 64) continue;
            int capX = capSq % 8;
            if (std::abs(capX - x) > 1) continue;

            bool isEnemy     = getBit(them, capSq);
            bool isEnPassant = (capSq == enPassantTarget_ && !getBit(occ, capSq));

            // Never capture our own piece
            if (getBit(us, capSq)) continue;

            if (isEnemy || isEnPassant) {
                int r = capSq / 8;

                if (r == promotionRank) {
                    moves.emplace_back(sq, capSq, PieceType::Queen);  moves.back().isCapture = true;
                    moves.emplace_back(sq, capSq, PieceType::Rook);   moves.back().isCapture = true;
                    moves.emplace_back(sq, capSq, PieceType::Bishop); moves.back().isCapture = true;
                    moves.emplace_back(sq, capSq, PieceType::Knight); moves.back().isCapture = true;
                } else {
                    moves.emplace_back(sq, capSq);
                    moves.back().isCapture = true; // en passant is still a capture
                }
            }
        }
    }

    // --- 2. KNIGHTS ---
    Bitboard knights = bitboards_[c][static_cast<int>(PieceType::Knight)];
    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(knights, sq)) continue;
        int x = sq % 8;
        for (int offset : knightOffsets) {
            int target = sq + offset;
            if (target >= 0 && target < 64) {
                int tx = target % 8;
                if (std::abs(tx - x) > 2) continue;
                if (!getBit(us, target)) {
                    Move m(sq, target);
                    if (getBit(them, target)) m.isCapture = true;
                    moves.push_back(m);
                }
            }
        }
    }

    // --- 3. KING ---
    Bitboard king = bitboards_[c][static_cast<int>(PieceType::King)];
    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(king, sq)) continue;
        int x = sq % 8;

        // Normal king moves
        for (int offset : kingOffsets) {
            int target = sq + offset;
            if (target >= 0 && target < 64) {
                int tx = target % 8;
                if (std::abs(tx - x) > 1) continue;
                if (!getBit(us, target)) {
                    Move m(sq, target);
                    if (getBit(them, target)) m.isCapture = true;
                    moves.push_back(m);
                }
            }
        }

        // Castling attempts from king square sq
        auto tryCastling = [&](bool kingSide) {
            if (!canCastle(turn, kingSide)) return;

            int path1, path2, finalKingSq;
            if (turn == Color::White) {
                if (kingSide) { path1 = 5; path2 = 6; finalKingSq = 6; }
                else          { path1 = 3; path2 = 2; finalKingSq = 2; }
            } else {
                if (kingSide) { path1 = 61; path2 = 62; finalKingSq = 62; }
                else          { path1 = 59; path2 = 58; finalKingSq = 58; }
            }

            if (getBit(occupancies_[2], path1)) return;
            if (getBit(occupancies_[2], path2)) return;

            if (isInCheck(turn)) return;
            if (isSquareAttacked(path1, opposite(turn))) return;
            if (isSquareAttacked(path2, opposite(turn))) return;

            moves.emplace_back(sq, finalKingSq);
        };

        tryCastling(true);   // king side
        tryCastling(false);  // queen side
    }

    // --- 4. SLIDING PIECES ---
    auto generateSlidingMoves = [&](PieceType pt, const int* dirs, int numDirs) {
        Bitboard pieces = bitboards_[c][static_cast<int>(pt)];
        for (int sq = 0; sq < 64; ++sq) {
            if (!getBit(pieces, sq)) continue;
            int x = sq % 8;
            int y = sq / 8;

            for (int d = 0; d < numDirs; ++d) {
                int offset = dirs[d];
                int stepX = 0, stepY = 0;
                if (offset == -8) { stepX = 0;  stepY = -1; }
                else if (offset == 8) { stepX = 0;  stepY = 1; }
                else if (offset == -1) { stepX = -1; stepY = 0; }
                else if (offset == 1) { stepX = 1;  stepY = 0; }
                else if (offset == -9) { stepX = -1; stepY = -1; }
                else if (offset == -7) { stepX = 1;  stepY = -1; }
                else if (offset == 7) { stepX = -1; stepY = 1; }
                else if (offset == 9) { stepX = 1;  stepY = 1; }

                int curSq = sq;
                int curX = x;
                int curY = y;

                while (true) {
                    curX += stepX;
                    curY += stepY;
                    curSq = curY * 8 + curX;

                    if (curX < 0 || curX > 7 || curY < 0 || curY > 7) break;
                    if (getBit(us, curSq)) break;

                    Move m(sq, curSq);
                    if (getBit(them, curSq)) {
                        m.isCapture = true;
                        moves.push_back(m);
                        break;
                    }
                    moves.push_back(m);
                }
            }
        }
    };

    generateSlidingMoves(PieceType::Rook,   rookDirs,   4);
    generateSlidingMoves(PieceType::Bishop, bishopDirs, 4);
    generateSlidingMoves(PieceType::Queen,  rookDirs,   4);
    generateSlidingMoves(PieceType::Queen,  bishopDirs, 4);



    // --- 5. FAIRY PIECES ---

    // A. PRINCESSE (Princess / Archbishop) = Bishop + Knight
    // We generate Bishop (Sliding) moves + Knight jumps
    generateSlidingMoves(PieceType::Princess, bishopDirs, 4);
    
    Bitboard princesses = bitboards_[c][static_cast<int>(PieceType::Princess)];
    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(princesses, sq)) continue;
        int x = sq % 8;
        for (int offset : knightOffsets) {
            int target = sq + offset;
            if (target >= 0 && target < 64) {
                if (std::abs((target % 8) - x) > 2) continue; // Wrap guard
                if (!getBit(us, target)) {
                    Move m(sq, target);
                    if (getBit(them, target)) m.isCapture = true;
                    moves.push_back(m);
                }
            }
        }
    }

    // B. IMPÉRATRICE (Empress / Chancellor) = Rook + Knight
    // We generate Rook (Sliding) moves + Knight jumps
    generateSlidingMoves(PieceType::Empress, rookDirs, 4);

    Bitboard empresses = bitboards_[c][static_cast<int>(PieceType::Empress)];
    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(empresses, sq)) continue;
        int x = sq % 8;
        for (int offset : knightOffsets) {
            int target = sq + offset;
            if (target >= 0 && target < 64) {
                if (std::abs((target % 8) - x) > 2) continue; // Wrap guard
                if (!getBit(us, target)) {
                    Move m(sq, target);
                    if (getBit(them, target)) m.isCapture = true;
                    moves.push_back(m);
                }
            }
        }
    }

    // C. NOCTAMBULE (Nightrider)
    // Moves by successive knight jumps in the same direction
    Bitboard nightriders = bitboards_[c][static_cast<int>(PieceType::Nightrider)];
    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(nightriders, sq)) continue;
        
        // For each knight direction
        for (int offset : knightOffsets) {
            int curSq = sq;
            while (true) {
                int prevX = curSq % 8;
                int nextSq = curSq + offset;
                
                // Board boundary checks
                if (nextSq < 0 || nextSq >= 64) break;
                // Wrap-around check (columns a-h): a knight jump changes file by 1 or 2 max
                if (std::abs((nextSq % 8) - prevX) > 2) break;

                // If we encounter a friendly piece, we stop (blocked)
                if (getBit(us, nextSq)) break;

                // Adding the move
                Move m(sq, nextSq);
                if (getBit(them, nextSq)) {
                    m.isCapture = true;
                    moves.push_back(m);
                    break; // Capture stops the movement
                }
                moves.push_back(m);
                
                // If the square was empty, we continue in the same direction (next jump)
                curSq = nextSq;
            }
        }
    }

    // D. SAUTERELLE (Grasshopper)
    // Moves along Queen lines, but must jump over a piece (friendly or enemy)
    // and land on the square immediately behind.
    Bitboard grasshoppers = bitboards_[c][static_cast<int>(PieceType::Grasshopper)];
    const int allDirs[] = {-9, -8, -7, -1, 1, 7, 8, 9};
    
    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(grasshoppers, sq)) continue;
        int x = sq % 8;
        int y = sq / 8;

        for (int offset : allDirs) {
            int curSq = sq;
            int curX = x; 
            int curY = y;
            
            // Calculating step X/Y to properly check board edges
            int stepX = 0, stepY = 0;
            if (offset == -1 || offset == -9 || offset == 7) stepX = -1;
            if (offset == 1  || offset == 9  || offset == -7) stepX = 1;
            if (offset >= -9 && offset <= -7) stepY = -1;
            if (offset >= 7  && offset <= 9)  stepY = 1;
            if (offset == -8) stepY = -1;
            if (offset == 8)  stepY = 1;

            bool foundHurdle = false;

            while (true) {
                curX += stepX;
                curY += stepY;
                curSq = curY * 8 + curX;

                if (curX < 0 || curX > 7 || curY < 0 || curY > 7) break;

                if (!foundHurdle) {
                    // Looking for the hurdle (any piece)
                    if (isSquareOccupied(curSq)) {
                        foundHurdle = true;
                        // We continue the loop ONE more time to land behind
                    }
                } else {
                    // We have already jumped the hurdle, here is the landing square
                    if (!getBit(us, curSq)) { // Empty or enemy square
                        Move m(sq, curSq);
                        if (getBit(them, curSq)) m.isCapture = true;
                        moves.push_back(m);
                    }
                    break; // The grasshopper does not go further
                }
            }
        }
    }

    // --- Filter out moves that leave king in check ---
    std::vector<Move> realLegalMoves;
    realLegalMoves.reserve(moves.size());

    for (const auto& move : moves) {
        Board tempBoard = *this;
        tempBoard.movePiece(move.from, move.to, move.promotion);
        if (!tempBoard.isInCheck(turn)) {
            realLegalMoves.push_back(move);
        }
    }

    return realLegalMoves;
}
std::vector<Move> Board::generateCaptures(Color turn) const {
    std::vector<Move> moves;
    moves.reserve(10); // Captures are usually fewer

    int c = static_cast<int>(turn);
    int opp = c ^ 1;

    Bitboard us   = occupancies_[c];
    Bitboard them = occupancies_[opp];

    // --- 1. PAWNS  ---
    Bitboard pawns = bitboards_[c][static_cast<int>(PieceType::Pawn)];
    int up = (turn == Color::White) ? 8 : -8;
    int promotionRank = (turn == Color::White) ? 7 : 0;

    // Offsets for pawn captures ( -1 and +1 files )
    // White captures: +7 (diag left), +9 (diag right)
    // Black captures: -9 (diag left), -7 (diag right)
    int capOffsets[] = { (turn==Color::White ? 7 : -9), (turn==Color::White ? 9 : -7) };

    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(pawns, sq)) continue;
        
        int r = sq / 8;
        int file = sq % 8;

        for (int offset : capOffsets) {
            int target = sq + offset;
            // Wraparound check
            // If we are on file A (0) and move left -> invalid
            // If we are on file H (7) and move right -> invalid
            int targetFile = target % 8;
            if (std::abs(targetFile - file) > 1) continue; 
            if (target < 0 || target >= 64) continue;

            bool isEnemy = getBit(them, target);
            bool isEnPassant = (target == enPassantTarget_);

            if (isEnemy || isEnPassant) {
                if (r == promotionRank - (turn==Color::White?1:-1)) { // Rank just before promotion
                    // Captures with promotion
                    moves.emplace_back(sq, target, PieceType::Queen);  moves.back().isCapture = true;
                    // We can add R, B, N if we want to be exhaustive, but Q is often enough for quiescence
                } else {
                    moves.emplace_back(sq, target);
                    moves.back().isCapture = true;
                }
            }
        }
    }

    // --- 2. KNIGHTS ---
    Bitboard knights = bitboards_[c][static_cast<int>(PieceType::Knight)];
    const int kOffsets[] = {-17, -15, -10, -6, 6, 10, 15, 17};
    while (knights) {
        int sq = __builtin_ctzll(knights); // Bitboard optimization
        knights &= (knights - 1);
        
        int x = sq % 8;
        for (int offset : kOffsets) {
            int target = sq + offset;
            if (target >= 0 && target < 64) {
                if (std::abs((target % 8) - x) <= 2) {
                    // ONLY DIFFERENCE WITH generateLegalMoves:
                    // We keep only if it's an enemy
                    if (getBit(them, target)) {
                        Move m(sq, target);
                        m.isCapture = true;
                        moves.push_back(m);
                    }
                }
            }
        }
    }

    // --- 3. KING (Captures only, no castling) ---
    Bitboard king = bitboards_[c][static_cast<int>(PieceType::King)];
    if (king) {
        int sq = __builtin_ctzll(king);
        const int kiOffsets[] = {-9, -8, -7, -1, 1, 7, 8, 9};
        int x = sq % 8;
        for (int offset : kiOffsets) {
            int target = sq + offset;
            if (target >= 0 && target < 64 && std::abs((target % 8) - x) <= 1) {
                if (getBit(them, target)) {
                    Move m(sq, target);
                    m.isCapture = true;
                    moves.push_back(m);
                }
            }
        }
    }

    // --- 4. SLIDING PIECES (Bishops, Rooks, Queens) ---
    auto generateSlidingCaptures = [&](PieceType pt, const int* dirs, int numDirs) {
        Bitboard pieces = bitboards_[c][static_cast<int>(pt)];
        while (pieces) {
            int sq = __builtin_ctzll(pieces);
            pieces &= (pieces - 1);
            
            int x = sq % 8; 
            int y = sq / 8;

            for (int d = 0; d < numDirs; ++d) {
                int step = dirs[d];
                // Pre-calculate x/y deltas to avoid modulo operations in the while loop
                int dx = 0, dy = 0;
                if (step == -8) dy = -1; else if (step == 8) dy = 1;
                else if (step == -1) dx = -1; else if (step == 1) dx = 1;
                else if (step == -9) {dx=-1;dy=-1;} else if (step == -7) {dx=1;dy=-1;}
                else if (step == 7) {dx=-1;dy=1;} else if (step == 9) {dx=1;dy=1;}

                int curX = x; 
                int curY = y;
                
                while (true) {
                    curX += dx; curY += dy;
                    if (curX < 0 || curX > 7 || curY < 0 || curY > 7) break;
                    
                    int curSq = curY * 8 + curX;

                    if (getBit(us, curSq)) break; // Blocked by ally

                    if (getBit(them, curSq)) {
                        // It's an enemy -> CAPTURE and STOP
                        Move m(sq, curSq);
                        m.isCapture = true;
                        moves.push_back(m);
                        break; 
                    }
                    // If the square is empty, we continue sliding, BUT we do not add the move
                }
            }
        }
    };

    const int rookDirs[]   = {-8, 8, -1, 1};
    const int bishopDirs[] = {-9, -7, 7, 9};

    generateSlidingCaptures(PieceType::Rook,   rookDirs,   4);
    generateSlidingCaptures(PieceType::Bishop, bishopDirs, 4);
    generateSlidingCaptures(PieceType::Queen,  rookDirs,   4);
    generateSlidingCaptures(PieceType::Queen,  bishopDirs, 4);

    return moves;
}
int Board::getKingSquare(Color c) const {
    Bitboard kingBB = bitboards_[static_cast<int>(c)][static_cast<int>(PieceType::King)];
    if (kingBB == 0) return -1;
#if defined(_MSC_VER)
    // If using MSVC, you'd normally use _BitScanForward64; simplified fallback:
    for (int i = 0; i < 64; ++i)
        if (getBit(kingBB, i)) return i;
    return -1;
#else
    return __builtin_ctzll(kingBB);
#endif
}

bool Board::isInCheck(Color c) const {
    int kingSq = getKingSquare(c);
    if (kingSq == -1) return false;
    return isSquareAttacked(kingSq, opposite(c));
}

bool Board::isSquareAttacked(int square, Color attacker) const {
    Bitboard enemyPawns   = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Pawn)];
    Bitboard enemyKnights = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Knight)];
    Bitboard enemyKing    = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::King)];
    Bitboard enemyRooks   = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Rook)];
    Bitboard enemyBishops = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Bishop)];
    Bitboard enemyQueens  = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Queen)];

    int x = square % 8;

    // 1. Pawn attacks (from attacker towards square)
    if (attacker == Color::White) {
        // White pawns attack +7 and +9 (from pawn POV), so from target square we look at -7 and -9
        int s1 = square - 7;
        int s2 = square - 9;
        if (s1 >= 0 && (s1 % 8) != 0     && getBit(enemyPawns, s1)) return true; // file not 'a'
        if (s2 >= 0 && (s2 % 8) != 7     && getBit(enemyPawns, s2)) return true; // file not 'h'
    } else {
        // Black pawns attack -7 and -9, so from target square we look at +7 and +9
        int s1 = square + 7;
        int s2 = square + 9;
        if (s1 < 64 && (s1 % 8) != 7     && getBit(enemyPawns, s1)) return true; // file not 'h'
        if (s2 < 64 && (s2 % 8) != 0     && getBit(enemyPawns, s2)) return true; // file not 'a'
    }

    // 2. Knight attacks
    const int kOffsets[] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int offset : kOffsets) {
        int target = square + offset;
        if (target >= 0 && target < 64 && std::abs((target % 8) - x) <= 2) {
            if (getBit(enemyKnights, target)) return true;
        }
    }

    // 3. King attacks (adjacent squares)
    const int kiOffsets[] = {-9, -8, -7, -1, 1, 7, 8, 9};
    for (int offset : kiOffsets) {
        int target = square + offset;
        if (target >= 0 && target < 64 && std::abs((target % 8) - x) <= 1) {
            if (getBit(enemyKing, target)) return true;
        }
    }

    // 4. Sliding attacks: rooks/queens on orthogonals
    const int orthoDirs[] = {-8, 8, -1, 1};
    for (int step : orthoDirs) {
        int curr = square;
        while (true) {
            int cx = curr % 8;
            if ((step == 1 && cx == 7) || (step == -1 && cx == 0)) break;

            curr += step;
            if (curr < 0 || curr >= 64) break;

            if (isSquareOccupied(curr)) {
                if (getBit(enemyRooks, curr) || getBit(enemyQueens, curr)) return true;
                break;
            }
        }
    }

    // 5. Sliding attacks: bishops/queens on diagonals
    const int diagDirs[] = {-9, -7, 7, 9};
    for (int step : diagDirs) {
        int curr = square;
        while (true) {
            int cx = curr % 8;
            if ((step == -9 || step == 7) && cx == 0) break; // left edge
            if ((step == 9  || step == -7) && cx == 7) break; // right edge

            curr += step;
            if (curr < 0 || curr >= 64) break;

            if (isSquareOccupied(curr)) {
                if (getBit(enemyBishops, curr) || getBit(enemyQueens, curr)) return true;
                break;
            }
        }
    }

    // --- FAIRY PIECES ATTACKS ---

    Bitboard enemyPrincesses   = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Princess)];
    Bitboard enemyEmpresses    = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Empress)];
    Bitboard enemyNightriders  = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Nightrider)];
    Bitboard enemyGrasshoppers = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Grasshopper)];

    // 6. Princess & Empress (Knight Component)
    // We verify knight-like attacks for both pieces.
    // If we find an enemy Princess or Empress, the square is attacked.
    for (int offset : kOffsets) {
        int target = square + offset;
        if (target >= 0 && target < 64 && std::abs((target % 8) - x) <= 2) {
            if (getBit(enemyPrincesses, target) || getBit(enemyEmpresses, target)) return true;
        }
    }

    // 7. Princess (Bishop Component) & Empress (Rook Component)
    // We reuse the ray logic. 
    // Note: In your existing code (steps 4 and 5), you already check Rooks/Queens and Bishops/Queens.
    // You just need to add Empress to the orthogonal checks and Princess to the diagonal checks.
    
    // A. Update Orthogonal check (for Empress)
    for (int step : orthoDirs) {
        int curr = square;
        while (true) {
            int cx = curr % 8;
            if ((step == 1 && cx == 7) || (step == -1 && cx == 0)) break;
            curr += step;
            if (curr < 0 || curr >= 64) break;
            if (isSquareOccupied(curr)) {
                // ADD: also check enemyEmpresses
                if (getBit(enemyRooks, curr) || getBit(enemyQueens, curr) || getBit(enemyEmpresses, curr)) return true;
                break;
            }
        }
    }

    // B. Update Diagonal check (for Princess)
    for (int step : diagDirs) {
        int curr = square;
        while (true) {
            int cx = curr % 8;
            if ((step == -9 || step == 7) && cx == 0) break;
            if ((step == 9  || step == -7) && cx == 7) break;
            curr += step;
            if (curr < 0 || curr >= 64) break;
            if (isSquareOccupied(curr)) {
                // ADD: also check enemyPrincesses
                if (getBit(enemyBishops, curr) || getBit(enemyQueens, curr) || getBit(enemyPrincesses, curr)) return true;
                break;
            }
        }
    }

    // 8. Nightrider (Noctambule)
    // We look in knight directions, but continue as long as it's empty.
    for (int offset : kOffsets) {
        int curSq = square; // Starting from the attacked square (e.g., our King)
        while (true) {
            int prevX = curSq % 8;
            int nextSq = curSq + offset; // We move along the ray towards the potential attacker
            
            if (nextSq < 0 || nextSq >= 64) break;
            if (std::abs((nextSq % 8) - prevX) > 2) break;

            if (isSquareOccupied(nextSq)) {
                if (getBit(enemyNightriders, nextSq)) return true;
                break; // Blocked by a piece (friendly or other enemy)
            }
            curSq = nextSq;
        }
    }

    // 9. Grasshopper (Sauterelle)
    // A grasshopper attacks `square` if, looking from `square` in one of the 8 directions,
    // we find an obstacle (hopper) AND the square immediately behind contains an enemy grasshopper.
    const int allDirs[] = {-9, -8, -7, -1, 1, 7, 8, 9};
    for (int offset : allDirs) {
        int curSq = square;
        int curX = x;
        int curY = square / 8;

        // Definition of steps
        int stepX = 0, stepY = 0;
        if (offset == -1 || offset == -9 || offset == 7) stepX = -1;
        if (offset == 1  || offset == 9  || offset == -7) stepX = 1;
        if (offset >= -9 && offset <= -7) stepY = -1;
        if (offset >= 7  && offset <= 9)  stepY = 1;
        if (offset == -8) stepY = -1;
        if (offset == 8)  stepY = 1;

        while (true) {
            curX += stepX;
            curY += stepY;
            curSq = curY * 8 + curX;

            if (curX < 0 || curX > 7 || curY < 0 || curY > 7) break;

            if (isSquareOccupied(curSq)) {
                // We found the hopper. Let's look at the square just BEHIND the hopper (in the same direction)
                int behindX = curX + stepX;
                int behindY = curY + stepY;
                int behindSq = behindY * 8 + behindX;

                if (behindX >= 0 && behindX <= 7 && behindY >= 0 && behindY <= 7) {
                    if (getBit(enemyGrasshoppers, behindSq)) return true;
                }
                break; // We stop searching in this direction after the first obstacle
            }
        }
    }

    return false;
}

bool Board::isSquareOccupied(int square) const {
    return getBit(occupancies_[2], square);
}

bool Board::canCastle(Color c, bool kingSide) const {
    if (c == Color::White) return kingSide ? castleRights_[0] : castleRights_[1];
    else                   return kingSide ? castleRights_[2] : castleRights_[3];
}

void Board::disableCastle(Color c, bool kingSide) {
    if (c == Color::White) castleRights_[kingSide ? 0 : 1] = false;
    else                   castleRights_[kingSide ? 2 : 3] = false;
}

// =======================
//   ZOBRIST IMPLEMENTATION
// =======================
void Board::initZobristKeys() {
    if (zInitialized) return;
    
    // 64-bit random number generator (Mersenne Twister)
    std::mt19937_64 rng(123456789);

    for (int c = 0; c < 2; ++c) {
        for (int p = 0; p < 6; ++p) {
            for (int sq = 0; sq < 64; ++sq) {
                zPieceKeys[c][p][sq] = rng();
            }
        }
    }
    for (int sq = 0; sq < 65; ++sq) zEnPassantKeys[sq] = rng();
    for (int k = 0; k < 16; ++k) zCastleKeys[k] = rng();
    zSideKey = rng();

    zInitialized = true;
}

// Optimized function to calculate the hash (uses bitboards as evaluate)
uint64_t Board::calculateHash() const {
    uint64_t hash = 0;

    // 1. Pieces
    for (int c = 0; c < 2; ++c) {
        for (int p = 0; p < 6; ++p) {
            Bitboard bb = bitboards_[c][p];
            while (bb) {
                int sq = __builtin_ctzll(bb);
                hash ^= zPieceKeys[c][p][sq];
                bb &= (bb - 1);
            }
        }
    }

    // 2. En Passant
    if (enPassantTarget_ != -1) {
        hash ^= zEnPassantKeys[enPassantTarget_];
    }

    // 3. Castling
    // Construct an index 0-15 based on the 4 booleans
    int castleMask = 0;
    if (castleRights_[0]) castleMask |= 1; // WK
    if (castleRights_[1]) castleMask |= 2; // WQ
    if (castleRights_[2]) castleMask |= 4; // BK
    if (castleRights_[3]) castleMask |= 8; // BQ
    hash ^= zCastleKeys[castleMask];
    
    return hash;
}



