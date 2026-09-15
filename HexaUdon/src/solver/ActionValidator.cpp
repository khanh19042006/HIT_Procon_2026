#include "solver/ActionValidator.hpp"

bool ActionValidator::validate(
    const GameConfig& config,
    const GameState& state,
    const std::vector<std::vector<int>>& actions,
    const Map& map
) {
    // Check agent count matches
    if (actions.size() != state.agents.size()) {
        return false;
    }

    int expectedSteps = 0;
    if (state.day >= 0 && state.day < static_cast<int>(config.daySteps.size())) {
        expectedSteps = config.daySteps[state.day];
    }

    for (size_t i = 0; i < actions.size(); ++i) {
        int totalSteps = 0;
        int fuelRemaining = state.agents[i].fuel;
        bool isPatrol = (state.agents[i].kind == 0);
        Position currentPos = map.posToCoordinate(state.agents[i].pos);

        for (int act : actions[i]) {
            if (act < 0) {
                // Wait action: -K means wait K steps
                totalSteps += (-act);
            } else if (act >= 0 && act <= 5) {
                // Movement action: costs travel time steps from CURRENT cell
                int travelTime = map.getTravelTime(currentPos);
                int fuelCost = isPatrol ? map.getFuelCost(currentPos) : 0;

                totalSteps += travelTime;

                // Check fuel for patrol cars
                if (isPatrol) {
                    fuelRemaining -= fuelCost;
                    if (fuelRemaining < 0) return false;
                }

                // Move to next cell
                currentPos = map.nextPosition(currentPos, act);

                // Check destination is valid (not pond, not outside map)
                if (!map.canMove(currentPos)) {
                    return false;
                }
            } else {
                // Invalid action code
                return false;
            }
        }

        // Total steps must exactly match daySteps
        if (expectedSteps > 0 && totalSteps != expectedSteps) {
            return false;
        }
    }

    return true;
}
