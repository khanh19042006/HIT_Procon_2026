#include "solver/Solver.hpp"
#include <iostream>
#include <climits>
#include <algorithm>

// =============================================================================
// AgentStrategy — Smart agent type allocation
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

// =============================================================================
// Daily state reset
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
}

// =============================================================================
// Build action sequence from directions (respecting travel time + fuel)
// =============================================================================

std::vector<int> Solver::buildActionSequence(
    const std::vector<int>& pathDirs,
    const Map& map,
    Position startPos,
    int maxSteps,
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

        if (stepsUsed + travelTime > maxSteps) break;
        if (isPatrol && fuelRemaining < fuelCost) break;

        actions.push_back(dir);
        stepsUsed += travelTime;
        fuelRemaining -= fuelCost;
        currentPos = map.nextPosition(currentPos, dir);
    }

    return actions;
}

// =============================================================================
// Find patrol car with lowest fuel
// =============================================================================

int Solver::findLowestFuelPatrol(
    const std::vector<Agent>& agents,
    int excludeIdx
) {
    int bestIdx = -1;
    int lowestFuel = INT_MAX;

    for (size_t i = 0; i < agents.size(); ++i) {
        if (static_cast<int>(i) == excludeIdx) continue;
        if (agents[i].kind != 0) continue;

        if (agents[i].fuel < lowestFuel) {
            lowestFuel = agents[i].fuel;
            bestIdx = static_cast<int>(i);
        }
    }
    return bestIdx;
}

// =============================================================================
// Get patrol's current target spot
// =============================================================================

int Solver::getPatrolTargetSpot(int patrolIdx) const {
    if (patrolIdx >= 0 && patrolIdx < static_cast<int>(currentTargets_.size())) {
        return currentTargets_[patrolIdx];
    }
    return -1;
}

// =============================================================================
// Find the best NEXT spot for a patrol to visit
// Priority: new brand > old brand, closer is better
// =============================================================================

int Solver::findBestNextSpot(
    int patrolIdx,
    const GameConfig& config,
    const Map& map,
    Position currentPos,
    int fuelRemaining,
    int stepsRemaining
) {
    int bestSpot = -1;
    int bestScore = -1;
    int bestCost = INT_MAX;

    for (size_t si = 0; si < config.spots.size(); ++si) {
        // Skip if already visited today by this patrol
        if (visitedSpotsToday_[patrolIdx].count(static_cast<int>(si))) continue;

        // Skip if no stock remaining
        if (remainingStock_[si] <= 0) continue;

        // Check reachability
        Position spotPos = map.posToCoordinate(config.spots[si].pos);
        auto path = PathFinder::findPath(currentPos, spotPos, map, fuelRemaining);
        if (!path.found) continue;
        if (path.totalSteps > stepsRemaining) continue;

        // Score: new brand = 1000 bonus, otherwise 0
        int score = 0;
        int brand = config.spots[si].brand;
        if (collectedBrandsTotal_.find(brand) == collectedBrandsTotal_.end()) {
            score += 1000;  // Very high priority for new brand types
        }

        // Prefer closer spots (lower cost = better)
        // Among same score, prefer closer
        if (score > bestScore || (score == bestScore && path.totalSteps < bestCost)) {
            bestScore = score;
            bestCost = path.totalSteps;
            bestSpot = static_cast<int>(si);
        }
    }

    return bestSpot;
}

// =============================================================================
// Build multi-spot action plan for a patrol car
// =============================================================================

