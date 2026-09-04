# Cuộc Thi Lập Trình HexaUdon - Đề Bài (Lần 37)

## Tổng Quan
Đây là bài toán về quản lý và điều khiển các phương tiện (xe tuần tra và xe tiếp tế) trên một bản đồ lục giác để hoàn thành các nhiệm vụ giao hàng và tuần tra.

---

## I. Định Dạng Cấu Hình Bản Đồ (Trước Khi Trận Đấu Bắt Đầu)

Trước khi trận đấu bắt đầu, cấu hình bản đồ được cung cấp dưới dạng JSON với các thông tin sau:

### Cấu Trúc JSON

```json
{
  "startsAt": 1778227200,
  "daySeconds": [5, 5, 5, 10],
  "daySteps": [50, 100, 150, 200],
  "map": {
    "height": 8,
    "width": 8,
    "cells": [
      [3, 0, 1, 2, 0, 1, 2, 0],
      [3, 0, 1, 2, 0, 1, 2, 0],
      [3, 0, 1, 2, 0, 1, 2, 0],
      [3, 0, 1, 2, 0, 1, 2, 0],
      [3, 0, 1, 2, 0, 1, 2, 0],
      [3, 0, 1, 2, 0, 1, 2, 0],
      [3, 0, 1, 2, 0, 1, 2, 0],
      [3, 0, 1, 2, 0, 1, 2, 0]
    ]
  },
  "spots": [
    {"brand": 0, "pos": 1, "stocks": 4},
    {"brand": 1, "pos": 9, "stocks": 1},
    {"brand": 0, "pos": 17, "stocks": 1},
    {"brand": 1, "pos": 25, "stocks": 3}
  ],
  "agents": [4, 12, 20, 28],
  "fuelLimits": 20,
  "players": 8,
  "busyThreshold": 2,
  "jammedThreshold": 4
}
```

### Mô Tả Các Trường

| Trường | Mô Tả |
|--------|-------|
| **startsAt** | Thời gian bắt đầu trận đấu (UNIX TIME) |
| **daySeconds** | Mảng thời gian tối đa để trả lời cho mỗi ngày (tính bằng giây) |
| **daySteps** | Mảng số bước di chuyển cho mỗi ngày. Số phần tử của mảng này = tổng số ngày trong trận đấu |
| **map.height** | Chiều cao của bản đồ |
| **map.width** | Chiều rộng của bản đồ |
| **map.cells** | Loại địa hình của mỗi ô: 0=Bằng phẳng, 1=Đường, 2=Núi, 3=Hồ nước |
| **spots** | Thông tin các điểm dừng |
| **spots[].brand** | Hệ thống hoặc nhãn hiệu của điểm dừng |
| **spots[].pos** | Vị trí của điểm dừng trên bản đồ |
| **spots[].stocks** | Sức chứa tối đa của điểm dừng |
| **agents** | Vị trí khởi đầu của các agent của đội bạn |
| **fuelLimits** | Giới hạn dung lượng nhiên liệu của mỗi xe |
| **players** | Số lượng đội chơi tham gia |
| **busyThreshold** | Ngưỡng tiêu chuẩn để coi một đường là "bận rộn" (tắc nghẽn nhẹ) |
| **jammedThreshold** | Ngưỡng tiêu chuẩn để coi một đường là "kẹt xe" (tắc nghẽn nặng) |

---

## II. Định Dạng Thông Tin Trận Đấu (Mỗi Ngày)

Ở đầu mỗi ngày, thông tin trạng thái trận đấu được cung cấp dưới dạng JSON:

### Cấu Trúc JSON

```json
{
  "endsAt": 1778227205,
  "day": 1,
  "agents": [
    {"kind": 0, "pos": 1, "fuel": 20},
    {"kind": 1, "pos": 1, "fuel": 20},
    {"kind": 0, "pos": 9, "fuel": 10},
    {"kind": 0, "pos": 9, "fuel": 0}
  ],
  "others": [
    {
      "id": 0,
      "agents": [
        {"kind": 0, "pos": 1, "fuel": 2},
        {"kind": 0, "pos": 1, "fuel": 3},
        {"kind": 0, "pos": 1, "fuel": 4},
        {"kind": 0, "pos": 1, "fuel": 5}
      ]
    },
    {
      "id": 1,
      "agents": [
        {"kind": 1, "pos": 1, "fuel": 20},
        {"kind": 1, "pos": 9, "fuel": 20},
        {"kind": 1, "pos": 17, "fuel": 20},
        {"kind": 1, "pos": 25, "fuel": 20}
      ]
    }
  ],
  "traffics": [
    {"pos": 1, "status": 0},
    {"pos": 9, "status": 0},
    {"pos": 17, "status": 1},
    {"pos": 25, "status": 2}
  ]
}
```

### Mô Tả Các Trường

