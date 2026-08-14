#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include <vector>
#include <set>

class SpotAssigner {
public:
    /**
     * @brief Assign spots to patrol agents, prioritizing brand diversity.
     *
     * Scoring priority (per official rules):
     *   1. Maximize unique brand types collected
     *   2. Then maximize total servings
     *
     * @param agents       Current agent states
     * @param spots        All spots from config
     * @param map          Map with traffic info
     * @return spotTarget[i] = index into config.spots for agent i to go to
     *         -1 if agent is Supply car or no target available
     */
    static std::vector<int> assignSpotsToAgents(
        const std::vector<Agent>& agents,
        const std::vector<Spot>& spots,
        const Map& map
    );
};
