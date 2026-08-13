# Kế hoạch triển khai code — Hexa Udon

## 1. Mục tiêu tổng thể

Xây dựng chương trình theo pipeline:

```text
JSON Input
    │
    ▼
JsonReader
    │
    ▼
GameConfig + GameState
    │
    ├──────────────┐
    ▼              ▼
   Map           Traffic / Spot / Agent
    │              │
    └──────┬───────┘
           ▼
        Solver
           │
           ▼
      Action Plan
           │
           ▼
       Validator
           │
           ▼
       JsonWriter
           │
           ▼
      JSON Output
```

Nguyên tắc chính:

- Input/Output, dữ liệu, map và thuật toán phải được tách biệt.
- `main.cpp` chỉ điều phối chương trình.
- `JsonReader` chỉ đọc JSON.
- `JsonWriter` chỉ tạo JSON output.
- `GameConfig` chứa thông tin cố định của trận đấu.
- `GameState` chứa trạng thái thay đổi theo từng ngày.
- `Solver` chịu trách nhiệm về chiến thuật.
- `Validator` kiểm tra action trước khi gửi.

---

# 2. Cấu trúc project

Ban đầu:

```text
HexaUdon/
│
├── CMakeLists.txt
│
├── include/
│   ├── model/
│   │   ├── Agent.hpp
│   │   ├── Spot.hpp
│   │   ├── Traffic.hpp
│   │   ├── MapData.hpp
│   │   ├── GameConfig.hpp
│   │   └── GameState.hpp
│   │
│   ├── io/
│   │   ├── JsonReader.hpp
│   │   └── JsonWriter.hpp
│   │
│   └── map/
│       └── Map.hpp
│
├── src/
│   ├── main.cpp
│   │
│   ├── model/
│   │   └── ...
│   │
│   ├── io/
│   │   └── ...
│   │
│   └── map/
│       └── ...
│
├── tests/
│   ├── test_json.cpp
│   ├── test_map.cpp
│   └── test_pathfinding.cpp
│
└── third_party/
    └── json.hpp
```

Sau khi phần nền tảng hoàn thành:

```text
include/
└── solver/
    ├── Solver.hpp
    ├── PathFinder.hpp
    ├── AgentStrategy.hpp
    └── ActionGenerator.hpp

src/
└── solver/
    ├── Solver.cpp
    ├── PathFinder.cpp
    ├── AgentStrategy.cpp
    └── ActionGenerator.cpp
```

---

# 3. Giai đoạn 1 — Xây dựng Model

Mục tiêu: biểu diễn toàn bộ dữ liệu của đề bằng C++ `struct/class`.

## Agent

```text
Agent
├── kind
├── pos
└── fuel
```

## Spot

```text
Spot
├── brand
├── pos
└── stocks
```

## Traffic

```text
Traffic
├── pos
└── status
```

## MapData

```text
MapData
├── height
├── width
└── cells
```

## GameConfig

Chứa thông tin cố định:

```text
GameConfig
├── startsAt
├── daySeconds
├── daySteps
├── map
├── spots
├── initialAgentPositions
├── fuelLimit
├── players
├── busyThreshold
└── jammedThreshold
```

## GameState

Chứa thông tin thay đổi theo ngày:

```text
GameState
├── endsAt
├── day
├── agents
├── others
└── traffics
```

### Kết quả cần đạt

Có thể tạo:

```cpp
GameConfig config;
GameState state;
```

và tất cả dữ liệu của đề đều có nơi lưu trữ.

---

# 4. Giai đoạn 2 — JsonReader

Mục tiêu: đọc JSON và chuyển thành các struct ở trên.

Hai hàm chính:

```cpp
GameConfig readGameConfig();
GameState readGameState();
```

## Config

Đọc:

```text
startsAt
daySeconds
daySteps
map
spots
agents
fuelLimits
players
busyThreshold
jammedThreshold
```

## State

Đọc:

```text
endsAt
day
agents
others
traffics
```

Sau giai đoạn này, code bên ngoài không cần biết JSON có cấu trúc như thế nào.

Ví dụ:

```cpp
GameConfig config = JsonReader::readGameConfig();
```

thay vì:

```cpp
input["map"]["height"];
input["map"]["width"];
input["map"]["cells"];
...
```

---

# 5. Giai đoạn 3 — JsonWriter

Mục tiêu: chuẩn hóa việc xuất answer.

Có hai loại output.

## Agent type

```cpp
writeAgentTypes({
    0, 1, 0, 1
});
```

Output:

```json
[0,1,0,1]
```

## Action plan

```cpp
writeActions({
    {-15},
    {0,1,-10}
});
```

Output:

```json
[
    [-15],
    [0,1,-10]
]
```

`main.cpp` không tự tạo JSON.

---

# 6. Giai đoạn 4 — Xây dựng Map

Đây là module rất quan trọng.

`Map` chịu trách nhiệm:

## Kiểm tra tọa độ

```cpp
bool isInside(...);
```

## Lấy loại địa hình

```cpp
int getCell(...);
```

## Kiểm tra có thể đi hay không

```cpp
bool canMove(...);
```

