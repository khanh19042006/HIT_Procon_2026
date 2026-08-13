# Hướng Dẫn Sử Dụng Dự Án HexaUdon

Tài liệu này hướng dẫn cách biên dịch, chạy chương trình, thực hiện kiểm thử tự động và mở rộng chiến thuật cho dự án **HexaUdon** (Chương trình thi đấu cuộc thi Procon - Lưới lục giác Hex Grid).

---

## 1. Yêu cầu hệ thống & Công cụ

- **Trình biên dịch C++**: `g++` (hỗ trợ C++17 trở lên) hoặc `Clang` / `MSVC`.
- **Thư viện phụ thuộc**: `nlohmann/json` (đã được tích hợp sẵn dưới dạng header-only tại `third_party/nlohmann/json.hpp`).
- **CMake** (tùy chọn, nếu muốn sử dụng build system CMake).

---

## 2. Cấu trúc Dự án

```text
HexaUdon/
├── CMakeLists.txt              # File cấu hình biên dịch bằng CMake
├── HexaUdon.exe                # File thực thi chính (sau khi compile)
├── docs/                       # THƯ MỤC TÀI LIỆU HƯỚNG DẪN DỰ ÁN
│   ├── GUIDE.md                # Tra cứu trích xuất dữ liệu thực thể bản đồ (Spot, Agent, Traffic...)
│   ├── GUIDE_CONFIG_AND_CONTROL.md # Hướng dẫn đọc Config/State và mã lệnh điều khiển
│   └── GUIDE_TESTING.md        # Hướng dẫn kiểm thử và công cụ debug Visualizer
├── include/                    # Chứa các file header (.hpp)
│   ├── GameState.hpp           # Struct quản lý trạng thái từng ngày
│   ├── io/
│   │   ├── JsonReader.hpp      # Module đọc dữ liệu JSON từ stdin
│   │   └── JsonWriter.hpp      # Module ghi dữ liệu JSON ra stdout
│   ├── map/
│   │   └── Map.hpp             # Module quản lý bản đồ và di chuyển Hex Grid
│   ├── model/
│   │   ├── Agent.hpp           # Struct thông tin Agent (巡回車 / 補給車)
│   │   ├── GameConfig.hpp      # Struct cấu hình trận đấu ban đầu
│   │   └── Traffic.hpp        # Struct thông tin giao thông
│   └── solver/
│       ├── ActionValidator.hpp # Module kiểm tra tính hợp lệ của Action Plan
│       ├── PathFinder.hpp      # Module tìm đường BFS/A* trên Hex Map
│       └── Solver.hpp          # Bộ não chiến thuật chính
├── src/                        # Chứa các file source (.cpp)
│   ├── main.cpp                # Luồng điều phối chính
│   ├── io/
│   ├── map/
│   └── solver/
├── tests/                      # Bộ kiểm thử tự động (Unit Tests)
│   ├── test_all.cpp            # Suite unit test thuật toán
│   └── test_runner.cpp        # Debug visualizer in thông số agent
└── third_party/                # Thư viện ngoài (nlohmann/json.hpp)
```

---

## 3. Lệnh Biên Dịch & Rút Gọn (Build Scripts)

Đã tích hợp sẵn script tự động thu gọn các câu lệnh biên dịch dài:

### Dành cho Windows (`build.bat`):

- **Biên dịch app thi đấu chính** (`HexaUdon.exe`):
  ```cmd
  .\build.bat
  ```
- **Biên dịch và chạy Unit Tests tự động**:
  ```cmd
  .\build.bat test
  ```
- **Biên dịch và chạy Agent Debug Visualizer**:
  ```cmd
  .\build.bat runner
  ```
- **Dọn dẹp file executable (`.exe`)**:
  ```cmd
  .\build.bat clean
  ```

### Dành cho Linux / macOS (`build.sh`):

- **Biên dịch app chính**: `./build.sh`
- **Chạy Unit Tests**: `./build.sh test`
- **Dọn dẹp**: `./build.sh clean`

---

## 4. Cách Trực Tiếp Chạy & Test Chạy Thử

Chương trình trao đổi dữ liệu với Server thi đấu thông qua **Standard Input (stdin)** và **Standard Output (stdout)** dưới dạng JSON.

