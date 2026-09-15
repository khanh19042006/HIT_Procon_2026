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
    int& lastTargetSpot,
    Position& plannedTargetPos,
    std::vector<int>& plannedStepSpots,
    std::vector<Position>& plannedStepPositions
) {
    std::vector<int> allActions;
    int stepsUsed = 0;
    int fuelRemaining = availableFuel;
    Position currentPos = startPos;
    plannedTargetPos = startPos;
    plannedStepSpots.assign(daySteps, -1);
    plannedStepPositions.assign(daySteps, startPos);

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

        int traceStep = stepsUsed;
        Position tracePosition = currentPos;
        for (int simulatedAction : sim.actions) {
            int actionSteps = map.getTravelTime(tracePosition);
            for (int offset = 0; offset < actionSteps && traceStep + offset < daySteps; ++offset) {
                plannedStepSpots[traceStep + offset] = nextSpot;
                plannedStepPositions[traceStep + offset] = spotPos;
            }
            traceStep += actionSteps;
            tracePosition = map.nextPosition(tracePosition, simulatedAction);
        }

        // Bước 4: Cập nhật trạng thái
        allActions.insert(allActions.end(), sim.actions.begin(), sim.actions.end());
        stepsUsed += sim.stepsUsed;
        fuelRemaining -= sim.fuelUsed;
        currentPos = sim.finalPos;
        plannedTargetPos = spotPos;

        // Bước 5: Đánh dấu Spot đã ghé + cập nhật stock + brand
        visitedToday.insert(nextSpot);
        remainingStock[nextSpot]--;
        collectedBrands.insert(config.spots[nextSpot].brand);

        // Ghi nhớ Spot cuối cùng (để Supply biết đón đầu ở đâu)
        lastTargetSpot = nextSpot;
        plannedTargetPos = spotPos;
    }

    // Padding: thêm Wait cho đủ daySteps
    MoveSimulator::padWithWait(allActions, stepsUsed, daySteps);

    for (int step = stepsUsed; step < daySteps; ++step) {
        plannedStepSpots[step] = lastTargetSpot;
        plannedStepPositions[step] = plannedTargetPos;
    }

    return allActions;
}
