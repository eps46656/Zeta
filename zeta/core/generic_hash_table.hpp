#pragma once

#include <zeta/core/array.hpp>
#include <zeta/core/array.ipp>
#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/fixed_point.hpp>
#include <zeta/core/fixed_point.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/multi_level_ptr_table.hpp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList(suffix)                                  \
    typename HasherLike##suffix, typename ComparatorLike##suffix, \
        typename SaltRandomEngineLike##suffix,                    \
        typename TableNodeAllocatorLike##suffix

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList \
    HasherLike, ComparatorLike, SaltRandomEngineLike, TableNodeAllocatorLike

namespace zeta::core::generic_hash_table {

using ActiveMap = unsigned long long;

constexpr auto max_level{ multi_level_ptr_table::max_level };

constexpr multi_level_ptr_table::BranchNum branch_num{
    comparison_utils::BasicMin(integral::WidthOf<ActiveMap>,
                               multi_level_ptr_table::max_branch_num)
};

constexpr array::Array<multi_level_ptr_table::BranchNum, max_level> branch_nums{
    [] {
        array::Array<multi_level_ptr_table::BranchNum, max_level> arr;
        for (unsigned i{ 0 }; i < max_level; ++i) { arr[i] = branch_num; }
        return arr;
    }()
};

using UFP = fixed_point::FixedPoint<meta::AutoValueWrapper<false>,
                                    meta::ValueWrapper<size_t, 16>,
                                    meta::ValueWrapper<size_t, 16>>;

constexpr size_t min_bucket_size{ 37 };

#if 32 <= ZETA_Core_size_width
constexpr size_t max_bucket_size{ 1'000'000'007 };
#else
#error "Unsupported architecture."
#endif

constexpr size_t min_move_quata_per_op{ 2 };
constexpr size_t max_move_quata_per_op{ integral::RangeMaxOf<size_t> };

constexpr UFP min_center_load_ratio{ UFP::FromFraction(50U, 100U) };  // 0.5
constexpr UFP max_center_load_ratio{ UFP::FromIntegral(32U) };

constexpr UFP min_drift_ratio{ UFP::FromFraction(150U, 100U) };  // 1.5
constexpr UFP max_drift_ratio{ UFP::FromIntegral(16U) };         // 16

using TreeNode =
    basic_bin_tree_node::Node<void*,                          // LinkType
                              meta::AutoValueWrapper<true>,   // PColorTag
                              meta::AutoValueWrapper<false>,  // LColorTag
                              meta::AutoValueWrapper<false>,  // RColorTag
                              meta::AutoValueWrapper<false>,  // AccSizeTag
                              basic_bin_tree_node::PrimaryColorTagEnum::P>;

}  // namespace zeta::core::generic_hash_table

namespace zeta::core::generic_hash_table {

struct RehashingConfig {
    unsigned move_quata_per_op;
    UFP center_load_ratio;
    UFP drift_ratio;
};

/*

1 < move quata per op
1 < drift ratio

cur good elem cnt = [
    cur bucket size * center load ratio / drift ratio,
    cur bucket size * center load ratio * drift ratio,
]

move quata per op / (move quata per op - 1) < drift ratio
    => move quata per op / (move quata per op - 1) < drift ratio
    => 1 + 1 / (move quata per op - 1) < drift ratio
    => 1 / (move quata per op - 1) < drift ratio - 1
    => 1 / (drift ratio - 1) < move quata per op - 1

*/

RehashingConfig constexpr default_rehashing_config{
    .move_quata_per_op = 4,
    .center_load_ratio = UFP::FromIntegral(8U),
    .drift_ratio = UFP::FromIntegral(4U),
};

struct Node {
    ZETA_Core_DebugStructPadding;

    TreeNode tn;

    unsigned long long hash_code;

    ZETA_Core_DebugStructPadding;

    constexpr void Init();
};

template <CntrTplParamList(_)>
struct Cntr {
    using HasherLike = HasherLike_;
    using ComparatorLike = ComparatorLike_;
    using SaltRandomEngineLike = SaltRandomEngineLike_;
    using TableNodeAllocatorLike = TableNodeAllocatorLike_;

    unsigned long long cur_salt;
    unsigned long long nxt_salt;

    void* cur_table_root;
    void* nxt_table_root;

    size_t cur_table_node_cnt;
    size_t nxt_table_node_cnt;

    size_t cur_bucket_size;
    size_t nxt_bucket_size;

    size_t node_cnt;

    RehashingConfig rehashing_config;

    HasherLike hasher;

    ComparatorLike cmptr;

    SaltRandomEngineLike salt_random_engine;

    TableNodeAllocatorLike table_node_alctr;

    template <typename NodeHashLikeInitArg, typename ComparatorInitArg,
              typename TableNodeAllocatorInitArg,
              typename SaltRandomEngineInitArg>
    constexpr void Init(this Cntr& ght, RehashingConfig const& rehashing_config,
                        NodeHashLikeInitArg&& hasher_init_arg,
                        ComparatorInitArg&& cmptr_init_arg,
                        SaltRandomEngineInitArg&& salt_random_engine_init_arg,
                        TableNodeAllocatorInitArg&& table_node_alctr_init_arg);

    constexpr void Deinit(this Cntr& ght);

    constexpr size_t GetNodeCnt(this Cntr const& ght);

    constexpr bool Contain(this Cntr const& ght, Node const* node);

    template <typename Key, hash::CanHash<Key const*> KeyHasher,
              comparison::CanCompare<Key const*, Node const*> KeyElemComparator>
    constexpr Node* Find(this Cntr const& ght, Key const* key,
                         KeyHasher const& key_hasher,
                         KeyElemComparator const& key_elem_cmptr);

    template <typename Key, hash::CanHash<Key const*> KeyHasher,
              comparison::CanCompare<Key const*, Node const*> KeyElemComparator>
    constexpr void Insert(this Cntr& ght, Key const* key,
                          KeyHasher const& key_hasher,
                          KeyElemComparator const& key_elem_cmptr, Node* node);

    constexpr void Extract(this Cntr& ght, Node* node);

    constexpr Node* ExtractAny(this Cntr& ght);

    constexpr void ExtractAll(this Cntr& ght);

    constexpr bool RunPending(this Cntr& ght, size_t quata);

    constexpr auto GetEffFactor(this Cntr const& ght);

    constexpr void Sanitize(this Cntr const& ght,
                            mem_recorder::MemRecorder* dst_table,
                            mem_recorder::MemRecorder* dst_node);
};

}  // namespace zeta::core::generic_hash_table

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
