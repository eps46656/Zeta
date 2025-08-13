

import dataclasses
import math
import random

import beartype
import numpy as np


@dataclasses.dataclass
class Seg:
    is_ref: bool
    beg: int
    size: int


def compress_segs(segs: list[Seg]):
    ret: list[Seg] = list()

    for seg in segs:
        if len(ret) == 0 or not seg.is_ref:
            ret.append(seg)
            continue

        last_seg = ret[-1]

        if not last_seg.is_ref:
            ret.append(seg)
            continue

        last_ref_end = last_seg.beg + last_seg.size

        assert last_ref_end <= seg.beg

        if last_ref_end == seg.beg:
            last_seg.size += seg.size

    return ret


def get_rand_segs(origin_size: int, max_dat_seg_size: int):
    # [1, origin_size]

    pivots = random.sample(range(1, origin_size),
                           round(math.sqrt(origin_size) * 2))

    pivots.append(0)
    pivots.append(origin_size)

    pivots.sort()

    N = len(pivots)

    segs = list()

    segs.append(Seg(False, 0, random.randint(1, max_dat_seg_size)))

    for i in range(1, N):
        segs.append(Seg(True, pivots[i-1], pivots[i] - pivots[i-1]))
        segs.append(Seg(False, 0, random.randint(1, max_dat_seg_size)))

    N = len(segs)

    idxes = random.sample(range(N), round(N * 0.75))
    idxes.sort()

    ret = [segs[idx] for idx in idxes]

    return compress_segs(ret)


def check_segs(origin_size: int, segs: list[Seg]):
    assert 0 <= origin_size

    prv_is_ref = False
    prv_ref_end = -1

    for seg in segs:
        if not seg.is_ref:
            prv_is_ref = False
            continue

        assert 0 <= seg.beg
        assert 0 < seg.size
        assert seg.beg + seg.size <= origin_size

        assert prv_ref_end <= seg.beg

        if prv_is_ref:
            assert prv_ref_end < seg.beg

        prv_is_ref = True
        prv_ref_end = seg.beg + seg.size


@beartype.beartype
def solve3(arr: list[int],
           brr: list[int],
           cost_coeff_inc: int,
           cost_coeff_dec: int):
    N = len(arr)

    assert 1 <= N
    assert len(brr) == N - 1

    dp = np.zeros((N + 1, N + 1), dtype=int)

    for l in range(1, N + 1):
        for i in range(N - l + 1):
            end = i + l

            arr_sum = sum(arr[i:end])
            brr_sum = sum(brr[i:end-1])

            ans = 0

            if arr_sum < 0:
                ans = cost_coeff_inc * -arr_sum + brr_sum
            else:
                ans = cost_coeff_dec * arr_sum + brr_sum

            for j in range(i+1, end):
                ans = min(ans, dp[i][j] + dp[j][end])

            dp[i][end] = ans

    return dp[0][N]


'''

g(i, j)
= sum(brr[i:j-1]) + f(sum(arr[i:j])))


dp[0] = 0

dp[i] = min(
    dp[j] + g(j, i)     for j in [0, i-1)
)

'''


@beartype.beartype
def solve2(arr: list[int],
           brr: list[int],
           cost_coeff_inc: int,
           cost_coeff_dec: int,
           max_scan_size: int):
    def f(x):
        return -x * cost_coeff_inc if x < 0 else x * cost_coeff_dec

    N = len(arr)

    assert 1 <= N
    assert len(brr) == N - 1

    acc_arr = np.zeros((N + 1,), dtype=int)
    acc_brr = np.zeros((N,), dtype=int)

    for i in range(1, N+1):
        acc_arr[i] = acc_arr[i-1] + arr[i-1]

    for i in range(1, N):
        acc_brr[i] = acc_brr[i-1] + brr[i-1]

    dp = np.zeros((N + 1,), dtype=int)

    for i in range(1, N + 1):
        ans = 2**32

        for j in range(max(0, i-max_scan_size), i):
            ans = min(ans, dp[j] + acc_brr[i-1] -
                      acc_brr[j] + f(acc_arr[i] - acc_arr[j]))

        dp[i] = ans

    return int(dp[N])


