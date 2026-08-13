# Hướng Dẫn Truy Xuất Dữ Liệu Config & Điều Khiển Agent

Tài liệu này giải thích chi tiết cách **lấy thông tin dữ liệu từ `GameConfig` / `GameState`** và cách **điều khiển các Agent** trong dự án **HexaUdon**.

---

## 1. Cấu Trúc Dữ Liệu Config & State

Toàn bộ dữ liệu JSON truyền từ Server thi đấu đã được tự động parse thành các C++ `struct` trong thư mục `include/model/` và `include/GameState.hpp`.

### 1.1 `GameConfig` (Thông tin cố định ban đầu của trận đấu)

Truy cập thông tin cấu hình qua biến `config` (kiểu `GameConfig`):

```cpp
// 1. Thời gian & Số bước (Steps) của từng ngày
long long startTime = config.startsAt;          // Thời gian bắt đầu (UNIX TIME)
int totalDays = config.daySteps.size();         // Tổng số ngày thi đấu
int stepsOfDay1 = config.daySteps[0];           // Số bước (step) của Ngày 0 (ví dụ: 50)
int secondsOfDay1 = config.daySeconds[0];       // Số giây thời gian làm bài Ngày 0 (ví dụ: 5s)

// 2. Thông tin Bản đồ (Map)
int mapHeight = config.map.height;              // Chiều cao bản đồ (ví dụ: 8)
int mapWidth = config.map.width;                // Chiều rộng bản đồ (ví dụ: 8)
int cellType = config.map.cells[y][x];          // Loại địa hình tại (x, y):
                                                // 0: 平地 (Đồng bằng/Flat)
                                                // 1: 道路 (Đường đi/Road)
                                                // 2: 山地 (Núi/Mountain)
                                                // 3: 池 (Hồ nước/Pond - KHÔNG THỂ ĐI VÀO)

// 3. Danh sách các gian hàng (Spot / Brand Stores)
for (const Spot& spot : config.spots) {
    int brand = spot.brand;                     // Thương hiệu (0 hoặc 1)
    int posIndex = spot.pos;                    // Vị trí trên bản đồ dạng Pos Index
    int maxStock = spot.stocks;                 // Số lượng tồn kho tối đa
}

// 4. Thông tin Agent ban đầu & Giới hạn
int agentCount = config.initialAgentPositions.size(); // Số lượng agent của đội
int startPosAgent0 = config.initialAgentPositions[0]; // Vị trí xuất phát xe 0
int maxFuel = config.fuelLimit;                 // Giới hạn bình nhiên liệu (ví dụ: 20)

// 5. Ngưỡng giao thông & Số đội chơi
int totalPlayers = config.players;              // Số đội tham gia
int busyThreshold = config.busyThreshold;       // Ngưỡng đông xe (Busy status = 1)
int jammedThreshold = config.jammedThreshold;   // Ngưỡng kẹt xe (Jammed status = 2)
```

---

### 1.2 `GameState` (Trạng thái thay đổi theo từng ngày)

Truy cập thông tin trạng thái thời gian thực qua biến `state` (kiểu `GameState`):

```cpp
// 1. Thông tin ngày hiện tại
int currentDay = state.day;                     // Ngày hiện tại (bắt đầu từ 0)
long long deadline = state.endsAt;              // Thời gian kết thúc nộp bài ngày này

// 2. Trạng thái các Agent của ĐỘI NÀY
for (size_t i = 0; i < state.agents.size(); ++i) {
    const Agent& agent = state.agents[i];
    int kind = agent.kind;                      // 0: 巡回車 (Xe tuần tra), 1: 補給車 (Xe tiếp tế)
    int posIndex = agent.pos;                   // Vị trí hiện tại của xe (Index = y * width + x)
    int currentFuel = agent.fuel;               // Nhiên liệu còn lại trong bình
}

// 3. Trạng thái các xe của ĐỐI THỦ (Other Players)
for (const OtherPlayer& other : state.others) {
    int enemyTeamId = other.id;                 // ID của đội đối thủ
    for (const Agent& enemyAgent : other.agents) {
        int enemyPos = enemyAgent.pos;          // Vị trí xe đối thủ
        int enemyFuel = enemyAgent.fuel;        // Nhiên liệu xe đối thủ
    }
}

// 4. Trạng thái Tắc nghẽn Giao thông (Traffic Status)
for (const Traffic& tr : state.traffics) {
    int roadPos = tr.pos;                       // Vị trí ô đường
    int status = tr.status;                     // 0: Normal, 1: Busy, 2: Jammed
}
```

