#include "solver/Solver.hpp"
#include <iostream>
#include <limits>
#include <random>

/**
 * @brief Chuyển đổi loại địa hình / terrain sang chi phí di chuyển tương đối.
 *
 * Mục đích:
 * - Đánh giá mức độ khó của một ô dựa trên loại địa hình.
 * - Dùng cho debug, heuristic và đánh giá sơ bộ đường đi.
 *
 * Đầu vào:
 * @param cellType Loại ô trong bản đồ (0: đất bằng, 1: cỏ / khó đi, 2: đường, 3: hồ nước / cấm, 4: núi).
 *
 * Đầu ra:
 * @return Chi phí tương ứng với ô đó; nếu ô bị chặn thì trả về INF_COST.
 *
 * Thuật toán / logic:
 * - Gán chi phí cố định cho từng loại terrain.
 * - O ô cấm hoặc hồ nước sẽ trả về giá trị vô cùng để biểu thị không thể đi vào.
 *
 * Giới hạn:
 * - Đây chỉ là mô hình heuristic nhanh, không phải chi phí chính xác của engine.
 * - Không tính thêm yếu tố fuel, thời gian, hoặc tương tác giữa các xe.
 *
 * Ghi chú:
 * - Dùng trong giai đoạn phát triển và debug để kiểm tra đường đi sơ bộ.
 */
static double terrainCostFromCell(int cellType) {
    switch (cellType) {
        case 0: return 1.0;   // flat ground
        case 1: return 2.0;   // rough / grass
        case 2: return 1.5;   // road
        case 3: return Solver::INF_COST; // lake / blocked
        case 4: return 3.0;   // mountain / obstacle
        default: return 2.5;  // generic terrain
    }
}

/**
 * @brief Tính chi phí tổng hợp của một ô theo mã 1D (cell index).
 *
 * Mục đích:
 * - Chuyển một ô trong hệ số 1D sang chi phí đánh giá để dùng trong thuật toán đường đi.
 * - Kiểm tra xem ô đó có nằm trong bản đồ và có thể đi vào hay không.
 *
 * Đầu vào:
 * @param map Bản đồ hex hiện tại.
 * @param state Trạng thái trò chơi, bao gồm danh sách traffic.
 * @param pos Chỉ số 1D của ô cần đánh giá.
 *
 * Đầu ra:
 * @return Chi phí tổng hợp của ô đó; nếu ô ngoài bản đồ hoặc không đi được thì trả về INF_COST.
 *
 * Thuật toán / logic:
 * - Nếu pos nằm ngoài phạm vi bản đồ thì trả về INF_COST.
 * - Nếu ô hợp lệ thì chuyển pos về tọa độ 2D và gọi overload 2D.
 *
 * Giới hạn:
 * - Hàm này chỉ đánh giá cho một ô, không tìm đường.
 * - Không bao gồm chi phí nhiên liệu, thời gian hoặc tương tác giữa các xe.
 *
 * Ghi chú:
 * - Hàm này là nền tảng để xây dựng hệ thống pathfinding sau này.
 */
double Solver::getCellCost(const Map& map, const GameState& state, int pos) {
    if (pos < 0 || pos >= map.getHeight() * map.getWidth()) {
        return INF_COST;
    }
    return getCellCost(map, state, map.posToCoordinate(pos));
}

/**
 * @brief Tính chi phí tổng hợp của một ô theo tọa độ 2D (x, y).
 *
 * Mục đích:
 * - Đánh giá mức độ tốn chi phí khi đi qua ô đó.
 * - Kết hợp giữa địa hình và tình trạng giao thông hiện tại.
 *
 * Đầu vào:
 * @param map Bản đồ hex hiện tại.
 * @param state Trạng thái trò chơi, gồm danh sách traffic.
 * @param pos Tọa độ 2D của ô cần đánh giá.
 *
 * Đầu ra:
 * @return Chi phí của ô đó; nếu ô ngoài map hoặc bị chặn thì trả về INF_COST.
 *
 * Thuật toán / logic:
 * - Kiểm tra nếu tọa độ ngoài bản đồ thì trả về INF_COST.
 * - Nếu terrain là hồ nước / ô cấm thì trả về INF_COST.
 * - Cộng chi phí theo loại địa hình và chi phí phụ do traffic.
 *
 * Giới hạn:
 * - Chỉ tính chi phí theo ô hiện tại, chưa tính tiền lời / lợi ích toàn tuyến đường.
 * - Không tính chi phí nhiên liệu và không xét mục tiêu của xe.
 *
 * Ghi chú:
 * - Đây là hàm cơ sở để viết heuristic cho thuật toán A* / Dijkstra sau này.
 */
