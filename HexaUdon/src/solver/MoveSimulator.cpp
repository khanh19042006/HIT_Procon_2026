#include "solver/MoveSimulator.hpp"

// =============================================================================
// Mô phỏng di chuyển: biến path directions -> actions + trạng thái cuối
// =============================================================================

SimResult MoveSimulator::simulate(
    const std::vector<int>& pathDirs,
    const Map& map,
    Position startPos,
    int maxSteps,
    int availableFuel,
    bool isPatrol
) {
    SimResult result;
    result.finalPos = startPos;
    result.stepsUsed = 0;
    result.fuelUsed = 0;

    Position currentPos = startPos;
    int fuelRemaining = availableFuel;

    for (int dir : pathDirs) {
        int travelTime = map.getTravelTime(currentPos);
        int fuelCost = isPatrol ? map.getFuelCost(currentPos) : 0;

        // Kiểm tra giới hạn bước
        if (result.stepsUsed + travelTime > maxSteps) break;

        // Kiểm tra giới hạn xăng (chỉ xe Patrol)
        if (isPatrol && fuelRemaining < fuelCost) break;

        // Thực hiện di chuyển
        result.actions.push_back(dir);
        result.stepsUsed += travelTime;
        result.fuelUsed += fuelCost;
        fuelRemaining -= fuelCost;
        currentPos = map.nextPosition(currentPos, dir);
    }

    result.finalPos = currentPos;
    return result;
}

// =============================================================================
// Thêm hành động "Chờ" cho đủ daySteps
// =============================================================================

void MoveSimulator::padWithWait(
    std::vector<int>& actions,
    int stepsUsed,
    int daySteps
) {
    int remaining = daySteps - stepsUsed;
    if (remaining > 0) {
        actions.push_back(-remaining);
    }
}