## Chuyển đổi vị trí

```text
pos → coordinate
coordinate → pos
```

API dự kiến:

```cpp
Position posToCoordinate(int pos);

int coordinateToPos(Position position);
```

---

# 7. Giai đoạn 5 — Xác định chính xác hình học Hex Map

Đây là giai đoạn cần làm trước khi viết pathfinding.

Cần xác định chính xác 6 hướng theo specification của contest:

```text
0 = ?
1 = ?
2 = ?
3 = ?
4 = ?
5 = ?
```

Sau đó định nghĩa duy nhất một hàm:

```cpp
Position nextPosition(
    Position current,
    int direction
);
```

Không nên hard-code logic 6 hướng ở nhiều nơi.

Chỉ `Map` hoặc `HexGeometry` biết cách di chuyển.

---

# 8. Giai đoạn 6 — Kiểm tra movement

Viết test riêng để kiểm tra:

```text
agent ở A
+
direction
=
ô tiếp theo
```

Các trường hợp cần test:

- Đi trong map.
- Đi ra ngoài map.
- Đi vào hồ.
- Đi vào ô hợp lệ.
- Đi vào núi.
- Đi vào đường.
- Đi vào ô bằng.

Đặc biệt phải xác nhận chính xác rule của đề đối với từng loại terrain.

---

# 9. Giai đoạn 7 — Agent Type

Đầu trận cần quyết định:

```text
0 = 巡回車
1 = 補給車
```

Ban đầu có thể dùng strategy đơn giản:

```text
agent 0 → type 0
agent 1 → type 0
...
```

Sau khi framework chạy ổn mới tối ưu.

Tách thành:

```text
AgentStrategy
```

API dự kiến:

```cpp
std::vector<int> decideAgentTypes(
    const GameConfig& config
);
```

Như vậy thay đổi chiến thuật không ảnh hưởng JSON parser.

---

# 10. Giai đoạn 8 — Xây dựng PathFinder

Sau khi `Map` hoạt động chính xác mới bắt đầu pathfinding.

Module:

```text
PathFinder
```

Input:

```text
start
goal
map
traffic
fuel
```

Output:

```text
path
```

Ban đầu có thể implement:

```text
BFS
```

Sau đó cân nhắc:

```text
Dijkstra
A*
```

tùy theo cost thực tế của terrain, traffic và rule của game.

API dự kiến:

```cpp
std::vector<int> findPath(
    Position start,
    Position goal,
    const Map& map
);
```

---

# 11. Giai đoạn 9 — Traffic

Xây dựng module xử lý traffic.

Quy đổi:

```text
status = 0 → bình thường
status = 1 → busy
status = 2 → jammed
```

Có thể cung cấp:

```cpp
int getTrafficStatus(int pos);
```

hoặc xây dựng riêng:

```text
TrafficMap
```

để `PathFinder` sử dụng.

---

# 12. Giai đoạn 10 — Fuel

Tạo module quản lý fuel.

Cần xác định chính xác:

```text
Agent có bao nhiêu fuel?
Fuel limit?
Một bước di chuyển tốn bao nhiêu?
Khi nào cần supply?
Agent nào có thể supply?
```

Không nên để logic fuel rải rác trong solver.

API ví dụ:

```cpp
bool canReach(
    const Agent& agent,
    Position destination
);
```

---

# 13. Giai đoạn 11 — Spot

Xử lý:

```text
brand
stocks
pos
```

Có thể tạo abstraction:

```text
SpotManager
```

API dự kiến:

```cpp
getSpot(position);
getStock(position);
getBrand(position);
```

Sau này strategy có thể quyết định:

- Điểm nào quan trọng.
- Điểm nào cần patrol.
- Điểm nào cần supply.
- Điểm nào cần ưu tiên.

---

# 14. Giai đoạn 12 — Solver

Đây là bộ não của chương trình.

Solver nhận:

```text
GameConfig
+
GameState
```

và trả về:

```text
ActionPlan
```

Kiến trúc:

```text
Solver
   │
   ├── AgentStrategy
   │
   ├── PathFinder
   │
   ├── Traffic
   │
   ├── Fuel
   │
   └── Spot
        │
        ▼
      Actions
```

API:

```cpp
ActionPlan solve(
    const GameConfig& config,
    const GameState& state
);
```

---

# 15. Giai đoạn 13 — Action Generator

Solver không nên trực tiếp tạo JSON.

Nó chỉ tạo:

```cpp
std::vector<std::vector<int>>
```

Ví dụ:

```text
Agent 0:
[-10, 0, 1, -5]

Agent 1:
[0, 0, 2, -7]
```

Sau đó `ActionGenerator` kiểm tra:

```text
tổng step == daySteps[day]
```

và:

```text
movement hợp lệ
```

---

# 16. Giai đoạn 14 — Validator

Đây là module rất nên có.

```text
ActionValidator
```

Nó kiểm tra trước khi output.

## Số agent

```text
actions.size() == agent_count
```

## Số bước

```text
total_steps == daySteps[day]
```

## Movement

Không được:

```text
ra ngoài map
vào hồ
```

## Direction

