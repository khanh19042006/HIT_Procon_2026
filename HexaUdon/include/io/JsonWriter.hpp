#pragma once

#include <vector>
#include <string>

class JsonWriter {
public:
    static std::string agentTypesJson(const std::vector<int>& agentTypes);
    static std::string actionsJson(const std::vector<std::vector<int>>& actions);
    static void writeAgentTypes(const std::vector<int>& agentTypes);
    static void writeActions(const std::vector<std::vector<int>>& actions);
};
