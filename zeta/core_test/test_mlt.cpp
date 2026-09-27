#include <vector>
#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/recording_allocator.ipp>
#include <zeta/core/debug_utils/sanity.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lin_seq_endpoint.ipp>
#include <zeta/core/multi_level_data_table.ipp>
#include <zeta/core/multi_level_ptr_table.ipp>
#include <zeta/core_test/pod_value.hpp>
#include <zeta/core_test/random.hpp>
#include <zeta/core_test/std_allocator.hpp>
#include <zeta/core_test/timer.hpp>

namespace MLPT = zeta::core::multi_level_ptr_table;
namespace MLDT = zeta::core::multi_level_data_table;

struct MultiLevelPtrTableMap {
    MLPT::BranchNum branch_nums[MLPT::max_level];

    MLPT::Cntr<unsigned short,
               zeta::core::debug_utils::recording_allocator::Allocator<
                   zeta::core_test::std_allocator::Allocator>>
        mlpt;

    using K =
        zeta::core::static_seq::MakeLinearStaticIntegralSeq<size_t, 0, 1, 0>;

    static void SanityCheck(
        void const* self_,
        zeta::core::debug_utils::sanity::SanityCheckScope scope) {
        auto& self{ *static_cast<MultiLevelPtrTableMap const*>(self_) };

        if (scope ==
            zeta::core::debug_utils::sanity::SanityCheckScope::Complete) {
            zeta::core::debug_utils::sanity::SanityCheck(&self.mlpt, scope);
        }
    }

    MultiLevelPtrTableMap(std::string const& name)
        : branch_nums{
              5, 6, 7, 8, 9, 10, 11, 12,
          },
          mlpt{
              8,
              this->branch_nums,
              ZETA_Core_Lifecycle_PackConstructArgs(
                  std::make_shared<zeta::core_test::std_allocator::Allocator>(),
                  zeta::core::debug_utils::memory::default_mem_recorder_server
                      .MakeMemRecorderClient(name + "/mlpt.node_alctr")),
          } {
        ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

        zeta::core::debug_utils::sanity::RegisterSanityCheckFunc(
            static_cast<void const*>(this), SanityCheck);
    }

    constexpr ~MultiLevelPtrTableMap() {
        zeta::core::debug_utils::sanity::UnregisterSanityCheckFunc(
            static_cast<void const*>(this));
    }

    template <typename Idx>
    size_t GetIdx_(Idx* idxes) {
        using UnsignedIdx = zeta::core::integral::MakeUnsignedOf<Idx>;

        int level{ static_cast<int>(this->mlpt.level) };

        size_t idx{ 0 };

        for (int level_i{ level - 1 }; 0 <= level_i; --level_i) {
            idx = idx * this->mlpt.branch_nums[level_i] +
                  static_cast<UnsignedIdx>(idxes[level_i]);
        }

        return idx;
    }

