# Nhat ky hanh trinh - Ngay 1

- So step trong ngay: 40
- So xe: 4
- Ke hoach: Solver

### Xe #0 - Patrol

- Vi tri dau ngay: (8, 3) (cell=38)
- Nhien lieu dau ngay: 20
- Muc tieu hien tai: Spot #1 (brand=1, o=(8, 3))
- Dia diem den: Spot #1 (brand=1, pos=38)
- Mang action cuoi ngay: `[-40]`

| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |
|---:|---|---|---|---|---:|
| 0-39 | Cho 40 step (`-40`) | (8, 3) | (8, 3) | Dung yen tai (8, 3) | 20 |

### Xe #1 - Patrol

- Vi tri dau ngay: (1, 2) (cell=21)
- Nhien lieu dau ngay: 16
- Muc tieu hien tai: Spot #3 (brand=3, o=(1, 2))
- Dia diem den: Spot #3 (brand=3, pos=21)
- Mang action cuoi ngay: `[-40]`

| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |
|---:|---|---|---|---|---:|
| 0-39 | Cho 40 step (`-40`) | (1, 2) | (1, 2) | Dung yen tai (1, 2) | 16 |

### Xe #2 - Patrol

- Vi tri dau ngay: (4, 5) (cell=54)
- Nhien lieu dau ngay: 20
- Muc tieu hien tai: Khong co muc tieu
- Dia diem den: Khong co Spot muc tieu
- Mang action cuoi ngay: `[-40]`

| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |
|---:|---|---|---|---|---:|
| 0-39 | Cho 40 step (`-40`) | (4, 5) | (4, 5) | Dung yen tai (4, 5) | 20 |

### Xe #3 - Supply

- Vi tri dau ngay: (8, 3) (cell=38)
- Nhien lieu dau ngay: 20
- Muc tieu hien tai: Spot #3 (brand=3, o=(1, 2))
- Dia diem den: Spot #3 (brand=3, pos=21)
- Mang action cuoi ngay: `[0, 5, 5, 5, 5, 5, 5, -29]`

| Step | Hanh dong | Tu o | Den o | Muc tieu/spot | Fuel con lai |
|---:|---|---|---|---|---:|
| 0-1 | Di chuyen huong 0 (`0`) | (8, 3) | (7, 2) | Di chuyen den (7, 2); huong toi Spot #3 (brand=3, pos=21) | 20 |
| 2-3 | Di chuyen huong 5 (`5`) | (7, 2) | (6, 2) | Di chuyen den (6, 2); huong toi Spot #3 (brand=3, pos=21) | 20 |
| 4-5 | Di chuyen huong 5 (`5`) | (6, 2) | (5, 2) | Di chuyen den (5, 2); huong toi Spot #3 (brand=3, pos=21) | 20 |
| 6 | Di chuyen huong 5 (`5`) | (5, 2) | (4, 2) | Di chuyen den (4, 2); huong toi Spot #3 (brand=3, pos=21) | 20 |
| 7-8 | Di chuyen huong 5 (`5`) | (4, 2) | (3, 2) | Di chuyen den (3, 2); huong toi Spot #3 (brand=3, pos=21) | 20 |
| 9 | Di chuyen huong 5 (`5`) | (3, 2) | (2, 2) | Di chuyen den (2, 2); huong toi Spot #3 (brand=3, pos=21) | 20 |
| 10 | Di chuyen huong 5 (`5`) | (2, 2) | (1, 2) | Dat muc tieu Spot #3 (brand=3, pos=21) | 20 |
| 11-39 | Cho 29 step (`-29`) | (1, 2) | (1, 2) | Dung yen tai (1, 2) | 20 |

