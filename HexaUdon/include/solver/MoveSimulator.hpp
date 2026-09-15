#pragma once

#include "map/Map.hpp"
#include <vector>

/**
 * @brief Kết quả sau khi mô phỏng 1 đoạn di chuyển.
 */
struct SimResult {
    std::vector<int> actions;  // Chuỗi hành động (directions + waits)
    Position finalPos;         // Vị trí cuối cùng sau di chuyển
    int stepsUsed = 0;         // Tổng số bước đã tiêu tốn
    int fuelUsed = 0;          // Tổng xăng đã tiêu tốn
};

/**
 * @brief Mô phỏng di chuyển trên bản đồ.
 *
 * Module này chịu trách nhiệm DUY NHẤT: biến danh sách hướng đi (từ PathFinder)
 * thành chuỗi hành động hợp lệ, đồng thời tính toán vị trí + xăng sau di chuyển.
 */
class MoveSimulator {
public:
    /**
     * @brief Biến path directions -> chuỗi hành động + mô phỏng kết quả.
     *
     * Duyệt từng hướng đi, kiểm tra xăng/bước, tạo actions và cập nhật
     * vị trí/xăng cuối cùng sau di chuyển.
     *
     * @param pathDirs       Mảng hướng đi từ PathFinder (VD: [2, 3, 4])
     * @param map            Bản đồ (tra cứu travel time, fuel cost)
     * @param startPos       Toạ độ xuất phát
     * @param maxSteps       Số bước tối đa cho phép
     * @param availableFuel  Xăng hiện có
     * @param isPatrol       true = xe Patrol (tốn xăng), false = xe Supply
     * @return SimResult chứa actions, vị trí cuối, steps/fuel đã dùng
     */
    static SimResult simulate(
        const std::vector<int>& pathDirs,
        const Map& map,
        Position startPos,
        int maxSteps,
        int availableFuel,
        bool isPatrol
    );

    /**
     * @brief Thêm hành động "Chờ" vào cuối chuỗi actions cho đủ daySteps.
     *
     * @param actions    [IN/OUT] Chuỗi hành động sẽ được thêm Wait vào cuối
     * @param stepsUsed  Tổng bước đã dùng
     * @param daySteps   Tổng bước yêu cầu của ngày
     */
    static void padWithWait(
        std::vector<int>& actions,
        int stepsUsed,
        int daySteps
    );
};
