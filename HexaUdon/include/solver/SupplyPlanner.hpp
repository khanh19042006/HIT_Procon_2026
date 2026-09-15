#pragma once

#include "model/GameConfig.hpp"
#include "model/Agent.hpp"
#include "map/Map.hpp"
#include <vector>

/**
 * @brief Lập kế hoạch di chuyển cho xe Tiếp tế (Supply).
 *
 * Module này chịu trách nhiệm DUY NHẤT: tìm xe Patrol cần cứu nhất,
 * xác định điểm hẹn (rendezvous), và lập đường đi tới đó.
 */
class SupplyPlanner {
public:
    /**
     * @brief Tìm xe Patrol đang cần xăng nhất.
     *
     * Duyệt tất cả xe, lọc lấy xe Patrol (kind=0), trả về xe có
     * lượng xăng thấp nhất.
     *
     * @param agents      Danh sách tất cả xe
     * @param excludeIdx  Index của xe Supply đang gọi (để không tự chọn mình)
     * @return Index của xe Patrol ít xăng nhất, -1 nếu không tìm thấy
     */
    static int findTargetPatrol(
        const std::vector<Agent>& agents,
        int excludeIdx
    );

    /**
     * @brief Lập kế hoạch cho 1 xe Supply trong 1 ngày.
     *
     * Tìm xe Patrol cần cứu → đọc Spot mục tiêu của xe đó →
     * tìm đường đến điểm hẹn → tạo chuỗi hành động.
     *
     * @param config             Cấu hình trận đấu
     * @param map                Bản đồ
     * @param supplyAgent        Thông tin xe Supply
     * @param allAgents          Danh sách tất cả xe
     * @param supplyIdx          Index của xe Supply
     * @param daySteps           Tổng bước trong ngày
     * @param patrolTargetSpots  Spot mục tiêu của từng xe Patrol
    * @param patrolTargetPositions Vị trí đích chính xác của từng xe Patrol
    * @param plannedTargetPatrol  [OUT] Xe Patrol mà Supply đang hỗ trợ
        * @param plannedTargetSpot  [OUT] Spot điểm hẹn mà xe Supply đã chọn
        * @param plannedTargetPos   [OUT] Vị trí chính xác của điểm hẹn
    * @param plannedStepSpots   [OUT] Spot điểm hẹn tại từng step
    * @param plannedStepPositions [OUT] Vị trí điểm hẹn tại từng step
     * @return Chuỗi hành động hoàn chỉnh cho 1 ngày (có padding Wait)
     */
    static std::vector<int> planDay(
        const GameConfig& config,
        const Map& map,
        const Agent& supplyAgent,
        const std::vector<Agent>& allAgents,
        int supplyIdx,
        int daySteps,
        const std::vector<int>& patrolTargetSpots,
        const std::vector<Position>& patrolTargetPositions,
        int& plannedTargetPatrol,
        int& plannedTargetSpot,
        Position& plannedTargetPos,
        std::vector<int>& plannedStepSpots,
        std::vector<Position>& plannedStepPositions
    );
};
