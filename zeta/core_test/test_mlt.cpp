#include <zeta/core/multi_level_data_table.h>
#include <zeta/core/multi_level_ptr_table.h>
#include <zeta/core_test/pod_value.h>
#include <zeta/core_test/random.h>
#include <zeta/core_test/std_allocator.h>
#include <zeta/core_test/timer.h>

#include <map>

struct MultiLevelPtrTableMap {
    Zeta_CoreTest_StdAllocator nav_node_allocator;

    unsigned short branch_nums[ZETA_Core_MultiLevelTable_max_level];

    Zeta_Core_MultiLevelPtrTable mlpt;

    MultiLevelPtrTableMap() {
        ZETA_Core_PrintCurPos;

        int level{ 8 };

        this->branch_nums[0] = 5;
        this->branch_nums[1] = 6;
        this->branch_nums[2] = 7;
        this->branch_nums[3] = 8;
        this->branch_nums[4] = 9;
        this->branch_nums[5] = 10;
        this->branch_nums[6] = 11;
        this->branch_nums[7] = 12;

        this->mlpt.level = level;

        this->mlpt.branch_nums = this->branch_nums;

        this->mlpt.nav_node_allocator = {
            &Zeta_CoreTest_StdAllocator_allocator_vtable,
            &this->nav_node_allocator,
        };

        Zeta_Core_MultiLevelPtrTable_Init(&this->mlpt);

        ZETA_Core_PrintCurPos;

        this->Sanitize();
    }

    size_t GetIdx_(size_t* idxes) {
        int level = this->mlpt.level;

        size_t idx = 0;

        for (int level_i{ level - 1 }; 0 <= level_i; --level_i) {
            idx = idx * this->mlpt.branch_nums[level_i] + idxes[level_i];
        }

        return idx;
    }

    void SetIdxes_(size_t idx, size_t* dst_idxes) {
        int level = this->mlpt.level;

        for (int level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idx % this->mlpt.branch_nums[level_i];
            idx /= this->mlpt.branch_nums[level_i];
        }
    }

    void Sanitize() {
        Zeta_Core_MemRecorder* nav_node_mem_recorder =
            Zeta_Core_MemRecorder_Create();

        Zeta_Core_MultiLevelPtrTable_Sanitize(&this->mlpt,
                                              nav_node_mem_recorder);

        Zeta_Core_MemCheck_MatchRecords(this->nav_node_allocator.mem_recorder,
                                        nav_node_mem_recorder);

        Zeta_Core_MemRecorder_Destroy(nav_node_mem_recorder);
    }

    size_t GetCapacity() {
        return Zeta_Core_MultiLevelPtrTable_GetCapacity(&this->mlpt);
    }

    void** Access(size_t idx) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];

        this->SetIdxes_(idx, idxes);

        void* n = Zeta_Core_MultiLevelPtrTable_Access(&this->mlpt, idxes);

        this->Sanitize();

        return n == NULL ? NULL : (void**)n;
    }

    void Insert(size_t idx, void* val) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];

        this->SetIdxes_(idx, idxes);

        void* n = Zeta_Core_MultiLevelPtrTable_Insert(&this->mlpt, idxes).addr;

        ZETA_Core_DebugAssert(n != NULL);

        *(void**)n = val;

        this->Sanitize();
    }

    void Erase(size_t idx) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];

        this->SetIdxes_(idx, idxes);

        Zeta_Core_MultiLevelPtrTable_Erase(&this->mlpt, idxes);
    }

    size_t FindPrev(size_t idx) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];
        SetIdxes_(idx, idxes);

        void* n =
            Zeta_Core_MultiLevelPtrTable_FindPrev(&this->mlpt, idxes, TRUE);

        this->Sanitize();

        return n == NULL ? -1 : GetIdx_(idxes);
    }

    size_t FindNext(size_t idx) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];
        SetIdxes_(idx, idxes);

        void* n =
            Zeta_Core_MultiLevelPtrTable_FindNext(&this->mlpt, idxes, TRUE);

        this->Sanitize();

        return n == NULL ? -1 : GetIdx_(idxes);
    }

    std::vector<std::pair<size_t, void*>> Dump() {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];
        SetIdxes_(0, idxes);

        std::vector<std::pair<size_t, void*>> ret;

        void* n =
            Zeta_Core_MultiLevelPtrTable_FindNext(&this->mlpt, idxes, TRUE);

        while (n != NULL) {
            ret.push_back({ GetIdx_(idxes), *(void**)n });

            n = Zeta_Core_MultiLevelPtrTable_FindNext(&this->mlpt, idxes,
                                                      FALSE);
        }

        return ret;
    }
};

