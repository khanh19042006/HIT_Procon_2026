#include "solver/SpotScorer.hpp"
#include <climits>

// =============================================================================
// Chấm điểm 1 Spot cụ thể
// =============================================================================

int SpotScorer::scoreSpot(
    int brand,
    int distanceSteps,
    const std::set<int>& collectedBrands,
    int remainingStock
) {
    if (remainingStock <= 0) return -1;

    int score = 0;

    // Brand diversity: massive bonus for new brand types (Tiêu chí 1 quan trọng nhất)
    if (collectedBrands.find(brand) == collectedBrands.end()) {
        score += 1000;
    }

    // Distance penalty: closer is better
    score -= distanceSteps;

    return score;
}

// =============================================================================
// Tìm Spot tốt nhất trong tất cả Spot trên bản đồ
// =============================================================================

int SpotScorer::findBestSpot(
    Position currentPos,
    const GameConfig& config,
    const Map& map,
    int fuelRemaining,
    int stepsRemaining,
    const std::set<int>& visitedToday,
    const std::vector<int>& remainingStock,
    const std::set<int>& collectedBrands
) {
    int bestSpot = -1;
    int bestScore = -1;
    int bestCost = INT_MAX;

    for (size_t si = 0; si < config.spots.size(); ++si) {
        // Lọc 1: Đã ghé hôm nay chưa?
        if (visitedToday.count(static_cast<int>(si))) continue;

        // Lọc 2: Còn hàng không?
        if (remainingStock[si] <= 0) continue;

        // Lọc 3: Có đến được không? (Dijkstra)
        Position spotPos = map.posToCoordinate(config.spots[si].pos);
        auto path = PathFinder::findPath(currentPos, spotPos, map, fuelRemaining);
        if (!path.found) continue;
        if (path.totalSteps > stepsRemaining) continue;

        // Chấm điểm
        int score = scoreSpot(
            config.spots[si].brand,
            path.totalSteps,
            collectedBrands,
            remainingStock[si]
        );

        // So sánh: điểm cao hơn thắng, bằng điểm thì gần hơn thắng
        if (score > bestScore || (score == bestScore && path.totalSteps < bestCost)) {
            bestScore = score;
            bestCost = path.totalSteps;
            bestSpot = static_cast<int>(si);
        }
    }

    return bestSpot;
}
