#include <vector>
#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/multi_level_data_table.ipp>
#include <zeta/core/multi_level_ptr_table.ipp>
#include <zeta/core_test/pod_value.hpp>
#include <zeta/core_test/random.hpp>
#include <zeta/core_test/std_allocator.hpp>
#include <zeta/core_test/timer.hpp>

namespace MLPT = zeta::core::multi_level_ptr_table;
namespace MLDT = zeta::core::multi_level_data_table;

struct MultiLevelPtrTableMap {
    unsigned short branch_nums[MLPT::max_level];

    zeta::core_test::std_allocator::Allocator nav_node_alctr;

    MLPT::Cntr<zeta::core_test::std_allocator::Allocator> mlpt;

    MultiLevelPtrTableMap() {
        ZETA_Core_PrintCurPos;

        unsigned level{ 8 };

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

        new (&this->nav_node_alctr) decltype(this->nav_node_alctr);

        MLPT::Init(&this->mlpt);

        ZETA_Core_PrintCurPos;

        this->Sanitize();
    }

    size_t GetIdx_(size_t* idxes) {
        int level{ static_cast<int>(this->mlpt.level) };

        size_t idx{ 0 };

        for (int level_i{ level - 1 }; 0 <= level_i; --level_i) {
            idx = idx * this->mlpt.branch_nums[level_i] + idxes[level_i];
        }

        return idx;
    }

    void SetIdxes_(size_t idx, size_t* dst_idxes) const {
        int level{ static_cast<int>(this->mlpt.level) };

        for (int level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idx % this->mlpt.branch_nums[level_i];
            idx /= this->mlpt.branch_nums[level_i];
        }
    }

    void Sanitize() {
        zeta::core::mem_recorder::MemRecorder nav_node_mem_recorder;

        MLPT::Sanitize(&this->mlpt, &nav_node_mem_recorder);

        zeta::core::mem_recorder::MatchRecords(
            &this->mlpt.nav_node_alctr.mem_recorder, &nav_node_mem_recorder);
    }

    size_t GetCapacity() { return MLPT::GetCapacity(&this->mlpt); }

