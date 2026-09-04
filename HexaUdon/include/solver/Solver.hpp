#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include <vector>
#include <set>

/**
 * @brief Agent type decision (Patrol vs Supply)
 */
class AgentStrategy {
public:
    static std::vector<int> decideAgentTypes(const GameConfig& config);
};

/**
 * @brief Core solver — generates action plans for all agents each day.
 *
 * STATEFUL: maintains cross-day memory for brand tracking and daily stock.
 */
class Solver {
public:
    std::vector<int> decideAgentTypes(const GameConfig& config);

    /**
     * @brief Main solver: generates action plans for the day.
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
    // === Cross-day state ===
    std::set<int> collectedBrandsTotal_;   // All brands collected across entire match
    int currentDay_ = -1;

    // === Daily state (reset each day) ===
    std::vector<int> remainingStock_;                  // Stock left per spot today
    std::vector<std::set<int>> visitedSpotsToday_;     // Spots visited today per patrol

    void resetDailyState(const GameConfig& config, int numAgents);

    /**
     * @brief Build multi-spot action plan for a patrol car.
     * Keeps moving to next best spot until out of fuel or steps.
     */
    std::vector<int> buildMultiSpotPlan(
        int patrolIdx,
        const GameConfig& config,
        const Map& map,
        Position startPos,
        int daySteps,
        int availableFuel
    );

    /**
     * @brief Find the best next spot for a patrol to visit.
     * Priority: new brand > old brand, closer > farther, higher stock > lower.
     * Returns spot index or -1 if no viable spot.
     */
    int findBestNextSpot(
        int patrolIdx,
        const GameConfig& config,
        const Map& map,
        Position currentPos,
        int fuelRemaining,
        int stepsRemaining
    );

    /**
     * @brief Build action sequence from a path with travel time + fuel.
     */
    static std::vector<int> buildActionSequence(
        const std::vector<int>& pathDirs,
        const Map& map,
        Position startPos,
        int maxSteps,
        int availableFuel,
        bool isPatrol
    );

    /**
     * @brief Find the patrol car index most in need of fuel.
     */
    static int findLowestFuelPatrol(
        const std::vector<Agent>& agents,
        int excludeIdx
    );

    /**
     * @brief Find the target spot a patrol car is heading to.
     * Used by supply car to do rendezvous.
     */
    int getPatrolTargetSpot(int patrolIdx) const;

    // Target spots assigned to each patrol for current day
    std::vector<int> currentTargets_;
};
