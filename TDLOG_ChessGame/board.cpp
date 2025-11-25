#include "board.h"
#include <cstring> // for std::memset
#include <cmath>   // for std::abs
#include <vector>

// =======================
//       CONSTRUCTOR
// =======================
Board::Board() {
    // Reset everything to 0
    std::memset(bitboards_, 0, sizeof(bitboards_));
    std::memset(occupancies_, 0, sizeof(occupancies_));

    // --- White initialization ---
    // Pawns (Rank 2 -> index 8-15)
    for (int i = 8; i < 16; ++i) setBit(bitboards_[0][0], i);
    // Rooks
    setBit(bitboards_[0][3], 0); setBit(bitboards_[0][3], 7);
    // Knights
    setBit(bitboards_[0][1], 1); setBit(bitboards_[0][1], 6);
    // Bishops
    setBit(bitboards_[0][2], 2); setBit(bitboards_[0][2], 5);
    // Queen
    setBit(bitboards_[0][4], 3);
    // King
    setBit(bitboards_[0][5], 4);

    // --- Black initialization ---
    // Pawns (Rank 7 -> index 48-55)
    for (int i = 48; i < 56; ++i) setBit(bitboards_[1][0], i);
    // Rooks
    setBit(bitboards_[1][3], 56); setBit(bitboards_[1][3], 63);
    // Knights
    setBit(bitboards_[1][1], 57); setBit(bitboards_[1][1], 62);
    // Bishops
    setBit(bitboards_[1][2], 58); setBit(bitboards_[1][2], 61);
    // Queen
    setBit(bitboards_[1][4], 59);
    // King
    setBit(bitboards_[1][5], 60);

    updateOccupancies();
}

// =======================
//        HELPERS
// =======================

void Board::updateOccupancies() {
    occupancies_[0] = 0; // White
    occupancies_[1] = 0; // Black
    occupancies_[2] = 0; // Both

    for (int p = 0; p < 6; ++p) {
        occupancies_[0] |= bitboards_[0][p];
        occupancies_[1] |= bitboards_[1][p];
    }
    occupancies_[2] = occupancies_[0] | occupancies_[1];
}

PieceType Board::getPieceTypeAt(int square, Color& color) const {
    for (int c = 0; c < 2; ++c) {
        for (int p = 0; p < 6; ++p) {
            if (getBit(bitboards_[c][p], square)) {
                color = static_cast<Color>(c);
                return static_cast<PieceType>(p);
            }
        }
    }
    color = Color::None;
    return PieceType::None;
}

// =======================
//        MOVE PIECE
// =======================
// This is the function that was missing for the linker!

void Board::movePiece(int from, int to) {
    Color color;
    PieceType pt = getPieceTypeAt(from, color);
    if (pt == PieceType::None) return;

    // Capture handling: remove the opponent piece if there is one
    Color targetColor;
    PieceType targetPt = getPieceTypeAt(to, targetColor);
    if (targetPt != PieceType::None) {
        // We assume this is a valid capture (validated beforehand by generateLegalMoves)
        popBit(bitboards_[static_cast<int>(targetColor)][static_cast<int>(targetPt)], to);
    }

    // Move our piece
    popBit(bitboards_[static_cast<int>(color)][static_cast<int>(pt)], from);
    setBit(bitboards_[static_cast<int>(color)][static_cast<int>(pt)], to);

    updateOccupancies();
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

    Bitboard us = occupancies_[c];
    Bitboard them = occupancies_[opp];
    Bitboard occ = occupancies_[2];

    // --- 1. PAWNS ---
    Bitboard pawns = bitboards_[c][static_cast<int>(PieceType::Pawn)];
    int up = (turn == Color::White) ? 8 : -8;
    int startRank = (turn == Color::White) ? 1 : 6;

    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(pawns, sq)) continue;

        int x = sq % 8;
        int y = sq / 8;

        // A. Single push
        int target = sq + up;
        if (target >= 0 && target < 64 && !getBit(occ, target)) {
            moves.emplace_back(sq, target);
            // B. Double push
            if (y == startRank) {
                int doubleTarget = sq + (up * 2);
                if (!getBit(occ, doubleTarget)) {
                    moves.emplace_back(sq, doubleTarget);
                }
            }
        }
        // C. Captures
        int captureOffsets[] = {up - 1, up + 1};
        for (int offset : captureOffsets) {
            int capSq = sq + offset;
            if (capSq < 0 || capSq >= 64) continue;
            int capX = capSq % 8;
            if (std::abs(capX - x) > 1) continue;

            if (getBit(them, capSq)) {
                Move m(sq, capSq);
                m.isCapture = true;
                moves.push_back(m);
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
                // Manual direction extraction for ray casting
                int stepX = 0, stepY = 0;
                if (offset == -8) { stepX=0; stepY=-1; }
                else if (offset == 8) { stepX=0; stepY=1; }
                else if (offset == -1) { stepX=-1; stepY=0; }
                else if (offset == 1) { stepX=1; stepY=0; }
                else if (offset == -9) { stepX=-1; stepY=-1; }
                else if (offset == -7) { stepX=1; stepY=-1; }
                else if (offset == 7) { stepX=-1; stepY=1; }
                else if (offset == 9) { stepX=1; stepY=1; }

                int curSq = sq;
                int curX = x;
                int curY = y;

                while (true) {
                    curX += stepX;
                    curY += stepY;
                    curSq = curY * 8 + curX;

                    if (curX < 0 || curX > 7 || curY < 0 || curY > 7) break;
                    if (getBit(us, curSq)) break; // Blocked by friendly piece

                    Move m(sq, curSq);
                    if (getBit(them, curSq)) {
                        m.isCapture = true;
                        moves.push_back(m);
                        break; // Blocked by enemy after capture
                    }
                    moves.push_back(m);
                }
            }
        }
    };

    generateSlidingMoves(PieceType::Rook, rookDirs, 4);
    generateSlidingMoves(PieceType::Bishop, bishopDirs, 4);
    generateSlidingMoves(PieceType::Queen, rookDirs, 4);
    generateSlidingMoves(PieceType::Queen, bishopDirs, 4);

    std::vector<Move> realLegalMoves;
    realLegalMoves.reserve(moves.size());

    for (const auto& move : moves) {
        // 1. Copy the current board
        Board tempBoard = *this; // Copy by value (very fast, only integers)

        // 2. Play the move on the copy
        tempBoard.movePiece(move.from, move.to);

        // 3. Check if my king is in check after this move
        // (Note: movePiece updates occupancies, so tempBoard is up to date)
        if (!tempBoard.isInCheck(turn)) {
            realLegalMoves.push_back(move);
        }
    }

    return realLegalMoves;
}

