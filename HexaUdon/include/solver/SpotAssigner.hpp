#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include <vector>
#include <set>

/**
 * @brief SpotAssigner is now simplified — most logic moved to Solver's
 * multi-spot chaining. This class provides utility functions for spot scoring.
 */
class SpotAssigner {
public:
    /**
     * @brief Score a spot for a patrol agent.
     * Higher score = higher priority.
     *
     * Factors:
     * - Brand diversity bonus (new brand = +1000)
     * - Stock availability
     * - Distance cost (negative: closer is better)
     */
    static int scoreSpot(
        int spotIdx,
        const std::vector<Spot>& spots,
        int distanceSteps,
        const std::set<int>& collectedBrands,
        int remainingStock
    );
};
