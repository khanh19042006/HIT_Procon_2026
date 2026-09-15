#include <iostream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif
#include "io/JsonReader.hpp"
#include "map/Map.hpp"
#include "solver/Solver.hpp"
#include "solver/ActionValidator.hpp"

/**
 * @brief Chương trình Runner & Test Visualizer hiển thị Tiếng Việt có dấu và chú thích Tiếng Anh
 */
int main() {
#ifdef _WIN32
    // Cấu hình Console Output hiển thị chuẩn UTF-8 (Tiếng Việt & Nhật) trên Windows
    SetConsoleOutputCP(65001);
#endif

    std::cout << "========================================================================\n";
    std::cout << "  HE THTONG KIEM THU & CONG CU DEBUG THEO DOI AGENT (TEST VISUALIZER)   \n";
    std::cout << "========================================================================\n\n";

    // 1. Đọc thông tin GameConfig từ stdin
    std::cout << "[BƯỚC 1 / STEP 1] Đang đọc cấu hình trận đấu (GameConfig) từ stdin...\n";
    GameConfig config = JsonReader::readGameConfig();
    Map map(config.map.height, config.map.width, config.map.cells);

    std::cout << " -> Kích thước bản đồ (Map Size)     : " << config.map.width << "x" << config.map.height << "\n";
    std::cout << " -> Số lượng xe (Agent Count)        : " << config.initialAgentPositions.size() << "\n";
    std::cout << " -> Số gian hàng Udon (Spot Count)   : " << config.spots.size() << "\n";
    std::cout << " -> Số ngày thi đấu (Total Days)     : " << config.daySteps.size() << "\n";
    std::cout << " -> Giới hạn bình nhiên liệu (Fuel)  : " << config.fuelLimit << "\n\n";

    Solver solver;

    // 2. Lựa chọn phân bổ loại xe Agent
    auto agentTypes = solver.decideAgentTypes(config);
    std::cout << "[BƯỚC 2 / STEP 2] Quyền phân bổ loại xe (Agent Types: 0 = Patrol, 1 = Supply):\n";
    for (size_t i = 0; i < agentTypes.size(); ++i) {
        std::cout << " -> Xe (Agent) [" << i << "]: " 
                  << (agentTypes[i] == 0 ? "Xe Tuần Tra (Patrol Car)" : "Xe Tiếp Tế (Supply Car)") << "\n";
    }
    std::cout << "\n";

    // 3. Duyệt kiểm tra từng ngày thi đấu và in chỉ số xe
    for (size_t day = 0; day < config.daySteps.size(); ++day) {
        std::cout << "------------------------------------------------------------------------\n";
        std::cout << "       NGÀY THI ĐẤU " << day << " (DAY " << day << " - Bước quy định / Steps: " << config.daySteps[day] << ")\n";
        std::cout << "------------------------------------------------------------------------\n";

        GameState state = JsonReader::readGameState();

        std::cout << " [THEO DÕI TRẠNG THÁI AGENT TRƯỚC KHI TÍNH TOÁN / INSPECTING AGENTS]:\n";
        for (size_t i = 0; i < state.agents.size(); ++i) {
            Position pos = map.posToCoordinate(state.agents[i].pos);
            std::cout << "  * Xe (Agent) #" << i 
                      << " | Vị trí (Pos Index): " << std::setw(3) << state.agents[i].pos 
                      << " (Tọa độ 2D: x=" << pos.x << ", y=" << pos.y << ")"
                      << " | Nhiên liệu (Fuel): " << std::setw(2) << state.agents[i].fuel
                      << " | Loại xe (Kind): " << (state.agents[i].kind == 0 ? "Xe Tuần Tra (Patrol)" : "Xe Tiếp Tế (Supply)") << "\n";
        }

        // Tính toán kế hoạch nước đi
        auto actions = solver.solve(config, state, map);
        bool isValid = ActionValidator::validate(config, state, actions, map);

        std::cout << "\n [KẾT QUẢ KẾ HOẠCH HÀNH ĐỘNG & KIỂM TRA / ACTION PLAN & VALIDATION]:\n";
        std::cout << "  * Kết quả kiểm tra (Validation): " 
                  << (isValid ? "HỢP LỆ (VALID - CHÍNH XÁC)" : "KHÔNG HỢP LỆ (INVALID - LỖI LOGIC!)") << "\n";

        for (size_t i = 0; i < actions.size(); ++i) {
            std::cout << "  * Xe (Agent) #" << i << " Lệnh (Actions): [";
            int stepSum = 0;
            Position currentPos = map.posToCoordinate(state.agents[i].pos);
            
            for (size_t k = 0; k < actions[i].size(); ++k) {
                int act = actions[i][k];
                if (act < 0) {
                    stepSum += (-act);
                    std::cout << "Chờ(Wait " << -act << ")";
                } else {
                    int tTime = map.getTravelTime(currentPos);
                    stepSum += tTime;
                    std::cout << "Đi_hướng_" << act << "(tốn " << tTime << " step)";
                    currentPos = map.nextPosition(currentPos, act);
                }
                if (k + 1 < actions[i].size()) std::cout << ", ";
            }
            std::cout << "] (Tổng bước / Total steps: " << stepSum << "/" << config.daySteps[day] << ")\n";
        }
        std::cout << "\n";
    }

    std::cout << "========================================================================\n";
    std::cout << "          HOÀN THÀNH QUÁ TRÌNH KIỂM THỬ (RUNNER COMPLETED)             \n";
    std::cout << "========================================================================\n";

    return 0;
}
