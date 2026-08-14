#include "solver/Solver.hpp"
#include <iostream>
#include <climits>

// =============================================================================
// AgentStrategy — Decide agent types at match start
// =============================================================================

std::vector<int> AgentStrategy::decideAgentTypes(const GameConfig& config) {
    size_t n = config.initialAgentPositions.size();
    int mapArea = config.map.height * config.map.width;

    std::vector<int> types(n, 0); // Default: all Patrol (0)

    // Always need at least 1 Supply car because fuel is limited
    // and patrol cars MUST be refueled to keep moving
    if (n >= 4) {
        types[n - 1] = 1; // Last agent = Supply
    } else if (n == 3) {
        types[n - 1] = 1; // 3 agents: 2 Patrol + 1 Supply
    }

    // Large map + low fuel → need 2 supply cars
    if (n >= 6 && config.fuelLimit <= 15 && mapArea > 400) {
        types[n - 2] = 1;
    }

    return types;
}

std::vector<int> Solver::decideAgentTypes(const GameConfig& config) {
    return AgentStrategy::decideAgentTypes(config);
}

// =============================================================================
// Helper: Build action sequence from a path, respecting travel time and fuel
// =============================================================================

std::vector<int> Solver::buildActionSequence(
    const std::vector<int>& pathDirs,
    const Map& map,
    Position startPos,
    int daySteps,
    int availableFuel,
    bool isPatrol
) {
    std::vector<int> actions;
    int stepsUsed = 0;
    int fuelRemaining = availableFuel;
    Position currentPos = startPos;

    for (int dir : pathDirs) {
        int travelTime = map.getTravelTime(currentPos);
        int fuelCost = isPatrol ? map.getFuelCost(currentPos) : 0;

        // Check: enough steps remaining AND enough fuel?
        if (stepsUsed + travelTime > daySteps) break;
        if (isPatrol && fuelRemaining < fuelCost) break;

        actions.push_back(dir);
        stepsUsed += travelTime;
        fuelRemaining -= fuelCost;
        currentPos = map.nextPosition(currentPos, dir);
    }

    // Pad with wait to exactly fill daySteps
    int remaining = daySteps - stepsUsed;
    if (remaining > 0) {
        actions.push_back(-remaining);
    }

    return actions;
}

// =============================================================================
// Helper: Find patrol car with lowest fuel (for Supply car targeting)
// =============================================================================

int Solver::findLowestFuelPatrol(
    const std::vector<Agent>& agents,
    int excludeIdx
) {
    int bestIdx = -1;
    int lowestFuel = INT_MAX;

    for (size_t i = 0; i < agents.size(); ++i) {
        if (static_cast<int>(i) == excludeIdx) continue;
        if (agents[i].kind != 0) continue; // Skip non-patrol

        if (agents[i].fuel < lowestFuel) {
            lowestFuel = agents[i].fuel;
            bestIdx = static_cast<int>(i);
        }
    }

    return bestIdx;
}

// =============================================================================
// MAIN SOLVER — Generate action plans for all agents in a day
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

    if (daySteps <= 0) {
        // No steps this day — shouldn't happen but handle gracefully
        return actions;
    }

    // 1. Update traffic status on the map
    map.updateTraffic(state.traffics);

    // 2. Assign spots to patrol agents (brand-priority greedy)
    auto spotTargets = SpotAssigner::assignSpotsToAgents(
        state.agents, config.spots, map
    );

    // 3. Generate action plan for each agent
    for (int i = 0; i < numAgents; ++i) {
        const Agent& agent = state.agents[i];
        Position agentPos = map.posToCoordinate(agent.pos);
        bool isPatrol = (agent.kind == 0);

        if (isPatrol && spotTargets[i] >= 0) {
            // === PATROL CAR: Navigate to assigned spot ===
            Position targetPos = map.posToCoordinate(
                config.spots[spotTargets[i]].pos
            );

            auto pathResult = PathFinder::findPath(
                agentPos, targetPos, map, agent.fuel
            );

            if (pathResult.found) {
                actions[i] = buildActionSequence(
                    pathResult.directions, map, agentPos,
                    daySteps, agent.fuel, true
                );
            } else {
                // Cannot reach target with current fuel — wait for supply
                actions[i] = {-daySteps};
            }

        } else if (!isPatrol) {
            // === SUPPLY CAR: Go to patrol car with lowest fuel ===
            int targetAgent = findLowestFuelPatrol(state.agents, i);

            if (targetAgent >= 0) {
                Position targetPos = map.posToCoordinate(
                    state.agents[targetAgent].pos
                );

                // Supply cars have unlimited fuel — pass INT_MAX
                auto pathResult = PathFinder::findPath(
                    agentPos, targetPos, map, INT_MAX
                );

                if (pathResult.found) {
                    actions[i] = buildActionSequence(
                        pathResult.directions, map, agentPos,
                        daySteps, INT_MAX, false
                    );
                } else {
                    actions[i] = {-daySteps};
                }
            } else {
                // No patrol cars to supply — wait
                actions[i] = {-daySteps};
            }

        } else {
            // Patrol car with no target — wait
            actions[i] = {-daySteps};
        }
    }

    return actions;
}

// =============================================================================
// Fallback — All agents wait entire day (safe plan)
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