double Solver::getCellCost(const Map& map, const GameState& state, Position pos) {
    if (!map.isInside(pos)) {
        return INF_COST;
    }

    const int terrainType = map.getCell(pos);
    if (terrainType == 3) {
        return INF_COST;
    }

    double totalCost = terrainCostFromCell(terrainType);
    int cellPos = map.coordinateToPos(pos);

    for (const auto& traffic : state.traffics) {
        if (traffic.pos == cellPos) {
            totalCost += 5.0 * std::max(0, traffic.status);
        }
    }

    return totalCost;
}

/**
 * @brief Quyết định loại xe cho từng agent (0: Patrol, 1: Supply).
 *
 * Mục đích:
 * - Gán loại ban đầu cho từng xe.
 * - Dùng để phân biệt xe tuần tra và xe tiếp tế.
 *
 * Đầu vào:
 * @param config Cấu hình trò chơi, bao gồm số lượng agent và các thông tin ban đầu.
 *
 * Đầu ra:
 * @return Vector kiểu xe tương ứng với từng agent.
 *
 * Thuật toán / logic:
 * - Hiện tại đang trả về mặc định là tất cả xe là Patrol (0).
 * - Logic có thể được mở rộng dựa trên vị trí, hàng hóa, hoặc chiến thuật đội hình.
 *
 * Giới hạn:
 * - Đây không phải thuật toán di chuyển.
 * - Chưa có phân tích về fuel, traffic, hay mục tiêu chiến thuật cụ thể.
 *
 * Ghi chú:
 * - Nếu muốn dùng xe Supply, cần viết logic riêng ở đây.
 */
std::vector<int> AgentStrategy::decideAgentTypes(const GameConfig& config) {
    size_t numAgents = config.initialAgentPositions.size();
    
    // TODO [USER]: Viet logic chon xe o day nếu muon.
    // Vi du: set xe dau la Supply (1), cac xe sau la Patrol (0)
    std::vector<int> agentTypes(numAgents, 0);
    return agentTypes;
}

std::vector<int> Solver::decideAgentTypes(const GameConfig& config) {
    return AgentStrategy::decideAgentTypes(config);
}

/**
 * @brief Hàm chính sinh logic di chuyển cho các agent trong một ngày.
 *
 * Mục đích:
 * - Tạo ra vector actions cho từng agent.
 * - Mỗi action biểu thị một bước di chuyển hoặc chờ.
 *
 * Đầu vào:
 * @param config Cấu hình cố định của trận đấu: bản đồ, spots, daySteps, fuelLimit...
 * @param state Trạng thái thời gian thực: vị trí xe, fuel, traffic, ngày hiện tại...
 * @param map Đối tượng Map hỗ trợ chuyển đổi pos ↔ tọa độ, tính nextPosition, kiểm tra canMove...
 *
 * Đầu ra:
 * @return Vector các hành động theo từng agent: actions[i] là chuỗi hành động của xe i.
 *
 * Thuật toán / logic:
 * - Lấy số bước của ngày hiện tại từ config.daySteps[state.day].
 * - Với từng agent và từng step, chọn ngẫu nhiên một hướng hợp lệ hoặc đứng im.
 * - Mỗi agent luôn nhận đủ số action bằng daySteps để mô phỏng từng step.
 * - Đây là logic random để kiểm thử đường API và giao diện, chưa phải thuật toán tìm đường tối ưu.
 *
 * Giới hạn:
 * - Chưa xét mục tiêu cụ thể như spot, hàng hóa, fuel, hay vị trí tiếp tế.
 * - Chưa dùng A*, Dijkstra, BFS hoặc heuristic toàn tuyến.
 * - Chưa có logic xe hết nhiên liệu hoặc refill.
 *
 * Ghi chú:
 * - Hiện tại đang dùng để debug 1 xe trước, còn nhiều xe đang giữ im để dễ kiểm tra thuật toán.
 */
