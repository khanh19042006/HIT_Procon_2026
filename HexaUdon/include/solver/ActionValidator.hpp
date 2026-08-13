#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include <vector>

class ActionValidator {
public:
    static bool validate(
        const GameConfig& config,
        const GameState& state,
        const std::vector<std::vector<int>>& actions
    );
};
