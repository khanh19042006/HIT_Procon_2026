#include <iostream>
#include <cassert>
#include <sstream>
#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include "solver/ActionValidator.hpp"
#include "solver/Solver.hpp"

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
    assert(map.canMove(1, 0));  // Flat ground
    assert(map.canMove(2, 0));  // Road
    assert(map.canMove(3, 0));  // Mountain

    Position p = map.posToCoordinate(5); // 5 / 4 = row 1, 5 % 4 = col 1 -> (1, 1)
    assert(p.x == 1 && p.y == 1);
    assert(map.coordinateToPos(p) == 5);

    // Test direction movement
    Position start{1, 1}; // odd row
    Position nxt = map.nextPosition(start, 0); // direction 0 on odd row -> (1, 0)
    assert(nxt.x == 1 && nxt.y == 0);

    std::cout << "[PASS] Map and Geometry tests passed!" << std::endl;
}

void test_pathfinder() {
    std::vector<std::vector<int>> cells = {
        {0, 0, 0},
        {0, 3, 0},
        {0, 0, 0}
    };
    Map map(3, 3, cells);
    Position start{0, 0};
    Position goal{2, 0};

    auto path = PathFinder::findPath(start, goal, map);
    assert(!path.empty());

    Position curr = start;
    for (int dir : path) {
        curr = map.nextPosition(curr, dir);
        assert(map.canMove(curr));
    }
    assert(curr == goal);

    std::cout << "[PASS] PathFinder tests passed!" << std::endl;
}

void test_validator() {
    GameConfig config;
    config.map.height = 3;
    config.map.width = 3;
    config.map.cells = {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    };
    config.daySteps = {15};

    GameState state;
    state.day = 0;
    Agent a;
    a.kind = 0;
    a.pos = 0;
    a.fuel = 20;
    state.agents.push_back(a);

    // Test valid wait action [-15]
    std::vector<std::vector<int>> validActions = {{-15}};
    assert(ActionValidator::validate(config, state, validActions));

    // Test invalid step count [-10]
    std::vector<std::vector<int>> invalidActions = {{-10}};
    assert(!ActionValidator::validate(config, state, invalidActions));

    std::cout << "[PASS] ActionValidator tests passed!" << std::endl;
}

void test_solver_cell_cost() {
    std::vector<std::vector<int>> cells = {
        {0, 1, 0},
        {2, 3, 0}
    };
    Map map(2, 3, cells);

    GameState state;
    state.traffics.push_back({1, 2});

    double normalCost = Solver::getCellCost(map, state, 0);
    double trafficCost = Solver::getCellCost(map, state, 1);
    double blockedCost = Solver::getCellCost(map, state, Position{1, 1});
    double outOfBoundsCost = Solver::getCellCost(map, state, Position{-1, 0});

    assert(normalCost > 0.0);
    assert(trafficCost > normalCost);
    assert(blockedCost >= Solver::INF_COST);
    assert(outOfBoundsCost >= Solver::INF_COST);

    std::cout << "[PASS] Solver cell cost tests passed!" << std::endl;
}

int main() {
    test_map_and_geometry();
    test_pathfinder();
    test_validator();
    test_solver_cell_cost();
    std::cout << "All unit tests completed successfully!" << std::endl;
    return 0;
}
