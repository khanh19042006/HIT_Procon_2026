#include "map/Map.hpp"

Map::Map(int height, int width, const std::vector<std::vector<int>>& cells)
    : height(height), width(width), cells(cells),
      trafficStatus_(height * width, 0) {}

bool Map::isInside(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool Map::isInside(Position pos) const {
    return isInside(pos.x, pos.y);
}

bool Map::canMove(int x, int y) const {
    if (!isInside(x, y)) return false;
    // Terrain 3 is pond, which cannot be entered
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
    // Hexagonal grid directions:
    // 0: top-left, 1: top-right, 2: right, 3: bottom-right, 4: bottom-left, 5: left
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

// === NEW: Traffic, Travel Time, Fuel Cost ===

void Map::updateTraffic(const std::vector<Traffic>& traffics) {
    // Reset all to smooth (0)
    std::fill(trafficStatus_.begin(), trafficStatus_.end(), 0);
    // Apply current traffic data
    for (const auto& t : traffics) {
        if (t.pos >= 0 && t.pos < static_cast<int>(trafficStatus_.size())) {
            trafficStatus_[t.pos] = t.status;
        }
    }
}

int Map::getTrafficStatus(int pos) const {
    if (pos < 0 || pos >= static_cast<int>(trafficStatus_.size())) return 0;
    return trafficStatus_[pos];
}

int Map::getTravelTime(int pos) const {
    Position p = posToCoordinate(pos);
    if (!isInside(p)) return 999;
    int terrain = cells[p.y][p.x];
    switch (terrain) {
        case 0: return 2;  // Plain: 2 steps
        case 1: {          // Road: depends on traffic
            int ts = trafficStatus_[pos];
            if (ts == 0) return 1;  // Smooth: 1 step
            if (ts == 1) return 2;  // Congested: 2 steps
            return 4;               // Jammed: 4 steps
        }
        case 2: return 3;  // Mountain: 3 steps
        default: return 999; // Pond: cannot move
    }
}

int Map::getTravelTime(Position pos) const {
    return getTravelTime(coordinateToPos(pos));
}

int Map::getFuelCost(int pos) const {
    Position p = posToCoordinate(pos);
    if (!isInside(p)) return 999;
    int terrain = cells[p.y][p.x];
    switch (terrain) {
        case 0: return 1;  // Plain: 1 fuel
        case 1: return 2;  // Road: 2 fuel (regardless of traffic)
        case 2: return 2;  // Mountain: 2 fuel
        default: return 999; // Pond
    }
}

int Map::getFuelCost(Position pos) const {
    return getFuelCost(coordinateToPos(pos));
}