    void SetIdxes_(size_t idx, MLPT::BranchNum* dst_idxes) const {
        unsigned level{ this->mlpt.level };

        for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idx % this->mlpt.branch_nums[level_i];

            idx /= this->mlpt.branch_nums[level_i];
        }
    }

    size_t GetCapacity() { return this->mlpt.GetMaxElemCnt(); }

    void** Access(size_t idx) {
        MLPT::BranchNum idxes[MLPT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n{ this->mlpt.Access(
            zeta::core::lin_seq_endpoint::provider::Provider{
                .data = idxes + (this->mlpt.level - 1),
                .elem_size = sizeof(MLPT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                .elem_cnt = MLPT::max_level,
            }) };

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);

        return n == nullptr ? nullptr : static_cast<void**>(n);
    }

    void Insert(size_t idx, void* val) {
        MLPT::BranchNum idxes[MLPT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n{ this->mlpt
                     .Insert(zeta::core::lin_seq_endpoint::provider::Provider{
                         .data = idxes + (this->mlpt.level - 1),
                         .elem_size = sizeof(MLPT::BranchNum),
                         .elem_stride =
                             -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                         .elem_cnt = MLPT::max_level,
                     })
                     .first };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(n != nullptr);

        *static_cast<void**>(n) = val;

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);
    }

    void Erase(size_t idx) {
        MLPT::BranchNum idxes[MLPT::max_level];

        this->SetIdxes_(idx, idxes);

        this->mlpt.Erase(zeta::core::lin_seq_endpoint::provider::Provider{
            .data = idxes + (this->mlpt.level - 1),
            .elem_size = sizeof(MLPT::BranchNum),
            .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
            .elem_cnt = MLPT::max_level,
        });

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);
    }

    size_t FindPrev(size_t idx) {
        MLPT::BranchNum idxes[MLPT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n{ this->mlpt.FindPrevIncl(
            zeta::core::lin_seq_endpoint::provider::Provider{
                .data = idxes + (this->mlpt.level - 1),
                .elem_size = sizeof(MLPT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                .elem_cnt = MLPT::max_level,
            },
            zeta::core::lin_seq_endpoint::acceptor::Acceptor{
                .data = idxes + (this->mlpt.level - 1),
                .elem_size = sizeof(MLPT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                .elem_cnt = MLPT::max_level,
            }) };

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);

        return n == nullptr ? static_cast<size_t>(-1) : GetIdx_(idxes);
    }

    size_t FindNext(size_t idx) {
        MLPT::BranchNum idxes[MLPT::max_level];
        SetIdxes_(idx, idxes);

        void* n{ this->mlpt.FindNextIncl(
            zeta::core::lin_seq_endpoint::provider::Provider{
                .data = idxes + (this->mlpt.level - 1),
                .elem_size = sizeof(MLPT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                .elem_cnt = MLPT::max_level,
            },
            zeta::core::lin_seq_endpoint::acceptor::Acceptor{
                .data = idxes + (this->mlpt.level - 1),
                .elem_size = sizeof(MLPT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                .elem_cnt = MLPT::max_level,
            }) };

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);

        return n == nullptr ? static_cast<size_t>(-1) : GetIdx_(idxes);
    }

    std::vector<std::pair<size_t, void*>> Dump() {
        MLPT::BranchNum idxes[MLPT::max_level];
        SetIdxes_(0, idxes);

        std::vector<std::pair<size_t, void*>> ret;

        void* n{ this->mlpt.FindNextIncl(
            zeta::core::lin_seq_endpoint::provider::Provider{
                .data = idxes + (this->mlpt.level - 1),
                .elem_size = sizeof(MLPT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                .elem_cnt = MLPT::max_level,
            },
            zeta::core::lin_seq_endpoint::acceptor::Acceptor{
                .data = idxes + (this->mlpt.level - 1),
                .elem_size = sizeof(MLPT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                .elem_cnt = MLPT::max_level,
            }) };

        while (n != nullptr) {
            size_t k{ GetIdx_(idxes) };

            ZETA_Core_DebugUtils_Diag_LogVar(k);

            if (!ret.empty()) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(ret.back().first < k);
            }

            ret.emplace_back(k, *static_cast<void**>(n));

            n = this->mlpt.FindNextExcl(
                zeta::core::lin_seq_endpoint::provider::Provider{
                    .data = idxes + (this->mlpt.level - 1),
                    .elem_size = sizeof(MLPT::BranchNum),
                    .elem_stride =
                        -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                    .elem_cnt = MLPT::max_level,
                },
                zeta::core::lin_seq_endpoint::acceptor::Acceptor{
                    .data = idxes + (this->mlpt.level - 1),
                    .elem_size = sizeof(MLPT::BranchNum),
                    .elem_stride =
                        -static_cast<ptrdiff_t>(sizeof(MLPT::BranchNum)),
                    .elem_cnt = MLPT::max_level,
                });
        }

        return ret;
    }
};

template <typename T>
struct MultiLevelDataTableMap {
    MLDT::BranchNum branch_nums[MLDT::max_level];

    MLDT::Cntr<unsigned _BitInt(128),
               zeta::core::debug_utils::recording_allocator::Allocator<
                   zeta::core_test::std_allocator::Allocator>,
               zeta::core::debug_utils::recording_allocator::Allocator<
                   zeta::core_test::std_allocator::Allocator>>
        mldt;

    static constexpr void SanityCheck(
        void const* self_,
        zeta::core::debug_utils::sanity::SanityCheckScope scope) {
        MultiLevelDataTableMap const& self{
            *static_cast<MultiLevelDataTableMap const*>(self_)
        };

        zeta::core::debug_utils::sanity::SanityCheck(&self.mldt, scope);
    }

    constexpr MultiLevelDataTableMap(std::string const& name)
        : branch_nums{
              5, 6, 7, 8, 9, 10, 11, 12,
          },
          mldt{
              8,
              this->branch_nums,
              sizeof(T),
              ZETA_Core_Lifecycle_PackConstructArgs(
                  std::make_shared<zeta::core_test::std_allocator::Allocator>(),
                  zeta::core::debug_utils::memory::default_mem_recorder_server
                      .MakeMemRecorderClient(name + "/mldt.node_alctr")),
              ZETA_Core_Lifecycle_PackConstructArgs(
                  std::make_shared<zeta::core_test::std_allocator::Allocator>(),
                  zeta::core::debug_utils::memory::default_mem_recorder_server
                      .MakeMemRecorderClient(name + "/mldt.seg_alctr")),
          } {
        zeta::core::debug_utils::sanity::RegisterSanityCheckFunc(
            static_cast<void const*>(this), SanityCheck);
    }

    constexpr ~MultiLevelDataTableMap() {
        zeta::core::debug_utils::sanity::UnregisterSanityCheckFunc(
            static_cast<void const*>(this));
    }

    template <typename Idx>
    constexpr size_t GetIdx_(Idx* idxes) {
        using UnsignedIdx = zeta::core::integral::MakeUnsignedOf<Idx>;

        int level{ static_cast<int>(this->mldt.level) };

        size_t idx{ 0 };

        for (int level_i{ level - 1 }; 0 <= level_i; --level_i) {
            idx = idx * this->mldt.branch_nums[level_i] +
                  static_cast<UnsignedIdx>(idxes[level_i]);
        }

        return idx;
    }

    constexpr void SetIdxes_(size_t idx, MLDT::BranchNum* dst_idxes) const {
        int level{ static_cast<int>(this->mldt.level) };

        for (int level_i{ 0 }; level_i < level; ++level_i) {
            dst_idxes[level_i] = idx % this->mldt.branch_nums[level_i];

            idx /= this->mldt.branch_nums[level_i];
        }
    }

    constexpr size_t GetCapacity() { return this->mldt.GetMaxElemCnt(); }

    constexpr T* Access(size_t idx) {
        MLDT::BranchNum idxes[MLDT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n =
            this->mldt.Access(zeta::core::lin_seq_endpoint::provider::Provider{
                .data = idxes + (this->mldt.level - 1),
                .elem_size = sizeof(MLDT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                .elem_cnt = MLDT::max_level,
            });

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);

        return n == nullptr ? nullptr : static_cast<T*>(n);
    }

    constexpr void Insert(size_t idx, T const& val) {
        MLDT::BranchNum idxes[MLDT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n{ this->mldt
                     .Insert(zeta::core::lin_seq_endpoint::provider::Provider{
                         .data = idxes + (this->mldt.level - 1),
                         .elem_size = sizeof(MLDT::BranchNum),
                         .elem_stride =
                             -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                         .elem_cnt = MLDT::max_level,
                     })
                     .first };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(n != nullptr);

        *static_cast<T*>(n) = val;

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);
    }

    constexpr void Erase(size_t idx) {
        MLDT::BranchNum idxes[MLDT::max_level];

        this->SetIdxes_(idx, idxes);

        this->mldt.Erase(zeta::core::lin_seq_endpoint::provider::Provider{
            .data = idxes + (this->mldt.level - 1),
            .elem_size = sizeof(MLDT::BranchNum),
            .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
            .elem_cnt = MLDT::max_level,
        });

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);
    }

    constexpr size_t FindPrev(size_t idx) {
        MLDT::BranchNum idxes[MLDT::max_level];

        this->SetIdxes_(idx, idxes);

        void* n{ this->mldt.FindPrevIncl(
            zeta::core::lin_seq_endpoint::provider::Provider{
                .data = idxes + (this->mldt.level - 1),
                .elem_size = sizeof(MLDT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                .elem_cnt = MLDT::max_level,
            },
            zeta::core::lin_seq_endpoint::acceptor::Acceptor{
                .data = idxes + (this->mldt.level - 1),
                .elem_size = sizeof(MLDT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                .elem_cnt = MLDT::max_level,
            }) };

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);

        return n == nullptr ? static_cast<size_t>(-1) : GetIdx_(idxes);
    }

    constexpr size_t FindNext(size_t idx) {
        MLDT::BranchNum idxes[MLDT::max_level];
        this->SetIdxes_(idx, idxes);

        void* n{ this->mldt.FindNextIncl(
            zeta::core::lin_seq_endpoint::provider::Provider{
                .data = idxes + (this->mldt.level - 1),
                .elem_size = sizeof(MLDT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                .elem_cnt = MLDT::max_level,
            },
            zeta::core::lin_seq_endpoint::acceptor::Acceptor{
                .data = idxes + (this->mldt.level - 1),
                .elem_size = sizeof(MLDT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                .elem_cnt = MLDT::max_level,
            }) };

        zeta::core::debug_utils::sanity::SanityCheck(
            this, zeta::core::debug_utils::sanity::SanityCheckScope::Complete);

        return n == nullptr ? static_cast<size_t>(-1) : GetIdx_(idxes);
    }

    constexpr std::vector<std::pair<size_t, T>> Dump() {
        MLDT::BranchNum idxes[MLDT::max_level];

        this->SetIdxes_(0, idxes);

        std::vector<std::pair<size_t, T>> ret;

        void* n{ this->mldt.FindNextIncl(
            zeta::core::lin_seq_endpoint::provider::Provider{
                .data = idxes + (this->mldt.level - 1),
                .elem_size = sizeof(MLDT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                .elem_cnt = MLDT::max_level,
            },
            zeta::core::lin_seq_endpoint::acceptor::Acceptor{
                .data = idxes + (this->mldt.level - 1),
                .elem_size = sizeof(MLDT::BranchNum),
                .elem_stride = -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                .elem_cnt = MLDT::max_level,
            }) };

        while (n != nullptr) {
            size_t k{ GetIdx_(idxes) };

            if (!ret.empty()) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(ret.back().first < k);
            }

            ret.push_back({ k, *static_cast<T*>(n) });

            n = this->mldt.FindNextExcl(
                zeta::core::lin_seq_endpoint::provider::Provider{
                    .data = idxes + (this->mldt.level - 1),
                    .elem_size = sizeof(MLDT::BranchNum),
                    .elem_stride =
                        -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                    .elem_cnt = MLDT::max_level,
                },
                zeta::core::lin_seq_endpoint::acceptor::Acceptor{
                    .data = idxes + (this->mldt.level - 1),
                    .elem_size = sizeof(MLDT::BranchNum),
                    .elem_stride =
                        -static_cast<ptrdiff_t>(sizeof(MLDT::BranchNum)),
                    .elem_cnt = MLDT::max_level,
                });
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

    size_t idx{ zeta::core_test::GenUniformRandomInt<size_t>(0, capacity - 1) };

    T* addr_a{ static_cast<T*>(cntr_a.Access(idx)) };
    T* addr_b{ static_cast<T*>(cntr_b.Access(idx)) };

    if (addr_a == nullptr) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(addr_b == nullptr);
    } else {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(*addr_a == *addr_b);
    }
}

template <typename T, typename CntrA, typename CntrB>
void SyncInsert(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GenUniformRandomInt<size_t>(0, capacity - 1) };

    T val{ zeta::core_test::GetRandom<T>() };

    cntr_a.Insert(idx, val);
    T* addr_a{ static_cast<T*>(cntr_a.Access(idx)) };
    ZETA_Core_DebugUtils_Diag_PromiseAssert(addr_a != nullptr &&
                                            *addr_a == val);

    cntr_b.Insert(idx, val);
    T* addr_b{ static_cast<T*>(cntr_b.Access(idx)) };
    ZETA_Core_DebugUtils_Diag_PromiseAssert(addr_b != nullptr &&
                                            *addr_b == val);
}

template <typename CntrA, typename CntrB>
void SyncErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GenUniformRandomInt<size_t>(0, capacity - 1) };

    cntr_a.Erase(idx);
    cntr_b.Erase(idx);
}

template <typename CntrA, typename CntrB>
void SyncFindPrevThenErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GenUniformRandomInt<size_t>(0, capacity - 1) };

    size_t prv_idx_a{ cntr_a.FindPrev(idx) };
    size_t prv_idx_b{ cntr_b.FindPrev(idx) };

    if (prv_idx_a != prv_idx_b) {
        ZETA_Core_DebugUtils_Logging_ImmLogVar(prv_idx_a);
        ZETA_Core_DebugUtils_Logging_ImmLogVar(prv_idx_b);
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(prv_idx_a == prv_idx_b);

    if (prv_idx_a == static_cast<size_t>(-1)) { return; }

    cntr_a.Erase(prv_idx_a);
    cntr_b.Erase(prv_idx_b);
}

template <typename CntrA, typename CntrB>
void SyncFindNextThenErase(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GenUniformRandomInt<size_t>(0, capacity - 1) };

    size_t nxt_idx_a{ cntr_a.FindNext(idx) };
    size_t nxt_idx_b{ cntr_b.FindNext(idx) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(nxt_idx_a == nxt_idx_b);

    if (nxt_idx_a == static_cast<size_t>(-1)) { return; }

    cntr_a.Erase(nxt_idx_a);
    cntr_b.Erase(nxt_idx_b);
}

template <typename CntrA, typename CntrB>
void SyncFindPrev(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GenUniformRandomInt<size_t>(0, capacity - 1) };

    size_t prv_idx_a{ cntr_a.FindPrev(idx) };
    size_t prv_idx_b{ cntr_b.FindPrev(idx) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(prv_idx_a == prv_idx_b);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cntr_a.Access(prv_idx_a) ==
                                            cntr_b.Access(prv_idx_b));
}

template <typename CntrA, typename CntrB>
void SyncFindNext(CntrA& cntr_a, CntrB& cntr_b) {
    size_t capacity{ cntr_a.GetCapacity() };

    size_t idx{ zeta::core_test::GenUniformRandomInt<size_t>(0, capacity - 1) };

    size_t nxt_idx_a{ cntr_a.FindNext(idx) };
    size_t nxt_idx_b{ cntr_b.FindNext(idx) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(nxt_idx_a == nxt_idx_b);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cntr_a.Access(nxt_idx_a) ==
                                            cntr_b.Access(nxt_idx_b));
}

template <typename CntrA, typename CntrB>
void SyncCompare(CntrA& cntr_a, CntrB& cntr_b) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cntr_a.Dump() == cntr_b.Dump());
}

