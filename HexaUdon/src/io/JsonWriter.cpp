#include "io/JsonWriter.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void JsonWriter::writeAgentTypes(const std::vector<int>& agentTypes) {
    json output = agentTypes;
    std::cout << output.dump() << std::endl;
}

void JsonWriter::writeActions(const std::vector<std::vector<int>>& actions) {
    json output = actions;
    std::cout << output.dump() << std::endl;
}
