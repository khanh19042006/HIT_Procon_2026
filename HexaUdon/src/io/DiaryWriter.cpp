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
    return kind == 0 ? "Patrol" : "Supply";
}

std::string DiaryWriter::spotName(int spotIndex, const GameConfig& config) {
    if (spotIndex < 0 || spotIndex >= static_cast<int>(config.spots.size())) {
        return "Khong co Spot muc tieu";
    }

    const Spot& spot = config.spots[spotIndex];
    return "Spot #" + std::to_string(spotIndex) +
           " (brand=" + std::to_string(spot.brand) +
           ", pos=" + std::to_string(spot.pos) + ")";
}

static std::string formatCellSummary(const Position& position) {
    return formatPosition(position) + " (cell=" + std::to_string(position.x + position.y * 1000) + ")";
}

static std::string describeCurrentTarget(
    int targetSpot,
    const GameConfig& config,
    const Map& map
) {
    if (targetSpot < 0 || targetSpot >= static_cast<int>(config.spots.size())) {
        return "Khong co muc tieu";
    }

    const Spot& spot = config.spots[targetSpot];
    Position spotPos = map.posToCoordinate(spot.pos);
    return "Spot #" + std::to_string(targetSpot) +
           " (brand=" + std::to_string(spot.brand) + ", o=" +
           formatPosition(spotPos) + ")";
}

std::string DiaryWriter::actionName(int action) {
    if (action < 0) {
        return "Cho " + std::to_string(-action) + " step";
    }
    if (action <= 5) {
        return "Di chuyen huong " + std::to_string(action);
    }
    return "Action khong hop le " + std::to_string(action);
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
    output << "- Vi tri dau ngay: " << formatPosition(currentPosition)
           << " (cell=" << agent.pos << ")\n";
    output << "- Nhien lieu dau ngay: " << agent.fuel << "\n";
    output << "- Muc tieu hien tai: " << describeCurrentTarget(targetSpot, config, map) << "\n";
    output << "- Dia diem den: " << spotName(targetSpot, config) << "\n";
    output << "- Mang action cuoi ngay: " << formatActions(actions) << "\n\n";
    output << "| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |\n";
    output << "|---:|---|---|---|---|---:|\n";

    for (int action : actions) {
        int startStep = currentStep;
        int duration = 1;
        Position nextPosition = currentPosition;
        std::string targetDescription = "Dung yen tai " + formatPosition(currentPosition);

        if (action < 0) {
            duration = -action;
        } else if (action <= 5) {
            duration = map.getTravelTime(currentPosition);
            if (agent.kind == 0) {
                currentFuel -= map.getFuelCost(currentPosition);
            }
            nextPosition = map.nextPosition(currentPosition, action);
            targetDescription = "Di chuyen den " + formatPosition(nextPosition);

            if (targetSpot >= 0 && targetSpot < static_cast<int>(config.spots.size())) {
                Position spotPos = map.posToCoordinate(config.spots[targetSpot].pos);
                if (nextPosition == spotPos) {
                    targetDescription = "Dat muc tieu " + spotName(targetSpot, config);
                } else {
                    targetDescription += "; huong toi " + spotName(targetSpot, config);
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
        output << "| - | Khong co action | " << formatPosition(currentPosition)
               << " | " << formatPosition(currentPosition)
               << " | Dung yen tai vi tri hien tai | " << currentFuel << " |\n";
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

    output << "# Nhat ky hanh trinh - Ngay " << day << "\n\n";
    output << "- So step trong ngay: " << daySteps << "\n";
    output << "- So xe: " << state.agents.size() << "\n";
    output << "- Ke hoach: " << (usedFallback ? "Fallback" : "Solver") << "\n\n";

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
