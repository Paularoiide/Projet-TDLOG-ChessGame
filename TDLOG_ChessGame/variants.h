#pragma once
#include <memory>
#include "rules.h"
#include "move.h"


class Board;
class Variant {
public:
    virtual ~Variant() = default;
    virtual int boardSize() const = 0;
    virtual void initialSetup(Board& board) const = 0;
    virtual bool isLegalMove(const Board& board, const Move& move, Color sideToMove) const = 0; // hook pour roque, en passant, etc.
};


class ClassicVariant : public Variant {
public:
    int boardSize() const override { return 8; }
    void initialSetup(Board& board) const override;
    bool isLegalMove(const Board& board, const Move& move, Color sideToMove) const override;
};

