# Nhật ký hành trình - Ngày 0

- Số bước trong ngày: 20
- Số xe: 4
- Kế hoạch: Bộ giải

### Xe #0 - Tuần tra

- Vị trí đầu ngày: (8, 8) (ô=88)
- Nhiên liệu đầu ngày: 20
- Mục tiêu hiện tại: Spot #3 (thương hiệu=3, tọa độ=(4, 2))
- Địa điểm đến: Spot #3 (thương hiệu=3, tọa độ=(4, 2))
- Mảng hành động cuối ngày: `[0, 0, 2, 0, 1, 1, 0, 5, 5, 5, 5, -1]`

| Bước | Hành động | Từ ô | Đến ô | Mục tiêu/Spot | Nhiên liệu còn lại |
|---:|---|---|---|---|---:|
| 0-1 | Di chuyển hướng 0 (`0`) | (8, 8) | (8, 7) | Di chuyển đến (8, 7); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 19 |
| 2-3 | Di chuyển hướng 0 (`0`) | (8, 7) | (7, 6) | Di chuyển đến (7, 6); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 18 |
| 4-5 | Di chuyển hướng 2 (`2`) | (7, 6) | (8, 6) | Di chuyển đến (8, 6); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 17 |
| 6-7 | Di chuyển hướng 0 (`0`) | (8, 6) | (8, 5) | Di chuyển đến (8, 5); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 16 |
| 8 | Di chuyển hướng 1 (`1`) | (8, 5) | (8, 4) | Di chuyển đến (8, 4); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 14 |
| 9-10 | Di chuyển hướng 1 (`1`) | (8, 4) | (9, 3) | Di chuyển đến (9, 3); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 13 |
| 11-12 | Di chuyển hướng 0 (`0`) | (9, 3) | (8, 2) | Di chuyển đến (8, 2); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 12 |
| 13-14 | Di chuyển hướng 5 (`5`) | (8, 2) | (7, 2) | Di chuyển đến (7, 2); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 11 |
| 15 | Di chuyển hướng 5 (`5`) | (7, 2) | (6, 2) | Di chuyển đến (6, 2); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 9 |
| 16 | Di chuyển hướng 5 (`5`) | (6, 2) | (5, 2) | Di chuyển đến (5, 2); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 7 |
| 17-18 | Di chuyển hướng 5 (`5`) | (5, 2) | (4, 2) | Đã đạt mục tiêu Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 6 |
| 19 | Chờ 1 bước (`-1`) | (4, 2) | (4, 2) | Đứng yên tại (4, 2) | 6 |

### Xe #1 - Tuần tra

- Vị trí đầu ngày: (1, 9) (ô=91)
- Nhiên liệu đầu ngày: 20
- Mục tiêu hiện tại: Không có mục tiêu
- Địa điểm đến: Không có Spot mục tiêu
- Mảng hành động cuối ngày: `[-20]`

| Bước | Hành động | Từ ô | Đến ô | Mục tiêu/Spot | Nhiên liệu còn lại |
|---:|---|---|---|---|---:|
| 0-19 | Chờ 20 bước (`-20`) | (1, 9) | (1, 9) | Đứng yên tại (1, 9) | 20 |

### Xe #2 - Tuần tra

- Vị trí đầu ngày: (4, 4) (ô=44)
- Nhiên liệu đầu ngày: 20
- Mục tiêu hiện tại: Không có mục tiêu
- Địa điểm đến: Không có Spot mục tiêu
- Mảng hành động cuối ngày: `[-20]`

| Bước | Hành động | Từ ô | Đến ô | Mục tiêu/Spot | Nhiên liệu còn lại |
|---:|---|---|---|---|---:|
| 0-19 | Chờ 20 bước (`-20`) | (4, 4) | (4, 4) | Đứng yên tại (4, 4) | 20 |

### Xe #3 - Tiếp tế

- Vị trí đầu ngày: (6, 8) (ô=86)
- Nhiên liệu đầu ngày: 20
- Mục tiêu hiện tại: Spot #3 (thương hiệu=3, tọa độ=(4, 2))
- Địa điểm đến: Spot #3 (thương hiệu=3, tọa độ=(4, 2))
- Mảng hành động cuối ngày: `[0, 0, 0, 0, 0, 1, -8]`

| Bước | Hành động | Từ ô | Đến ô | Mục tiêu/Spot | Nhiên liệu còn lại |
|---:|---|---|---|---|---:|
| 0-1 | Di chuyển hướng 0 (`0`) | (6, 8) | (6, 7) | Di chuyển đến (6, 7); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 20 |
| 2-3 | Di chuyển hướng 0 (`0`) | (6, 7) | (5, 6) | Di chuyển đến (5, 6); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 20 |
| 4-6 | Di chuyển hướng 0 (`0`) | (5, 6) | (5, 5) | Di chuyển đến (5, 5); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 20 |
| 7 | Di chuyển hướng 0 (`0`) | (5, 5) | (4, 4) | Di chuyển đến (4, 4); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 20 |
| 8-9 | Di chuyển hướng 0 (`0`) | (4, 4) | (4, 3) | Di chuyển đến (4, 3); hướng tới Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 20 |
| 10-11 | Di chuyển hướng 1 (`1`) | (4, 3) | (4, 2) | Đã đạt mục tiêu Spot #3 (thương hiệu=3, tọa độ=(4, 2)) | 20 |
| 12-19 | Chờ 8 bước (`-8`) | (4, 2) | (4, 2) | Đứng yên tại (4, 2) | 20 |