Direction hợp lệ:

```text
0..5
```

Wait phải là:

```text
-1, -2, -3, ...
```

## Fuel

Không được vi phạm giới hạn fuel theo rule của game.

Pipeline:

```text
Solver
  ↓
ActionPlan
  ↓
Validator
  ↓
JsonWriter
```

---

# 17. Giai đoạn 15 — Main cuối cùng

Mục tiêu là `main.cpp` cực ngắn:

```cpp
int main() {

    GameConfig config =
        JsonReader::readGameConfig();

    auto agentTypes =
        solver.decideAgentTypes(config);

    JsonWriter::writeAgentTypes(agentTypes);

    while (true) {

        GameState state =
            JsonReader::readGameState();

        auto actions =
            solver.solve(config, state);

        if (!validator.validate(
                config,
                state,
                actions)) {

            actions =
                solver.createFallbackActions(
                    config,
                    state
                );
        }

        JsonWriter::writeActions(actions);
    }

    return 0;
}
```

Đây là mục tiêu kiến trúc, không phải code cuối cùng ngay từ đầu.

---

# 18. Thứ tự triển khai thực tế

Không nên code tất cả cùng lúc.

## Phase 1 — Project

```text
CMake
↓
nlohmann/json
↓
Compile project
```

## Phase 2 — Model

```text
Agent
Spot
Traffic
MapData
GameConfig
GameState
```

## Phase 3 — Input

```text
JsonReader
```

## Phase 4 — Output

```text
JsonWriter
```

## Phase 5 — Map

```text
Map
```

## Phase 6 — Hex movement

```text
pos ↔ coordinate
coordinate + direction
```

## Phase 7 — Validator

```text
ActionValidator
```

## Phase 8 — Agent type

```text
AgentStrategy
```

## Phase 9 — Pathfinding

```text
PathFinder
```

## Phase 10 — Game mechanics

```text
Traffic
Fuel
Spot
```

## Phase 11 — Solver

```text
Solver
```

## Phase 12 — Optimization

```text
Performance
Strategy
Scoring
```

---

# 19. Tiêu chí hoàn thành từng phase

Mỗi phase phải có một mục tiêu test rõ ràng.

### Phase 3 — JSON

Input JSON:

```text
↓
JsonReader
↓
GameConfig
```

Có thể in:

```text
height = 8
width = 8
agents = 4
spots = 4
days = 4
```

### Phase 4 — Output

```text
GameConfig
↓
JsonWriter
↓
[0,1,0,1]
```

### Phase 5 — Map

```text
pos
↓
(x,y)
↓
terrain
↓
canMove()
```

### Phase 6 — Movement

```text
(x,y) + direction
↓
next position
```

### Phase 9 — PathFinder

```text
start
↓
PathFinder
↓
goal
```

### Phase 11 — Solver

```text
GameState
+
GameConfig
↓
Solver
↓
ActionPlan
```

---

# 20. Nguyên tắc kiến trúc

Giữ 4 tầng độc lập:

```text
┌─────────────────────────────┐
│         Algorithm           │
│       Solver / Strategy     │
└──────────────┬──────────────┘
               │
┌──────────────▼──────────────┐
│           Domain            │
│ Map / Agent / Spot / Fuel   │
└──────────────┬──────────────┘
               │
┌──────────────▼──────────────┐
│           Model             │
│ GameConfig / GameState      │
└──────────────┬──────────────┘
               │
┌──────────────▼──────────────┐
│          IO Layer            │
│ JSON Reader / Writer        │
└─────────────────────────────┘
```

### Quy tắc

**Solver không được biết JSON.**

**JsonReader không được biết thuật toán.**

**Map không được biết chiến thuật.**

**JsonWriter không được biết agent nên đi đâu.**

Nếu giữ được 4 nguyên tắc này, sau này có thể thay đổi BFS → A*, thay đổi strategy của agent hoặc thay đổi cách xử lý traffic mà không phải sửa toàn bộ project.

---

# 21. Mục tiêu cuối cùng

Kiến trúc hoàn chỉnh:

```text
                    ┌─────────────┐
                    │ JSON Input  │
                    └──────┬──────┘
                           ▼
                    ┌─────────────┐
                    │ JsonReader  │
                    └──────┬──────┘
                           ▼
              ┌─────────────────────────┐
              │ GameConfig / GameState  │
              └────────────┬────────────┘
                           ▼
              ┌─────────────────────────┐
              │        Game Domain      │
              │                         │
              │ Map                     │
              │ Agent                   │
              │ Spot                    │
              │ Traffic                 │
              │ Fuel                    │
              └────────────┬────────────┘
                           ▼
                    ┌─────────────┐
                    │   Solver    │
                    └──────┬──────┘
                           ▼
                    ┌─────────────┐
                    │  Validator  │
                    └──────┬──────┘
                           ▼
                    ┌─────────────┐
                    │ JsonWriter  │
                    └──────┬──────┘
                           ▼
                    ┌─────────────┐
                    │ JSON Output │
                    └─────────────┘
```

Đây là kiến trúc nên hướng tới trước khi bắt đầu viết thuật toán thi đấu.
