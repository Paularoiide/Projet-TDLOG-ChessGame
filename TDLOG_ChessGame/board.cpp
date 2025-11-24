#include "board.h"
#include <cstring> // pour std::memset
#include <cmath>   // pour std::abs
#include <vector>

// =======================
//     CONSTRUCTEUR
// =======================
Board::Board() {
    // Reset tout à 0
    std::memset(bitboards_, 0, sizeof(bitboards_));
    std::memset(occupancies_, 0, sizeof(occupancies_));

    // --- Initialisation des Blancs ---
    // Pions (Rangée 2 -> index 8-15)
    for (int i = 8; i < 16; ++i) setBit(bitboards_[0][0], i);
    // Tours
    setBit(bitboards_[0][3], 0); setBit(bitboards_[0][3], 7);
    // Cavaliers
    setBit(bitboards_[0][1], 1); setBit(bitboards_[0][1], 6);
    // Fous
    setBit(bitboards_[0][2], 2); setBit(bitboards_[0][2], 5);
    // Dame
    setBit(bitboards_[0][4], 3);
    // Roi
    setBit(bitboards_[0][5], 4);

    // --- Initialisation des Noirs ---
    // Pions (Rangée 7 -> index 48-55)
    for (int i = 48; i < 56; ++i) setBit(bitboards_[1][0], i);
    // Tours
    setBit(bitboards_[1][3], 56); setBit(bitboards_[1][3], 63);
    // Cavaliers
    setBit(bitboards_[1][1], 57); setBit(bitboards_[1][1], 62);
    // Fous
    setBit(bitboards_[1][2], 58); setBit(bitboards_[1][2], 61);
    // Dame
    setBit(bitboards_[1][4], 59);
    // Roi
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
//      MOVE PIECE
// =======================
// C'est cette fonction qui manquait à l'éditeur de liens (linker) !

void Board::movePiece(int from, int to) {
    Color color;
    PieceType pt = getPieceTypeAt(from, color);
    if (pt == PieceType::None) return;

    // Gestion de la capture : supprimer la pièce adverse si elle existe
    Color targetColor;
    PieceType targetPt = getPieceTypeAt(to, targetColor);
    if (targetPt != PieceType::None) {
        // On suppose ici que c'est une capture valide (vérifié par generateLegalMoves avant)
        popBit(bitboards_[static_cast<int>(targetColor)][static_cast<int>(targetPt)], to);
    }

    // Déplacement de notre pièce
    popBit(bitboards_[static_cast<int>(color)][static_cast<int>(pt)], from);
    setBit(bitboards_[static_cast<int>(color)][static_cast<int>(pt)], to);

    updateOccupancies();
}

// =======================
//   GENERATE LEGAL MOVES
// =======================

// Tableaux de décalages
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

    // --- 1. PIONS ---
    Bitboard pawns = bitboards_[c][static_cast<int>(PieceType::Pawn)];
    int up = (turn == Color::White) ? 8 : -8;
    int startRank = (turn == Color::White) ? 1 : 6;
    
    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(pawns, sq)) continue;

        int x = sq % 8;
        int y = sq / 8;

        // A. Avancée simple
        int target = sq + up;
        if (target >= 0 && target < 64 && !getBit(occ, target)) {
            moves.emplace_back(sq, target);
            // B. Avancée double
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

    // --- 2. CAVALIERS ---
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

    // --- 3. ROI ---
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

    // --- 4. PIÈCES GLISSANTES ---
    auto generateSlidingMoves = [&](PieceType pt, const int* dirs, int numDirs) {
        Bitboard pieces = bitboards_[c][static_cast<int>(pt)];
        for (int sq = 0; sq < 64; ++sq) {
            if (!getBit(pieces, sq)) continue;
            int x = sq % 8;
            int y = sq / 8;

            for (int d = 0; d < numDirs; ++d) {
                int offset = dirs[d];
                // Direction manuelle pour ray casting
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
                    if (getBit(us, curSq)) break; // Bloqué par ami

                    Move m(sq, curSq);
                    if (getBit(them, curSq)) {
                        m.isCapture = true;
                        moves.push_back(m);
                        break; // Bloqué par ennemi (après capture)
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

    return moves;
}