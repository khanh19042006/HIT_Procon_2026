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

/**
 * @brief Single-Source Shortest Path result.
 * Contains distance/fuel/parent arrays for ALL cells from a single source.
 */
struct SSSPResult {
    std::vector<int> dist;      // dist[pos] = shortest steps from source
    std::vector<int> fuel;      // fuel[pos] = fuel consumed to reach pos
    std::vector<int> prevDir;   // prevDir[pos] = direction taken to reach pos
    std::vector<int> prevCell;  // prevCell[pos] = previous cell index
    int sourcePos = -1;         // Source position index

    // Extract path from source to goalPos
    PathResult extractPath(int goalPos) const;
};

class PathFinder {
public:
    /**
     * @brief Dijkstra pathfinding on hex grid with travel time weights.
     */
    static PathResult findPath(
        Position start,
        Position goal,
        const Map& map,
        int maxFuel = INT_MAX
    );

    /**
     * @brief Single-Source Shortest Path — run Dijkstra ONCE from source,
     *        compute distances to ALL reachable cells.
     *
     * Much faster than calling findPath() N times for the same source.
     *
     * @param source   Starting position
     * @param map      Map with terrain + traffic info
     * @param maxFuel  Fuel limit (INT_MAX for supply cars)
     * @return SSSPResult with dist/fuel/prev arrays for all cells
     */
    static SSSPResult computeSSSP(
        Position source,
        const Map& map,
        int maxFuel = INT_MAX
    );
};
