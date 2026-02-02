#include "move.h"

Position operator+(Position p1, Position p2) {
    return {p1.x + p2.x, p1.y + p2.y};
}

Position operator-(Position p1, Position p2) {
    return {p1.x - p2.x, p1.y - p2.y};
}
