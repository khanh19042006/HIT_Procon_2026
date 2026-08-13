#pragma once

#include <vector>

struct Position {
    int x;
    int y;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

struct Agent {
    int kind; // 0: 巡回車 (patrol car), 1: 補給車 (supply car)
    int pos;  // Cell index (y * width + x)
    int fuel;
};
