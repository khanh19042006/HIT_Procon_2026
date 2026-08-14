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
// Test 1: Map basics and hex geometry (kept from original)
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

    Position p = map.posToCoordinate(5); // 5 / 4 = row 1, 5 % 4 = col 1 -> (1, 1)
    assert(p.x == 1 && p.y == 1);
    assert(map.coordinateToPos(p) == 5);

    // Test direction movement on odd row
    Position start{1, 1};
    Position nxt = map.nextPosition(start, 0); // direction 0 on odd row -> (1, 0)
    assert(nxt.x == 1 && nxt.y == 0);

    std::cout << "[PASS] Map and Geometry tests passed!" << std::endl;
}

// =============================================================================
// Test 2: Travel time and fuel cost (NEW)
// =============================================================================
void test_travel_time_and_fuel() {
    // cells: row 0 = [Plain, Road, Mountain, Pond]
    std::vector<std::vector<int>> cells = {
        {0, 1, 2, 3}
    };
    Map map(1, 4, cells);

    // Plain (pos 0): travel time = 2, fuel = 1
    assert(map.getTravelTime(0) == 2);
    assert(map.getFuelCost(0) == 1);

    // Road (pos 1) default smooth: travel time = 1, fuel = 2
    assert(map.getTravelTime(1) == 1);
    assert(map.getFuelCost(1) == 2);

    // Mountain (pos 2): travel time = 3, fuel = 2
    assert(map.getTravelTime(2) == 3);
    assert(map.getFuelCost(2) == 2);

    // Pond (pos 3): travel time = 999 (blocked)
    assert(map.getTravelTime(3) == 999);

    // Test traffic status updates
    std::vector<Traffic> traffics = {{1, 1}}; // Road at pos 1 is Congested
    map.updateTraffic(traffics);
    assert(map.getTrafficStatus(1) == 1);
    assert(map.getTravelTime(1) == 2); // Congested road = 2 steps

    traffics = {{1, 2}}; // Road at pos 1 is Jammed
    map.updateTraffic(traffics);
    assert(map.getTravelTime(1) == 4); // Jammed road = 4 steps

    // Fuel cost for road is always 2 regardless of traffic
    assert(map.getFuelCost(1) == 2);

    std::cout << "[PASS] Travel time and fuel cost tests passed!" << std::endl;
}

// =============================================================================
// Test 3: Dijkstra PathFinder with travel time (NEW — replaces old BFS test)
// =============================================================================
void test_pathfinder_dijkstra() {
    // 3x3 map: all plains except center is pond
    std::vector<std::vector<int>> cells = {
        {0, 0, 0},
        {0, 3, 0},
        {0, 0, 0}
    };
    Map map(3, 3, cells);

    Position start{0, 0};
    Position goal{2, 0};

    auto path = PathFinder::findPath(start, goal, map);
    assert(path.found);
    assert(!path.directions.empty());

    // Verify path leads to goal
    Position curr = start;
    for (int dir : path.directions) {
        curr = map.nextPosition(curr, dir);
        assert(map.canMove(curr));
    }
    assert(curr == goal);

    // Total steps should be >= 2 * num_moves (each plain costs 2 steps)
    assert(path.totalSteps == static_cast<int>(path.directions.size()) * 2);

    // Total fuel = 1 per plain move
    assert(path.totalFuel == static_cast<int>(path.directions.size()) * 1);

    std::cout << "[PASS] Dijkstra PathFinder tests passed!" << std::endl;
}

// =============================================================================
// Test 4: PathFinder fuel constraint (NEW)
// =============================================================================
void test_pathfinder_fuel_limit() {
    // 1x5 map: all plains
    std::vector<std::vector<int>> cells = {
        {0, 0, 0, 0, 0}
    };
    Map map(1, 5, cells);

    Position start{0, 0};
    Position goal{4, 0};

    // With enough fuel (10) — should find path
    auto path1 = PathFinder::findPath(start, goal, map, 10);
    assert(path1.found);

    // With very limited fuel (1) — might not reach far goal
    auto path2 = PathFinder::findPath(start, goal, map, 1);
    // 4 moves * 1 fuel each = 4 fuel needed, but only 1 available
    assert(!path2.found);

    std::cout << "[PASS] PathFinder fuel constraint tests passed!" << std::endl;
}

