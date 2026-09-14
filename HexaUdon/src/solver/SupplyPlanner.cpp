#include "solver/SupplyPlanner.hpp"
#include "solver/MoveSimulator.hpp"
#include "solver/PathFinder.hpp"
#include <climits>

// =============================================================================
// Tìm xe Patrol đang cần xăng nhất
// =============================================================================

int SupplyPlanner::findTargetPatrol(
    const std::vector<Agent>& agents,
    int excludeIdx
) {
    int bestIdx = -1;
    int lowestFuel = INT_MAX;

    for (size_t i = 0; i < agents.size(); ++i) {
        if (static_cast<int>(i) == excludeIdx) continue;
        if (agents[i].kind != 0) continue; // Chỉ xét xe Patrol

        if (agents[i].fuel < lowestFuel) {
            lowestFuel = agents[i].fuel;
            bestIdx = static_cast<int>(i);
        }
    }
    return bestIdx;
}

// =============================================================================
// Lập kế hoạch cho 1 xe Supply trong 1 ngày
// =============================================================================

std::vector<int> SupplyPlanner::planDay(
    const GameConfig& config,
    const Map& map,
    const Agent& supplyAgent,
    const std::vector<Agent>& allAgents,
    int supplyIdx,
    int daySteps,
    const std::vector<int>& patrolTargetSpots,
    int& plannedTargetSpot
) {
    plannedTargetSpot = -1;

    // Tìm xe Patrol cần cứu
    int targetPatrol = findTargetPatrol(allAgents, supplyIdx);

    if (targetPatrol < 0) {
        // Không tìm thấy xe Patrol → đứng yên cả ngày
        return {-daySteps};
    }

    // Xác định điểm hẹn (Rendezvous)
    Position targetPos;
    int patrolTarget = -1;
    if (targetPatrol >= 0 && targetPatrol < static_cast<int>(patrolTargetSpots.size())) {
        patrolTarget = patrolTargetSpots[targetPatrol];
    }

    if (patrolTarget >= 0) {
        // Đón đầu: đi tới Spot mà xe Patrol đang nhắm tới
        targetPos = map.posToCoordinate(config.spots[patrolTarget].pos);
        plannedTargetSpot = patrolTarget;
    } else {
        // Fallback: đi tới vị trí hiện tại của xe Patrol
        targetPos = map.posToCoordinate(allAgents[targetPatrol].pos);
    }

    // Tìm đường đến điểm hẹn
    Position agentPos = map.posToCoordinate(supplyAgent.pos);
    auto pathResult = PathFinder::findPath(agentPos, targetPos, map, INT_MAX);

    if (!pathResult.found) {
        // Không tìm được đường → đứng yên
        return {-daySteps};
    }

    // Mô phỏng di chuyển (xe Supply không tốn xăng)
    auto sim = MoveSimulator::simulate(
        pathResult.directions, map, agentPos,
        daySteps, INT_MAX, false
    );

    // Padding Wait cho đủ daySteps
    std::vector<int> actions = sim.actions;
    MoveSimulator::padWithWait(actions, sim.stepsUsed, daySteps);

    return actions;
}
