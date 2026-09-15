#pragma once

#include "model/GameConfig.hpp"
#include "map/Map.hpp"
#include <vector>
#include <set>

/**
 * @brief Lập kế hoạch di chuyển cho xe Tuần tra (Patrol).
 *
 * Module này chịu trách nhiệm DUY NHẤT: cho 1 xe Patrol, tìm lộ trình
 * ghé thăm nhiều Spot nhất có thể trong 1 ngày (Greedy chaining).
 */
class PatrolPlanner {
public:
    /**
     * @brief Lập kế hoạch chuỗi Spot cho 1 xe Patrol trong 1 ngày.
     *
     * Vòng lặp Greedy: liên tục tìm Spot tốt nhất → đi tới → lấy hàng → lặp lại
     * cho đến khi hết xăng hoặc hết bước.
     *
     * @param config           Cấu hình trận đấu
     * @param map              Bản đồ
     * @param startPos         Vị trí đầu ngày
     * @param daySteps         Tổng bước trong ngày
     * @param availableFuel    Xăng đầu ngày
     * @param remainingStock   [IN/OUT] Stock bị giảm khi xe lấy hàng
     * @param visitedToday     [IN/OUT] Spot đã ghé trong ngày
     * @param collectedBrands  [IN/OUT] Brand đã thu thập (toàn trận)
     * @param lastTargetSpot   [OUT] Spot cuối cùng xe nhắm tới (để Supply đón đầu)
    * @param plannedTargetPos [OUT] Vị trí chính xác xe đang hướng tới
    * @param plannedStepSpots [OUT] Spot mục tiêu tại từng step trong ngày
    * @param plannedStepPositions [OUT] Vị trí mục tiêu tại từng step trong ngày
     * @return Chuỗi hành động hoàn chỉnh cho 1 ngày (có padding Wait)
     */
    static std::vector<int> planDay(
        const GameConfig& config,
        const Map& map,
        Position startPos,
        int daySteps,
        int availableFuel,
        std::vector<int>& remainingStock,
        std::set<int>& visitedToday,
        std::set<int>& collectedBrands,
        int& lastTargetSpot,
        Position& plannedTargetPos,
        std::vector<int>& plannedStepSpots,
        std::vector<Position>& plannedStepPositions
    );
};
