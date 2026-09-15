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

    /**
     * @brief Trả về Spot mục tiêu cuối cùng Solver đã lập cho một xe.
     *
     * Hàm chỉ đọc trạng thái kế hoạch, phục vụ SupplyPlanner và DiaryWriter.
     */
    int getPlannedTargetSpot(int agentIdx) const;

    /**
     * @brief Trả về đúng vị trí đích mà planner đã truyền cho PathFinder.
     */
    Position getPlannedTargetPosition(int agentIdx) const;

    /**
     * @brief Trả về xe Patrol mà một xe Supply đã chọn để hỗ trợ.
     */
    int getSupportedPatrol(int agentIdx) const;

    /** @brief Trả về Spot mục tiêu được planner ghi cho một step cụ thể. */
    int getPlannedStepSpot(int agentIdx, int step) const;

    /** @brief Trả về tọa độ mục tiêu được planner ghi cho một step cụ thể. */
    Position getPlannedStepPosition(int agentIdx, int step) const;

private:
    // === Trạng thái xuyên trận ===
    std::set<int> collectedBrandsTotal_;   // Brand đã thu thập toàn trận
    int currentDay_ = -1;

    // === Trạng thái hàng ngày (reset mỗi ngày) ===
    std::vector<int> remainingStock_;                  // Stock còn lại mỗi Spot
    std::vector<std::set<int>> visitedSpotsToday_;     // Spot đã ghé mỗi xe
    std::vector<int> currentTargets_;                  // Spot mục tiêu mỗi xe (cho Supply)
    std::vector<Position> currentTargetPositions_;     // Vị trí đích chính xác mỗi xe
    std::vector<int> supportedPatrols_;                // Patrol được Supply hỗ trợ
    std::vector<std::vector<int>> plannedStepSpots_;   // Spot mục tiêu theo từng step
    std::vector<std::vector<Position>> plannedStepPositions_; // Tọa độ mục tiêu theo từng step

    void resetDailyState(const GameConfig& config, int numAgents);
};
