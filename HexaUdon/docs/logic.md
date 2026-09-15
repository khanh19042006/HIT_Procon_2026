# Tài liệu Logic — Folder `solver`

> Tài liệu này mô tả chi tiết chức năng của **từng Class, từng hàm** trong folder `solver`.
> Tất cả các Class ở đây đều nằm trong 2 thư mục:
> - **Header (.hpp):** `include/solver/`
> - **Source (.cpp):** `src/solver/`

---

## Mục lục

1. [Solver — Nhạc trưởng điều phối](#1-solver--nhạc-trưởng-điều-phối)
2. [PatrolPlanner — Lập kế hoạch xe Tuần tra](#2-patrolplanner--lập-kế-hoạch-xe-tuần-tra)
3. [SupplyPlanner — Lập kế hoạch xe Tiếp tế](#3-supplyplanner--lập-kế-hoạch-xe-tiếp-tế)
4. [SpotScorer — Chấm điểm & chọn Spot](#4-spotscorer--chấm-điểm--chọn-spot)
5. [MoveSimulator — Mô phỏng di chuyển](#5-movesimulator--mô-phỏng-di-chuyển)
6. [PathFinder — Tìm đường Dijkstra](#6-pathfinder--tìm-đường-dijkstra)
7. [ActionValidator — Kiểm tra tính hợp lệ](#7-actionvalidator--kiểm-tra-tính-hợp-lệ)
8. [DiaryWriter — Ghi nhật ký hành trình](#8-diarywriter--ghi-nhật-ký-hành-trình)
9. [Hàm test/debug](#9-hàm-testdebug)
10. [Sơ đồ quan hệ giữa các Class](#10-sơ-đồ-quan-hệ-giữa-các-class)

---

## 1. Solver — Nhạc trưởng điều phối

**File:** `Solver.hpp` / `Solver.cpp` (~150 dòng)

**Vai trò:** Không tự tính toán gì phức tạp. Nó chỉ quản lý **trạng thái xuyên trận đấu** và gọi các module khác (PatrolPlanner, SupplyPlanner) để làm việc nặng.

### Biến nội bộ (Trạng thái lưu trữ)

| Biến | Kiểu | Mô tả | Vòng đời |
|------|------|-------|----------|
| `collectedBrandsTotal_` | `set<int>` | Tập hợp các mã brand (loại Udon) đã thu thập được. VD: `{0, 1, 3}` nghĩa là đã lấy được 3 loại. | Xuyên trận đấu (không bao giờ reset) |
| `currentDay_` | `int` | Số ngày hiện tại đã xử lý. Dùng để phát hiện khi nào chuyển sang ngày mới. | Xuyên trận đấu |
| `remainingStock_` | `vector<int>` | Số hàng còn lại tại mỗi Spot hôm nay. VD: `[3, 2, 0, 5]` — Spot 2 đã hết hàng. | Reset mỗi ngày |
| `visitedSpotsToday_` | `vector<set<int>>` | Với mỗi xe, ghi nhớ các Spot đã ghé trong ngày. VD: xe 0 đã ghé Spot `{1, 4}`. | Reset mỗi ngày |
| `currentTargets_` | `vector<int>` | Spot mục tiêu hiện tại của mỗi xe. Xe Supply đọc mảng này để biết Patrol đang đi đâu. | Reset mỗi ngày |

### Hàm

#### `AgentStrategy::decideAgentTypes(config) → vector<int>`

**Mục đích:** Quyết định đội hình xe (mấy Patrol, mấy Supply).

**Logic chi tiết:**
1. Đếm tổng số xe `n` từ `config.initialAgentPositions.size()`.
2. Tính diện tích bản đồ `mapArea = height × width`.
3. Các luật:
   - Nếu `fuelLimit >= mapArea * 2` → xăng thừa mứa → **tất cả Patrol**, không cần Supply.
   - Nếu `n <= 2` → đội quá nhỏ → **tất cả Patrol**.
   - Nếu `n >= 6` VÀ `fuelLimit <= 10` VÀ `mapArea > 400` → bản đồ lớn + xăng ít → dùng **2 Supply**.
   - Mặc định → `(n-1) Patrol + 1 Supply`.
   - An toàn: luôn giữ ít nhất 1 Patrol.
4. Xe Supply được xếp ở cuối mảng. VD: 4 xe → `[0, 0, 0, 1]` (3 Patrol + 1 Supply).

**Đầu ra:** Mảng int. `0` = Patrol, `1` = Supply.

---

#### `Solver::resetDailyState(config, numAgents) → void`

**Mục đích:** Xóa sạch dữ liệu ngày cũ, chuẩn bị cho ngày mới.

**Logic chi tiết:**
1. `remainingStock_[i]` = `config.spots[i].stocks` → nạp lại hàng cho tất cả Spot (vì hàng tái tạo mỗi ngày).
2. `visitedSpotsToday_` = tất cả rỗng `{}` → chưa xe nào ghé đâu.
3. `currentTargets_` = tất cả `-1` → chưa xe nào có mục tiêu.

---

#### `Solver::solve(config, state, map) → vector<vector<int>>`

**Mục đích:** Hàm chính — tạo kế hoạch hành động cho TẤT CẢ xe trong 1 ngày.

**Logic chi tiết (theo thứ tự):**
1. **Đọc số bước trong ngày:** `daySteps = config.daySteps[state.day]`. VD: Ngày 0 = 20 bước, Ngày 3 = 80 bước.
2. **Cập nhật giao thông:** Gọi `map.updateTraffic(state.traffics)` → cập nhật ô nào đang kẹt xe.
3. **Reset nếu ngày mới:** So sánh `state.day` với `currentDay_`. Nếu khác → gọi `resetDailyState()`.
4. **Vòng 1 — Xe Patrol:** Duyệt tất cả xe có `kind == 0`:
   - Gọi `PatrolPlanner::planDay(...)` → trả về mảng hành động cho xe đó.
   - Truyền vào các biến trạng thái `remainingStock_`, `visitedSpotsToday_`, `collectedBrandsTotal_`, `currentTargets_` bằng **tham chiếu (reference)** để PatrolPlanner cập nhật trực tiếp.
5. **Vòng 2 — Xe Supply:** Duyệt tất cả xe có `kind == 1`:
   - Gọi `SupplyPlanner::planDay(...)` → truyền vào `currentTargets_` để Supply biết Patrol đang nhắm đâu.
   - **Lưu ý:** Vòng 2 PHẢI chạy SAU vòng 1, vì Supply cần biết kết quả mục tiêu của Patrol.

**Đầu ra:** Mảng 2 chiều. VD: `[[2,2,3,-14], [5,5,4,-12], [1,1,-18], [3,3,3,-14]]` — 4 xe, mỗi xe 1 mảng hành động.

---

#### `Solver::createFallbackActions(config, state) → vector<vector<int>>`

**Mục đích:** Kế hoạch "khẩn cấp" — tất cả xe đứng yên cả ngày. Dùng khi solver gặp lỗi hoặc actions bị validator từ chối.

**Logic:** Mỗi xe nhận `[-daySteps]`. VD: `[[-20], [-20], [-20], [-20]]`.

---

## 2. PatrolPlanner — Lập kế hoạch xe Tuần tra

**File:** `PatrolPlanner.hpp` / `PatrolPlanner.cpp` (~73 dòng)

**Vai trò:** Lập kế hoạch chuỗi Spot (multi-spot chaining) cho 1 xe Patrol trong 1 ngày. Đây là **class chiến thuật quan trọng nhất** vì nó trực tiếp quyết định xe đi lấy bao nhiêu Udon.

### Hàm

#### `PatrolPlanner::planDay(...) → vector<int>`

**Mục đích:** Cho 1 xe Patrol, tìm lộ trình ghé nhiều Spot nhất có thể trong 1 ngày.

**Đầu vào:**
- `config`: Cấu hình trận (danh sách Spot, brand, stock).
- `map`: Bản đồ (địa hình, giao thông).
- `startPos`: Toạ độ xe đầu ngày.
- `daySteps`: Tổng bước trong ngày (VD: 20, 40, 60, 80).
- `availableFuel`: Xăng đầu ngày.
- `remainingStock` [IN/OUT]: Stock mỗi Spot — sẽ bị giảm khi xe lấy hàng.
- `visitedToday` [IN/OUT]: Spot đã ghé hôm nay — sẽ được thêm vào khi xe ghé.
- `collectedBrands` [IN/OUT]: Brand đã thu thập — sẽ được thêm vào khi xe lấy loại mới.
- `lastTargetSpot` [OUT]: Spot cuối cùng xe nhắm tới — để xe Supply biết đón đầu.

**Logic chi tiết (Vòng lặp Greedy):**
```
Biến nội bộ: stepsUsed = 0, fuelRemaining = availableFuel, currentPos = startPos

WHILE (true):
    stepsRemaining = daySteps - stepsUsed
    IF stepsRemaining <= 0 HOẶC fuelRemaining <= 0 → THOÁT

    BƯỚC 1: Gọi SpotScorer::findBestSpot()
            → Trả về index Spot tốt nhất (hoặc -1 nếu hết)
            → Nếu -1 → THOÁT

    BƯỚC 2: Gọi PathFinder::findPath(currentPos → spotPos)
            → Trả về danh sách hướng đi [2, 3, 4, ...]
            → Nếu không tìm được đường hoặc quá xa → THOÁT

    BƯỚC 3: Gọi MoveSimulator::simulate(directions)
            → Trả về: actions thực tế, vị trí cuối, steps/fuel đã dùng

    BƯỚC 4: Cập nhật trạng thái:
            - allActions += sim.actions
            - stepsUsed += sim.stepsUsed
            - fuelRemaining -= sim.fuelUsed
            - currentPos = sim.finalPos

    BƯỚC 5: Đánh dấu:
            - visitedToday.insert(nextSpot)     → Spot này đã ghé
            - remainingStock[nextSpot]--         → Hàng giảm 1
            - collectedBrands.insert(brand)      → Ghi nhớ brand
            - lastTargetSpot = nextSpot          → Cho Supply biết

    QUAY LẠI BƯỚC 1 (tìm Spot tiếp theo từ vị trí mới)

SAU KHI THOÁT:
    Gọi MoveSimulator::padWithWait() → thêm Wait cho đủ daySteps
```

**Đầu ra:** Mảng hành động hoàn chỉnh. VD: `[2, 2, 3, 4, 1, 1, -10]` (đi 6 bước + chờ 10 bước = 20 bước).

---

## 3. SupplyPlanner — Lập kế hoạch xe Tiếp tế

**File:** `SupplyPlanner.hpp` / `SupplyPlanner.cpp` (~86 dòng)

**Vai trò:** Tìm xe Patrol cần cứu nhất, xác định điểm hẹn, và lập đường đi tới đó.

### Hàm

#### `SupplyPlanner::findTargetPatrol(agents, excludeIdx) → int`

**Mục đích:** Tìm xe Patrol đang có lượng xăng thấp nhất.

**Logic chi tiết:**
1. Duyệt tất cả xe trong mảng `agents`.
2. Bỏ qua xe có index = `excludeIdx` (chính nó — xe Supply).
3. Bỏ qua xe có `kind != 0` (không phải Patrol).
4. So sánh `fuel` → ghi nhớ xe có fuel thấp nhất.

**Đầu ra:** Index xe Patrol ít xăng nhất. Trả `-1` nếu không có xe Patrol.

---

#### `SupplyPlanner::planDay(...) → vector<int>`

**Mục đích:** Lập kế hoạch 1 ngày cho 1 xe Supply.

**Đầu vào:**
- `config`, `map`: Cấu hình trận + bản đồ.
- `supplyAgent`: Thông tin xe Supply (vị trí hiện tại).
- `allAgents`: Tất cả xe (để tìm Patrol cần cứu).
- `supplyIdx`: Index của xe Supply đang tính.
- `daySteps`: Tổng bước trong ngày.
- `patrolTargetSpots`: Mảng Spot mục tiêu của từng Patrol (do Solver cung cấp).

**Logic chi tiết:**
```
BƯỚC 1: Gọi findTargetPatrol()
        → Tìm xe Patrol ít xăng nhất
        → Nếu không tìm thấy → trả [-daySteps] (đứng yên)

BƯỚC 2: Xác định điểm hẹn (Rendezvous):
        IF patrolTargetSpots[targetPatrol] >= 0:
            → Đón đầu: điểm hẹn = Spot mà Patrol đang nhắm tới
        ELSE:
            → Fallback: điểm hẹn = vị trí hiện tại của Patrol

BƯỚC 3: Gọi PathFinder::findPath(agentPos → targetPos)
        → Nếu không tìm được đường → trả [-daySteps] (đứng yên)

BƯỚC 4: Gọi MoveSimulator::simulate(directions)
        → isPatrol = false (xe Supply không tốn xăng)
        → maxFuel = INT_MAX (vô hạn xăng)

BƯỚC 5: Gọi MoveSimulator::padWithWait() → thêm Wait cho đủ daySteps
```

**Đầu ra:** Mảng hành động. VD: `[3, 3, 3, 3, -14]` (đi 4 bước + chờ 14 bước).

---

## 4. SpotScorer — Chấm điểm & chọn Spot

**File:** `SpotScorer.hpp` / `SpotScorer.cpp` (~78 dòng)

**Vai trò:** Đánh giá mức độ hấp dẫn của các Spot (cửa hàng Udon) và chọn ra Spot ngon nhất.

### Hàm

#### `SpotScorer::scoreSpot(brand, distanceSteps, collectedBrands, remainingStock) → int`

**Mục đích:** Chấm điểm cho **1 Spot cụ thể**.

**Logic chấm điểm:**
```
NẾU remainingStock <= 0 → trả -1 (không hợp lệ, hết hàng)

score = 0

NẾU brand CHƯA CÓ trong collectedBrands:
    score += 1000    ← Bonus cực lớn cho loại Udon mới
                       (Vì Tiêu chí 1 quan trọng nhất)

score -= distanceSteps  ← Trừ điểm theo khoảng cách
                          (Xa hơn = điểm thấp hơn)

Trả về score
```

**Ví dụ:**
| Spot | Brand | Đã có? | Khoảng cách | Tính điểm | Kết quả |
|------|-------|--------|-------------|-----------|---------|
| A | 2 | Chưa | 5 bước | 0 + 1000 - 5 | **995** |
| B | 0 | Rồi | 3 bước | 0 + 0 - 3 | **-3** |
| C | 3 | Chưa | 12 bước | 0 + 1000 - 12 | **988** |

→ Spot A thắng (995 > 988 > -3).

---

#### `SpotScorer::findBestSpot(...) → int`

**Mục đích:** Duyệt TẤT CẢ Spot trên bản đồ, lọc bỏ các Spot không hợp lệ, chấm điểm các Spot còn lại, và trả về Spot tốt nhất.

**Logic chi tiết:**
```
bestSpot = -1, bestScore = -1, bestCost = INT_MAX

FOR mỗi Spot si trong config.spots:
    LỌC 1: NẾU si đã có trong visitedToday → bỏ qua (đã ghé hôm nay)
    LỌC 2: NẾU remainingStock[si] <= 0    → bỏ qua (hết hàng)
    LỌC 3: Gọi PathFinder::findPath(currentPos → spotPos)
            NẾU không tìm được đường      → bỏ qua (bị chặn hoặc hết xăng)
    LỌC 4: NẾU path.totalSteps > stepsRemaining → bỏ qua (không đủ thời gian)

    Gọi scoreSpot() → lấy điểm

    SO SÁNH:
        NẾU score > bestScore → cập nhật bestSpot = si
        NẾU score == bestScore VÀ totalSteps < bestCost → chọn Spot gần hơn

Trả về bestSpot (hoặc -1 nếu không có Spot nào đạt)
```

---

## 5. MoveSimulator — Mô phỏng di chuyển

**File:** `MoveSimulator.hpp` / `MoveSimulator.cpp` (~59 dòng)

**Vai trò:** Biến danh sách hướng đi (từ PathFinder) thành chuỗi hành động hợp lệ, đồng thời tính toán vị trí và xăng sau khi di chuyển.

### Struct `SimResult`

```cpp
struct SimResult {
    vector<int> actions;   // Chuỗi hành động đã thực hiện
    Position finalPos;     // Toạ độ cuối cùng sau di chuyển
    int stepsUsed = 0;     // Tổng bước đã tiêu tốn
    int fuelUsed = 0;      // Tổng xăng đã tiêu tốn
};
```

### Hàm

#### `MoveSimulator::simulate(pathDirs, map, startPos, maxSteps, availableFuel, isPatrol) → SimResult`

**Mục đích:** "Lái thử" xe theo hướng dẫn, dừng lại ngay khi hết xăng hoặc hết bước.

**Logic chi tiết:**
```
currentPos = startPos
fuelRemaining = availableFuel

FOR mỗi hướng dir trong pathDirs:
    travelTime = map.getTravelTime(currentPos)
        → Đồng bằng: 2 bước
        → Đường thoáng: 1 bước
        → Đường ùn: 2 bước
        → Đường kẹt: 4 bước
        → Núi: 3 bước

    fuelCost = (isPatrol ? map.getFuelCost(currentPos) : 0)
        → Xe Supply: luôn = 0 (không tốn xăng)
        → Xe Patrol Đồng bằng: 1 xăng
        → Xe Patrol Đường/Núi: 2 xăng

    NẾU stepsUsed + travelTime > maxSteps → DỪNG (hết bước)
    NẾU isPatrol VÀ fuelRemaining < fuelCost → DỪNG (hết xăng)

    actions.push_back(dir)
    stepsUsed += travelTime
    fuelUsed += fuelCost
    fuelRemaining -= fuelCost
    currentPos = map.nextPosition(currentPos, dir)

Trả về { actions, finalPos=currentPos, stepsUsed, fuelUsed }
```

**Ví dụ:**
- Input: `pathDirs = [2, 2, 3, 4]`, xe Patrol, xăng = 3, maxSteps = 20
- Ô hiện tại là Đồng bằng (travelTime=2, fuelCost=1)
- Bước 1: dir=2, steps=2, fuel=2 ✓
- Bước 2: dir=2, steps=4, fuel=1 ✓
- Bước 3: dir=3, steps=6, fuel=0 ✓
- Bước 4: dir=4, fuelRemaining=0 < fuelCost=1 → **DỪNG**
- Output: `actions=[2,2,3]`, stepsUsed=6, fuelUsed=3

---

#### `MoveSimulator::padWithWait(actions, stepsUsed, daySteps) → void`

**Mục đích:** Thêm hành động "Chờ" vào cuối để tổng bước = daySteps.

**Logic:**
```
remaining = daySteps - stepsUsed
NẾU remaining > 0:
    actions.push_back(-remaining)    ← Số âm = chờ bấy nhiêu bước
```

**Ví dụ:** `stepsUsed=6`, `daySteps=20` → thêm `-14` → tổng = 6 + 14 = 20.

---

## 6. PathFinder — Tìm đường Dijkstra

**File:** `PathFinder.hpp` / `PathFinder.cpp` (~138 dòng)

**Vai trò:** Tìm đường đi ngắn nhất (ít bước nhất) giữa 2 điểm trên bản đồ lục giác, có tính đến địa hình và xăng.

### Struct `PathResult`

```cpp
struct PathResult {
    bool found = false;          // Có tìm được đường không?
    int totalSteps = 0;          // Tổng số bước để đến đích
    int totalFuel = 0;           // Tổng xăng cần thiết
    vector<int> directions;      // Chuỗi hướng đi: [2, 3, 4, 0, ...]
};
```

### Struct `SSSPResult` (Single-Source Shortest Path)

```cpp
struct SSSPResult {
    vector<int> dist;        // dist[v] = khoảng cách ngắn nhất từ nguồn → v
    vector<int> fuel;        // fuel[v] = xăng cần thiết từ nguồn → v
    vector<int> prevDir;     // prevDir[v] = hướng đi cuối cùng để tới v
    vector<int> prevCell;    // prevCell[v] = ô trước v trên đường đi ngắn nhất
    int sourcePos;           // Ô nguồn (index 1 chiều)

    PathResult extractPath(int goalPos);  // Trích xuất đường đi tới 1 ô cụ thể
};
```

### Hàm

#### `runDijkstra(source, map, maxFuel, earlyStopPos) → SSSPResult` (hàm nội bộ)

**Mục đích:** Chạy thuật toán Dijkstra từ 1 điểm xuất phát.

**Logic chi tiết:**
```
Khởi tạo:
    dist[] = ∞ cho tất cả ô, dist[source] = 0
    fuel[] = ∞ cho tất cả ô, fuel[source] = 0
    Priority Queue pq (min-heap theo dist)
    pq.push({0, source})

WHILE pq không rỗng:
    Lấy ô u có dist nhỏ nhất ra
    NẾU dist đã lỗi thời (d > dist[u]) → bỏ qua
    NẾU earlyStopPos >= 0 VÀ u == earlyStopPos → DỪNG SỚM (đã tới đích)

    Với mỗi hướng dir từ 0 → 5:
        Tính ô kế tiếp v = map.nextPosition(u, dir)
        NẾU v là hồ hoặc ngoài bản đồ → bỏ qua

        newDist = dist[u] + travelTime(u)
        newFuel = fuel[u] + fuelCost(u)

        NẾU newFuel > maxFuel → bỏ qua (hết xăng)
        NẾU newDist < dist[v]:
            Cập nhật dist[v], fuel[v], prevDir[v], prevCell[v]
            pq.push({newDist, v})

Trả về SSSPResult
```

---

#### `SSSPResult::extractPath(goalPos) → PathResult`

**Mục đích:** Từ kết quả Dijkstra, trích xuất đường đi cụ thể tới 1 ô đích.

**Logic:**
1. Nếu `dist[goalPos] == ∞` → trả `found = false`.
2. Đi ngược từ `goalPos` theo `prevCell[]` cho đến `sourcePos`. Ghi lại `prevDir[]` tại mỗi bước.
3. Đảo ngược mảng hướng đi (vì đi ngược).

---

#### `PathFinder::findPath(start, goal, map, maxFuel) → PathResult`

**Mục đích:** Tìm đường ngắn nhất từ A đến B (có **dừng sớm** khi tới B để tiết kiệm thời gian).

**Logic:** Gọi `runDijkstra(start, map, maxFuel, goalIdx)` → `extractPath(goalIdx)`.

---

#### `PathFinder::computeSSSP(source, map, maxFuel) → SSSPResult`

**Mục đích:** Tính đường ngắn nhất từ 1 điểm đến **TẤT CẢ** ô trên bản đồ (không dừng sớm).

**Logic:** Gọi `runDijkstra(source, map, maxFuel, -1)`. Sau đó có thể gọi `extractPath()` cho bất kỳ ô nào.

---

## 7. ActionValidator — Kiểm tra tính hợp lệ

**File:** `ActionValidator.hpp` / `ActionValidator.cpp` (~63 dòng)

**Vai trò:** Kiểm tra chuỗi hành động có hợp lệ không **TRƯỚC** khi gửi lên server. Hoạt động như "trọng tài nội bộ".

### Hàm

#### `ActionValidator::validate(config, state, actions, map) → bool`

**Mục đích:** Mô phỏng lại toàn bộ chuỗi hành động để phát hiện lỗi.

**Các lỗi được kiểm tra:**

| Lỗi | Cách kiểm tra |
|-----|---------------|
| Số xe không khớp | `actions.size() != agents.size()` |
| Tổng bước sai | `totalSteps != daySteps` |
| Hết xăng | `fuelRemaining < 0` (chỉ xe Patrol) |
| Đi vào hồ nước | `!map.canMove(newPos)` |
| Đi ra ngoài bản đồ | `!map.canMove(newPos)` |
| Mã hành động sai | Không phải 0-5 (di chuyển) hoặc số âm (chờ) |

**Logic:**
```
FOR mỗi xe i:
    totalSteps = 0, fuelRemaining = agents[i].fuel
    currentPos = agents[i].pos

    FOR mỗi hành động act:
        NẾU act < 0:
            totalSteps += (-act)      ← Chờ
        NẾU 0 <= act <= 5:
            totalSteps += travelTime  ← Di chuyển
            fuelRemaining -= fuelCost
            NẾU fuelRemaining < 0 → return FALSE
            currentPos = nextPosition(currentPos, act)
            NẾU !canMove(currentPos) → return FALSE
        KHÁC:
            return FALSE               ← Mã lệnh sai

    NẾU totalSteps != daySteps → return FALSE

return TRUE
```

---

## 8. DiaryWriter — Ghi nhật ký hành trình

**File:** `include/io/DiaryWriter.hpp` / `src/io/DiaryWriter.cpp`

**Vai trò:** Ghi lại kế hoạch đã được server chấp nhận thành Markdown. Module
này thuộc lớp `io`, chỉ định dạng dữ liệu và ghi file; không thay đổi trạng thái
Solver hoặc action của xe.

### `DiaryWriter::writeDay(...) → bool`

Tạo thư mục `diary/<matchId>/` nếu chưa có và ghi file
`day_<day>.md`. Mỗi file gồm:

- số step và loại kế hoạch (`Solver` hoặc `Fallback`);
- từng xe, loại xe, vị trí và fuel đầu ngày;
- Spot mục tiêu mà Solver đã chọn;
- tọa độ đích chính xác do planner trả ra khi gọi `PathFinder`;
- bảng từng action, khoảng step bị tiêu thụ, vị trí sau action và fuel còn lại;
- mảng action hoàn chỉnh cuối ngày.

Trong bảng hành trình, vị trí sau action, khoảng step và fuel còn lại là **mô
phỏng cục bộ** từ action đã gửi và bản đồ/traffic lúc lập kế hoạch. API hiện
chỉ cung cấp trạng thái server ở đầu ngày, không cung cấp trạng thái thực tế
sau từng action; vì vậy diary không gọi các giá trị mô phỏng này là trạng thái
đã được server xác nhận.

Hàm được gọi **sau khi** `submitActions()` thành công, vì vậy diary chỉ ghi
kế hoạch thực sự đã gửi được server chấp nhận. Action của xe Supply lấy từ
đúng điểm hẹn mà `SupplyPlanner` đã chọn, không tự tính lại trong module ghi
file.

### Dữ liệu đích chính xác từ Solver

`PatrolPlanner::planDay(...)` ghi vị trí Spot cuối cùng đã được chọn vào
`plannedTargetPos`. `SupplyPlanner::planDay(...)` nhận trực tiếp vị trí đích của
Patrol trong cùng lượt `solve()` rồi ghi vị trí điểm hẹn thực tế vào cùng loại
dữ liệu; điểm hẹn có thể là Spot của Patrol hoặc vị trí hiện tại của Patrol khi
không có Spot mục tiêu.

`Solver` lưu các vị trí này trong `currentTargetPositions_` và cung cấp qua
`getPlannedTargetPosition(...)`. `DiaryWriter` chỉ đọc giá trị đó để ghi mục
tiêu và đối chiếu từng bước, không suy đoán lại tọa độ từ mảng action.
Với xe Supply, `SupplyPlanner` đồng thời ghi `plannedTargetPatrol`; diary dùng
giá trị này để chỉ rõ Supply đang đi đến điểm hẹn của xe Patrol nào.

Ngoài mục tiêu cuối ngày, mỗi planner còn ghi `plannedStepSpots` và
`plannedStepPositions`. Hai mảng này có một phần tử cho từng step; vì vậy khi
Patrol chuyển sang Spot tiếp theo, hoặc khi Supply giữ điểm hẹn, diary đọc đúng
mục tiêu tại step tương ứng thay vì lặp lại mục tiêu cuối ngày.

### Các hàm định dạng nội bộ

`agentKindName()`, `spotName()`, `actionName()`, `formatActions()` và
`writeAgentTimeline()` chỉ phục vụ việc chuyển dữ liệu kế hoạch thành Markdown.
Chúng không tham gia chiến thuật và không được gọi từ Solver.

## 9. Hàm test/debug

Các hàm trong mục này chỉ có mục đích kiểm tra hoặc quan sát khi phát triển,
không phải logic thi đấu và không được dùng để thay đổi kế hoạch gửi server.

### `GameApiClient::getMatchStatus()` - debug response

Dòng in `[DEBUG] GET /status: ...` chỉ hiển thị các trường raw server trả về
để đối chiếu `day`, `currentDay`, `totalDays`, `finished` và `endsAt`. Đây là
nhật ký console phục vụ debug API, khác với diary Markdown của `DiaryWriter`.

### `tests/test_all.cpp` và `tests/test_runner.cpp`

Các hàm trong hai file này chỉ dùng để chạy unit test hoặc chạy thử solver với
input mẫu. Chúng không thuộc luồng thi đấu API và không tạo diary trận đấu.

## 10. Sơ đồ quan hệ giữa các Class

```
┌─────────────────────────────────────────────────────────────┐
│                    Solver (Nhạc trưởng)                     │
│  • Quản lý trạng thái: brands, stock, targets              │
│  • Gọi PatrolPlanner cho xe Patrol                         │
│  • Gọi SupplyPlanner cho xe Supply                         │
└──────────┬──────────────────────┬───────────────────────────┘
           │                      │
           ▼                      ▼
┌─────────────────────┐  ┌─────────────────────┐
│   PatrolPlanner     │  │   SupplyPlanner     │
│   (Xe Tuần tra)     │  │   (Xe Tiếp tế)      │
│                     │  │                     │
│  Gọi SpotScorer     │  │  Gọi PathFinder     │
│  Gọi PathFinder     │  │  Gọi MoveSimulator  │
│  Gọi MoveSimulator  │  │                     │
└──┬──────┬──────┬────┘  └──────┬──────┬───────┘
   │      │      │              │      │
   ▼      ▼      ▼              ▼      ▼
┌──────┐ ┌──────────┐ ┌───────────────────┐
│Spot  │ │ Path     │ │  MoveSimulator    │
│Scorer│ │ Finder   │ │  (Mô phỏng)      │
│      │ │(Dijkstra)│ │                   │
│Chấm  │ │Tìm đường│ │ Biến hướng đi →   │
│điểm  │ │ngắn nhất│ │ chuỗi hành động   │
│Spot  │ │         │ │ + tính xăng/bước  │
└──────┘ └────┬────┘ └───────────────────┘
              │
              ▼
        ┌──────────┐
        │   Map    │
        │(Bản đồ) │
        │          │
        │ Địa hình │
        │ Giao thông│
        │ Hex grid │
        └──────────┘
```

### Luồng dữ liệu 1 ngày thi đấu:

```
Server → GameApiClient → Solver.solve()
                            │
                            ├── PatrolPlanner.planDay()
                            │       ├── SpotScorer.findBestSpot()
                            │       │       └── PathFinder.findPath() ← tìm đường tới Spot
                            │       ├── PathFinder.findPath()         ← lấy directions
                            │       └── MoveSimulator.simulate()      ← tạo actions
                            │
                            ├── SupplyPlanner.planDay()
                            │       ├── findTargetPatrol()            ← tìm xe cần cứu
                            │       ├── PathFinder.findPath()         ← tìm đường tới điểm hẹn
                            │       └── MoveSimulator.simulate()      ← tạo actions
                            │
                            └── actions[][] → GameApiClient → Server
```
