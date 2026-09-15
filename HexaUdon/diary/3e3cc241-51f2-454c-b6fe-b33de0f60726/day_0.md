# Nhat ky hanh trinh - Ngay 0

- So step trong ngay: 20
- So xe: 4
- Ke hoach: Solver

### Xe #0 - Patrol

- Vi tri dau ngay: (6, 1) (cell=16)
- Nhien lieu dau ngay: 20
- Muc tieu hien tai: Spot #1 (brand=1, o=(8, 3))
- Dia diem den: Spot #1 (brand=1, pos=38)
- Mang action cuoi ngay: `[4, 3, 5, 4, 2, 2, 2, 1, -6]`

| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |
|---:|---|---|---|---|---:|
| 0-1 | Di chuyen huong 4 (`4`) | (6, 1) | (5, 2) | Di chuyen den (5, 2); huong toi Spot #1 (brand=1, pos=38) | 19 |
| 2 | Di chuyen huong 3 (`3`) | (5, 2) | (6, 3) | Di chuyen den (6, 3); huong toi Spot #1 (brand=1, pos=38) | 17 |
| 3-4 | Di chuyen huong 5 (`5`) | (6, 3) | (5, 3) | Di chuyen den (5, 3); huong toi Spot #1 (brand=1, pos=38) | 16 |
| 5-6 | Di chuyen huong 4 (`4`) | (5, 3) | (4, 4) | Di chuyen den (4, 4); huong toi Spot #1 (brand=1, pos=38) | 15 |
| 7-8 | Di chuyen huong 2 (`2`) | (4, 4) | (5, 4) | Di chuyen den (5, 4); huong toi Spot #1 (brand=1, pos=38) | 14 |
| 9-10 | Di chuyen huong 2 (`2`) | (5, 4) | (6, 4) | Di chuyen den (6, 4); huong toi Spot #1 (brand=1, pos=38) | 13 |
| 11-12 | Di chuyen huong 2 (`2`) | (6, 4) | (7, 4) | Di chuyen den (7, 4); huong toi Spot #1 (brand=1, pos=38) | 12 |
| 13 | Di chuyen huong 1 (`1`) | (7, 4) | (8, 3) | Dat muc tieu Spot #1 (brand=1, pos=38) | 10 |
| 14-19 | Cho 6 step (`-6`) | (8, 3) | (8, 3) | Dung yen tai (8, 3) | 10 |

### Xe #1 - Patrol

- Vi tri dau ngay: (1, 5) (cell=51)
- Nhien lieu dau ngay: 20
- Muc tieu hien tai: Spot #3 (brand=3, o=(1, 2))
- Dia diem den: Spot #3 (brand=3, pos=21)
- Mang action cuoi ngay: `[0, 1, 1, -15]`

| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |
|---:|---|---|---|---|---:|
| 0-1 | Di chuyen huong 0 (`0`) | (1, 5) | (0, 4) | Di chuyen den (0, 4); huong toi Spot #3 (brand=3, pos=21) | 19 |
| 2-3 | Di chuyen huong 1 (`1`) | (0, 4) | (1, 3) | Di chuyen den (1, 3); huong toi Spot #3 (brand=3, pos=21) | 18 |
| 4 | Di chuyen huong 1 (`1`) | (1, 3) | (1, 2) | Dat muc tieu Spot #3 (brand=3, pos=21) | 16 |
| 5-19 | Cho 15 step (`-15`) | (1, 2) | (1, 2) | Dung yen tai (1, 2) | 16 |

### Xe #2 - Patrol

- Vi tri dau ngay: (4, 5) (cell=54)
- Nhien lieu dau ngay: 20
- Muc tieu hien tai: Khong co muc tieu
- Dia diem den: Khong co Spot muc tieu
- Mang action cuoi ngay: `[-20]`

| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |
|---:|---|---|---|---|---:|
| 0-19 | Cho 20 step (`-20`) | (4, 5) | (4, 5) | Dung yen tai (4, 5) | 20 |

### Xe #3 - Supply

- Vi tri dau ngay: (7, 9) (cell=97)
- Nhien lieu dau ngay: 20
- Muc tieu hien tai: Spot #1 (brand=1, o=(8, 3))
- Dia diem den: Spot #1 (brand=1, pos=38)
- Mang action cuoi ngay: `[0, 1, 1, 1, 0, 1, -11]`

| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |
|---:|---|---|---|---|---:|
| 0-1 | Di chuyen huong 0 (`0`) | (7, 9) | (6, 8) | Di chuyen den (6, 8); huong toi Spot #1 (brand=1, pos=38) | 20 |
| 2 | Di chuyen huong 1 (`1`) | (6, 8) | (7, 7) | Di chuyen den (7, 7); huong toi Spot #1 (brand=1, pos=38) | 20 |
| 3-4 | Di chuyen huong 1 (`1`) | (7, 7) | (7, 6) | Di chuyen den (7, 6); huong toi Spot #1 (brand=1, pos=38) | 20 |
| 5-6 | Di chuyen huong 1 (`1`) | (7, 6) | (8, 5) | Di chuyen den (8, 5); huong toi Spot #1 (brand=1, pos=38) | 20 |
| 7 | Di chuyen huong 0 (`0`) | (8, 5) | (7, 4) | Di chuyen den (7, 4); huong toi Spot #1 (brand=1, pos=38) | 20 |
| 8 | Di chuyen huong 1 (`1`) | (7, 4) | (8, 3) | Dat muc tieu Spot #1 (brand=1, pos=38) | 20 |
| 9-19 | Cho 11 step (`-11`) | (8, 3) | (8, 3) | Dung yen tai (8, 3) | 20 |

