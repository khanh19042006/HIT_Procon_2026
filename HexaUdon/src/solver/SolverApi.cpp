#include "solver/SolverApi.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "io/JsonReader.hpp"
#include "map/Map.hpp"
#include "solver/ActionValidator.hpp"
#include "solver/Solver.hpp"

using json = nlohmann::json;

namespace {
template <typename Value>
Value readJsonValue(const json& value, Value (*reader)()) {
    std::stringstream input(value.dump());
    std::streambuf* original = std::cin.rdbuf(input.rdbuf());
    try {
        Value result = reader();
        std::cin.rdbuf(original);
        return result;
    } catch (...) {
        std::cin.rdbuf(original);
        throw;
    }
}

GameConfig readConfig() { return JsonReader::readGameConfig(); }
GameState readState() { return JsonReader::readGameState(); }
}

json SolverApi::solve(const json& request) {
    if (!request.contains("config") || !request.contains("state")) {
        throw std::runtime_error("solve request requires config and state");
    }

    const GameConfig config = readJsonValue<GameConfig>(request["config"], readConfig);
    const GameState state = readJsonValue<GameState>(request["state"], readState);
    const Map map(config.map.height, config.map.width, config.map.cells);
    Solver solver;
    const auto proposedActions = solver.solve(config, state, map);
    const bool valid = ActionValidator::validate(config, state, proposedActions);
    const auto actions = valid ? proposedActions : solver.createFallbackActions(config, state);

    return { {"actions", actions}, {"valid", valid}, {"day", state.day} };
}

json SolverApi::agentTypes(const json& request) {
    if (!request.contains("config")) throw std::runtime_error("agent-types request requires config");
    const GameConfig config = readJsonValue<GameConfig>(request["config"], readConfig);
    Solver solver;
    return { {"agentTypes", solver.decideAgentTypes(config)} };
}