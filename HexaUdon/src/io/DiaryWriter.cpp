#include "io/DiaryWriter.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

/** @brief Chuyển Position thành tọa độ dạng (x, y) cho Markdown. */
std::string formatPosition(Position position) {
    return "(" + std::to_string(position.x) + ", " +
           std::to_string(position.y) + ")";
}

/** @brief Chuyển danh sách số nguyên thành chuỗi đặt trong code span. */
std::string formatActionList(const std::vector<int>& actions) {
    std::ostringstream output;
    output << "`[";
    for (size_t i = 0; i < actions.size(); ++i) {
        if (i > 0) output << ", ";
        output << actions[i];
    }
    output << "]`";
    return output.str();
}

} // namespace

std::string DiaryWriter::agentKindName(int kind) {
    return kind == 0 ? "Tuần tra" : "Tiếp tế";
}

std::string DiaryWriter::spotName(int spotIndex, const GameConfig& config) {
    if (spotIndex < 0 || spotIndex >= static_cast<int>(config.spots.size())) {
        return "Không có Spot mục tiêu";
    }

    const Spot& spot = config.spots[spotIndex];
        int width = config.map.width > 0 ? config.map.width : 1;
        int x = spot.pos % width;
        int y = spot.pos / width;
    return "Spot #" + std::to_string(spotIndex) +
           " (thương hiệu=" + std::to_string(spot.brand) +
            ", tọa độ=" + formatPosition({x, y}) + ")";
}

static std::string describeCurrentTarget(
    int targetSpot,
    const GameConfig& config,
    const Map& map
) {
    if (targetSpot < 0 || targetSpot >= static_cast<int>(config.spots.size())) {
        return "Không có mục tiêu";
    }

    const Spot& spot = config.spots[targetSpot];
    Position spotPos = map.posToCoordinate(spot.pos);
    return "Spot #" + std::to_string(targetSpot) +
            " (thương hiệu=" + std::to_string(spot.brand) + ", tọa độ=" +
           formatPosition(spotPos) + ")";
}

std::string DiaryWriter::actionName(int action) {
    if (action < 0) {
        return "Chờ " + std::to_string(-action) + " bước";
    }
    if (action <= 5) {
        return "Di chuyển hướng " + std::to_string(action);
    }
    return "Hành động không hợp lệ " + std::to_string(action);
}

std::string DiaryWriter::formatActions(const std::vector<int>& actions) {
    return formatActionList(actions);
}

void DiaryWriter::writeAgentTimeline(
    std::ostream& output,
    int agentIndex,
    const Agent& agent,
    int targetSpot,
    const GameConfig& config,
    const Map& map,
    const std::vector<int>& actions
) {
    Position currentPosition = map.posToCoordinate(agent.pos);
    int currentStep = 0;
    int currentFuel = agent.fuel;

    output << "### Xe #" << agentIndex << " - " << agentKindName(agent.kind) << "\n\n";
        output << "- Vị trí đầu ngày: " << formatPosition(currentPosition)
            << " (ô=" << agent.pos << ")\n";
        output << "- Nhiên liệu đầu ngày: " << agent.fuel << "\n";
        output << "- Mục tiêu hiện tại: " << describeCurrentTarget(targetSpot, config, map) << "\n";
        output << "- Địa điểm đến: " << spotName(targetSpot, config) << "\n";
        output << "- Mảng hành động cuối ngày: " << formatActions(actions) << "\n\n";
        output << "| Bước | Hành động | Từ ô | Đến ô | Mục tiêu/Spot | Nhiên liệu còn lại |\n";
    output << "|---:|---|---|---|---|---:|\n";

    for (int action : actions) {
        int startStep = currentStep;
        int duration = 1;
        Position nextPosition = currentPosition;
        std::string targetDescription = "Đứng yên tại " + formatPosition(currentPosition);

        if (action < 0) {
            duration = -action;
        } else if (action <= 5) {
            duration = map.getTravelTime(currentPosition);
            if (agent.kind == 0) {
                currentFuel -= map.getFuelCost(currentPosition);
            }
            nextPosition = map.nextPosition(currentPosition, action);
            targetDescription = "Di chuyển đến " + formatPosition(nextPosition);

            if (targetSpot >= 0 && targetSpot < static_cast<int>(config.spots.size())) {
                Position spotPos = map.posToCoordinate(config.spots[targetSpot].pos);
                if (nextPosition == spotPos) {
                    targetDescription = "Đã đạt mục tiêu " + spotName(targetSpot, config);
                } else {
                    targetDescription += "; hướng tới " + spotName(targetSpot, config);
                }
            }
        }

        currentStep += duration;
        int endStep = currentStep - 1;
        std::string stepRange = startStep == endStep
            ? std::to_string(startStep)
            : std::to_string(startStep) + "-" + std::to_string(endStep);

        output << "| " << stepRange << " | " << actionName(action)
               << " (`" << action << "`) | "
               << formatPosition(currentPosition) << " | "
               << formatPosition(nextPosition) << " | "
               << targetDescription << " | " << currentFuel << " |\n";
        currentPosition = nextPosition;
    }

    if (actions.empty()) {
        output << "| - | Không có hành động | " << formatPosition(currentPosition)
               << " | " << formatPosition(currentPosition)
               << " | Đứng yên tại vị trí hiện tại | " << currentFuel << " |\n";
    }
    output << "\n";
}

bool DiaryWriter::writeDay(
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
) {
    std::filesystem::path directory = std::filesystem::path(diaryRoot) / matchId;
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error) return false;

    std::filesystem::path filePath = directory / ("day_" + std::to_string(day) + ".md");
    std::ofstream output(filePath);
    if (!output) return false;

    output << "# Nhật ký hành trình - Ngày " << day << "\n\n";
    output << "- Số bước trong ngày: " << daySteps << "\n";
    output << "- Số xe: " << state.agents.size() << "\n";
    output << "- Kế hoạch: " << (usedFallback ? "Dự phòng" : "Bộ giải") << "\n\n";

    for (size_t i = 0; i < state.agents.size(); ++i) {
        const std::vector<int> emptyActions;
        const std::vector<int>& agentActions = i < actions.size() ? actions[i] : emptyActions;
        writeAgentTimeline(
            output,
            static_cast<int>(i),
            state.agents[i],
            solver.getPlannedTargetSpot(static_cast<int>(i)),
            config,
            map,
            agentActions
        );
    }

    return output.good();
}
