#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include <vector>
#include <climits>

struct PathResult {
    std::vector<int> directions;  // Sequence of hex directions (0-5)
    int totalSteps = 0;           // Total travel time steps consumed
    int totalFuel = 0;            // Total fuel consumed (for patrol cars)
    bool found = false;           // Whether a path was found
};

class PathFinder {
public:
    /**
     * @brief Dijkstra pathfinding on hex grid with travel time weights.
     *
     * Edge weight = getTravelTime(current_pos) (steps consumed when moving FROM current cell).
     * Fuel is tracked and constrained by maxFuel (set INT_MAX for supply cars).
     *
     * @param start     Starting coordinate
     * @param goal      Goal coordinate
     * @param map       Map with terrain + traffic info
     * @param maxFuel   Maximum fuel available (INT_MAX for supply cars)
     * @return PathResult with directions, total steps, total fuel, found flag
     */
    static PathResult findPath(
        Position start,
        Position goal,
        const Map& map,
        int maxFuel = INT_MAX
    );
};
