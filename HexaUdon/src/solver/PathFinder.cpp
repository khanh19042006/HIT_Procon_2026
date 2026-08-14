#include "solver/PathFinder.hpp"
#include <queue>
#include <algorithm>

PathResult PathFinder::findPath(
    Position start,
    Position goal,
    const Map& map,
    int maxFuel
) {
    PathResult result;

    if (start == goal) {
        result.found = true;
        return result;
    }

    int H = map.getHeight();
    int W = map.getWidth();
    int totalCells = H * W;

    // Flat arrays for O(1) access — replaces std::map<pair, ...>
    std::vector<int> dist(totalCells, INT_MAX);
    std::vector<int> fuel(totalCells, INT_MAX);
    std::vector<int> prevDir(totalCells, -1);
    std::vector<int> prevCell(totalCells, -1);

    // Priority queue: (total_steps, pos_index)
    using PII = std::pair<int, int>;
    std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;

    int startIdx = map.coordinateToPos(start);
    int goalIdx = map.coordinateToPos(goal);

    dist[startIdx] = 0;
    fuel[startIdx] = 0;
    pq.push({0, startIdx});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue; // Stale entry
        if (u == goalIdx) break;   // Found shortest path

        Position uPos = map.posToCoordinate(u);
        int travelTime = map.getTravelTime(u);  // Steps to leave this cell
        int fuelCost = map.getFuelCost(u);       // Fuel to leave this cell

        for (int dir = 0; dir < 6; ++dir) {
            Position nPos = map.nextPosition(uPos, dir);
            if (!map.canMove(nPos)) continue;

            int v = map.coordinateToPos(nPos);
            int newDist = dist[u] + travelTime;
            int newFuel = fuel[u] + fuelCost;

            // Fuel constraint (only matters for patrol cars; supply pass INT_MAX)
            if (newFuel > maxFuel) continue;

            if (newDist < dist[v]) {
                dist[v] = newDist;
                fuel[v] = newFuel;
                prevDir[v] = dir;
                prevCell[v] = u;
                pq.push({newDist, v});
            }
        }
    }

    // Trace back path
    if (dist[goalIdx] == INT_MAX) {
        result.found = false;
        return result;
    }

    result.found = true;
    result.totalSteps = dist[goalIdx];
    result.totalFuel = fuel[goalIdx];

    int cur = goalIdx;
    while (cur != startIdx) {
        result.directions.push_back(prevDir[cur]);
        cur = prevCell[cur];
    }
    std::reverse(result.directions.begin(), result.directions.end());

    return result;
}
