#include <iostream>
#include <cassert>
#include <cmath>
#include <climits>
#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include "solver/ActionValidator.hpp"
#include "solver/Solver.hpp"
#include "solver/SpotAssigner.hpp"

// =============================================================================
// Test 1: Map basics and hex geometry
// =============================================================================
void test_map_and_geometry() {
    std::vector<std::vector<int>> cells = {
        {3, 0, 1, 2},
        {0, 1, 2, 0},
        {1, 2, 0, 3}
    };
    Map map(3, 4, cells);

    assert(map.getHeight() == 3);
    assert(map.getWidth() == 4);
    assert(!map.canMove(0, 0)); // Pond
    assert(map.canMove(1, 0));  // Plain
    assert(map.canMove(2, 0));  // Road
    assert(map.canMove(3, 0));  // Mountain

    Position p = map.posToCoordinate(5);
    assert(p.x == 1 && p.y == 1);
    assert(map.coordinateToPos(p) == 5);

    std::cout << "[PASS] Map and Geometry tests passed!" << std::endl;
}

// =============================================================================
// Test 2: Travel time and fuel cost
// =============================================================================
void test_travel_time_and_fuel() {
    std::vector<std::vector<int>> cells = {{0, 1, 2, 3}};
    Map map(1, 4, cells);

    assert(map.getTravelTime(0) == 2);   // Plain
    assert(map.getFuelCost(0) == 1);
    assert(map.getTravelTime(1) == 1);   // Road smooth
    assert(map.getFuelCost(1) == 2);
    assert(map.getTravelTime(2) == 3);   // Mountain
    assert(map.getFuelCost(2) == 2);
    assert(map.getTravelTime(3) == 999); // Pond

    std::vector<Traffic> traffics = {{1, 1}};
    map.updateTraffic(traffics);
    assert(map.getTravelTime(1) == 2);   // Congested
    traffics = {{1, 2}};
    map.updateTraffic(traffics);
    assert(map.getTravelTime(1) == 4);   // Jammed
    assert(map.getFuelCost(1) == 2);     // Always 2 for road

    std::cout << "[PASS] Travel time and fuel cost tests passed!" << std::endl;
}

// =============================================================================
// Test 3: SSSP — Single-Source Shortest Path
// =============================================================================
void test_sssp() {
    std::vector<std::vector<int>> cells = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    };
    Map map(3, 3, cells);

    Position source{0, 0};
    auto sssp = PathFinder::computeSSSP(source, map);

    // Source to itself = 0
    int srcPos = map.coordinateToPos(source);
    assert(sssp.dist[srcPos] == 0);
    assert(sssp.fuel[srcPos] == 0);

    // All cells should be reachable (all plains, no ponds)
    for (int i = 0; i < 9; ++i) {
        assert(sssp.dist[i] != INT_MAX);
    }

    // Extract path from SSSP should match findPath
    Position goal{2, 2};
    auto pathFromSSP = sssp.extractPath(map.coordinateToPos(goal));
    auto pathDirect = PathFinder::findPath(source, goal, map);

    assert(pathFromSSP.found == pathDirect.found);
    assert(pathFromSSP.totalSteps == pathDirect.totalSteps);

    std::cout << "[PASS] SSSP tests passed!" << std::endl;
}

// =============================================================================
// Test 4: PathFinder fuel constraint
// =============================================================================
void test_pathfinder_fuel_limit() {
    std::vector<std::vector<int>> cells = {{0, 0, 0, 0, 0}};
    Map map(1, 5, cells);

    Position start{0, 0};
    Position goal{4, 0};

    auto path1 = PathFinder::findPath(start, goal, map, 10);
    assert(path1.found);

    auto path2 = PathFinder::findPath(start, goal, map, 1);
    assert(!path2.found);

    std::cout << "[PASS] PathFinder fuel constraint tests passed!" << std::endl;
}

// =============================================================================
// Test 5: ActionValidator with travel time
// =============================================================================
void test_validator_travel_time() {
    GameConfig config;
    config.map.height = 1;
    config.map.width = 3;
    config.map.cells = {{0, 0, 0}};
    config.daySteps = {10};

    GameState state;
    state.day = 0;
    state.agents.push_back({0, 0, 20}); // Patrol at pos 0

    Map map(1, 3, config.map.cells);

    std::vector<std::vector<int>> validWait = {{-10}};
    assert(ActionValidator::validate(config, state, validWait, map));

    std::vector<std::vector<int>> validMove = {{2, -8}}; // move(2 steps) + wait(8)
    assert(ActionValidator::validate(config, state, validMove, map));

    std::vector<std::vector<int>> wrongSteps = {{2, -7}};
    assert(!ActionValidator::validate(config, state, wrongSteps, map));

    std::cout << "[PASS] ActionValidator travel time tests passed!" << std::endl;
}

