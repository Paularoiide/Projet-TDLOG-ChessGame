#include "board.h"
#include <cstring> // pour memset

Board::Board() {
    // Reset tout à 0
    std::memset(bitboards_, 0, sizeof(bitboards_));
    std::memset(occupancies_, 0, sizeof(occupancies_));

    // Initialisation standard (Exemple pour les Blancs)
    // Pions blancs sur la rangée 1 (index 8 à 15)
    for (int i = 8; i < 16; ++i) setBit(bitboards_[0][0], i);
    // Pions noirs (48 à 55)
    for (int i = 48; i < 56; ++i) setBit(bitboards_[1][0], i);

    // Initialisation Rooks, Knights, etc... (simplifié ici pour l'exemple)
    // Rooks
    setBit(bitboards_[0][3], 0); setBit(bitboards_[0][3], 7);
    setBit(bitboards_[1][3], 56); setBit(bitboards_[1][3], 63);
    // Kings
    setBit(bitboards_[0][5], 4); 
    setBit(bitboards_[1][5], 60);
    // Knights
    setBit(bitboards_[0][1], 1); setBit(bitboards_[0][1], 6);
    setBit(bitboards_[1][1], 57); setBit(bitboards_[1][1], 62);
    // Bishops
    setBit(bitboards_[0][2], 2); setBit(bitboards_[0][2], 5);
    setBit(bitboards_[1][2], 58); setBit(bitboards_[1][2], 61);
    // Queens
    setBit(bitboards_[0][4], 3);
    setBit(bitboards_[1][4], 59);

    updateOccupancies();
}

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

void Board::movePiece(int from, int to) {
    Color color;
    PieceType pt = getPieceTypeAt(from, color);
    if (pt == PieceType::None) return;

    // Gestion de la capture (si destination occupée par l'adversaire)
    Color targetColor;
    PieceType targetPt = getPieceTypeAt(to, targetColor);
    if (targetPt != PieceType::None) {
        popBit(bitboards_[static_cast<int>(targetColor)][static_cast<int>(targetPt)], to);
    }

    // Déplacement
    popBit(bitboards_[static_cast<int>(color)][static_cast<int>(pt)], from);
    setBit(bitboards_[static_cast<int>(color)][static_cast<int>(pt)], to);

    updateOccupancies();
}

// Génération de coups SIMPLIFIÉE (pour démonstration)
std::vector<Move> Board::generateLegalMoves(Color turn) const {
    std::vector<Move> moves;
    int c = static_cast<int>(turn);
    Bitboard us = occupancies_[c];
    Bitboard them = occupancies_[c ^ 1]; // 0->1, 1->0

    // Exemple : Génération des pions (Avancée simple de 1 case)
    Bitboard pawns = bitboards_[c][static_cast<int>(PieceType::Pawn)];
    // Direction: Blancs (+8), Noirs (-8)
    int offset = (turn == Color::White) ? 8 : -8;
    
    // On itère sur tous les pions (méthode bitboard naive)
    for (int sq = 0; sq < 64; ++sq) {
        if (getBit(pawns, sq)) {
            int target = sq + offset;
            if (target >= 0 && target < 64 && !getBit(occupancies_[2], target)) {
                moves.emplace_back(sq, target);
            }
            // TODO: Ajouter captures, double push, etc.
        }
    }
    
    // TODO: Ajouter les Cavaliers, Fous, Tours, Dames, Rois...
    // Avec les bitboards, on utilise souvent des tables pré-calculées pour ça.
    
    return moves;
}