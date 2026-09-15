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
    Position targetPosition
) {
    if (targetSpot < 0 || targetSpot >= static_cast<int>(config.spots.size())) {
        return "Điểm đích tọa độ=" + formatPosition(targetPosition);
    }

    const Spot& spot = config.spots[targetSpot];
    return "Spot #" + std::to_string(targetSpot) +
            " (thương hiệu=" + std::to_string(spot.brand) + ", tọa độ=" +
            formatPosition(targetPosition) + ")";
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
    Position targetPosition,
    int supportedPatrol,
    const std::vector<int>& plannedStepSpots,
    const std::vector<Position>& plannedStepPositions,
    const GameConfig& config,
    const Map& map,
    const std::vector<int>& actions
) {
    Position currentPosition = map.posToCoordinate(agent.pos);
    int currentStep = 0;
    int currentFuel = agent.fuel;
    bool isSupply = agent.kind == 1;

    output << "### Xe #" << agentIndex << " - " << agentKindName(agent.kind) << "\n\n";
        output << "- Vị trí đầu ngày: " << formatPosition(currentPosition)
            << " (ô=" << agent.pos << ")\n";
        output << "- Nhiên liệu đầu ngày: " << agent.fuel << "\n";
        if (isSupply && supportedPatrol >= 0) {
            output << "- Vai trò: Hỗ trợ xe tuần tra #" << supportedPatrol << "\n";
            output << "- Điểm hẹn của xe tuần tra: " << describeCurrentTarget(targetSpot, config, targetPosition) << "\n";
        } else {
            output << "- Mục tiêu kế hoạch từ Solver: " << describeCurrentTarget(targetSpot, config, targetPosition) << "\n";
            output << "- Địa điểm đích kế hoạch: " << describeCurrentTarget(targetSpot, config, targetPosition) << "\n";
        }
        output << "- Mảng hành động đã gửi server: " << formatActions(actions) << "\n\n";
        output << "Bảng dưới đây là mô phỏng theo action đã gửi, không phải trạng thái server xác nhận sau từng bước.\n\n";
         output << "| Bước dự kiến | Hành động đã gửi | Từ ô theo mô phỏng | Đến ô dự kiến | "
             << (isSupply ? "Điểm hẹn kế hoạch" : "Mục tiêu kế hoạch")
             << " | Nhiên liệu dự kiến còn lại |\n";
    output << "|---:|---|---|---|---|---:|\n";

    for (int action : actions) {
        int startStep = currentStep;
        int duration = 1;
        Position nextPosition = currentPosition;
        int stepTargetSpot = targetSpot;
        Position stepTargetPosition = targetPosition;
        if (startStep >= 0 && startStep < static_cast<int>(plannedStepSpots.size())) {
            stepTargetSpot = plannedStepSpots[startStep];
        }
        if (startStep >= 0 && startStep < static_cast<int>(plannedStepPositions.size())) {
            stepTargetPosition = plannedStepPositions[startStep];
        }
        std::string targetDescription = "Dự kiến đứng yên tại " + formatPosition(currentPosition);

        if (action < 0) {
            duration = -action;
            if (stepTargetPosition.x >= 0 && stepTargetPosition.y >= 0) {
                if (isSupply && supportedPatrol >= 0) {
                    targetDescription += "; điểm hẹn của xe tuần tra #" +
                        std::to_string(supportedPatrol) + " tại " +
                        formatPosition(stepTargetPosition);
                } else if (stepTargetSpot >= 0 && stepTargetSpot < static_cast<int>(config.spots.size())) {
                    targetDescription += "; mục tiêu " + spotName(stepTargetSpot, config);
                } else {
                    targetDescription += "; hướng tới tọa độ " + formatPosition(stepTargetPosition);
                }
            }
        } else if (action <= 5) {
            duration = map.getTravelTime(currentPosition);
            if (agent.kind == 0) {
                currentFuel -= map.getFuelCost(currentPosition);
            }
            nextPosition = map.nextPosition(currentPosition, action);
            targetDescription = "Dự kiến di chuyển đến " + formatPosition(nextPosition);

            if (stepTargetPosition.x >= 0 && stepTargetPosition.y >= 0) {
                if (nextPosition == stepTargetPosition) {
                    if (isSupply && supportedPatrol >= 0) {
                        targetDescription = "Dự kiến đến điểm hẹn của xe tuần tra #" +
                            std::to_string(supportedPatrol) + " tại " +
                            formatPosition(stepTargetPosition);
                    } else if (stepTargetSpot >= 0 && stepTargetSpot < static_cast<int>(config.spots.size())) {
                        targetDescription = "Dự kiến đạt mục tiêu " + spotName(stepTargetSpot, config);
                    } else {
                        targetDescription = "Dự kiến đến điểm hẹn tọa độ " + formatPosition(stepTargetPosition);
                    }
                } else {
                    targetDescription += isSupply && supportedPatrol >= 0
                        ? "; hướng tới điểm hẹn của xe tuần tra #" + std::to_string(supportedPatrol) +
                            " tại " + formatPosition(stepTargetPosition)
                        : "; hướng tới tọa độ " + formatPosition(stepTargetPosition);
                    if (stepTargetSpot >= 0 && stepTargetSpot < static_cast<int>(config.spots.size())) {
                        targetDescription += " (" + spotName(stepTargetSpot, config) + ")";
                    }
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
         output << "| - | Không có hành động đã gửi | " << formatPosition(currentPosition)
               << " | " << formatPosition(currentPosition)
             << " | Không có mô phỏng di chuyển | " << currentFuel << " |\n";
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
        output << "> Vị trí, loại xe và nhiên liệu đầu ngày lấy trực tiếp từ GET /status. "
            << "Các dòng vị trí sau action và nhiên liệu còn lại là mô phỏng từ "
            << "action đã gửi, vì API không trả trạng thái sau từng bước.\n\n";

    for (size_t i = 0; i < state.agents.size(); ++i) {
        const std::vector<int> emptyActions;
        const std::vector<int>& agentActions = i < actions.size() ? actions[i] : emptyActions;
        writeAgentTimeline(
            output,
            static_cast<int>(i),
            state.agents[i],
            solver.getPlannedTargetSpot(static_cast<int>(i)),
            solver.getPlannedTargetPosition(static_cast<int>(i)),
            solver.getSupportedPatrol(static_cast<int>(i)),
            [&solver, i, daySteps]() {
                std::vector<int> spots;
                for (int step = 0; step < daySteps; ++step) {
                    spots.push_back(solver.getPlannedStepSpot(static_cast<int>(i), step));
                }
                return spots;
            }(),
            [&solver, i, daySteps]() {
                std::vector<Position> positions;
                for (int step = 0; step < daySteps; ++step) {
                    positions.push_back(solver.getPlannedStepPosition(static_cast<int>(i), step));
                }
                return positions;
            }(),
            config,
            map,
            agentActions
        );
    }

    return output.good();
}
