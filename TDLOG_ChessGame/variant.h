#pragma once
#include <iostream>

#include "board.h"



class Variant{
    std::string name;

    Board getInitialSetup();
    int getBoardSize();
};