---

## 2. Cách Điều Khiển Agent (Action Plan Format)

Để điều khiển các xe di chuyển hoặc chờ, thuật toán cần trả về danh sách lệnh dạng:
`std::vector<std::vector<int>>` chứa lệnh của từng xe.

### 2.1 Các Loại Hành Động (Action Codes)

| Mã Hành Động (Action Code) | Ý Nghĩa / Chi Tiết |
| :--- | :--- |
| **`0`** | Di chuyển sang hướng **Top-Left (Tây Bắc)** |
| **`1`** | Di chuyển sang hướng **Top-Right (Đông Bắc)** |
| **`2`** | Di chuyển sang hướng **Right (Đông)** |
| **`3`** | Di chuyển sang hướng **Bottom-Right (Đông Nam)** |
| **`4`** | Di chuyển sang hướng **Bottom-Left (Tây Nam)** |
| **`5`** | Di chuyển sang hướng **Left (Tây)** |
| **`-K`** (Số âm $\le -1$) | **Chờ (Wait)** $K$ bước (Ví dụ: `-15` nghĩa là chờ 15 steps) |

> ⚠️ **LƯU Ý QUAN TRỌNG:**
> Tổng số bước (Steps) trong mảng hành động của từng Agent phải **BẰNG CHÍNH XÁC** số bước yêu cầu của ngày đó (`config.daySteps[day]`).

---

### 2.2 Ví Dụ Code Điều Khiển Đơn Giản

Ví dụ: Ngày 0 yêu cầu **50 steps**. Bạn có 2 Xe.
- **Xe 0**: Muốn đi hướng `0` (Tây Bắc), sau đó hướng `1` (Đông Bắc), sau đó chờ 48 steps còn lại (Tổng: 1 + 1 + 48 = 50 steps).
- **Xe 1**: Không di chuyển, đứng yên chờ 50 steps (`-50`).

```cpp
#include "solver/Solver.hpp"

std::vector<std::vector<int>> MyCustomSolver(
    const GameConfig& config,
    const GameState& state
) {
    int requiredSteps = config.daySteps[state.day]; // Lấy số bước của ngày hiện tại (VD: 50)

    std::vector<std::vector<int>> actions(state.agents.size());

    // Lập kế hoạch cho Xe 0:
    actions[0].push_back(0);                       // Đi Tây Bắc (Step 1)
    actions[0].push_back(1);                       // Đi Đông Bắc (Step 2)
    actions[0].push_back(-(requiredSteps - 2));    // Chờ 48 steps còn lại (Step 3..50)

    // Lập kế hoạch cho Xe 1:
    actions[1].push_back(-requiredSteps);          // Chờ toàn bộ 50 steps

    return actions;
}
```

---

## 3. Chuyển Đổi Vị Trí Vẫn Dùng Lớp `Map`

Để hỗ trợ kiểm tra vị trí và di chuyển trên lưới lục giác:

```cpp
#include "map/Map.hpp"

Map map(config.map.height, config.map.width, config.map.cells);

// 1. Chuyển pos index (1D) sang Tọa độ Position (x, y)
Position coord = map.posToCoordinate(agent.pos);
int x = coord.x;
int y = coord.y;

// 2. Chuyển Tọa độ (x, y) ngược lại pos index (1D)
int posIndex = map.coordinateToPos({x, y});

// 3. Tính vị trí tiếp theo nếu đi theo hướng dir (0..5)
Position nextCoord = map.nextPosition(coord, dir);

// 4. Kiểm tra vị trí đó có hợp lệ để đi vào không (Không out of map và không phải 池/Hồ)
if (map.canMove(nextCoord)) {
    // Cho phép đi vào
}
```