    void** Access(size_t idx) {
        size_t idxes[MLPT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n{ MLPT::Access(&this->mlpt, idxes) };

        this->Sanitize();

        return n == nullptr ? nullptr : static_cast<void**>(n);
    }

    void Insert(size_t idx, void* val) {
        size_t idxes[MLPT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n{ MLPT::Insert(&this->mlpt, idxes).first };

        ZETA_Core_DebugAssert(n != nullptr);

        *static_cast<void**>(n) = val;

        this->Sanitize();
    }

    void Erase(size_t idx) {
        size_t idxes[MLPT::max_level];

        this->SetIdxes_(idx, idxes);

        MLPT::Erase(&this->mlpt, idxes);
    }

    size_t FindPrev(size_t idx) {
        size_t idxes[MLPT::max_level];
        SetIdxes_(idx, idxes);

        void* n = MLPT::FindPrev(&this->mlpt, idxes, true);

        this->Sanitize();

        return n == nullptr ? static_cast<size_t>(-1) : GetIdx_(idxes);
    }

    size_t FindNext(size_t idx) {
        size_t idxes[MLPT::max_level];
        SetIdxes_(idx, idxes);

        void* n = MLPT::FindNext(&this->mlpt, idxes, true);

        this->Sanitize();

        return n == nullptr ? static_cast<size_t>(-1) : GetIdx_(idxes);
    }

    std::vector<std::pair<size_t, void*>> Dump() {
        size_t idxes[MLPT::max_level];
        SetIdxes_(0, idxes);

        std::vector<std::pair<size_t, void*>> ret;

        void* n{ MLPT::FindNext(&this->mlpt, idxes, true) };

        while (n != nullptr) {
            ret.emplace_back(GetIdx_(idxes), *static_cast<void**>(n));

            n = MLPT::FindNext(&this->mlpt, idxes, false);
        }

        return ret;
    }
};

template <typename T>
struct MultiLevelDataTableMap {
    zeta::core_test::std_allocator::Allocator nav_node_alctr;
    zeta::core_test::std_allocator::Allocator dat_node_alctr;

    unsigned short branch_nums[MLDT::max_level];

    MLDT::Cntr<zeta::core_test::std_allocator::Allocator,
               zeta::core_test::std_allocator::Allocator>
        mldt;

    MultiLevelDataTableMap() {
        unsigned level{ 8 };

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

        new (&this->mldt.nav_node_alctr) decltype(this->mldt.nav_node_alctr);
        new (&this->mldt.data_node_alctr) decltype(this->mldt.data_node_alctr);

        MLDT::Init(&this->mldt);

        this->Sanitize();
    }

    size_t GetIdx_(size_t const* idxes) const {
        int level{ static_cast<int>(this->mldt.level) };

        size_t idx = 0;

        for (int level_i{ level - 1 }; 0 <= level_i; --level_i) {
            idx = idx * this->mldt.branch_nums[level_i] + idxes[level_i];
        }

        return idx;
    }

    void SetIdxes_(size_t idx, size_t* dst_idxes) {
        int level{ static_cast<int>(this->mldt.level) };

        for (int level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idx % this->mldt.branch_nums[level_i];
            idx /= this->mldt.branch_nums[level_i];
        }
    }

    void Sanitize() {
        zeta::core::mem_recorder::MemRecorder nav_node_mem_recorder;

        zeta::core::mem_recorder::MemRecorder dat_node_mem_recorder;

        MLDT::Sanitize(&this->mldt, nav_node_mem_recorder,
                       &dat_node_mem_recorder);

        zeta::core::mem_recorder::MatchRecords(
            &this->nav_node_alctr.mem_recorder, &nav_node_mem_recorder);

        zeta::core::mem_recorder::MatchRecords(
            &this->dat_node_alctr.mem_recorder, &dat_node_mem_recorder);
    }

    size_t GetCapacity() { return MLDT::GetCapacity(&this->mldt); }

    T* Access(size_t idx) {
        size_t idxes[MLDT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n = MLDT::Access(&this->mldt, idxes);

        this->Sanitize();

        return n == nullptr ? nullptr : static_cast<T*>(n);
    }

    void Insert(size_t idx, T const& val) {
        size_t idxes[MLDT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n = MLDT::Insert(&this->mldt, idxes).first;

        ZETA_Core_DebugAssert(n != nullptr);

        *static_cast<T*>(n) = val;

        this->Sanitize();
    }

    void Erase(size_t idx) {
        size_t idxes[MLDT::max_level];

        this->SetIdxes_(idx, idxes);

        MLDT::Erase(&this->mldt, idxes);

        this->Sanitize();
    }

    size_t FindPrev(size_t idx) {
        size_t idxes[MLDT::max_level];
        SetIdxes_(idx, idxes);

        void* n = MLDT::FindPrev(&this->mldt, idxes, true);

        this->Sanitize();

        return n == nullptr ? static_cast<size_t>(-1) : GetIdx_(idxes);
    }

    size_t FindNext(size_t idx) {
        size_t idxes[MLDT::max_level];
        SetIdxes_(idx, idxes);

        void* n = MLDT::FindNext(&this->mldt, idxes, true);

        this->Sanitize();

        return n == nullptr ? static_cast<size_t>(-1) : GetIdx_(idxes);
    }

    std::vector<std::pair<size_t, T>> Dump() {
        size_t idxes[MLDT::max_level];
        SetIdxes_(0, idxes);

        std::vector<std::pair<size_t, T>> ret;

        void* n = MLDT::FindNext(&this->mldt, idxes, true);

        while (n != nullptr) {
            ret.push_back({ GetIdx_(idxes), *static_cast<T*>(n) });

            n = MLDT::FindNext(&this->mldt, idxes, false);
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
        return iter == m.end() ? nullptr : &iter->second;
    }

    void Insert(size_t idx, T const& val) { this->m[idx] = val; }

    void Erase(size_t idx) {
        auto iter{ this->m.find(idx) };
        if (iter != this->m.end()) { this->m.erase(iter); }
    }

    size_t FindPrev(size_t idx) {
        auto iter{ this->m.upper_bound(idx) };
        return iter == this->m.begin() ? static_cast<size_t>(-1)
                                       : std::prev(iter)->first;
    }

    size_t FindNext(size_t idx) {
        auto iter{ this->m.lower_bound(idx) };
        return iter == this->m.end() ? static_cast<size_t>(-1) : iter->first;
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

    size_t idx{ zeta::core_test::GetRandomInt<size_t>(0, capacity - 1) };

    T* addr_a{ static_cast<T*>(cntr_a.Access(idx)) };
    T* addr_b{ static_cast<T*>(cntr_b.Access(idx)) };

    if (addr_a == nullptr) {
        ZETA_Core_DebugAssert(addr_b == nullptr);
    } else {
        ZETA_Core_DebugAssert(*addr_a == *addr_b);
    }
}

template <typename T, typename CntrA, typename CntrB>
void SyncInsert(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GetRandomInt<size_t>(0, capacity - 1) };

    T val{ zeta::core_test::GetRandom<T>() };

    cntr_a.Insert(idx, val);
    T* addr_a{ static_cast<T*>(cntr_a.Access(idx)) };
    ZETA_Core_DebugAssert(addr_a != nullptr && *addr_a == val);

    cntr_b.Insert(idx, val);
    T* addr_b{ static_cast<T*>(cntr_b.Access(idx)) };
    ZETA_Core_DebugAssert(addr_b != nullptr && *addr_b == val);
}

template <typename CntrA, typename CntrB>
void SyncErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GetRandomInt<size_t>(0, capacity - 1) };

    cntr_a.Erase(idx);
    cntr_b.Erase(idx);
}

template <typename CntrA, typename CntrB>
void SyncFindPrevThenErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GetRandomInt<size_t>(0, capacity - 1) };

    size_t prv_idx_a{ cntr_a.FindPrev(idx) };
    size_t prv_idx_b{ cntr_b.FindPrev(idx) };

    if (prv_idx_a != prv_idx_b) {
        ZETA_Core_PrintVar(prv_idx_a);
        ZETA_Core_PrintVar(prv_idx_b);
    }

    ZETA_Core_DebugAssert(prv_idx_a == prv_idx_b);

    if (prv_idx_a == static_cast<size_t>(-1)) { return; }

    cntr_a.Erase(prv_idx_a);
    cntr_b.Erase(prv_idx_b);

    cntr_a.Sanitize();
    cntr_b.Sanitize();
}

template <typename CntrA, typename CntrB>
void SyncFindNextThenErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GetRandomInt<size_t>(0, capacity - 1) };

    size_t nxt_idx_a{ cntr_a.FindNext(idx) };
    size_t nxt_idx_b{ cntr_b.FindNext(idx) };

    ZETA_Core_DebugAssert(nxt_idx_a == nxt_idx_b);

    if (nxt_idx_a == static_cast<size_t>(-1)) { return; }

    cntr_a.Erase(nxt_idx_a);
    cntr_b.Erase(nxt_idx_b);
}