template <typename T>
struct MultiLevelDataTableMap {
    Zeta_CoreTest_StdAllocator nav_node_allocator;
    Zeta_CoreTest_StdAllocator dat_node_allocator;

    unsigned short branch_nums[ZETA_Core_MultiLevelTable_max_level];

    Zeta_Core_MultiLevelDataTable mldt;

    MultiLevelDataTableMap() {
        int level{ 8 };

        this->branch_nums[0] = 5;
        this->branch_nums[1] = 6;
        this->branch_nums[2] = 7;
        this->branch_nums[3] = 8;
        this->branch_nums[4] = 9;
        this->branch_nums[5] = 10;
        this->branch_nums[6] = 11;
        this->branch_nums[7] = 12;

        this->mldt.level = level;

        this->mldt.branch_nums = this->branch_nums;

        this->mldt.stride = sizeof(T);

        this->mldt.nav_node_allocator = {
            &Zeta_CoreTest_StdAllocator_allocator_vtable,
            &this->nav_node_allocator,
        };

        this->mldt.dat_node_allocator = {
            &Zeta_CoreTest_StdAllocator_allocator_vtable,
            &this->dat_node_allocator,
        };

        Zeta_Core_MultiLevelDataTable_Init(&this->mldt);

        this->Sanitize();
    }

    size_t GetIdx_(size_t* idxes) {
        int level = this->mldt.level;

        size_t idx = 0;

        for (int level_i{ level - 1 }; 0 <= level_i; --level_i) {
            idx = idx * this->mldt.branch_nums[level_i] + idxes[level_i];
        }

        return idx;
    }

    void SetIdxes_(size_t idx, size_t* dst_idxes) {
        int level = this->mldt.level;

        for (int level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idx % this->mldt.branch_nums[level_i];
            idx /= this->mldt.branch_nums[level_i];
        }
    }

    void Sanitize() {
        Zeta_Core_MemRecorder* nav_node_mem_recorder =
            Zeta_Core_MemRecorder_Create();

        Zeta_Core_MemRecorder* dat_node_mem_recorder =
            Zeta_Core_MemRecorder_Create();

        Zeta_Core_MultiLevelDataTable_Sanitize(
            &this->mldt, nav_node_mem_recorder, dat_node_mem_recorder);

        Zeta_Core_MemCheck_MatchRecords(this->nav_node_allocator.mem_recorder,
                                        nav_node_mem_recorder);

        Zeta_Core_MemCheck_MatchRecords(this->dat_node_allocator.mem_recorder,
                                        dat_node_mem_recorder);

        Zeta_Core_MemRecorder_Destroy(nav_node_mem_recorder);
        Zeta_Core_MemRecorder_Destroy(dat_node_mem_recorder);
    }

    size_t GetCapacity() {
        return Zeta_Core_MultiLevelDataTable_GetCapacity(&this->mldt);
    }