#define FOR_LOOP_(tmp_end, var, beg, end) \
    for (auto var{ beg }, tmp_end{ end }; var != tmp_end; ++var)

#define FOR_LOOP(var, beg, end) FOR_LOOP_(ZETA_Core_TmpName, var, (beg), (end))

inline void main1() {
    unsigned random_seed{ static_cast<unsigned>(time(nullptr)) };
    unsigned fixed_seed{ 1790008707 };

    // unsigned seed{ random_seed };
    unsigned seed{ fixed_seed };

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(random_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(fixed_seed);
    ZETA_Core_DebugUtils_Logging_ImmLogVar(seed);

    zeta::core_test::SetRandomSeed(seed);

    // using T = void*;
    using T = zeta::core_test::PODValue;

    // MultiLevelPtrTableMap zeta_map;
    MultiLevelDataTableMap<T> zeta_map{ "zeta_map" };
    StdMap<T> std_map;

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    FOR_LOOP(insert_i, 0, 128) {
        ZETA_Core_DebugUtils_Logging_ImmLogVar(insert_i);
        SyncInsert<T>(zeta_map, std_map);
    }

    ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

    FOR_LOOP(test_i, 0, 16) {
        ZETA_Core_DebugUtils_Logging_ImmLogVar(test_i);

        ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

        FOR_LOOP(test_j, 0, 128) { SyncInsert<T>(zeta_map, std_map); }

        ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

        FOR_LOOP(test_j, 0, 128) { SyncErase(zeta_map, std_map); }

        ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

        FOR_LOOP(test_j, 0, 128) { SyncFindPrevThenErase(zeta_map, std_map); }

        ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

        FOR_LOOP(test_j, 0, 128) {
            SyncFindNextThenErase(zeta_map, std_map);
            SyncCompare(zeta_map, std_map);
        }

        ZETA_Core_DebugUtils_Logging_ImmLogCurPos();

        SyncCompare(zeta_map, std_map);

        ZETA_Core_DebugUtils_Logging_ImmLogCurPos();
    }
}

template <typename Integral>
constexpr unsigned long long FindPrevBit_Base(Integral num,
                                              unsigned long long pos) {
    for (;; --pos) {
        if (num & (static_cast<Integral>(1)
                   << static_cast<unsigned long long>(pos))) {
            return pos;
        }

        if (pos == 0) {
            return zeta::core::integral::RangeMaxOf<unsigned long long>;
        }
    }
}

inline void main2() {
    for (int _{ 0 }; _ < 1'000'000; ++_) {
        unsigned long long x{
            zeta::core_test::GenUniformRandomInt<unsigned long long>(
                0, zeta::core::integral::RangeMaxOf<unsigned long long>)
        };

        unsigned long long pos{
            zeta::core_test::GenUniformRandomInt<unsigned long long>(
                0, zeta::core::integral::WidthOf<unsigned long long> - 1)
        };

        unsigned long long a{ FindPrevBit_Base(x, pos) };
        unsigned long long b{ zeta::core::integral_bit::FindPrevBit(x, pos) };

        if (a != b) {
            ZETA_Core_DebugUtils_Logging_ImmLogVar(x);
            ZETA_Core_DebugUtils_Logging_ImmLogVar(pos);
            ZETA_Core_DebugUtils_Logging_ImmLogVar(a);
            ZETA_Core_DebugUtils_Logging_ImmLogVar(b);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(a == b);
        }
    }
}

inline void main3() {
    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<1>>());

    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<7>>());

    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<8>>());

    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<31>>());

    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<32>>());

    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<63>>());

    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<64>>());

    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<127>>());

    ZETA_Core_DebugUtils_Logging_ImmLogVar(
        zeta::core::debug_utils::logging::GetTypeStr<
            zeta::core::integral_utils::UnsignedFastIntegral<128>>());
}

int main() {
    unsigned long long beg_time{ zeta::core_test::GetTime() };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(beg_time);

    main1();
    // main2();
    // main3();

    ZETA_Core_DebugUtils_Logging_ImmLogVar(beg_time);

    unsigned long long end_time{ zeta::core_test::GetTime() };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(end_time);

    unsigned long long duration{ end_time - beg_time };
    ZETA_Core_DebugUtils_Logging_ImmLogVar(duration);

    ZETA_Core_DebugUtils_Logging_ImmLogVar("ok");

    return 0;
}