template <typename CntrA, typename CntrB>
void SyncFindPrev(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GetRandomInt<size_t>(0, capacity - 1) };

    size_t prv_idx_a{ cntr_a.FindNext(idx) };
    size_t prv_idx_b{ cntr_b.FindNext(idx) };

    ZETA_Core_DebugAssert(prv_idx_a == prv_idx_b);

    ZETA_Core_DebugAssert(cntr_a.Access(prv_idx_a) == cntr_b.Access(prv_idx_b));
}

template <typename CntrA, typename CntrB>
void SyncFindNext(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GetRandomInt<size_t>(0, capacity - 1) };

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
    for (auto var{ beg }, tmp_end{ end }; var != tmp_end; ++var)

#define FOR_LOOP(var, beg, end) FOR_LOOP_(ZETA_Core_TmpName, var, (beg), (end))

inline void main1() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1729615114 };

    unsigned seed{ random_seed };
    // unsigned seed = fixed_seed;

    ZETA_Core_PrintCurPos;

    ZETA_Core_PrintVar(random_seed);
    ZETA_Core_PrintVar(fixed_seed);
    ZETA_Core_PrintVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    using T = void*;
    // using T = zeta::core_test::PODValue;

    MultiLevelPtrTableMap zeta_map;
    // MultiLevelDataTableMap<T> zeta_map;
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
    unsigned long long beg_time{ zeta::core_test::GetTime() };
    ZETA_Core_PrintVar(beg_time);

    main1();

    ZETA_Core_PrintVar(beg_time);

    unsigned long long end_time{ zeta::core_test::GetTime() };
    ZETA_Core_PrintVar(end_time);

    unsigned long long duration{ end_time - beg_time };
    ZETA_Core_PrintVar(duration);

    ZETA_Core_PrintVar("ok");

    return 0;
}
