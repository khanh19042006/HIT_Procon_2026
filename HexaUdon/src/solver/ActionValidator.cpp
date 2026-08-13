#include "solver/ActionValidator.hpp"

bool ActionValidator::validate(
    const GameConfig& config,
    const GameState& state,
    const std::vector<std::vector<int>>& actions
) {
    if (actions.size() != state.agents.size()) {
        return false;
    }

    int expectedSteps = 0;
    if (state.day >= 0 && state.day < static_cast<int>(config.daySteps.size())) {
        expectedSteps = config.daySteps[state.day];
    }

    Map gameMap(config.map.height, config.map.width, config.map.cells);

    for (size_t i = 0; i < actions.size(); ++i) {
        int totalSteps = 0;
        Position currentPos = gameMap.posToCoordinate(state.agents[i].pos);

        for (int act : actions[i]) {
            if (act < 0) {
                totalSteps += (-act);
            } else if (act >= 0 && act <= 5) {
                totalSteps += 1;
                currentPos = gameMap.nextPosition(currentPos, act);
                if (!gameMap.canMove(currentPos)) {
                    return false;
                }
            } else {
                return false;
            }
        }

        if (expectedSteps > 0 && totalSteps != expectedSteps) {
            return false;
        }
    }

    return true;
}