std::vector<int> Solver::buildMultiSpotPlan(
    int patrolIdx,
    const GameConfig& config,
    const Map& map,
    Position startPos,
    int daySteps,
    int availableFuel
) {
    std::vector<int> allActions;
    int stepsUsed = 0;
    int fuelRemaining = availableFuel;
    Position currentPos = startPos;

    // Keep visiting spots until we can't anymore
    while (true) {
        int stepsRemaining = daySteps - stepsUsed;
        if (stepsRemaining <= 0) break;
        if (fuelRemaining <= 0) break;

        // Find best next spot
        int nextSpot = findBestNextSpot(
            patrolIdx, config, map, currentPos, fuelRemaining, stepsRemaining
        );

        if (nextSpot < 0) break; // No more reachable spots

        // Get path to next spot
        Position spotPos = map.posToCoordinate(config.spots[nextSpot].pos);
        auto path = PathFinder::findPath(currentPos, spotPos, map, fuelRemaining);

        if (!path.found || path.totalSteps > stepsRemaining) break;

        // Build action sequence for this leg
        auto legActions = buildActionSequence(
            path.directions, map, currentPos,
            stepsRemaining, fuelRemaining, true
        );

        // Simulate the leg to update position/fuel
        Position simPos = currentPos;
        int simSteps = 0;
        int simFuel = fuelRemaining;
        for (int act : legActions) {
            if (act >= 0 && act <= 5) {
                int tt = map.getTravelTime(simPos);
                int fc = map.getFuelCost(simPos);
                simSteps += tt;
                simFuel -= fc;
                simPos = map.nextPosition(simPos, act);
            }
        }

        // Append leg actions
        allActions.insert(allActions.end(), legActions.begin(), legActions.end());
        stepsUsed += simSteps;
        fuelRemaining = simFuel;
        currentPos = simPos;

        // Mark spot as visited + update stock + track brand
        visitedSpotsToday_[patrolIdx].insert(nextSpot);
        remainingStock_[nextSpot]--;
        collectedBrandsTotal_.insert(config.spots[nextSpot].brand);

        // Update current target for supply car rendezvous
        currentTargets_[patrolIdx] = nextSpot;
    }

    // Pad with wait for remaining steps
    int remaining = daySteps - stepsUsed;
    if (remaining > 0) {
        allActions.push_back(-remaining);
    }

    return allActions;
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

    if (daySteps <= 0) return actions;

    // 1. Update traffic status on the map
    map.updateTraffic(state.traffics);

    // 2. Reset daily state (stock replenishes, visited resets)
    if (state.day != currentDay_) {
        resetDailyState(config, numAgents);
        currentDay_ = state.day;
    }

    // 3. Generate action plans — Patrol cars FIRST (so supply knows targets)
    for (int i = 0; i < numAgents; ++i) {
        const Agent& agent = state.agents[i];
        if (agent.kind != 0) continue; // Skip supply cars for now

        Position agentPos = map.posToCoordinate(agent.pos);

        // Multi-spot chaining: visit as many spots as possible
        actions[i] = buildMultiSpotPlan(
            i, config, map, agentPos, daySteps, agent.fuel
        );
    }

    // 4. Generate action plans — Supply cars (AFTER patrols, so we know targets)
    for (int i = 0; i < numAgents; ++i) {
        const Agent& agent = state.agents[i];
        if (agent.kind != 1) continue; // Skip patrol cars

        Position agentPos = map.posToCoordinate(agent.pos);

        // Find patrol with lowest fuel
        int targetPatrol = findLowestFuelPatrol(state.agents, i);

        if (targetPatrol >= 0) {
            // PROACTIVE: Go to patrol's TARGET SPOT (rendezvous point)
            // instead of patrol's current position
            Position targetPos;
            int patrolTarget = getPatrolTargetSpot(targetPatrol);

            if (patrolTarget >= 0) {
                // Go to the spot the patrol is heading to
                targetPos = map.posToCoordinate(config.spots[patrolTarget].pos);
            } else {
                // Fallback: go to patrol's current position
                targetPos = map.posToCoordinate(state.agents[targetPatrol].pos);
            }

            auto pathResult = PathFinder::findPath(agentPos, targetPos, map, INT_MAX);

            if (pathResult.found) {
                auto legActions = buildActionSequence(
                    pathResult.directions, map, agentPos,
                    daySteps, INT_MAX, false
                );
                // Calculate steps used in leg
                int stepsUsed = 0;
                Position simPos = agentPos;
                for (int act : legActions) {
                    if (act >= 0 && act <= 5) {
                        stepsUsed += map.getTravelTime(simPos);
                        simPos = map.nextPosition(simPos, act);
                    }
                }
                actions[i] = legActions;
                int remaining = daySteps - stepsUsed;
                if (remaining > 0) {
                    actions[i].push_back(-remaining);
                }
            } else {
                actions[i] = {-daySteps};
            }
        } else {
            actions[i] = {-daySteps};
        }
    }

    return actions;
}

// =============================================================================
// Fallback — All agents wait entire day
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
