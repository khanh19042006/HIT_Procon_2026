# Cẩm Nang Tra Cứu & Trích Xuất Dữ Liệu Thực Thể Bản Đồ HexaUdon

Tài liệu này hướng dẫn chi tiết **cách trích xuất thông tin của TẤT CẢ các thực thể xuất hiện trên bản đồ** (Gian hàng Udon, Xe đội nhà, Xe đối thủ, Tình trạng tắc đường, Ô địa hình...) bên trong hàm `Solver::solve(config, state, map)`.

---

## 1. Gian Hàng Udon (`Spot`) - Vị trí, Thương hiệu & Số lượng Tồn kho

Danh sách gian hàng Udon nằm cố định trong `config.spots`.

### Các thuộc tính của `Spot`:
- `spot.brand`: Thương hiệu chuỗi cửa hàng Udon (Giá trị `0` hoặc `1`).
- `spot.pos`: Chỉ số ô (Pos Index `0 .. width*height - 1`).
- `spot.stocks`: Số lượng Udon tồn kho tối đa của gian hàng này.

### Ví dụ Code lấy thông tin tất cả gian hàng Udon:
```cpp
for (size_t i = 0; i < config.spots.size(); ++i) {
    const Spot& spot = config.spots[i];

    int brand = spot.brand;                    // Thương hiệu Udon (0 hoặc 1)
    int posIndex = spot.pos;                   // Pos Index trên bản đồ
    int udonStock = spot.stocks;               // Số lượng tồn kho Udon

    // Chuyển posIndex sang Tọa độ 2D (x, y) để tính khoảng cách/tìm đường
    Position coord = map.posToCoordinate(posIndex);

    std::cerr << "Gian hang #" << i 
              << " | Brand: " << brand 
              << " | Stock Udon: " << udonStock 
              << " | Pos Index: " << posIndex 
              << " (x=" << coord.x << ", y=" << coord.y << ")\n";
}
```

---

## 2. Agent Đội Nhà (`state.agents`) - Vị Trí, Nhiên Liệu & Loại Xe

Danh sách các xe của đội nhà cập nhật thời gian thực từng ngày trong `state.agents`.

### Các thuộc tính của `Agent`:
- `agent.kind`: Loại xe (`0`: 巡回車 - Xe tuần tra bán Udon, `1`: 補給車 - Xe tiếp tế nhiên liệu).
- `agent.pos`: Vị trí hiện tại trên bản đồ (Pos Index).
- `agent.fuel`: Lượng nhiên liệu (`fuel`) hiện tại trong bình của xe.

### Ví dụ Code lấy thông tin xe đội nhà:
```cpp
for (size_t i = 0; i < state.agents.size(); ++i) {
    const Agent& agent = state.agents[i];

    int agentId = i;                          // ID xe (0, 1, 2, ...)
    int kind = agent.kind;                    // 0: Xe tuần tra, 1: Xe tiếp tế
    int currentPosIndex = agent.pos;          // Pos Index hiện tại
    int currentFuel = agent.fuel;             // Lượng xăng hiện tại trong bình

    // Lấy tọa độ (x, y) của xe
    Position pos2D = map.posToCoordinate(currentPosIndex);

    std::cerr << "Xe doi nha #" << agentId 
              << " | Loai xe: " << (kind == 0 ? "Xe Tuan Tra (Patrol)" : "Xe Tiep Te (Supply)")
              << " | Vi tri: (x=" << pos2D.x << ", y=" << pos2D.y << ")"
              << " | Fuel: " << currentFuel << "/" << config.fuelLimit << "\n";
}
```

---

## 3. Xe Của Các Đội Đối Thủ (`state.others`)

Danh sách các xe của các đội chơi khác trên bản đồ nằm trong `state.others`.

### Ví dụ Code lấy thông tin tất cả xe đối thủ:
```cpp
for (const OtherPlayer& enemyTeam : state.others) {
    int enemyTeamId = enemyTeam.id; // ID đội đối thủ

    for (size_t k = 0; k < enemyTeam.agents.size(); ++k) {
        const Agent& enemyAgent = enemyTeam.agents[k];

        int enemyKind = enemyAgent.kind;       // 0: Patrol, 1: Supply
        int enemyPosIndex = enemyAgent.pos;    // Vị trí xe đối thủ
        int enemyFuel = enemyAgent.fuel;       // Lượng xăng còn lại của đối thủ

        Position enemyCoord = map.posToCoordinate(enemyPosIndex);

        std::cerr << "Xe doi thu (Team " << enemyTeamId << ", Xe #" << k << ")"
                  << " | Vi tri: (x=" << enemyCoord.x << ", y=" << enemyCoord.y << ")"
                  << " | Fuel: " << enemyFuel << "\n";
    }
}
```

