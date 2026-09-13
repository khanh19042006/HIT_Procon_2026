#pragma once

#include "model/GameConfig.hpp"
#include "GameState.hpp"
#include "map/Map.hpp"
#include <vector>
#include <set>

/**
 * @brief Agent type decision (Patrol vs Supply)
 */
class AgentStrategy {
public:
    static std::vector<int> decideAgentTypes(const GameConfig& config);
};

/**
 * @brief Core solver — "Nhạc trưởng" điều phối các module.
 *
 * STATEFUL: duy trì bộ nhớ xuyên ngày (brand đã thu thập, stock, targets).
 * Logic tính toán thực sự được ủy quyền cho:
 *   - SpotScorer:     Chấm điểm & chọn Spot
 *   - MoveSimulator:  Mô phỏng di chuyển
 *   - PatrolPlanner:  Lập kế hoạch xe Tuần tra
 *   - SupplyPlanner:  Lập kế hoạch xe Tiếp tế
 */
class Solver {
public:
    std::vector<int> decideAgentTypes(const GameConfig& config);

    /**
     * @brief Main solver: tạo kế hoạch hành động cho tất cả xe trong 1 ngày.
     */
    std::vector<std::vector<int>> solve(
        const GameConfig& config,
        const GameState& state,
        Map& map
    );

    /**
     * @brief Fallback: tất cả xe đứng yên cả ngày (kế hoạch an toàn)
     */
    std::vector<std::vector<int>> createFallbackActions(
        const GameConfig& config,
        const GameState& state
    );

private:
    // === Trạng thái xuyên trận ===
    std::set<int> collectedBrandsTotal_;   // Brand đã thu thập toàn trận
    int currentDay_ = -1;

    // === Trạng thái hàng ngày (reset mỗi ngày) ===
    std::vector<int> remainingStock_;                  // Stock còn lại mỗi Spot
    std::vector<std::set<int>> visitedSpotsToday_;     // Spot đã ghé mỗi xe
    std::vector<int> currentTargets_;                  // Spot mục tiêu mỗi xe (cho Supply)

    void resetDailyState(const GameConfig& config, int numAgents);
};