| Trường | Mô Tả |
|--------|-------|
| **endsAt** | Thời hạn cuối cùng để trả lời cho ngày này (UNIX TIME) |
| **day** | Số ngày hiện tại (ngày đầu tiên = 0) |
| **agents** | Thông tin chi tiết về các agent của đội bạn |
| **agents[].kind** | Loại xe: 0=Xe tuần tra, 1=Xe tiếp tế |
| **agents[].pos** | Vị trí hiện tại của xe |
| **agents[].fuel** | Lượng nhiên liệu còn lại |
| **others** | Thông tin về các xe của đội đối thủ |
| **others[].id** | ID của đội đối thủ |
| **others[].agents** | Mảng thông tin xe của đội đối thủ (định dạng giống như agents của đội bạn) |
| **traffics** | Thông tin tắc đường trên các đoạn đường |
| **traffics[].pos** | Vị trí của đoạn đường |
| **traffics[].status** | Tình trạng giao thông: 0=Bình thường, 1=Bận rộn (busy), 2=Kẹt xe (jammed) |

---

## III. Định Dạng Lựa Chọn Loại Agent

Trước khi trận đấu bắt đầu, bạn phải quy định loại xe cho mỗi agent.

### Cấu Trúc JSON

```json
[0, 1, 0, 1]
```

### Mô Tả

- **Mảng cấp cao nhất chứa loại xe cho mỗi agent**
- **Giá trị**: 0 = Xe tuần tra (Patrol), 1 = Xe tiếp tế (Supply)
- **Số phần tử của mảng phải bằng số lượng agent**

### Quy Tắc Validation

- Nếu số phần tử không khớp với số agent → **bị từ chối**
- Nếu có giá trị khác 0 hoặc 1 → **bị từ chối**
- Nếu không trả lời đúng trong thời hạn → **mặc định tất cả agent là xe tuần tra (0)**

---

## IV. Định Dạng Kế Hoạch Hành Động

Ở mỗi ngày, bạn phải đưa ra kế hoạch hành động chi tiết cho tất cả các agent.

### Cấu Trúc JSON

```json
[
  [-15],
  [0, 1, -10]
]
```

### Mô Tả

- **Mảng ngoài**: Chứa kế hoạch hành động cho mỗi agent
- **Mảng trong (agents[i])**: Chuỗi các hành động của agent i

### Định Nghĩa Hành Động

| Hành Động | Ý Nghĩa | Chi Tiết |
|-----------|---------|---------|
| **-1 trở lên (âm)** | Chờ/Dừng lại | -K = chờ K bước tại chỗ |
| **0-5 (dương)** | Di chuyển | Hướng di chuyển (hình lục giác có 6 hướng) |
| | | 0 = Trái trên (↖) |
| | | 1 = Phải trên (↗) |
| | | 2 = Phải (→) |
| | | 3 = Phải dưới (↘) |
| | | 4 = Trái dưới (↙) |
| | | 5 = Trái (←) |

### Ví Dụ Chi Tiết

```json
[0, 1, -10]
```

- Bước 1: Di chuyển theo hướng 0 (↖)
- Bước 2: Di chuyển theo hướng 1 (↗)
- Bước 3-12: Chờ 10 bước tại chỗ
- **Tổng cộng = 12 bước**

### Quy Tắc Validation

| Quy Tắc | Hậu Quả |
|--------|---------|
| Số bước của mỗi agent **phải bằng đúng `daySteps[day]`** | Nếu lệch → bị từ chối |
| **Không được di chuyển tới ô không thể đi** (Hồ nước, ngoài bản đồ) | Nếu vi phạm → bị từ chối |
| Nếu **1 agent có lỗi**, cả đội **bị từ chối** | Tất cả agent dừng lại cả ngày (-daySteps) |
| Nếu không trả lời trong thời hạn | Mặc định tất cả agent dừng lại (-daySteps) |

---

## V. Tóm Tắt Quy Trình Trán Đấu

### Bước 1: Chuẩn Bị (Trước Khi Trận Đấu)
1. Nhận cấu hình bản đồ (GameConfig)
2. **Chọn loại xe cho mỗi agent** (Patrol hoặc Supply)
3. Gửi danh sách loại xe về

### Bước 2: Mỗi Ngày
1. Nhận thông tin trạng thái trận đấu hiện tại (GameState)
2. **Thiết kế kế hoạch hành động** cho tất cả agent
3. Gửi kế hoạch hành động về
4. Hệ thống xử lý các hành động và cập nhật trạng thái

### Bước 3: Lặp Lại
- Lặp lại Bước 2 cho đến khi kết thúc trận đấu

---

## VI. Lưu Ý Quan Trọng

1. **Thời gian**: Mỗi ngày có giới hạn thời gian trả lời (`daySeconds`)
2. **Bước di chuyển**: Phải sử dụng đúng số bước quy định (`daySteps[day]`)
3. **Nhiên liệu**: Không được vượt quá `fuelLimits`
4. **An toàn**: Nếu sai, hệ thống sẽ tự động cho agent dừng lại (fallback)
5. **Hình lục giác**: Bản đồ sử dụng lưới hình lục giác (hexagonal grid), không phải lưới vuông

---

## VII. Tham Khảo Thêm

- Xem [GUIDE.md](GUIDE.md) để hiểu thêm về cơ chế trò chơi
- Xem [GUIDE_TESTING.md](GUIDE_TESTING.md) để học cách kiểm thử
- Xem [GUIDE_CONFIG_AND_CONTROL.md](GUIDE_CONFIG_AND_CONTROL.md) để biết chi tiết điều khiển