---

## 4. Tình Trạng Tắc Nghẽn Giao Thông (`state.traffics`)

Thông tin độ kẹt xe trên các ô đường giao thông cập nhật từng ngày trong `state.traffics`.

### Các trạng thái kẹt xe (`status`):
- `0`: Normal (Bình thường - Xe đi qua mượt mà).
- `1`: Busy (Đông xe - Đạt ngưỡng `config.busyThreshold`).
- `2`: Jammed (Tắc nghẽn/Kẹt xe - Đạt ngưỡng `config.jammedThreshold`).

### Ví dụ Code tra cứu trạng thái giao thông:
```cpp
for (const Traffic& tf : state.traffics) {
    int roadPosIndex = tf.pos;   // Vị trí ô đường
    int trafficStatus = tf.status; // 0: Normal, 1: Busy, 2: Jammed

    Position roadCoord = map.posToCoordinate(roadPosIndex);

    if (trafficStatus == 2) {
        std::cerr << "WARNING: O duong (x=" << roadCoord.x << ", y=" << roadCoord.y << ") dang bi TAC NGHEAN!\n";
    }
}
```

---

## 5. Địa Hình Bản Đồ Lục Giác (`config.map` & `map`)

Kiểm tra loại đất/địa hình tại từng ô trên lưới Hex:

### Mã loại địa hình (`cellType`):
- `0`: 平地 (Đồng bằng / Flat ground)
- `1`: 道路 (Đường giao thông / Road)
- `2`: 山地 (Núi / Mountain)
- `3`: 池 (Hồ nước / Pond - **KHÔNG THỂ DI CHUYỂN VÀO**)

### Ví dụ Code kiểm tra ô bất kỳ:
```cpp
// 1. Kiểm tra bằng Tọa độ (x, y)
int cellType = map.getCell(x, y);

// 2. Kiểm tra ô đó có đi vào được hay không (Trả về true nếu KHÔNG PHẢI là Pond hay ngoài map)
bool walkable = map.canMove(x, y);

// 3. Lấy tọa độ 6 ô lân cận theo hướng lục giác (0 -> 5)
for (int dir = 0; dir < 6; ++dir) {
    Position neighbor = map.nextPosition({x, y}, dir);
    if (map.canMove(neighbor)) {
        // Ô lân cận hợp lệ
    }
}
```

---

## 6. Tổng Kết Bảng Tra Cứu Nhanh Dữ Liệu

| Thực Thể Cần Lấy | Cách Lấy Trong Code C++ | Biến / Trường Thuộc Tính |
| :--- | :--- | :--- |
| **Số Lượng Udon Của Gian Hàng** | `config.spots[i].stocks` | `stocks` (Số lượng tồn kho) |
| **Vị Trí Gian Hàng Udon** | `config.spots[i].pos` | `pos` (Pos Index -> map.posToCoordinate) |
| **Thương Hiệu Gian Hàng** | `config.spots[i].brand` | `brand` (0 hoặc 1) |
| **Vị Trí Xe Đội Nhà** | `state.agents[i].pos` | `pos` (Pos Index -> map.posToCoordinate) |
| **Xăng Xe Đội Nhà** | `state.agents[i].fuel` | `fuel` (Lượng nhiên liệu) |
| **Loại Xe Đội Nhà** | `state.agents[i].kind` | `kind` (0: Xe tuần tra, 1: Xe tiếp tế) |
| **Vị Trí Xe Đối Thủ** | `state.others[p].agents[k].pos` | `pos` (Vị trí ô của xe đối thủ) |
| **Tắc Đường / Giao Thông** | `state.traffics[t].status` | `status` (0: Bình thường, 1: Đông, 2: Kẹt) |
| **Số Bước Quy Định Ngày** | `config.daySteps[state.day]` | Tổng số steps cần tạo ra cho mỗi xe |
