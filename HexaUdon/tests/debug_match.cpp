#include <iostream>
#include <iomanip>
#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include "solver/Solver.hpp"

int main() {
    // === REAL MATCH DATA from 7bdcc34e ===
    GameConfig config;
    config.startsAt = 0;
    config.daySeconds = {60, 60, 60, 60};
    config.daySteps = {20, 40, 60, 80};
    config.fuelLimit = 20;
    config.players = 2;
    config.busyThreshold = 4;
    config.jammedThreshold = 8;

    config.map.height = 10;
    config.map.width = 10;
    config.map.cells = {
        {0, 0, 0, 0, 0, 0, 2, 0, 0, 0},
        {3, 0, 3, 0, 1, 0, 0, 1, 0, 1},
        {0, 0, 0, 0, 1, 2, 2, 0, 0, 0},
        {0, 0, 0, 2, 1, 0, 0, 1, 1, 2},
        {1, 0, 1, 1, 0, 1, 0, 0, 0, 1},
        {0, 1, 3, 0, 0, 0, 0, 0, 0, 0},
        {2, 3, 0, 1, 0, 3, 3, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 3, 0, 0, 0, 0, 2, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0}
    };

    config.spots = {
        {0, 15, 4},  // brand 0, pos 15 -> (5,1)
        {1, 57, 2},  // brand 1, pos 57 -> (7,5)
        {2, 97, 1},  // brand 2, pos 97 -> (7,9)
        {3, 51, 1},  // brand 3, pos 51 -> (1,5)
        {0, 66, 4},  // brand 0, pos 66 -> (6,6)
        {1, 28, 3}   // brand 1, pos 28 -> (8,2)
    };

    config.initialAgentPositions = {8, 81, 45, 33};

    Map map(10, 10, config.map.cells);

    // === VERIFY SPOT POSITIONS ===
    std::cout << "=== SPOT POSITIONS ===" << std::endl;
    for (size_t i = 0; i < config.spots.size(); ++i) {
        int pos = config.spots[i].pos;
        Position p = map.posToCoordinate(pos);
        int terrain = map.getCell(p);
        std::cout << "  Spot #" << i
                  << " brand=" << config.spots[i].brand
                  << " pos=" << pos
                  << " -> (" << p.x << "," << p.y << ")"
                  << " terrain=" << terrain
                  << (terrain == 0 ? " (Plain)" : terrain == 1 ? " (Road)" : terrain == 2 ? " (Mountain)" : " (Pond)")
                  << " canMove=" << map.canMove(p)
                  << std::endl;
    }

    // === VERIFY AGENT POSITIONS ===
    std::cout << "\n=== AGENT POSITIONS ===" << std::endl;
    for (size_t i = 0; i < config.initialAgentPositions.size(); ++i) {
        int pos = config.initialAgentPositions[i];
        Position p = map.posToCoordinate(pos);
        int terrain = map.getCell(p);
        std::cout << "  Agent #" << i
                  << " pos=" << pos
                  << " -> (" << p.x << "," << p.y << ")"
                  << " terrain=" << terrain
                  << std::endl;
    }

    // === TEST PATHFINDING: Agent #0 (pos=8) -> Each Spot ===
    std::cout << "\n=== PATHFINDING: Agent #0 (pos=8) -> ALL SPOTS ===" << std::endl;
    Position agentPos = map.posToCoordinate(8);
    for (size_t i = 0; i < config.spots.size(); ++i) {
        Position spotPos = map.posToCoordinate(config.spots[i].pos);
        auto path = PathFinder::findPath(agentPos, spotPos, map, 20);
        std::cout << "  -> Spot #" << i
                  << " (" << spotPos.x << "," << spotPos.y << ")"
                  << " found=" << path.found
                  << " steps=" << path.totalSteps
                  << " fuel=" << path.totalFuel
                  << " dirs=" << path.directions.size()
                  << std::endl;

        if (path.found && !path.directions.empty()) {
            std::cout << "    Path: ";
            Position cur = agentPos;
            for (int d : path.directions) {
                Position next = map.nextPosition(cur, d);
                std::cout << "(" << cur.x << "," << cur.y << ")->"
                          << "d" << d << "->"
                          << "(" << next.x << "," << next.y << ") ";
                cur = next;
            }
            std::cout << std::endl;
        }
    }

    // === RUN SOLVER for Day 0 ===
    std::cout << "\n=== SOLVER DAY 0 (20 steps, fuel=20) ===" << std::endl;
    GameState state;
    state.day = 0;
    state.endsAt = 60;
    state.agents = {
        {0, 8, 20},   // Patrol
        {0, 81, 20},  // Patrol
        {0, 45, 20},  // Patrol
        {1, 33, 20}   // Supply
    };

    Solver solver;
    auto actions = solver.solve(config, state, map);

    for (size_t i = 0; i < actions.size(); ++i) {
        std::cout << "  Agent #" << i << " actions(" << actions[i].size() << "): [";
        int totalSteps = 0;
        int moveCount = 0;
        for (size_t j = 0; j < actions[i].size(); ++j) {
            if (j > 0) std::cout << ", ";
            if (actions[i][j] >= 0 && actions[i][j] <= 5) {
                std::cout << "d" << actions[i][j];
                moveCount++;
            } else {
                std::cout << "wait" << (-actions[i][j]);
            }
        }
        std::cout << "] moves=" << moveCount << std::endl;

        // Simulate to show final position
        Position pos = map.posToCoordinate(state.agents[i].pos);
        for (int act : actions[i]) {
            if (act >= 0 && act <= 5) {
                pos = map.nextPosition(pos, act);
            }
        }
        std::cout << "    Final pos: (" << pos.x << "," << pos.y << ") = "
                  << map.coordinateToPos(pos) << std::endl;
    }

    return 0;
}
