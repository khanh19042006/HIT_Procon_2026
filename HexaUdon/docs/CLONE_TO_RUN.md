# Hướng Dẫn Khôi Phục & Chạy Dự Án Từ GitHub (Clone to Run Guide)

Tài liệu này hướng dẫn cách **setup lại toàn bộ môi trường và khởi chạy dự án HexaUdon** ngay sau khi bạn hoặc thành viên khác clone mã nguồn từ GitHub về một máy tính mới.

---

## 1. Danh Sách Những Gì Đã Được Push Lên GitHub

Sau khi lọc qua `.gitignore`, trên GitHub hiện đang lưu giữ đầy đủ mã nguồn thuần (không chứa file rác hay file thực thi nhị phân):

- 📁 **`HexaUdon/include/`**: Toàn bộ file header định nghĩa cấu trúc dữ liệu (`GameConfig`, `GameState`, `Map`, `Agent`, `Solver`...).
- 📁 **`HexaUdon/src/`**: Mã nguồn xử lý chính (`main.cpp`, `JsonReader.cpp`, `JsonWriter.cpp`, `Map.cpp`, `Solver.cpp`...).
- 📁 **`HexaUdon/tests/`**: Các bộ kiểm thử tự động (`test_all.cpp`, `test_runner.cpp`).
- 📁 **`HexaUdon/third_party/`**: Thư viện `nlohmann/json.hpp` (Đã có sẵn, **KHÔNG NÊN** thiếu).
- 📁 **`HexaUdon/docs/`**: Toàn bộ tài liệu hướng dẫn tra cứu (`GUIDE.md`, `GUIDE_CONFIG_AND_CONTROL.md`, `GUIDE_TESTING.md`).
- 📄 **`CMakeLists.txt`**, **`build.bat`**, **`build.sh`**: Các script đóng gói biên dịch tự động.

---

## 2. Những Gì Đã Bị Loại Bỏ Bởi `.gitignore` & Cách Khôi Phục

Khi clone từ GitHub về máy mới, các tài nguyên sau **không có sẵn** (do bị `.gitignore` chặn):

| Loại Tài Nguyên Bị Ẩn | Mục Đích | Cách Khôi Phục / Tự Động Sinh Ra |
| :--- | :--- | :--- |
| **`HexaUdon.exe`** | File chạy chương trình thi đấu | Tự động sinh ra khi gõ lệnh `.\build.bat` |
| **`test_all.exe`** | File chạy bộ test unit | Tự động sinh ra khi gõ lệnh `.\build.bat test` |
| **`test_runner.exe`** | File chạy bộ debug visualizer | Tự động sinh ra khi gõ lệnh `.\build.bat runner` |
| **`demo_input.json`** | File dữ liệu test mẫu | Cần tạo lại (Hướng dẫn ở Mục 4 bên dưới) |

---

## 3. Quy Trình 3 Bước Để Chạy Dự Án Trên Máy Mới

### Bước 1: Yêu cầu Công cụ (Prerequisites)
Đảm bảo máy mới đã cài đặt một trong các trình biên dịch C++ hỗ trợ **C++17**:
- **Windows**: `g++` (MinGW / MSYS2) hoặc MSVC (Visual Studio).
- **Linux / macOS**: `g++` hoặc `clang++`.

> *(Đảm bảo lệnh `g++ --version` có thể chạy được trong Terminal/PowerShell)*.

---

### Bước 2: Clone Mã Nguồn Từ GitHub

Mở Terminal và clone dự án về máy:
```bash
git clone <URL_REPOSITORY_GITHUB_CUA_BAN>
cd HIT_Procon_2026/HexaUdon  # Hoặc cd HexaUdon
```

---

### Bước 3: Biên Dịch Dự Án (Build)

Tại thư mục dự án, chạy 1 lệnh duy nhất để biên dịch mã nguồn thành file thực thi:

- **Trên Windows (`PowerShell` / `CMD`)**:
  ```powershell
  .\build.bat
  ```
- **Trên Linux / macOS**:
  ```bash
  chmod +x build.sh
  ./build.sh
  ```

👉 **Kết quả**: File thực thi `HexaUdon.exe` sẽ được tạo ra sẵn sàng thi đấu!

---

## 4. Cách Tạo Lại File Dữ Liệu Test Mẫu (`demo_input.json`)

Do file `demo_input.json` chứa dữ liệu test tạm thời bị loại bỏ bởi `.gitignore`, trên máy mới bạn có thể tạo lại file `demo_input.json` tại thư mục `HexaUdon/demo_input.json` với nội dung mẫu như sau:

```json
{
  "startsAt": 1778227200,
  "daySeconds": [5, 5],
  "daySteps": [10, 10],
  "map": {
    "height": 4,
    "width": 4,
    "cells": [
      [0, 0, 1, 2],
      [0, 3, 1, 0],
      [1, 0, 0, 0],
      [2, 0, 0, 0]
    ]
  },
  "spots": [
    {"brand": 0, "pos": 1, "stocks": 4}
  ],
  "agents": [0, 5],
  "fuelLimits": 20,
  "players": 2,
  "busyThreshold": 2,
  "jammedThreshold": 4
}
{"endsAt": 1778227205, "day": 0, "agents": [{"kind": 0, "pos": 0, "fuel": 20}, {"kind": 0, "pos": 5, "fuel": 20}], "others": [], "traffics": []}
{"endsAt": 1778227210, "day": 1, "agents": [{"kind": 0, "pos": 1, "fuel": 19}, {"kind": 0, "pos": 5, "fuel": 20}], "others": [], "traffics": []}
```

---

## 5. Chạy Kiểm Thử & Debug Visualizer Trên Máy Mới

Sau khi biên dịch và có file `demo_input.json`, bạn có thể kiểm thử ngay lập tức:

1. **Chạy Debug Visualizer xem trạng thái các Agent**:
   ```powershell
   .\build.bat runner
   ```
2. **Chạy Unit Test thuật toán**:
   ```powershell
   .\build.bat test
   ```
3. **Test trực tiếp app thi đấu với dữ liệu**:
   ```powershell
   type demo_input.json | .\HexaUdon.exe
   ```
