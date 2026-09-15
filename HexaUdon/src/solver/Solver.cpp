#include "solver/Solver.hpp"
#include "solver/PatrolPlanner.hpp"
#include "solver/SupplyPlanner.hpp"
#include <iostream>

// =============================================================================
// AgentStrategy — Quyết định đội hình xe
// =============================================================================

std::vector<int> AgentStrategy::decideAgentTypes(const GameConfig& config) {
    size_t n = config.initialAgentPositions.size();
    int mapArea = config.map.height * config.map.width;

    // === CORE PRINCIPLE: Maximize Patrol cars, minimize Supply ===
    // Patrol cars score points by visiting spots.
    // Supply cars score ZERO — they only refuel patrols.
    // So: use as many Patrols as possible, only 1 Supply.

    // If fuel is enormous relative to map → no supply needed at all
    if (config.fuelLimit >= mapArea * 2) {
        return std::vector<int>(n, 0); // All Patrol
    }

    // Default strategy: (N-1) Patrol + 1 Supply
    int supplyCount = 1;

    // Very small team (1-2 agents): no supply, all patrol
    if (n <= 2) {
        supplyCount = 0;
    }

    // Large map + very low fuel + many agents → 2 supply
    if (n >= 6 && config.fuelLimit <= 10 && mapArea > 400) {
        supplyCount = 2;
    }

    // Safety: at least 1 patrol
    if (supplyCount >= static_cast<int>(n)) {
        supplyCount = static_cast<int>(n) - 1;
    }

    std::vector<int> types(n, 0); // Default: Patrol
    for (int i = 0; i < supplyCount; ++i) {
        types[n - 1 - i] = 1; // Supply from the end
    }
    return types;
}

std::vector<int> Solver::decideAgentTypes(const GameConfig& config) {
    return AgentStrategy::decideAgentTypes(config);
}

int Solver::getPlannedTargetSpot(int agentIdx) const {
    if (agentIdx >= 0 && agentIdx < static_cast<int>(currentTargets_.size())) {
        return currentTargets_[agentIdx];
    }
    return -1;
}

Position Solver::getPlannedTargetPosition(int agentIdx) const {
    if (agentIdx >= 0 && agentIdx < static_cast<int>(currentTargetPositions_.size())) {
        return currentTargetPositions_[agentIdx];
    }
    return {-1, -1};
}

int Solver::getPlannedStepSpot(int agentIdx, int step) const {
    if (agentIdx >= 0 && agentIdx < static_cast<int>(plannedStepSpots_.size()) &&
        step >= 0 && step < static_cast<int>(plannedStepSpots_[agentIdx].size())) {
        return plannedStepSpots_[agentIdx][step];
    }
    return -1;
}

Position Solver::getPlannedStepPosition(int agentIdx, int step) const {
    if (agentIdx >= 0 && agentIdx < static_cast<int>(plannedStepPositions_.size()) &&
        step >= 0 && step < static_cast<int>(plannedStepPositions_[agentIdx].size())) {
        return plannedStepPositions_[agentIdx][step];
    }
    return {-1, -1};
}

int Solver::getSupportedPatrol(int agentIdx) const {
    if (agentIdx >= 0 && agentIdx < static_cast<int>(supportedPatrols_.size())) {
        return supportedPatrols_[agentIdx];
    }
    return -1;
}

// =============================================================================
// Reset trạng thái đầu ngày
// =============================================================================

void Solver::resetDailyState(const GameConfig& config, int numAgents) {
    // Reset stock to max for each spot (stock replenishes each day)
    remainingStock_.resize(config.spots.size());
    for (size_t i = 0; i < config.spots.size(); ++i) {
        remainingStock_[i] = config.spots[i].stocks;
    }

    // Reset visited spots for each patrol
    visitedSpotsToday_.assign(numAgents, {});

    // Reset current targets
    currentTargets_.assign(numAgents, -1);
    currentTargetPositions_.assign(numAgents, {-1, -1});
    supportedPatrols_.assign(numAgents, -1);
    plannedStepSpots_.assign(numAgents, {});
    plannedStepPositions_.assign(numAgents, {});
}

// =============================================================================
// MAIN SOLVER — Nhạc trưởng điều phối các module
// =============================================================================

std::vector<std::vector<int>> Solver::solve(
    const GameConfig& config,
    const GameState& state,
    Map& map
) {
    int daySteps = 0;
    if (state.day >= 0 && state.day < static_cast<int>(config.daySteps.size())) {
        daySteps = config.daySteps[state.day];
    }

    int numAgents = static_cast<int>(state.agents.size());
    std::vector<std::vector<int>> actions(numAgents);

    if (daySteps <= 0) return actions;

    // 1. Cập nhật giao thông trên bản đồ
    map.updateTraffic(state.traffics);

    // 2. Reset trạng thái nếu là ngày mới
    if (state.day != currentDay_) {
        resetDailyState(config, numAgents);
        currentDay_ = state.day;
    }

    // 3. Lập kế hoạch cho xe TUẦN TRA (PatrolPlanner)
    //    Tính trước để xe Supply biết mục tiêu của Patrol
    for (int i = 0; i < numAgents; ++i) {
        const Agent& agent = state.agents[i];
        if (agent.kind != 0) continue; // Bỏ qua xe Supply

        Position agentPos = map.posToCoordinate(agent.pos);

        actions[i] = PatrolPlanner::planDay(
            config, map, agentPos, daySteps, agent.fuel,
            remainingStock_,
            visitedSpotsToday_[i],
            collectedBrandsTotal_,
            currentTargets_[i],
            currentTargetPositions_[i],
            plannedStepSpots_[i],
            plannedStepPositions_[i]
        );
    }

    // 4. Lập kế hoạch cho xe TIẾP TẾ (SupplyPlanner)
    for (int i = 0; i < numAgents; ++i) {
        const Agent& agent = state.agents[i];
        if (agent.kind != 1) continue; // Bỏ qua xe Patrol

        actions[i] = SupplyPlanner::planDay(
            config, map, agent, state.agents, i,
            daySteps, currentTargets_, currentTargetPositions_,
            supportedPatrols_[i], currentTargets_[i],
            currentTargetPositions_[i],
            plannedStepSpots_[i], plannedStepPositions_[i]
        );
    }

    return actions;
}

// =============================================================================
// Fallback — Tất cả xe đứng yên cả ngày
// =============================================================================

std::vector<std::vector<int>> Solver::createFallbackActions(
    const GameConfig& config,
    const GameState& state
) {
    int daySteps = 0;
    if (state.day >= 0 && state.day < static_cast<int>(config.daySteps.size())) {
        daySteps = config.daySteps[state.day];
    }

    std::vector<std::vector<int>> fallback(state.agents.size());
    for (size_t i = 0; i < state.agents.size(); ++i) {
        if (daySteps > 0) {
            fallback[i].push_back(-daySteps);
        }
    }
    return fallback;
}
