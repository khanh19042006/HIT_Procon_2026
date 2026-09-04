#pragma once

#include <string>

#include "GameState.hpp"
#include "model/GameConfig.hpp"

class ApiClient {
public:
    static ApiClient fromEnv(const std::string& envPath = ".env");

    GameConfig getConfig() const;
    GameState getStatus() const;
    void submitAgentTypes(const std::string& jsonBody) const;
    void submitActions(const std::string& jsonBody) const;

private:
    ApiClient(std::string baseUrl, std::string token, std::string matchId, bool trace);
    std::string request(const std::string& method, const std::string& path, const std::string& body = "") const;
    std::string endpoint(const std::string& suffix) const;

    std::string baseUrl;
    std::string token;
    std::string matchId;
    bool trace;
};