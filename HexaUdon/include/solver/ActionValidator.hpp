#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include <vector>

class ActionValidator {
public:
    /**
     * @brief Validate action plan: correct total steps (with travel time),
     *        valid movement (no pond/out-of-map), and fuel check for patrol cars.
     *
     * @param config  Match config
     * @param state   Current day state
     * @param actions Action plan for all agents
     * @param map     Map with current traffic info (must be pre-updated)
     * @return true if all agents' plans are valid
     */
    static bool validate(
        const GameConfig& config,
        const GameState& state,
        const std::vector<std::vector<int>>& actions,
        const Map& map
    );
};
