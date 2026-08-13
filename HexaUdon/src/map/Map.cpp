#include "map/Map.hpp"

Map::Map(int height, int width, const std::vector<std::vector<int>>& cells)
    : height(height), width(width), cells(cells) {}

bool Map::isInside(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool Map::isInside(Position pos) const {
    return isInside(pos.x, pos.y);
}

bool Map::canMove(int x, int y) const {
    if (!isInside(x, y)) return false;
    // Terrain 3 is lake (pond), which cannot be entered according to spec
    return cells[y][x] != 3;
}

bool Map::canMove(Position pos) const {
    return canMove(pos.x, pos.y);
}

int Map::getCell(int x, int y) const {
    if (!isInside(x, y)) return -1;
    return cells[y][x];
}

int Map::getCell(Position pos) const {
    return getCell(pos.x, pos.y);
}

int Map::getHeight() const {
    return height;
}

int Map::getWidth() const {
    return width;
}

Position Map::posToCoordinate(int pos) const {
    if (width == 0) return {0, 0};
    return {pos % width, pos / width};
}

int Map::coordinateToPos(Position position) const {
    return position.y * width + position.x;
}

Position Map::nextPosition(Position current, int direction) const {
    // Hexagonal grid directions (0: top-left, 1: top-right, 2: right, 3: bottom-right, 4: bottom-left, 5: left)
    // Offset depends on whether row y is even or odd
    static const int dx_even[6] = {-1,  0, 1,  0, -1, -1};
    static const int dy_even[6] = {-1, -1, 0,  1,  1,  0};

    static const int dx_odd[6]  = { 0,  1, 1,  1,  0, -1};
    static const int dy_odd[6]  = {-1, -1, 0,  1,  1,  0};

    if (direction < 0 || direction >= 6) {
        return current;
    }

    bool isOdd = (current.y % 2 != 0);
    int nx = current.x + (isOdd ? dx_odd[direction] : dx_even[direction]);
    int ny = current.y + (isOdd ? dy_odd[direction] : dy_even[direction]);

    return {nx, ny};
}