@beartype.beartype
def solve1(arr: list[int],
           brr: list[int],
           cost_coeff_inc: int,
           cost_coeff_dec: int):
    def f(x):
        return -x * cost_coeff_inc if x < 0 else x * cost_coeff_dec

    N = len(arr)

    assert 1 <= N
    assert len(brr) == N-1

    dp_a = [0 for i in range(N + 1)]
    dp_b = [0 for i in range(N + 1)]
    crr = [0 for i in range(N)]

    INT_MAX = 2**64

    dp_a[1] = f(arr[0])
    dp_b[1] = 0
    crr[0] = arr[0]

    for i in range(2, N + 1):
        cur_a = INT_MAX

        dp_a[i] = min(
            dp_a[i-1] + f(arr[i-1]),
            dp_b[i-1] + f(crr[i-1] + arr[i-1]),
            dp_b[i-1] + f(crr[i-2]) + f(arr[i-1]))

        cur_b = INT_MAX
        cur_crr = 0


@beartype.beartype
def cal_best_wb_cost(
        origin_size: int,
        segs: list[Seg],
        cost_coeff_insert: int,
        cost_coeff_erase: int,
        cost_coeff_read: int,
        cost_coeff_write: int,
        max_scan_size: int):

    check_segs(origin_size, segs)

    stage_size = 0

    origin_arr: list[int] = list()
    stage_arr: list[int] = list()
    brr: list[int] = list()

    prv_ref_end = 0
    acc_data_size = 0

    for seg in segs:
        stage_size += seg.size

        if not seg.is_ref:
            acc_data_size += seg.size
            continue

        origin_arr.append(seg.beg - prv_ref_end)
        stage_arr.append(acc_data_size)
        brr.append((cost_coeff_read + cost_coeff_write) * seg.size)

        prv_ref_end = seg.beg + seg.size
        acc_data_size = 0

    origin_arr.append(origin_size - prv_ref_end)
    stage_arr.append(acc_data_size)

    arr: list[int] = [a - b for a, b in zip(origin_arr, stage_arr)]

    return solve2(arr, brr, cost_coeff_insert, cost_coeff_erase, max_scan_size) + cost_coeff_write * acc_data_size


def main1():
    cost_coeff_insert = 10
    cost_coeff_erase = 10
    cost_coeff_read = 1
    cost_coeff_write = 2

    segs = [
        Seg(False, 0, 10),
        Seg(True, 0, 1024),
        Seg(False, 0, 512),
        Seg(True, 1024, 2048),
        Seg(False, 0, 128),
    ]

    cost = cal_best_wb_cost(
        1024 + 2048,
        segs,
        cost_coeff_insert,
        cost_coeff_erase,
        cost_coeff_read,
        cost_coeff_write,
    )

    print(f"{cost=}")


def main2():
    cost_coeff_read = 1
    cost_coeff_write = 2
    cost_coeff_insert = 10
    cost_coeff_erase = 10

    origin_size = 1024 * 16
    max_dat_seg_size = 7

    segs = get_rand_segs(origin_size, max_dat_seg_size)

    best_cost = cal_best_wb_cost(
        origin_size,
        segs,
        cost_coeff_read,
        cost_coeff_write,
        cost_coeff_insert,
        cost_coeff_erase,
        origin_size,
    )

    print(f"{best_cost=}")

    better_cost = cal_best_wb_cost(
        origin_size,
        segs,
        cost_coeff_read,
        cost_coeff_write,
        cost_coeff_insert,
        cost_coeff_erase,
        8,
    )

    print(f"{better_cost=}")

    assert best_cost == better_cost


if __name__ == "__main__":
    for _ in range(128):
        main2()
