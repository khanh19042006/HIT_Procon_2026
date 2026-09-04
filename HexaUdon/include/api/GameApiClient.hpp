#pragma once

#include "api/HttpClient.hpp"
#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include <string>
#include <vector>

/**
 * @brief Game API client for Procon 2026 competition server.
 *
 * Endpoints:
 *   GET  /api/game/matches              - List matches
 *   GET  /api/game/matches/:id/config   - Get map config
 *   POST /api/game/matches/:id/agents   - Submit agent types
 *   GET  /api/game/matches/:id/status   - Get daily status
 *   POST /api/game/matches/:id/answer   - Submit actions
 */

struct MatchInfo {
    std::string id;
    std::string status;  // e.g. "agent_select", "started", "ended"
};

class GameApiClient {
public:
    /**
     * @param serverUrl  e.g. "https://procon26.haui.ac.vn"
     * @param apiToken   Team's x-api-token
     */
    GameApiClient(const std::string& serverUrl, const std::string& apiToken);

    // 1. List matches for team
    std::vector<MatchInfo> listMatches();

    // 2. Get match config (map, spots, agents, etc.)
    GameConfig getMatchConfig(const std::string& matchId);

    // 3. Submit agent type selection [0, 0, 1, 0]
    bool submitAgentTypes(const std::string& matchId, const std::vector<int>& types);

    // 4. Get daily match status
    GameState getMatchStatus(const std::string& matchId);

    // 5. Submit action plan [[actions_agent0], [actions_agent1], ...]
    bool submitActions(const std::string& matchId, const std::vector<std::vector<int>>& actions);

    // Get last error message
    std::string getLastError() const { return lastError_; }

private:
    HttpClient http_;
    std::string lastError_;
};
