#include "io/JsonWriter.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string JsonWriter::agentTypesJson(const std::vector<int>& agentTypes) {
    return json(agentTypes).dump();
}

std::string JsonWriter::actionsJson(const std::vector<std::vector<int>>& actions) {
    return json(actions).dump();
}

void JsonWriter::writeAgentTypes(const std::vector<int>& agentTypes) {
    std::cout << agentTypesJson(agentTypes) << std::endl;
}

void JsonWriter::writeActions(const std::vector<std::vector<int>>& actions) {
    std::cout << actionsJson(actions) << std::endl;
}
