#include <iostream>
#include <chrono>
#include <memory>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "io/ApiClient.hpp"
#include "io/JsonReader.hpp"
#include "io/JsonWriter.hpp"
#include "map/Map.hpp"
#include "solver/Solver.hpp"
#include "solver/ActionValidator.hpp"

int main(int argc, char** argv) {
    // Tối ưu hóa tốc độ nhập/xuất chuẩn
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    const bool stdinMode = argc > 1 && std::string(argv[1]) == "--stdin";
    std::unique_ptr<ApiClient> client;
    if (!stdinMode) client = std::make_unique<ApiClient>(ApiClient::fromEnv());
    GameConfig config;
    if (stdinMode) {
        config = JsonReader::readGameConfig();
    } else {
        while (true) {
            try {
                config = client->getConfig();
                break;
            } catch (const std::exception& error) {
                std::cerr << "[API] Config is not available yet: " << error.what() << std::endl;
#ifdef _WIN32
                Sleep(1000);
#else
                usleep(1000000);
#endif
            }
        }
    }

    // 2. Khởi tạo bản đồ (Map) với đầy đủ thông tin kích thước và địa hình (cells)
    Map map(config.map.height, config.map.width, config.map.cells);

    // 3. Khởi tạo Bộ giải thuật (Solver) và Bộ kiểm tra tính hợp lệ (Validator)
    Solver solver;
    ActionValidator validator;

    // 4. Quyết định loại xe (巡回車 / 補給車) và ghi output JSON cho Server
    auto agentTypes = solver.decideAgentTypes(config);
    if (stdinMode) JsonWriter::writeAgentTypes(agentTypes);
    else client->submitAgentTypes(JsonWriter::agentTypesJson(agentTypes));

    // 5. Vòng lặp thực thi từng ngày thi đấu
    int lastDay = -1;
    for (size_t day = 0; day < config.daySteps.size(); ++day) {
        // Đọc trạng thái thay đổi của ngày thi đấu hiện tại (GameState)
        GameState state;
        if (stdinMode) {
            state = JsonReader::readGameState();
        } else {
            do {
                state = client->getStatus();
                if (state.day <= lastDay) {
#ifdef _WIN32
                    Sleep(250);
#else
                    usleep(250000);
#endif
                }
            } while (state.day <= lastDay);
        }
        lastDay = state.day;

        // Gọi hàm giải thuật chính (Solver::solve) truyền vào toàn bộ dữ liệu cần thiết
        auto actions = solver.solve(config, state, map);

        // Kiểm tra tính hợp lệ của Action Plan
        if (!validator.validate(config, state, actions)) {
            // Nếu bị vi phạm (sai số bước, ra ngoài bản đồ), tự động dùng kế hoạch an toàn (đứng yên)
            std::cerr << "[WARNING] Action plan for day " << day << " was invalid! Using fallback action." << std::endl;
            actions = solver.createFallbackActions(config, state);
        }

        // Xuất kết quả Action Plan dạng JSON ra stdout cho Server
        if (stdinMode) JsonWriter::writeActions(actions);
        else client->submitActions(JsonWriter::actionsJson(actions));
    }

    return 0;
}