    T* Access(size_t idx) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];

        this->SetIdxes_(idx, idxes);

        void* n = Zeta_Core_MultiLevelDataTable_Access(&this->mldt, idxes);

        this->Sanitize();

        return n == NULL ? NULL : (T*)n;
    }

    void Insert(size_t idx, const T& val) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];

        this->SetIdxes_(idx, idxes);

        void* n = Zeta_Core_MultiLevelDataTable_Insert(&this->mldt, idxes).addr;

        ZETA_Core_DebugAssert(n != NULL);

        *(T*)n = val;

        this->Sanitize();
    }

    void Erase(size_t idx) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];

        this->SetIdxes_(idx, idxes);

        Zeta_Core_MultiLevelDataTable_Erase(&this->mldt, idxes);

        this->Sanitize();
    }

    size_t FindPrev(size_t idx) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];
        SetIdxes_(idx, idxes);

        void* n =
            Zeta_Core_MultiLevelDataTable_FindPrev(&this->mldt, idxes, TRUE);

        this->Sanitize();

        return n == NULL ? -1 : GetIdx_(idxes);
    }

    size_t FindNext(size_t idx) {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];
        SetIdxes_(idx, idxes);

        void* n =
            Zeta_Core_MultiLevelDataTable_FindNext(&this->mldt, idxes, TRUE);

        this->Sanitize();

        return n == NULL ? -1 : GetIdx_(idxes);
    }

    std::vector<std::pair<size_t, T>> Dump() {
        size_t idxes[ZETA_Core_MultiLevelTable_max_level];
        SetIdxes_(0, idxes);

        std::vector<std::pair<size_t, T>> ret;

        void* n =
            Zeta_Core_MultiLevelDataTable_FindNext(&this->mldt, idxes, TRUE);

        while (n != NULL) {
            ret.push_back({ GetIdx_(idxes), *(T*)n });

            n = Zeta_Core_MultiLevelDataTable_FindNext(&this->mldt, idxes,
                                                       FALSE);
        }

        return ret;
    }
};

template <typename T>
struct StdMap {
    std::map<size_t, T> m;

    size_t GetSize() { return this->m.size(); }

    T* Access(size_t idx) {
        auto iter{ this->m.find(idx) };
        return iter == m.end() ? NULL : &iter->second;
    }

    void Insert(size_t idx, const T& val) { this->m[idx] = val; }

    void Erase(size_t idx) {
        auto iter{ this->m.find(idx) };
        if (iter != this->m.end()) { this->m.erase(iter); }
    }

    size_t FindPrev(size_t idx) {
        auto iter{ this->m.upper_bound(idx) };
        return iter == this->m.begin() ? -1 : std::prev(iter)->first;
    }

    size_t FindNext(size_t idx) {
        auto iter{ this->m.lower_bound(idx) };
        return iter == this->m.end() ? -1 : iter->first;
    }

    std::vector<std::pair<size_t, T>> Dump() {
        std::vector<std::pair<size_t, T>> ret;
        for (auto p : this->m) { ret.push_back(p); }
        return ret;
    }

    void Sanitize() {}
};

template <typename T, typename CntrA, typename CntrB>
void SyncAccess(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, capacity - 1) };

    T* addr_a{ static_cast<T*>(cntr_a.Access(idx)) };
    T* addr_b{ static_cast<T*>(cntr_b.Access(idx)) };

    if (addr_a == NULL) {
        ZETA_Core_DebugAssert(addr_b == NULL);
    } else {
        ZETA_Core_DebugAssert(*addr_a == *addr_b);
    }
}

template <typename T, typename CntrA, typename CntrB>
void SyncInsert(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, capacity - 1) };

    T val{ Zeta_CoreTest_GetRandom<T>() };

    cntr_a.Insert(idx, val);
    T* addr_a{ static_cast<T*>(cntr_a.Access(idx)) };
    ZETA_Core_DebugAssert(addr_a != NULL && *addr_a == val);

    cntr_b.Insert(idx, val);
    T* addr_b{ static_cast<T*>(cntr_b.Access(idx)) };
    ZETA_Core_DebugAssert(addr_b != NULL && *addr_b == val);
}

template <typename CntrA, typename CntrB>
void SyncErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, capacity - 1) };

    cntr_a.Erase(idx);
    cntr_b.Erase(idx);
}

template <typename CntrA, typename CntrB>
void SyncFindPrevThenErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, capacity - 1) };

    size_t prv_idx_a{ cntr_a.FindPrev(idx) };
    size_t prv_idx_b{ cntr_b.FindPrev(idx) };

    if (prv_idx_a != prv_idx_b) {
        ZETA_Core_PrintVar(prv_idx_a);
        ZETA_Core_PrintVar(prv_idx_b);
    }

    ZETA_Core_DebugAssert(prv_idx_a == prv_idx_b);

    if (prv_idx_a == (size_t)(-1)) { return; }

    cntr_a.Erase(prv_idx_a);
    cntr_b.Erase(prv_idx_b);

    cntr_a.Sanitize();
    cntr_b.Sanitize();
}

