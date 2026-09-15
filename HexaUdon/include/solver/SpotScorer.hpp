#pragma once

#include "model/GameConfig.hpp"
#include "map/Map.hpp"
#include "solver/PathFinder.hpp"
#include <vector>
#include <set>

/**
 * @brief Chấm điểm & chọn Spot tốt nhất cho xe Patrol.
 *
 * Module này chịu trách nhiệm DUY NHẤT: đánh giá mức độ hấp dẫn của 1 Spot
 * dựa trên brand mới/cũ, khoảng cách, và stock còn lại.
 */
class SpotScorer {
public:
    /**
     * @brief Chấm điểm 1 Spot cụ thể.
     * @param brand            Mã loại udon của Spot
     * @param distanceSteps    Số bước cần để tới Spot
     * @param collectedBrands  Tập hợp brand đã thu thập từ đầu trận
     * @param remainingStock   Hàng còn lại tại Spot
     * @return Điểm số (cao hơn = tốt hơn), -1 nếu không hợp lệ
     */
    static int scoreSpot(
        int brand,
        int distanceSteps,
        const std::set<int>& collectedBrands,
        int remainingStock
    );

    /**
     * @brief Tìm Spot tốt nhất trong tất cả Spot trên bản đồ.
     *
     * Duyệt tất cả Spot, lọc bỏ Spot không hợp lệ (hết hàng, đã ghé,
     * không đủ xăng/bước), chấm điểm các Spot còn lại, trả về Spot ngon nhất.
     *
     * @return Index của Spot tốt nhất, -1 nếu không có Spot nào khả thi
     */
    static int findBestSpot(
        Position currentPos,
        const GameConfig& config,
        const Map& map,
        int fuelRemaining,
        int stepsRemaining,
        const std::set<int>& visitedToday,
        const std::vector<int>& remainingStock,
        const std::set<int>& collectedBrands
    );
};
