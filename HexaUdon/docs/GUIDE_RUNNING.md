# Hướng dẫn chạy HexaUdon

## 1. Chuẩn bị

Mở PowerShell tại thư mục dự án:

```powershell
cd D:\HIT\Procon\HexaUdon
```

Giao diện mô phỏng cần Python và `solver_api.exe`. Giao diện không gọi game server.

## 2. Chạy giao diện với Solver C++ thật

Build Solver API:

```powershell
.\build.bat api
```

Nếu batch script gặp vấn đề encoding, build trực tiếp:

```powershell
& C:\MinGW\bin\g++.exe -std=c++17 -Iinclude -Ithird_party `
  src/solver_api_main.cpp src/solver/SolverApi.cpp `
  src/io/JsonReader.cpp src/io/JsonWriter.cpp `
  src/map/Map.cpp src/solver/ActionValidator.cpp `
  src/solver/PathFinder.cpp src/solver/Solver.cpp `
  -o solver_api.exe
```

Khởi động local bridge:

```powershell
python simulator/server.py
```

Nếu máy không nhận `python`, dùng Python executable đã cài:

```powershell
& C:\Users\lam05\AppData\Local\Programs\Python\Python39\python.exe simulator/server.py
```

Mở trình duyệt tại:

```text
http://localhost:4173
```

Khi bấm `Next step` hoặc `Play simulation`, luồng thực tế là:

```text
Browser -> POST /api/solve -> server.py -> solver_api.exe
        -> SolverApi::solve -> Solver::solve -> ActionValidator -> Browser
```

Kiểm tra server:

```powershell
Invoke-WebRequest http://localhost:4173/ -UseBasicParsing
```

Dừng server bằng `Ctrl+C` trong cửa sổ Python.

## 3. Kiểm tra console

1. Bấm `Generate map`.
2. Bấm `Next step` hoặc `Play simulation`.
3. Kiểm tra `CONSOLE / Day telemetry`.
4. Kiểm tra `Event log` có các dòng:

```text
Solver API returned a plan for day 0
Applied Solver action step 1
```

Console phải ghi vị trí và fuel của từng agent sau mỗi step. Nếu vị trí không đổi,
build lại `solver_api.exe` sau khi sửa `src/solver/Solver.cpp`.

## 4. Chạy unit test C++

```powershell
.\build.bat test
```

Hoặc build trực tiếp:

```powershell
& C:\MinGW\bin\g++.exe -std=c++17 -Iinclude -Ithird_party `
  tests/test_all.cpp `
  src/io/JsonReader.cpp src/io/JsonWriter.cpp `
  src/map/Map.cpp src/solver/ActionValidator.cpp `
  src/solver/PathFinder.cpp src/solver/Solver.cpp `
  -o test_all.exe
.\test_all.exe
```

## 5. Chạy bot local qua stdin

Chế độ này không mở giao diện và không gọi API server:

```powershell
Get-Content demo_input.json | .\HexaUdon.exe --stdin
```

## 6. Chạy bot qua Game API thật

Đảm bảo `.env` có:

```env
API_BASE_URL=https://procon26.haui.ac.vn
API_TOKEN=your_team_token
MATCH_ID=
API_TRACE=1
```

Build và chạy:

```powershell
.\build.bat
.\HexaUdon.exe 2> api_trace.log
```

`MATCH_ID` để trống sẽ dùng trận đầu tiên từ `GET /api/game/matches`. Đây là chế độ
kết nối server thi đấu thật, khác với simulator local ở phần 2. Dừng bằng `Ctrl+C`.