template <typename CntrA, typename CntrB>
void SyncFindNextThenErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, capacity - 1) };

    size_t nxt_idx_a{ cntr_a.FindNext(idx) };
    size_t nxt_idx_b{ cntr_b.FindNext(idx) };

    ZETA_Core_DebugAssert(nxt_idx_a == nxt_idx_b);

    if (nxt_idx_a == (size_t)(-1)) { return; }

    cntr_a.Erase(nxt_idx_a);
    cntr_b.Erase(nxt_idx_b);
}

template <typename CntrA, typename CntrB>
void SyncFindPrev(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, capacity - 1) };

    size_t prv_idx_a{ cntr_a.FindNext(idx) };
    size_t prv_idx_b{ cntr_b.FindNext(idx) };

    ZETA_Core_DebugAssert(prv_idx_a == prv_idx_b);

    ZETA_Core_DebugAssert(cntr_a.Access(prv_idx_a) == cntr_b.Access(prv_idx_b));
}

template <typename CntrA, typename CntrB>
void SyncFindNext(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ Zeta_CoreTest_GetRandomInt<size_t, size_t>(0, capacity - 1) };

    size_t nxt_idx_a{ cntr_a.FindNext(idx) };
    size_t nxt_idx_b{ cntr_b.FindNext(idx) };

    ZETA_Core_DebugAssert(nxt_idx_a == nxt_idx_b);

    ZETA_Core_DebugAssert(cntr_a.Access(nxt_idx_a) == cntr_b.Access(nxt_idx_b));
}

template <typename CntrA, typename CntrB>
void SyncCompare(CntrA& cntr_a, CntrB& cntr_b) {
    ZETA_Core_DebugAssert(cntr_a.Dump() == cntr_b.Dump());
}

#define FOR_LOOP_(tmp_end, var, beg, end) \
    for (auto var = (beg), tmp_end = (end); var != tmp_end; ++var)

#define FOR_LOOP(var, beg, end) FOR_LOOP_(ZETA_Core_TmpName, var, (beg), (end))

void main1() {
    unsigned random_seed = time(NULL);
    unsigned fixed_seed = 1729615114;

    unsigned seed = random_seed;
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    Zeta_CoreTest_SetRandomSeed(seed);

    using T = Zeta_CoreTest_PODValue;

    // MultiLevelPtrTableMap zeta_map;
    MultiLevelDataTableMap<T> zeta_map;
    StdMap<T> std_map;

    ZETA_Core_PrintCurPos;

    FOR_LOOP(insert_i, 0, 1024) { SyncInsert<T>(zeta_map, std_map); }

    ZETA_Core_PrintCurPos;

    FOR_LOOP(test_i, 0, 16) {
        ZETA_Core_PrintVar(test_i);

        ZETA_Core_PrintCurPos;

        FOR_LOOP(test_j, 0, 1024) { SyncInsert<T>(zeta_map, std_map); }

        ZETA_Core_PrintCurPos;

        FOR_LOOP(test_j, 0, 1024) { SyncErase(zeta_map, std_map); }

        ZETA_Core_PrintCurPos;

        FOR_LOOP(test_j, 0, 1024) { SyncFindPrevThenErase(zeta_map, std_map); }

        ZETA_Core_PrintCurPos;

        FOR_LOOP(test_j, 0, 1024) { SyncFindNextThenErase(zeta_map, std_map); }

        ZETA_Core_PrintCurPos;

        SyncCompare(zeta_map, std_map);

        ZETA_Core_PrintCurPos;
    }
}

int main() {
    unsigned long long beg_time{ GetTime() };
    ZETA_Core_PrintVar(beg_time);

    main1();

    ZETA_Core_PrintVar(beg_time);

    unsigned long long end_time{ GetTime() };
    ZETA_Core_PrintVar(end_time);

    unsigned long long duration{ end_time - beg_time };
    ZETA_Core_PrintVar(duration);

    ZETA_Core_PrintVar("ok");

    return 0;
}
