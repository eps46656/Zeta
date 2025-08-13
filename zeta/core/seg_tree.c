

#include <zeta/core/debugger.h>
#include <zeta/core/seg_tree.h>

#if ZETA_Core_IsDebug

#define CheckST_(st) Zeta_Core_SegTree_Check((st))

#else

#define CheckST_(st)

#endif

size_t Zeta_Core_SegTree_GetDataSize(size_t n) {
    ZETA_Core_DebugAssert(n <= ZETA_Core_max_capacity);

    /*
        ret = sum(
            floor(n / 2**i)
            for i in [0, inf)
        )
    */

    return n * 2 - __builtin_popcountll(n);
}

void Zeta_Core_SegTree_Init(void* st_) {
    Zeta_Core_SegTree* st = st_;
    CheckST_(st);

    size_t data_size = Zeta_Core_SegTree_GetDataSize(st->n);

    for (size_t i = 0; i < data_size; ++i) { st->data[i] = 0; }
}

size_t Zeta_Core_SegTree_Query(void* st_, size_t lb, size_t rb) {
    Zeta_Core_SegTree* st = st_;
    CheckST_(st);

    ZETA_Core_DebugAssert(lb <= rb);
    ZETA_Core_DebugAssert(rb <= st->n);

    size_t ret = 0;

    size_t* cur_data = st->data;

    for (size_t cur_n = st->n; lb < rb;) {
        if (lb % 2 == 1) { ret += cur_data[lb]; }
        if (rb % 2 == 1) { ret += cur_data[rb - 1]; }

        lb = (lb + 1) / 2;
        rb = rb / 2;

        cur_data += cur_n;

        cur_n = cur_n / 2;
    }

    return ret;
}

size_t Zeta_Core_SegTree_Set(void* st_, size_t idx, size_t val) {
    Zeta_Core_SegTree* st = st_;
    CheckST_(st);

    size_t n = st->n;
    ZETA_Core_DebugAssert(idx < n);

    size_t* data = st->data;

    size_t old_val = data[idx];
    size_t del = val - old_val;

    if (del == 0) { return old_val; }

    size_t cur_n = n;
    size_t* cur_data = data;

    while (idx < cur_n) {
        cur_data[idx] += del;

        idx /= 2;

        cur_data += cur_n;
        cur_n /= 2;
    }

    return old_val;
}

size_t Zeta_Core_SegTree_Del(void* st_, size_t idx, size_t del) {
    Zeta_Core_SegTree* st = st_;
    CheckST_(st);

    size_t n = st->n;
    ZETA_Core_DebugAssert(idx < n);

    size_t* data = st->data;

    size_t old_val = data[idx];

    if (del == 0) { return old_val; }

    size_t cur_n = n;
    size_t* cur_data = data;

    while (idx < cur_n) {
        cur_data[idx] += del;

        idx /= 2;

        cur_data += cur_n;
        cur_n /= 2;
    }

    return old_val;
}

size_t Zeta_Core_SegTree_Find(void* st_, size_t offset,
                              size_t* dst_res_offset) {
    Zeta_Core_SegTree* st = st_;
    CheckST_(st);

    size_t n = st->n;

    if (n == 0) {
        if (dst_res_offset != NULL) { *dst_res_offset = offset; }
        return 0;
    }

    size_t* data = st->data;

    size_t* cur_data = data + (Zeta_Core_SegTree_GetDataSize(n) - 1);

    size_t i;
    size_t k = ZETA_Core_FloorLog2(n);

    for (; 0 < k; --k) {
        size_t cur_n = n >> k;
        size_t nxt_n = n >> (k - 1);

        size_t* nxt_data = cur_data - nxt_n;

        if (cur_n % 2 == 0) {
            cur_data = nxt_data;
            i = nxt_n;
            continue;
        }

        if (offset <= cur_data[cur_n - 1]) {
            i = cur_n - 1;
            break;
        }

        offset -= cur_data[cur_n - 1];
        cur_data = nxt_data;
        i = nxt_n;
    }

    for (; 0 < k; --k) {
        size_t nxt_n = n >> (k - 1);

        cur_data -= nxt_n;

        size_t l_i = i * 2;
        size_t r_i = i * 2 + 1;

        if (offset < cur_data[l_i]) {
            i = l_i;
        } else {
            offset -= cur_data[l_i];
            i = r_i;
        }
    }

    if (dst_res_offset != NULL) { *dst_res_offset = offset; }

    return i;
}

void Zeta_Core_SegTree_Check(void* st_) {
    Zeta_Core_SegTree* st = st_;
    ZETA_Core_DebugAssert(st != NULL);

    size_t n = st->n;
    ZETA_Core_DebugAssert(n <= ZETA_Core_max_capacity);

    if (n == 0) { return; }

    size_t* data = st->data;
    ZETA_Core_DebugAssert(data != NULL);
}

void Zeta_Core_SegTree_Sanitize(void* st_) {
    Zeta_Core_SegTree* st = st_;
    CheckST_(st);

    size_t n = st->n;
    size_t* data = st->data;

    if (n == 0) { return; }

    size_t* cur_data = data + (Zeta_Core_SegTree_GetDataSize(n) - 1);

    for (size_t k = ZETA_Core_FloorLog2(n); 0 < k; --k) {
        size_t cur_n = n >> k;
        size_t nxt_n = n >> (k - 1);

        size_t* nxt_data = cur_data - nxt_n;

        for (size_t i = 0; i < cur_n; ++i) {
            ZETA_Core_DebugAssert(cur_data[i] ==
                                  nxt_data[i * 2] + nxt_data[i * 2 + 1]);
        }

        cur_data = nxt_data;
    }
}
