#include "board.h"
#include <cstring> // for std::memset
#include <cmath>   // for std::abs
#include <vector>

Board::Board(Variant v) {
    // 1. Reset everything to 0
    std::memset(bitboards_, 0, sizeof(bitboards_));
    std::memset(occupancies_, 0, sizeof(occupancies_));

    // Définition des indices
    const int PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5;
    const int PRINCESS = 6, EMPRESS = 7, NIGHTRIDER = 8, GRASSHOPPER = 9;

    // ==========================================
    // 1. PLACEMENT COMMUN (Pions, Tours, Dame, Roi)
    // ==========================================

    // --- White ---
    for (int i = 8; i < 16; ++i) setBit(bitboards_[0][PAWN], i); // Pions rangée 2
    setBit(bitboards_[0][ROOK], 0); setBit(bitboards_[0][ROOK], 7); // Tours a1, h1
    setBit(bitboards_[0][QUEEN], 3); // Dame d1
    setBit(bitboards_[0][KING], 4);  // Roi e1

    // --- Black ---
    for (int i = 48; i < 56; ++i) setBit(bitboards_[1][PAWN], i); // Pions rangée 7
    setBit(bitboards_[1][ROOK], 56); setBit(bitboards_[1][ROOK], 63); // Tours a8, h8
    setBit(bitboards_[1][QUEEN], 59); // Dame d8
    setBit(bitboards_[1][KING], 60);  // Roi e8


    // ==========================================
    // 2. BRANCHEMENT SELON LA VARIANTE
    // ==========================================

    if (v == Variant::FairyChess) {
        // --- MODE FÉERIQUE ---
        // Dans ce mode, on REMPLACE les Cavaliers/Fous par Nightrider/Princess/Empress
        // et on ajoute des Sauterelles.

        // WHITE
        setBit(bitboards_[0][NIGHTRIDER], 1); // b1 (remplace Cavalier)
        setBit(bitboards_[0][PRINCESS], 2);   // c1 (remplace Fou)
        setBit(bitboards_[0][EMPRESS], 5);    // f1 (remplace Fou)
        setBit(bitboards_[0][NIGHTRIDER], 6); // g1 (remplace Cavalier)


        // BLACK
        setBit(bitboards_[1][NIGHTRIDER], 57); // b8
        setBit(bitboards_[1][PRINCESS], 58);   // c8
        setBit(bitboards_[1][EMPRESS], 61);    // f8
        setBit(bitboards_[1][NIGHTRIDER], 62); // g8


    } else {
        // --- MODE CLASSIQUE ---
        // Configuration standard

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

    // A. PRINCESSE (Princess / Archbishop) = Fou + Cavalier
    // On génère les mouvements de Fou (Sliding) + les sauts de Cavalier
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

    // B. IMPÉRATRICE (Empress / Chancellor) = Tour + Cavalier
    // On génère les mouvements de Tour (Sliding) + les sauts de Cavalier
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
    // Se déplace par bonds de cavalier successifs dans la même direction
    Bitboard nightriders = bitboards_[c][static_cast<int>(PieceType::Nightrider)];
    for (int sq = 0; sq < 64; ++sq) {
        if (!getBit(nightriders, sq)) continue;
        
        // Pour chaque direction de cavalier
        for (int offset : knightOffsets) {
            int curSq = sq;
            while (true) {
                int prevX = curSq % 8;
                int nextSq = curSq + offset;
                
                // Vérifications de sortie de plateau
                if (nextSq < 0 || nextSq >= 64) break;
                // Vérification de wrapping (colonnes a-h) : un saut de cavalier change de file de 1 ou 2 max
                if (std::abs((nextSq % 8) - prevX) > 2) break;

                // Si on rencontre une pièce amie, on s'arrête (bloqué)
                if (getBit(us, nextSq)) break;

                // Ajout du coup
                Move m(sq, nextSq);
                if (getBit(them, nextSq)) {
                    m.isCapture = true;
                    moves.push_back(m);
                    break; // Capture arrête le mouvement
                }
                moves.push_back(m);
                
                // Si la case était vide, on continue dans la même direction (bond suivant)
                curSq = nextSq;
            }
        }
    }

    // D. SAUTERELLE (Grasshopper)
    // Se déplace sur les lignes de la Dame, mais doit sauter par-dessus une pièce (amie ou ennemie)
    // et atterrir sur la case juste derrière.
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
            
            // Calcul des pas X/Y pour vérifier les bords proprement
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
                    // On cherche le sautoir (n'importe quelle pièce)
                    if (isSquareOccupied(curSq)) {
                        foundHurdle = true;
                        // On continue la boucle UNE fois de plus pour atterrir derrière
                    }
                } else {
                    // On a déjà sauté le sautoir, voici la case d'arrivée
                    if (!getBit(us, curSq)) { // Case vide ou ennemi
                        Move m(sq, curSq);
                        if (getBit(them, curSq)) m.isCapture = true;
                        moves.push_back(m);
                    }
                    break; // La sauterelle ne va pas plus loin
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
    // White captures: +7 (diag gauche), +9 (diag droite)
    // Black captures: -9 (diag gauche), -7 (diag droite)
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

    // 6. Princess & Empress (Composante Cavalier)
    // On vérifie les cases de saut de cavalier autour de `square`
    // Si on y trouve une Princesse ou une Impératrice ennemie, on est attaqué.
    for (int offset : kOffsets) {
        int target = square + offset;
        if (target >= 0 && target < 64 && std::abs((target % 8) - x) <= 2) {
            if (getBit(enemyPrincesses, target) || getBit(enemyEmpresses, target)) return true;
        }
    }

    // 7. Princess (Composante Fou) & Empress (Composante Tour)
    // On réutilise la logique de rayon. 
    // Note: Dans ton code existant (étape 4 et 5), tu vérifies déjà Rooks/Queens et Bishops/Queens.
    // Il suffit d'ajouter Empress aux checks orthogonaux et Princess aux checks diagonaux.
    
    // A. Mise à jour check Orthogonal (pour Impératrice)
    for (int step : orthoDirs) {
        int curr = square;
        while (true) {
            int cx = curr % 8;
            if ((step == 1 && cx == 7) || (step == -1 && cx == 0)) break;
            curr += step;
            if (curr < 0 || curr >= 64) break;
            if (isSquareOccupied(curr)) {
                // AJOUT: on vérifie aussi enemyEmpresses
                if (getBit(enemyRooks, curr) || getBit(enemyQueens, curr) || getBit(enemyEmpresses, curr)) return true;
                break;
            }
        }
    }

    // B. Mise à jour check Diagonal (pour Princesse)
    for (int step : diagDirs) {
        int curr = square;
        while (true) {
            int cx = curr % 8;
            if ((step == -9 || step == 7) && cx == 0) break;
            if ((step == 9  || step == -7) && cx == 7) break;
            curr += step;
            if (curr < 0 || curr >= 64) break;
            if (isSquareOccupied(curr)) {
                // AJOUT: on vérifie aussi enemyPrincesses
                if (getBit(enemyBishops, curr) || getBit(enemyQueens, curr) || getBit(enemyPrincesses, curr)) return true;
                break;
            }
        }
    }

    // 8. Nightrider (Noctambule)
    // On regarde dans les directions de cavalier, mais on continue tant que c'est vide.
    for (int offset : kOffsets) {
        int curSq = square; // On part de la case attaquée (ex: notre Roi)
        while (true) {
            int prevX = curSq % 8;
            int nextSq = curSq + offset; // On remonte le rayon vers l'attaquant potentiel
            
            if (nextSq < 0 || nextSq >= 64) break;
            if (std::abs((nextSq % 8) - prevX) > 2) break;

            if (isSquareOccupied(nextSq)) {
                if (getBit(enemyNightriders, nextSq)) return true;
                break; // Bloqué par une pièce (amie ou autre ennemie)
            }
            curSq = nextSq;
        }
    }

    // 9. Grasshopper (Sauterelle)
    // Une sauterelle attaque `square` si, en regardant depuis `square` dans une des 8 directions,
    // on trouve un obstacle (sautoir) ET que la case immédiatement derrière contient une sauterelle ennemie.
    const int allDirs[] = {-9, -8, -7, -1, 1, 7, 8, 9};
    for (int offset : allDirs) {
        int curSq = square;
        int curX = x;
        int curY = square / 8;

        // Définition des pas
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
                // On a trouvé le sautoir. Regardons la case juste DERRIÈRE le sautoir (dans la même direction)
                int behindX = curX + stepX;
                int behindY = curY + stepY;
                int behindSq = behindY * 8 + behindX;

                if (behindX >= 0 && behindX <= 7 && behindY >= 0 && behindY <= 7) {
                    if (getBit(enemyGrasshoppers, behindSq)) return true;
                }
                break; // On arrête de chercher dans cette direction après le premier obstacle
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