std::vector<std::vector<int>> Solver::solve(
    const GameConfig& config,
    const GameState& state,
    const Map& map
) {
    int daySteps = 0;
    if (state.day >= 0 && state.day < static_cast<int>(config.daySteps.size())) {
        daySteps = config.daySteps[state.day];
    }

    std::vector<std::vector<int>> actions(state.agents.size());

    // =====================================================================
    // TODO [USER]: VIET LOGIC DI CHUYEN CHO NGUOI DUNG TAI DAY
    // =====================================================================
    // Logic random hiện tại dùng để kiểm thử API và giao diện mô phỏng.
    // ---------------------------------------------------------------------

    static std::mt19937 generator(std::random_device{}());
    for (size_t i = 0; i < state.agents.size(); ++i) {
        Position currPos = map.posToCoordinate(state.agents[i].pos);
        int currentFuel = state.agents[i].fuel;
        for (int step = 0; step < daySteps; ++step) {
            std::vector<int> validDirections;
            for (int dir = 0; dir < 6; ++dir) {
                Position nxt = map.nextPosition(currPos, dir);
                if (map.canMove(nxt)) validDirections.push_back(dir);
            }

            if (validDirections.empty() || currentFuel <= 0 || std::uniform_int_distribution<int>(0, 1)(generator) == 0) {
                actions[i].push_back(-1);
                continue;
            }

            std::uniform_int_distribution<size_t> distribution(0, validDirections.size() - 1);
            const int direction = validDirections[distribution(generator)];
            currPos = map.nextPosition(currPos, direction);
            currentFuel--;
            actions[i].push_back(direction);
        }
    }

    // =====================================================================
    return actions;
}

/**
 * @brief Kế hoạch an toàn: đứng im cả ngày khi logic chính bị lỗi hoặc không có đường đi.
 *
 * Mục đích:
 * - Trả về một phương án an toàn, không làm xe di chuyển.
 * - Sử dụng như fallback khi validate thất bại hoặc không tìm được lộ trình hợp lệ.
 *
 * Đầu vào:
 * @param config Cấu hình trò chơi hiện tại.
 * @param state Trạng thái thời gian thực của game.
 *
 * Đầu ra:
 * @return Vector hành động trong đó mỗi xe đứng yên toàn bộ số bước của ngày.
 *
 * Thuật toán / logic:
 * - Nếu số bước ngày được định nghĩa, mỗi xe sẽ thực hiện action -daySteps.
 * - Đây là chiến lược an toàn nhất để tránh vi phạm quy tắc di chuyển.
 *
 * Giới hạn:
 * - Không tối ưu, không tìm đường, không tấn công hay thu thập.
 * - Chỉ nhằm đảm bảo an toàn và không làm sai luật.
 *
 * Ghi chú:
 * - Dùng khi không còn phương án nào hợp lệ trong debug hoặc khi cần fallback nhanh.
 */
std::vector<std::vector<int>> Solver::createFallbackActions(
    const GameConfig& config,
    const GameState& state
) {
    int daySteps = 0;
    if (state.day >= 0 && state.day < static_cast<int>(config.daySteps.size())) {
        daySteps = config.daySteps[state.day];
    }

    std::vector<std::vector<int>> fallback(state.agents.size());
    for (size_t i = 0; i < state.agents.size(); ++i) {
        if (daySteps > 0) {
            fallback[i].push_back(-daySteps);
        }
    }
    return fallback;
}
