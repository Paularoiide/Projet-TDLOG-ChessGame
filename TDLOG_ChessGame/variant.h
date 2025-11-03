#pragma once
#include "board.h"

class Variant{
    std::string name;

    Board getInitialSetup();
    int getBoardSize();
};

