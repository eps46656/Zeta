#include <seg_tree.h>
#include <zeta/core/debugger.h>
#include <zeta/core_test/random.h>

#include <cstdlib>
#include <vector>

struct DebugTree {
    std::vector<size_t> data;

    DebugTree(size_t n) { this->data.resize(n, 0); }

    size_t GetN() const { return this->data.size(); }

    size_t Query(size_t lb, size_t rb) const {
        size_t ret{ 0 };
        for (; lb < rb; ++lb) { ret += this->data[lb]; }
        return ret;
    }

    size_t Set(size_t idx, size_t val) {
        size_t old_val{ this->data[idx] };
        this->data[idx] = val;
        return old_val;
    }

    size_t Del(size_t idx, size_t del) {
        size_t old_val{ this->data[idx] };
        this->data[idx] += del;
        return old_val;
    }

    size_t Find(size_t offset, size_t* dst_res_offset) {
        size_t i{ 0 };

        for (; i < this->data.size(); ++i) {
            size_t cur{ this->data[i] };
            if (offset < cur) { break; }
            offset -= cur;
        }

        if (dst_res_offset != nullptr) { *dst_res_offset = offset; }

        return i;
    }
};

#define M (256)

void SyncQuery(DebugTree* dt, Zeta_Core_SegTree* st) {
    size_t n{ dt->GetN() };

    ZETA_Core_DebugAssert(n == st->n);

    size_t a{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, n) };
    size_t b{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, n) };

    size_t lb{ std::min(a, b) };
    size_t rb{ std::max(a, b) };

    ZETA_Core_DebugAssert(dt->Query(lb, rb) ==
                          Zeta_Core_SegTree_Query(st, lb, rb));

    Zeta_Core_SegTree_Sanitize(st);
}

void SyncSet(DebugTree* dt, Zeta_Core_SegTree* st) {
    size_t n{ dt->GetN() };

    ZETA_Core_DebugAssert(n == st->n);

    if (n == 0) { return; }

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, n - 1) };
    size_t val{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, M) };

    ZETA_Core_DebugAssert(dt->Set(idx, val) ==
                          Zeta_Core_SegTree_Set(st, idx, val));

    Zeta_Core_SegTree_Sanitize(st);
}

void SyncDel(DebugTree* dt, Zeta_Core_SegTree* st) {
    size_t n{ dt->GetN() };

    ZETA_Core_DebugAssert(n == st->n);

    if (n == 0) { return; }

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, n - 1) };

    size_t del{ static_cast<size_t>(Zeta_CoreTest_GetRandomInt<size_t, size_t>(
        -dt->Query(idx, idx + 1), M)) };

    ZETA_Core_DebugAssert(dt->Del(idx, del) ==
                          Zeta_Core_SegTree_Del(st, idx, del));

    Zeta_Core_SegTree_Sanitize(st);
}

void SyncFind(DebugTree* dt, Zeta_Core_SegTree* st) {
    size_t n{ dt->GetN() };

    ZETA_Core_DebugAssert(n == st->n);

    if (n == 0) { return; }

    size_t offset{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, M * 2 * n) };

    size_t dt_res_offset;
    size_t dt_idx{ dt->Find(offset, &dt_res_offset) };

    size_t st_res_offset;
    size_t st_idx{ Zeta_Core_SegTree_Find(st, offset, &st_res_offset) };

    ZETA_Core_DebugAssert(dt_idx == st_idx);
    ZETA_Core_DebugAssert(dt_res_offset == st_res_offset);

    Zeta_Core_SegTree_Sanitize(st);
}

void main1() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1729615114;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    Zeta_CoreTest_SetRandomSeed(seed);

    size_t n = 127;

    Zeta_Core_SegTree st;

    st.n = n;
    st.data =
        (size_t*)malloc(sizeof(size_t) * Zeta_Core_SegTree_GetDataSize(n));

    Zeta_Core_SegTree_Init(&st);

    DebugTree dt{ n };

    for (int test_i{ 0 }; test_i < 16; ++test_i) {
        ZETA_Core_PrintCurPos;

        for (size_t _{ 0 }; _ < n * 2; ++_) {
            ZETA_Core_PrintCurPos;
            SyncSet(&dt, &st);
            ZETA_Core_PrintCurPos;
        }

        for (size_t _{ 0 }; _ < n * 2; ++_) {
            ZETA_Core_PrintCurPos;
            SyncDel(&dt, &st);
            ZETA_Core_PrintCurPos;
        }

        for (size_t _{ 0 }; _ < 16; ++_) {
            ZETA_Core_PrintCurPos;
            SyncQuery(&dt, &st);
            ZETA_Core_PrintCurPos;
        }

        for (size_t _{ 0 }; _ < 16; ++_) {
            ZETA_Core_PrintCurPos;
            SyncFind(&dt, &st);
            ZETA_Core_PrintCurPos;
        }
    }
}

int main() {
    printf("main start\n");
    main1();
    printf("ok\n");
    return 0;
}
