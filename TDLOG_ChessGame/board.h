#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include "piece.h"
#include "move.h"

// Un Bitboard est juste un entier 64 bits non signé
using Bitboard = uint64_t;

class Board {
    // 2 couleurs, 6 types de pièces
    // bitboards[0][0] = White Pawns, bitboards[1][5] = Black King, etc.
    Bitboard bitboards_[2][6]; 
    
    // Bitboards utilitaires (mis à jour à chaque coup)
    Bitboard occupancies_[3]; // 0: White, 1: Black, 2: Both

public:
    Board(); // Initialise la position de départ standard

    // Accesseurs rapides
    Bitboard getBitboard(Color c, PieceType pt) const { 
        return bitboards_[static_cast<int>(c)][static_cast<int>(pt)]; 
    }

    PieceType getPieceTypeAt(int square, Color& color) const;
    bool isSquareOccupied(int square) const;

    // Gestion des bits
    static void setBit(Bitboard& bb, int square) { bb |= (1ULL << square); }
    static void popBit(Bitboard& bb, int square) { bb &= ~(1ULL << square); }
    static bool getBit(Bitboard bb, int square) { return (bb & (1ULL << square)); }

    // Logique de jeu
    void movePiece(int from, int to);
    
    // Génération de coups (Exemple simplifié)
    std::vector<Move> generateLegalMoves(Color turn) const;

    // Mise à jour des occurences globales
    void updateOccupancies();
    
    // Pour l'affichage
    void print() const;
};