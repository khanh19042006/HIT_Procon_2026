#include "io/JsonReader.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

GameConfig JsonReader::readGameConfig() {
    json input;
    std::cin >> input;

    GameConfig config;

    config.startsAt = input["startsAt"];
    config.daySeconds = input["daySeconds"].get<std::vector<int>>();
    config.daySteps = input["daySteps"].get<std::vector<int>>();

    config.map.height = input["map"]["height"];
    config.map.width = input["map"]["width"];
    config.map.cells = input["map"]["cells"].get<std::vector<std::vector<int>>>();

    for (const auto& spotJson : input["spots"]) {
        Spot spot;
        spot.brand = spotJson["brand"];
        spot.pos = spotJson["pos"];
        spot.stocks = spotJson["stocks"];
        config.spots.push_back(spot);
    }

    config.initialAgentPositions = input["agents"].get<std::vector<int>>();
    config.fuelLimit = input["fuelLimits"];
    config.players = input["players"];
    config.busyThreshold = input["busyThreshold"];
    config.jammedThreshold = input["jammedThreshold"];

    return config;
}

GameState JsonReader::readGameState() {
    json input;
    std::cin >> input;

    GameState state;

    state.endsAt = input["endsAt"];
    state.day = input["day"];

    for (const auto& agentJson : input["agents"]) {
        Agent agent;
        agent.kind = agentJson["kind"];
        agent.pos = agentJson["pos"];
        agent.fuel = agentJson["fuel"];
        state.agents.push_back(agent);
    }

    for (const auto& playerJson : input["others"]) {
        OtherPlayer player;
        player.id = playerJson["id"];
        for (const auto& agentJson : playerJson["agents"]) {
            Agent agent;
            agent.kind = agentJson["kind"];
            agent.pos = agentJson["pos"];
            agent.fuel = agentJson["fuel"];
            player.agents.push_back(agent);
        }
        state.others.push_back(player);
    }

    for (const auto& trafficJson : input["traffics"]) {
        Traffic traffic;
        traffic.pos = trafficJson["pos"];
        traffic.status = trafficJson["status"];
        state.traffics.push_back(traffic);
    }

    return state;
}