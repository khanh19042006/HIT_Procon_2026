# Hướng Dẫn Kiểm Thử & Debug Logic Trong HexaUdon

Tài liệu này chi tiết **cách kiểm tra hoạt động của chương trình**, **bộ công cụ test agent visualizer** và **tiêu chí đánh giá mức độ hoàn thiện của logic di chuyển**.

---

## 1. Công Cụ Test Agent Visualizer (`test_runner.exe`)

Chúng tôi đã tích hợp công cụ test visualizer tại [tests/test_runner.cpp](file:///d:/HIT/Procon/HexaUdon/tests/test_runner.cpp). Khi chạy công cụ này, chương trình sẽ in toàn bộ thông số chi tiết của từng Agent ở thời gian thực.

### Cách chạy nhanh bằng 1 lệnh:

```powershell
.\build.bat runner
```

### Kết quả hiển thị mẫu trên Terminal:

```text
========================================================================
          HEXAUDON RUNNER & AGENT DEBUGGER VISUALIZER                  
========================================================================

[STEP 1] Dang doc thong tin GameConfig tu stdin...
 -> Map Size     : 4x4
 -> So Agent     : 2
 -> So Gian Hang : 1
 -> So Ngay Dinh : 2
 -> Fuel Limit   : 20

[STEP 2] Quyen chon loai Agent (0: Patrol, 1: Supply):
 -> Agent [0]: Patrol (巡回車)
 -> Agent [1]: Patrol (巡回車)

------------------------------------------------------------------------
                     NGAY THI DAU 0 (Steps: 10)
------------------------------------------------------------------------
 [INSPECTING AGENTS BEFORE SOLVE]:
  * Agent #0 | Pos Index:   0 (x=0, y=0) | Fuel: 20 | Kind: Patrol
  * Agent #1 | Pos Index:   5 (x=1, y=1) | Fuel: 20 | Kind: Patrol

 [ACTION PLAN OUTPUT & VALIDATION]:
  * Validation Result: VALID (CHINH XAC)
  * Agent #0 Actions: [MoveDir(2), Wait(9)] (Tong steps: 10/10)
  * Agent #1 Actions: [MoveDir(0), Wait(9)] (Tong steps: 10/10)
```

---

## 2. Cách Nhận Biết Logic Đang Ở Mức Nào

Dựa vào đầu ra của `.\build.bat runner`, bạn có thể đánh giá cấp độ logic của thuật toán:

| Cấp Độ Logic | Đặc Điểm Nhận Biết | Trạng Thái Trong Code |
| :--- | :--- | :--- |
| **Mức 0 (Cơ bản / Default)** | Các xe luôn chỉ ra mảng `Wait(-steps)` (Đứng yên toàn bộ). | Chưa viết logic di chuyển. |
| **Mức 1 (An toàn)** | Các xe di chuyển 1 ô hợp lệ rồi đứng yên `[MoveDir(X), Wait(N-1)]`. | Mẫu khởi tạo hiện tại trong [Solver.cpp](file:///d:/HIT/Procon/HexaUdon/src/solver/Solver.cpp). |
| **Mức 2 (Tìm đường BFS/A*)** | Xe di chuyển liên tục qua nhiều ô đến gian hàng `Spot`. `Validation Result: VALID`. | Đang dùng `PathFinder::findPath` đi tới mục tiêu. |
| **Mức 3 (Hoàn chỉnh / AI)** | Xe tự quản lý nhiên liệu `Fuel`, né tránh ô tắc đường `Traffic status` và xe đối thủ `others`. | Logic chiến thuật nâng cao. |

---

## 3. Các Vị Trí Viết Logic `// TODO [USER]`

Bạn chỉ cần tập trung chỉnh sửa logic ở 2 khu vực được đánh dấu comment `// TODO [USER]`:

1. **Phân bổ loại xe** tại [src/solver/Solver.cpp](file:///d:/HIT/Procon/HexaUdon/src/solver/Solver.cpp):
   ```cpp
   // TODO [USER]: Bạn có thể đổi loại xe ở đây (0: Xe tuần tra, 1: Xe tiếp tế)
   ```
2. **Logic di chuyển chính** tại [src/solver/Solver.cpp](file:///d:/HIT/Procon/HexaUdon/src/solver/Solver.cpp):
   ```cpp
   // TODO [USER]: VIẾT LOGIC DI CHUYỂN CHO NGUỜI DÙNG TẠI ĐÂY
   ```
