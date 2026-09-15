#pragma once

#include "GameState.hpp"
#include "map/Map.hpp"
#include "model/GameConfig.hpp"
#include "solver/Solver.hpp"
#include <ostream>
#include <string>
#include <vector>

/**
 * @brief Ghi nhật ký hành trình của toàn bộ xe trong một ngày ra Markdown.
 *
 * Module này chỉ lo việc định dạng và ghi file; nó không thay đổi kế hoạch
 * hành động hoặc trạng thái của Solver.
 */
class DiaryWriter {
public:
    /**
     * @brief Ghi một file day_<day>.md vào diary/<matchId>.
     *
     * @param diaryRoot      Thư mục gốc chứa các thư mục nhật ký.
     * @param matchId        Mã trận đấu, dùng làm tên thư mục con.
     * @param day             Số ngày đang ghi.
     * @param daySteps        Tổng số step trong ngày.
     * @param config          Cấu hình bản đồ và danh sách Spot.
     * @param state           Trạng thái xe ở đầu ngày.
     * @param map             Bản đồ dùng để mô phỏng vị trí theo action.
     * @param solver          Solver chứa mục tiêu Spot đã lập kế hoạch.
     * @param actions         Mảng action cuối cùng sẽ gửi lên server.
     * @param usedFallback    Cho biết action đã bị thay bằng kế hoạch fallback.
     * @return true nếu ghi file thành công.
     */
    static bool writeDay(
        const std::string& diaryRoot,
        const std::string& matchId,
        int day,
        int daySteps,
        const GameConfig& config,
        const GameState& state,
        const Map& map,
        const Solver& solver,
        const std::vector<std::vector<int>>& actions,
        bool usedFallback
    );

private:
    /** @brief Tạo chuỗi mô tả loại xe từ mã kind của server. */
    static std::string agentKindName(int kind);

    /** @brief Tạo chuỗi mô tả Spot mục tiêu hoặc trạng thái không có mục tiêu. */
    static std::string spotName(int spotIndex, const GameConfig& config);

    /** @brief Chuyển một mã action thành mô tả dễ đọc trong nhật ký. */
    static std::string actionName(int action);

    /** @brief Chuyển mảng action thành một dòng Markdown dễ sao chép. */
    static std::string formatActions(const std::vector<int>& actions);

    /** @brief Ghi bảng diễn biến từng action của một xe vào file Markdown. */
    static void writeAgentTimeline(
        std::ostream& output,
        int agentIndex,
        const Agent& agent,
        int targetSpot,
        Position targetPosition,
        int supportedPatrol,
        const std::vector<int>& plannedStepSpots,
        const std::vector<Position>& plannedStepPositions,
        const GameConfig& config,
        const Map& map,
        const std::vector<int>& actions
    );
};