int Board::getKingSquare(Color c) const {
    Bitboard kingBB = bitboards_[static_cast<int>(c)][static_cast<int>(PieceType::King)];
    // __builtin_ctzll is a fast GCC/Clang builtin to find the first set bit
    // If you are on Windows (MSVC), use _BitScanForward64 or a manual loop
    if (kingBB == 0) return -1; // Error case (no king)
    return __builtin_ctzll(kingBB);
}

bool Board::isInCheck(Color c) const {
    int kingSq = getKingSquare(c);
    if (kingSq == -1) return false;
    return isSquareAttacked(kingSq, opposite(c)); // Is it attacked by the opponent?
}

bool Board::isSquareAttacked(int square, Color attacker) const {
    Bitboard enemyPawns   = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Pawn)];
    Bitboard enemyKnights = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Knight)];
    Bitboard enemyKing    = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::King)];
    Bitboard enemyRooks   = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Rook)];
    Bitboard enemyBishops = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Bishop)];
    Bitboard enemyQueens  = bitboards_[static_cast<int>(attacker)][static_cast<int>(PieceType::Queen)];

    // 1. Pawn attacks
    // If the attacker is White, its pawns attack upwards (+7, +9).
    // So from the target square, we look downwards (-7, -9) for a white pawn.
    int pawnDir = (attacker == Color::White) ? -1 : 1;
    // Note: pawnDir is reversed here because we look from the attacked square backwards

    int x = square % 8;
    // Left diagonal (from the pawn's point of view)
    int attackSq1 = square + (pawnDir * 8) - 1;
    if (attackSq1 >= 0 && attackSq1 < 64 && std::abs((attackSq1 % 8) - x) == 1) {
        if (getBit(enemyPawns, attackSq1)) return true;
    }
    // Right diagonal
    int attackSq2 = square + (pawnDir * 8) + 1;
    if (attackSq2 >= 0 && attackSq2 < 64 && std::abs((attackSq2 % 8) - x) == 1) {
        if (getBit(enemyPawns, attackSq2)) return true;
    }

    // 2. Knight attacks
    // Reuse the offsets defined above (make sure they are visible or redeclare them)
    const int kOffsets[] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int offset : kOffsets) {
        int target = square + offset;
        if (target >= 0 && target < 64 && std::abs((target % 8) - x) <= 2) {
            if (getBit(enemyKnights, target)) return true;
        }
    }

    // 3. King attacks (to prevent two kings from being adjacent)
    const int kiOffsets[] = {-9, -8, -7, -1, 1, 7, 8, 9};
    for (int offset : kiOffsets) {
        int target = square + offset;
        if (target >= 0 && target < 64 && std::abs((target % 8) - x) <= 1) {
            if (getBit(enemyKing, target)) return true;
        }
    }

    // 4. Sliding attacks (Rooks/Queens and Bishops/Queens)
    // We cast rays from 'square'. If we hit a piece:
    // - If it's an enemy Rook/Queen (orthogonal ray) -> TRUE
    // - If it's an enemy Bishop/Queen (diagonal ray) -> TRUE
    // - Otherwise -> STOP (blocked)

    // Orthogonal (Rooks + Queens)
    const int orthoDirs[] = {-8, 8, -1, 1};
    for (int step : orthoDirs) {
        int curr = square;
        while (true) {
            // Edge checks (ugly but necessary without a 10x12 mailbox)
            int cx = curr % 8;
            if ((step == 1 && cx == 7) || (step == -1 && cx == 0)) break;

            curr += step;
            if (curr < 0 || curr >= 64) break;

            if (isSquareOccupied(curr)) {
                if (getBit(enemyRooks, curr) || getBit(enemyQueens, curr)) return true;
                break; // Blocked by another piece (friendly or non-attacking enemy)
            }
        }
    }

    // Diagonals (Bishops + Queens)
    const int diagDirs[] = {-9, -7, 7, 9};
    for (int step : diagDirs) {
        int curr = square;
        while (true) {
            int cx = curr % 8;
            // Precise edge checks for diagonals
            if ((step == -9 || step == 7) && cx == 0) break; // Left edge
            if ((step == 9 || step == -7) && cx == 7) break; // Right edge

            curr += step;
            if (curr < 0 || curr >= 64) break;

            if (isSquareOccupied(curr)) {
                if (getBit(enemyBishops, curr) || getBit(enemyQueens, curr)) return true;
                break;
            }
        }
    }

    return false;
}

bool Board::isSquareOccupied(int square) const {
    // occupancies_[2] contains the union of white and black pieces
    return getBit(occupancies_[2], square);
}
