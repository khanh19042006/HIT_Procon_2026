#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include <vector>

class PathFinder {
public:
    static std::vector<int> findPath(
        Position start,
        Position goal,
        const Map& map
    );
};