// =============================================================================
// Test 6: ActionValidator fuel check
// =============================================================================
void test_validator_fuel_check() {
    GameConfig config;
    config.map.height = 1;
    config.map.width = 5;
    config.map.cells = {{0, 0, 0, 0, 0}};
    config.daySteps = {100};

    GameState state;
    state.day = 0;
    state.agents.push_back({0, 0, 2}); // Patrol, fuel=2

    Map map(1, 5, config.map.cells);

    std::vector<std::vector<int>> valid = {{2, 2, -96}};
    assert(ActionValidator::validate(config, state, valid, map));

    std::vector<std::vector<int>> noFuel = {{2, 2, 2, -94}};
    assert(!ActionValidator::validate(config, state, noFuel, map));

    state.agents[0].kind = 1; // Supply
    state.agents[0].fuel = 0;
    std::vector<std::vector<int>> supplyMoves = {{2, 2, 2, -94}};
    assert(ActionValidator::validate(config, state, supplyMoves, map));

    std::cout << "[PASS] ActionValidator fuel check tests passed!" << std::endl;
}

// =============================================================================
// Test 7: Solver multi-spot chaining (NEW — key v2 test)
// =============================================================================
void test_solver_multi_spot() {
    GameConfig config;
    config.map.height = 1;
    config.map.width = 8;
    config.map.cells = {{0, 0, 0, 0, 0, 0, 0, 0}}; // All plains, 1 row
    config.daySteps = {100}; // Plenty of steps
    config.fuelLimit = 50;
    config.initialAgentPositions = {0};
    config.players = 2;
    config.busyThreshold = 5;
    config.jammedThreshold = 10;
    config.startsAt = 0;
    config.daySeconds = {10};

    // 3 spots at positions 2, 4, 6 — different brands
    config.spots = {
        {0, 2, 1},  // brand 0, pos 2
        {1, 4, 1},  // brand 1, pos 4
        {2, 6, 1}   // brand 2, pos 6
    };

    GameState state;
    state.day = 0;
    state.endsAt = 10;
    state.agents = {{0, 0, 50}}; // Patrol at pos 0, plenty of fuel

    Map map(1, 8, config.map.cells);

    Solver solver;
    auto actions = solver.solve(config, state, map);

    assert(actions.size() == 1);

    // Validate the plan
    bool isValid = ActionValidator::validate(config, state, actions, map);
    assert(isValid);

    // Count number of move actions (should be > 1 move since multi-spot)
    int moveCount = 0;
    for (int act : actions[0]) {
        if (act >= 0 && act <= 5) moveCount++;
    }
    // Should visit multiple spots: pos 2 = 2 moves, pos 4 = 2 more, pos 6 = 2 more
    // With 100 steps and all plains (2 steps each), can easily do 6+ moves
    assert(moveCount >= 2); // At minimum 2 moves to reach first spot

    std::cout << "[PASS] Solver multi-spot chaining test passed! (moves=" << moveCount << ")" << std::endl;
}

// =============================================================================
// Test 8: Solver with supply car (integration)
// =============================================================================
void test_solver_with_supply() {
    GameConfig config;
    config.map.height = 3;
    config.map.width = 3;
    config.map.cells = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    };
    config.daySteps = {20};
    config.fuelLimit = 20;
    config.initialAgentPositions = {0, 4, 8};
    config.players = 2;
    config.busyThreshold = 5;
    config.jammedThreshold = 10;
    config.startsAt = 0;
    config.daySeconds = {10};
    config.spots = {{0, 2, 2}, {1, 6, 1}};

    GameState state;
    state.day = 0;
    state.endsAt = 10;
    state.agents = {
        {0, 0, 20},  // Patrol
        {0, 4, 20},  // Patrol
        {1, 8, 20}   // Supply
    };

    Map map(3, 3, config.map.cells);

    Solver solver;
    auto actions = solver.solve(config, state, map);

    assert(actions.size() == 3);
    bool isValid = ActionValidator::validate(config, state, actions, map);
    assert(isValid);

    std::cout << "[PASS] Solver with supply car test passed!" << std::endl;
}

// =============================================================================
// Test 9: SpotAssigner scoring
// =============================================================================
void test_spot_scoring() {
    std::vector<Spot> spots = {
        {0, 1, 2},  // brand 0
        {1, 5, 1},  // brand 1
    };

    std::set<int> collected = {0}; // brand 0 already collected

    // Spot 1 (brand 1, new) should score higher than spot 0 (brand 0, old)
    int score0 = SpotAssigner::scoreSpot(0, spots, 10, collected, 2);
    int score1 = SpotAssigner::scoreSpot(1, spots, 10, collected, 1);
    assert(score1 > score0); // New brand bonus outweighs stock

    // Zero stock should return -1
    int scoreNoStock = SpotAssigner::scoreSpot(0, spots, 10, collected, 0);
    assert(scoreNoStock == -1);

    std::cout << "[PASS] SpotAssigner scoring test passed!" << std::endl;
}

// =============================================================================
// Main
// =============================================================================
int main() {
    test_map_and_geometry();
    test_travel_time_and_fuel();
    test_sssp();
    test_pathfinder_fuel_limit();
    test_validator_travel_time();
    test_validator_fuel_check();
    test_solver_multi_spot();
    test_solver_with_supply();
    test_spot_scoring();
    std::cout << "\nAll unit tests completed successfully!" << std::endl;
    return 0;
}
