#pragma once

#include <string>

#include <nlohmann/json.hpp>

class SolverApi {
public:
    static nlohmann::json solve(const nlohmann::json& request);
    static nlohmann::json agentTypes(const nlohmann::json& request);
};