#pragma once

#include <vector>

class JsonWriter {
public:
    static void writeAgentTypes(const std::vector<int>& agentTypes);
    static void writeActions(const std::vector<std::vector<int>>& actions);
};