// =============================================================================
// Test 5: ActionValidator with travel time (replaces old validator test)
// =============================================================================
void test_validator_travel_time() {
    GameConfig config;
    config.map.height = 1;
    config.map.width = 3;
    config.map.cells = {{0, 0, 0}}; // All plains
    config.daySteps = {10};

    GameState state;
    state.day = 0;
    Agent a;
    a.kind = 0; // Patrol
    a.pos = 0;  // At (0,0) = Plain
    a.fuel = 20;
    state.agents.push_back(a);

    Map map(config.map.height, config.map.width, config.map.cells);

    // Valid: Wait 10 steps [-10] (total = 10) ✓
    std::vector<std::vector<int>> validWait = {{-10}};
    assert(ActionValidator::validate(config, state, validWait, map));

    // Valid: Move right (dir 2) from plain = 2 steps + wait 8 = 10 ✓
    std::vector<std::vector<int>> validMove = {{2, -8}};
    assert(ActionValidator::validate(config, state, validMove, map));

    // Invalid: Move right = 2 steps + wait 7 = 9 (not 10) ✗
    std::vector<std::vector<int>> wrongSteps = {{2, -7}};
    assert(!ActionValidator::validate(config, state, wrongSteps, map));

    // Invalid: Old-style counting (1 step per move) would be wrong
    // Move + wait 9 = would be 10 with old logic, but move costs 2 so total = 11
    std::vector<std::vector<int>> oldStyleWrong = {{2, -9}};
    assert(!ActionValidator::validate(config, state, oldStyleWrong, map));

    std::cout << "[PASS] ActionValidator travel time tests passed!" << std::endl;
}

// =============================================================================
// Test 6: ActionValidator fuel check (NEW)
// =============================================================================
void test_validator_fuel_check() {
    GameConfig config;
    config.map.height = 1;
    config.map.width = 5;
    config.map.cells = {{0, 0, 0, 0, 0}}; // All plains
    config.daySteps = {100};

    GameState state;
    state.day = 0;
    Agent a;
    a.kind = 0; // Patrol
    a.pos = 0;
    a.fuel = 2; // Only 2 fuel — can move 2 plains (1 fuel each)
    state.agents.push_back(a);

    Map map(config.map.height, config.map.width, config.map.cells);

    // Valid: 2 moves right (2 fuel) + wait rest
    // Move costs: 2 steps + 2 steps = 4 steps, wait = 96
    std::vector<std::vector<int>> valid = {{2, 2, -96}};
    assert(ActionValidator::validate(config, state, valid, map));

    // Invalid: 3 moves (3 fuel needed, only 2 available)
    std::vector<std::vector<int>> noFuel = {{2, 2, 2, -94}};
    assert(!ActionValidator::validate(config, state, noFuel, map));

    // Supply car (kind=1) doesn't use fuel — should pass
    state.agents[0].kind = 1;
    state.agents[0].fuel = 0;
    std::vector<std::vector<int>> supplyMoves = {{2, 2, 2, -94}};
    assert(ActionValidator::validate(config, state, supplyMoves, map));

    std::cout << "[PASS] ActionValidator fuel check tests passed!" << std::endl;
}

// =============================================================================
// Test 7: Solver produces valid actions (integration test)
// =============================================================================
void test_solver_integration() {
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

    // 2 spots with different brands
    config.spots = {{0, 2, 2}, {1, 6, 1}};

    GameState state;
    state.day = 0;
    state.endsAt = 10;
    state.agents = {
        {0, 0, 20}, // Patrol at (0,0)
        {0, 4, 20}, // Patrol at (1,1)
        {1, 8, 20}  // Supply at (2,2)
    };

    Map map(config.map.height, config.map.width, config.map.cells);

    Solver solver;
    auto actions = solver.solve(config, state, map);

    // Must have actions for all 3 agents
    assert(actions.size() == 3);

    // All actions should be valid
    bool isValid = ActionValidator::validate(config, state, actions, map);
    assert(isValid);

    std::cout << "[PASS] Solver integration test passed!" << std::endl;
}

// =============================================================================
// Main
// =============================================================================
int main() {
    test_map_and_geometry();
    test_travel_time_and_fuel();
    test_pathfinder_dijkstra();
    test_pathfinder_fuel_limit();
    test_validator_travel_time();
    test_validator_fuel_check();
    test_solver_integration();
    std::cout << "\nAll unit tests completed successfully!" << std::endl;
    return 0;
}
