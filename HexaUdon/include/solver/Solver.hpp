#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include "solver/SpotAssigner.hpp"
#include <vector>

/**
 * @brief Agent type decision (Patrol vs Supply)
 */
class AgentStrategy {
public:
    static std::vector<int> decideAgentTypes(const GameConfig& config);
};

/**
 * @brief Core solver — generates action plans for all agents each day
 */
class Solver {
public:
    std::vector<int> decideAgentTypes(const GameConfig& config);

    /**
     * @brief Main solver: generates action plans for the day.
     * @param config  Match configuration (map, spots, daySteps, fuelLimit...)
     * @param state   Current day state (agent positions, fuel, traffic...)
     * @param map     Map object (non-const: traffic will be updated)
     * @return Action plan for each agent
     */
    std::vector<std::vector<int>> solve(
        const GameConfig& config,
        const GameState& state,
        Map& map
    );

    /**
     * @brief Fallback: all agents wait the entire day (safe plan)
     */
    std::vector<std::vector<int>> createFallbackActions(
        const GameConfig& config,
        const GameState& state
    );

private:
    /**
     * @brief Convert a path (list of directions) into a valid action sequence
     *        respecting travel time and fuel constraints.
     */
    static std::vector<int> buildActionSequence(
        const std::vector<int>& pathDirs,
        const Map& map,
        Position startPos,
        int daySteps,
        int availableFuel,
        bool isPatrol
    );

    /**
     * @brief Find the patrol car index with the lowest fuel remaining.
     */
    static int findLowestFuelPatrol(
        const std::vector<Agent>& agents,
        int excludeIdx
    );
};
