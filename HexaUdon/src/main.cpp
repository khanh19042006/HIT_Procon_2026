#include <iostream>
#include "io/JsonReader.hpp"
#include "io/JsonWriter.hpp"
#include "map/Map.hpp"
#include "solver/Solver.hpp"
#include "solver/ActionValidator.hpp"

int main() {
    // Tối ưu hóa tốc độ nhập/xuất chuẩn
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    // 1. Khởi tạo & nạp toàn bộ cấu hình trận đấu ban đầu (GameConfig) từ stdin
    GameConfig config = JsonReader::readGameConfig();

    // 2. Khởi tạo bản đồ (Map) với đầy đủ thông tin kích thước và địa hình (cells)
    Map map(config.map.height, config.map.width, config.map.cells);

    // 3. Khởi tạo Bộ giải thuật (Solver)
    Solver solver;

    // 4. Quyết định loại xe (巡回車 / 補給車) và ghi output JSON cho Server
    auto agentTypes = solver.decideAgentTypes(config);
    JsonWriter::writeAgentTypes(agentTypes);

    // 5. Vòng lặp thực thi từng ngày thi đấu
    for (size_t day = 0; day < config.daySteps.size(); ++day) {
        // Đọc trạng thái thay đổi của ngày thi đấu hiện tại (GameState)
        GameState state = JsonReader::readGameState();

        // Gọi hàm giải thuật chính (map sẽ được cập nhật traffic bên trong)
        auto actions = solver.solve(config, state, map);

        // Kiểm tra tính hợp lệ của Action Plan (map đã có traffic mới nhất)
        if (!ActionValidator::validate(config, state, actions, map)) {
            // Nếu bị vi phạm, tự động dùng kế hoạch an toàn (đứng yên)
            std::cerr << "[WARNING] Action plan for day " << day << " was invalid! Using fallback action." << std::endl;
            actions = solver.createFallbackActions(config, state);
        }

        // Xuất kết quả Action Plan dạng JSON ra stdout cho Server
        JsonWriter::writeActions(actions);
    }

    return 0;
}