#pragma once

#include <vector>
#include "model/Agent.hpp"

class Map {
private:
    int height;
    int width;
    std::vector<std::vector<int>> cells;

public:
    Map() : height(0), width(0) {}
    Map(int height, int width, const std::vector<std::vector<int>>& cells);

    bool isInside(int x, int y) const;
    bool isInside(Position pos) const;

    bool canMove(int x, int y) const;
    bool canMove(Position pos) const;

    int getCell(int x, int y) const;
    int getCell(Position pos) const;

    int getHeight() const;
    int getWidth() const;

    Position posToCoordinate(int pos) const;
    int coordinateToPos(Position position) const;

    Position nextPosition(Position current, int direction) const;
};