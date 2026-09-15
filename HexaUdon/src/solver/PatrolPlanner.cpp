#include "solver/PatrolPlanner.hpp"
#include "solver/SpotScorer.hpp"
#include "solver/MoveSimulator.hpp"
#include "solver/PathFinder.hpp"

// =============================================================================
// Lập kế hoạch chuỗi Spot cho 1 xe Patrol trong 1 ngày
// =============================================================================

std::vector<int> PatrolPlanner::planDay(
    const GameConfig& config,
    const Map& map,
    Position startPos,
    int daySteps,
    int availableFuel,
    std::vector<int>& remainingStock,
    std::set<int>& visitedToday,
    std::set<int>& collectedBrands,
    int& lastTargetSpot
) {
    std::vector<int> allActions;
    int stepsUsed = 0;
    int fuelRemaining = availableFuel;
    Position currentPos = startPos;

    // Vòng lặp Greedy: ghé Spot liên tục cho đến khi không thể nữa
    while (true) {
        int stepsRemaining = daySteps - stepsUsed;
        if (stepsRemaining <= 0) break;
        if (fuelRemaining <= 0) break;

        // Bước 1: Tìm Spot tốt nhất tiếp theo (dùng SpotScorer)
        int nextSpot = SpotScorer::findBestSpot(
            currentPos, config, map,
            fuelRemaining, stepsRemaining,
            visitedToday, remainingStock, collectedBrands
        );

        if (nextSpot < 0) break; // Không còn Spot nào khả thi

        // Bước 2: Tìm đường tới Spot đó (dùng PathFinder)
        Position spotPos = map.posToCoordinate(config.spots[nextSpot].pos);
        auto path = PathFinder::findPath(currentPos, spotPos, map, fuelRemaining);

        if (!path.found || path.totalSteps > stepsRemaining) break;

        // Bước 3: Mô phỏng di chuyển (dùng MoveSimulator)
        auto sim = MoveSimulator::simulate(
            path.directions, map, currentPos,
            stepsRemaining, fuelRemaining, true
        );

        // Bước 4: Cập nhật trạng thái
        allActions.insert(allActions.end(), sim.actions.begin(), sim.actions.end());
        stepsUsed += sim.stepsUsed;
        fuelRemaining -= sim.fuelUsed;
        currentPos = sim.finalPos;

        // Bước 5: Đánh dấu Spot đã ghé + cập nhật stock + brand
        visitedToday.insert(nextSpot);
        remainingStock[nextSpot]--;
        collectedBrands.insert(config.spots[nextSpot].brand);

        // Ghi nhớ Spot cuối cùng (để Supply biết đón đầu ở đâu)
        lastTargetSpot = nextSpot;
    }

    // Padding: thêm Wait cho đủ daySteps
    MoveSimulator::padWithWait(allActions, stepsUsed, daySteps);

    return allActions;
}