### Quyền quản lý luồng dữ liệu của `main.cpp`:
1. Đầu tiên, chương trình đọc JSON **GameConfig** từ `stdin`.
2. Ghi ra JSON danh sách loại xe `agentTypes` (ví dụ `[0, 1, 0, 1]`) ra `stdout`.
3. Với mỗi ngày thi đấu trong `daySteps`:
   - Đọc JSON **GameState** của ngày đó từ `stdin`.
   - Tính toán và kiểm tra hành động (`solve` & `validate`).
   - Ghi JSON **Action Plan** ra `stdout`.

### Test bằng file Input JSON mẫu:

Giả sử bạn có 1 file dữ liệu test (kết hợp Config + State từng ngày), bạn có thể chuyển hướng dữ liệu vào `HexaUdon.exe`:

```bash
Get-Content demo_input.json | .\HexaUdon.exe
```

---

## 5. Chạy Bộ Kiểm Thử Tự Động (Unit Tests)

Để đảm bảo các thuật toán di chuyển trên Hex Map, PathFinder và Validator hoạt động chính xác không có lỗi logic, hãy biên dịch và chạy bộ test suite:

```bash
.\build.bat test
```

**Kết quả mong đợi:**
```text
[PASS] Map and Geometry tests passed!
[PASS] PathFinder tests passed!
[PASS] ActionValidator tests passed!
All unit tests completed successfully!
```

---

## 6. Hướng Dẫn Mở Rộng Chiến Thuật (Cho Đội Phát Triển)

Để nâng cấp thuật toán thông minh hơn trong quá trình làm sản phẩm, bạn chỉ cần sửa đổi các module chuyên biệt mà **không làm ảnh hưởng đến luồng IO hay Parser JSON**:

### 1. Thay đổi phân bổ loại xe (巡回車 vs 補給車)
Sửa hàm `decideAgentTypes` trong [Solver.cpp](file:///d:/HIT/Procon/HexaUdon/src/solver/Solver.cpp):
```cpp
std::vector<int> AgentStrategy::decideAgentTypes(const GameConfig& config) {
    // TODO [USER]: Chọn loại xe (0: Xe tuần tra, 1: Xe tiếp tế)
    return {0, 0, 0, 1};
}
```

### 2. Nâng cấp thuật toán Tìm đường (Pathfinding)
Mở rộng file [PathFinder.cpp](file:///d:/HIT/Procon/HexaUdon/src/solver/PathFinder.cpp) để chuyển từ **BFS** sang **A*** hoặc **Dijkstra** xem xét đến chi phí nhiên liệu (`fuel`) và độ tắc nghẽn của các ô đường giao thông (`Traffic status`).

### 3. Tối ưu hóa bộ nào chiến thuật (Solver Logic)
Sửa hàm `solve` trong [Solver.cpp](file:///d:/HIT/Procon/HexaUdon/src/solver/Solver.cpp) để tính toán lịch trình tuần tra các gian hàng (`Spot`), tiếp nhiên liệu và tránh xe đối thủ (`others`).

---

## 7. Thư Mục Tài Liệu Hướng Dẫn Chi Tiết (`docs/`)

- 📖 **[docs/GUIDE.md](file:///d:/HIT/Procon/HexaUdon/docs/GUIDE.md)**: Cẩm nang tra cứu chi tiết cách lấy thông tin của TẤT CẢ thực thể trên bản đồ (Vị trí & tồn kho Udon của Gian hàng, Vị trí & Xăng của Agent, Xe đối thủ, Tắc đường...).
- 📖 **[docs/GUIDE_CONFIG_AND_CONTROL.md](file:///d:/HIT/Procon/HexaUdon/docs/GUIDE_CONFIG_AND_CONTROL.md)**: Hướng dẫn cấu trúc biến `GameConfig`, `GameState` và cú pháp lập trình điều khiển Agent.
- 📖 **[docs/GUIDE_TESTING.md](file:///d:/HIT/Procon/HexaUdon/docs/GUIDE_TESTING.md)**: Hướng dẫn công cụ test visualizer debug agent thời gian thực.
