#pragma once

#include <zeta/core/allocator.ipp>
#include <zeta/core/array.hpp>
#include <zeta/core/basic_bin_tree_node.ipp>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/fixed_point.ipp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/hash.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/multi_level_ptr_table.hpp>
#include <zeta/core/multi_level_ptr_table.ipp>
#include <zeta/core/pair.hpp>
#include <zeta/core/percent_prime_table.hpp>
#include <zeta/core/ptr_utils.ipp>
#include <zeta/core/random.ipp>
#include <zeta/core/rbtree.hpp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                             \
    typename NodeHashLike, typename NodeCompareLike, \
        typename SaltRandomEngineLike, typename TableNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList \
    NodeHashLike, NodeCompareLike, SaltRandomEngineLike, TableNodeAllocatorLike

#pragma push_macro("MLPT_CNTR")
#define MLPT_CNTR                \
    multi_level_ptr_table::Cntr< \
        ActiveMap,               \
        decltype(meta::GetInstPtr(meta::Declval<TableNodeAllocatorLike&>()))>

#pragma push_macro("BuildCurMLPTTable")
#define BuildCurMLPTTable                                             \
    MLPT_CNTR cur_table{                                              \
        .level = static_cast<unsigned>(                               \
            integral_math::CeilLog(cur_bucket_size, branch_num)),     \
        .branch_nums = branch_nums.elems,                             \
        .elem_cnt = ght.cur_table_node_cnt,                           \
        .root = ght.cur_table_root,                                   \
        .nav_node_alctr = meta::GetInstPtr(                           \
            const_cast<Cntr<CntrTplArgList>&>(ght).table_node_alctr), \
    };                                                                \
    ZETA_Core_StaticAssert(true)

#pragma push_macro("BuildNxtMLPTTable")
#define BuildNxtMLPTTable                                               \
    MLPT_CNTR nxt_table{                                                \
        .level = static_cast<unsigned>(                                 \
            nxt_bucket_size == 0                                        \
                ? 1                                                     \
                : integral_math::CeilLog(nxt_bucket_size, branch_num)), \
        .branch_nums = branch_nums.elems,                               \
        .elem_cnt = ght.nxt_table_node_cnt,                             \
        .root = ght.nxt_table_root,                                     \
        .nav_node_alctr = meta::GetInstPtr(                             \
            const_cast<Cntr<CntrTplArgList>&>(ght).table_node_alctr),   \
    };                                                                  \
    ZETA_Core_StaticAssert(true)

namespace zeta::core {

constexpr void generic_hash_table::Node::Init() { this->tn.Init(); }

namespace generic_hash_table::detail {

constexpr size_t GetBucketIdx_  // NOLINT(misc-use-internal-linkage)
    (unsigned long long hash_code, size_t bucket_size) {
    return utils::SimpleUnsignedIntegralHash(hash_code, 0ULL) % bucket_size;
}

struct BranchIdxesSource_ {
    size_t bucket_idx;

    size_t operator()() {
        size_t ret{ this->bucket_idx % generic_hash_table::branch_num };
        this->bucket_idx /= generic_hash_table::branch_num;
        return ret;

        /*

        LSB              MSB
        level - 1   ->   0

        */
    }
};

constexpr size_t BranchIdxesToBucketIdx_  // NOLINT(misc-use-internal-linkage)
    (unsigned level, size_t const* branch_idxes) {
    size_t bucket_idx{ 0 };

    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        bucket_idx = bucket_idx * branch_num + branch_idxes[level_i];
    }

    return bucket_idx;
}

constexpr size_t FindPrvBucketSize_  // NOLINT(misc-use-internal-linkage)
    (size_t bucket_size) {
    bucket_size = comparison_utils::BasicMax(bucket_size, min_bucket_size);

    size_t len{ percent_prime_table::table_length };

    if (bucket_size < percent_prime_table::table[0]) { return min_bucket_size; }

    size_t lb{ 0 };
    size_t rb{ len - 1 };

    while (lb < rb) {
        size_t mb{ (lb + rb + 1) / 2 };

        if (percent_prime_table::table[mb] <= bucket_size) {
            lb = mb;
        } else {
            rb = mb - 1;
        }
    }

    return comparison_utils::BasicMax(min_bucket_size,
                                      percent_prime_table::table[lb]);
}

constexpr size_t FindNxtBucketSize_  // NOLINT(misc-use-internal-linkage)
    (size_t bucket_size) {
    bucket_size = comparison_utils::BasicMin(bucket_size, max_bucket_size);

    size_t len{ percent_prime_table::table_length };

    if (percent_prime_table::table[len - 1] < bucket_size) {
        return max_bucket_size;
    }

    size_t lb{ 0 };
    size_t rb{ len - 1 };

    while (lb < rb) {
        size_t mb{ (lb + rb) / 2 };

        if (bucket_size <= percent_prime_table::table[mb]) {
            rb = mb;
        } else {
            lb = mb + 1;
        }
    }

    return comparison_utils::BasicMin(percent_prime_table::table[lb],
                                      max_bucket_size);
}

template <typename TableNodeAllocatorLike, typename Key,
          hash::CanHash<Key const*> KeyHasher,
          comparison::CanCompare<Key const*, Node const*> KeyElemComparator>
Node* FindInTable_  // NOLINT(misc-use-internal-linkage)
    (unsigned long long salt,
     multi_level_ptr_table::Cntr<ActiveMap, TableNodeAllocatorLike>& table,
     size_t bucket_size, Key const* key, KeyHasher& key_hasher,
     KeyElemComparator& key_elem_cmptr) {
    unsigned long long key_hash_code{ hash::Hash(key_hasher, key, salt) };

    TreeNode* target_tn{ nullptr };

    void** tmp{ static_cast<void**>(table.Access(detail::BranchIdxesSource_{
        (GetBucketIdx_)(key_hash_code, bucket_size) })) };

    for (TreeNode* tn{
             ({ tmp == nullptr ? nullptr : static_cast<TreeNode*>(*tmp); }) };
         tn != nullptr;) {
        unsigned long long node_hash_code{
            ZETA_Core_MemberToStruct(Node, tn, tn)->hash_code
        };

        if (key_hash_code < node_hash_code) {
            tn = bin_tree::GetL(tn);
            continue;
        }

        if (node_hash_code < key_hash_code) {
            tn = bin_tree::GetR(tn);
            continue;
        }

        comparison::Ordering cmp{ comparison::Compare(
            key_elem_cmptr, meta::AutoValueWrapper<comparison::OpEnum::Order>{},
            key, ZETA_Core_MemberToStruct(Node, tn, tn)) };

        if (cmp == comparison::Ordering::Greater) {
            tn = bin_tree::GetR(tn);
            continue;
        }

        if (cmp == comparison::Ordering::Equal) { target_tn = tn; }
        tn = bin_tree::GetL(tn);
    }

    return target_tn == nullptr ? nullptr
                                : ZETA_Core_MemberToStruct(Node, tn, target_tn);
}

template <typename MultiLevelPtrTable, typename Key,
          hash::CanHash<Key const*> KeyHasher,
          comparison::CanCompare<Key const*, Node const*> KeyElemComparator>
void InsertToTable_(unsigned long long salt, MultiLevelPtrTable& table,
                    size_t bucket_size, Key const* key, KeyHasher& key_hasher,
                    KeyElemComparator& key_elem_cmptr, Node* n) {
    unsigned long long key_hash_code{ hash::Hash(key_hasher, key, salt) };

    n->hash_code = key_hash_code;

    auto p{ table.Insert(detail::BranchIdxesSource_{
        (GetBucketIdx_)(key_hash_code, bucket_size) }) };

    void** root_entry{ static_cast<void**>(p.first) };

    if (p.second) {
        *root_entry = &n->tn;
        return;
    }

    TreeNode* tn{ static_cast<TreeNode*>(*root_entry) };

    TreeNode* le_tn{ nullptr };
    TreeNode* gt_tn{ nullptr };

    while (tn != nullptr) {
        unsigned long long cur_hash_code{
            ZETA_Core_MemberToStruct(Node, tn, tn)->hash_code
        };

        if (key_hash_code < cur_hash_code) {
            gt_tn = tn;
            tn = bin_tree::GetL(tn);
            continue;
        }

        if (cur_hash_code < key_hash_code) {
            le_tn = tn;
            tn = bin_tree::GetR(tn);
            continue;
        }

        comparison::Ordering cmp{ comparison::Compare(
            key_elem_cmptr, meta::AutoValueWrapper<comparison::OpEnum::Order>{},
            n, ZETA_Core_MemberToStruct(Node, tn, tn)) };

        if (cmp == comparison::Ordering::Less) {
            gt_tn = tn;
            tn = bin_tree::GetL(tn);
        } else {
            le_tn = tn;
            tn = bin_tree::GetR(tn);
        }
    }

    *root_entry = rbtree::Insert(le_tn, gt_tn, &n->tn);
}

template <typename MultiLevelPtrTable, typename Hasher>
bool TryExtractFromTable_  // NOLINT(misc-use-internal-linkage)
    (unsigned long long salt, MultiLevelPtrTable& table, size_t bucket_size,
     Node* n, TreeNode* n_root, Hasher const& hasher) {
    if (bucket_size == 0) { return false; }

    size_t branch_idxes[max_level];

    struct {
        size_t* branch_idxes;
        size_t bucket_idx;

        auto operator()() {
            size_t ret{ this->bucket_idx % generic_hash_table::branch_num };
            *(--this->branch_idxes) = ret;
            this->bucket_idx /= generic_hash_table::branch_num;
            return ret;
        }
    } branch_idxes_src{
        .branch_idxes = branch_idxes + table.level,
        .bucket_idx =
            detail::GetBucketIdx_(hash::Hash(hasher, n, salt), bucket_size),
    };

    void** root_entry{ static_cast<void**>(table.Access(branch_idxes_src)) };

    if (root_entry == nullptr || *root_entry != n_root) { return false; }

    void* new_root{ rbtree::Extract(&n->tn) };

    *root_entry = new_root;

    if (new_root == nullptr) { table.Erase(branch_idxes); }

    return true;
}

template <CntrTplParamList>
void TryRunPending_(Cntr<CntrTplArgList> const& ght_, MLPT_CNTR& cur_table,
                    MLPT_CNTR& nxt_table, size_t quata) {
    auto& ght{ const_cast<Cntr<CntrTplArgList>&>(ght_) };

    auto& hasher{ meta::GetInstRef(ght.hasher) };
    auto& cmptr{ meta::GetInstRef(ght.cmptr) };

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    if (nxt_bucket_size == 0) {
        ght.cur_table_node_cnt = cur_table.elem_cnt;
        ght.cur_table_root = cur_table.root;

        auto center_capacity{ fixed_point::FromIntegral(cur_bucket_size) *
                              ght.rehashing_config.center_load_ratio };

        auto lb_capacity{
            (center_capacity / ght.rehashing_config.drift_ratio).Floor()
        };

        auto rb_capacity{
            (center_capacity * ght.rehashing_config.drift_ratio).Ceil()
        };

        size_t nxt_bucket_size{ cur_bucket_size };

        if (cur_table.elem_cnt < lb_capacity) {
            nxt_bucket_size = (FindPrvBucketSize_)(static_cast<size_t>(
                (fixed_point::FromIntegral(cur_bucket_size) /
                 ght.rehashing_config.drift_ratio)
                    .Floor()));
        } else if (rb_capacity < cur_table.elem_cnt) {
            nxt_bucket_size = (FindNxtBucketSize_)(static_cast<size_t>(
                (fixed_point::FromIntegral(cur_bucket_size) *
                 ght.rehashing_config.drift_ratio)
                    .Ceil()));
        }

        if (cur_bucket_size != nxt_bucket_size) {
            ght.nxt_bucket_size = nxt_bucket_size;
            ght.nxt_salt = random::GetRandomInt<unsigned long long>(
                meta::GetInstRef(ght.salt_random_engine));
        }

        return;
    }

    size_t idxes[max_level];

    void** root_entry{ nullptr };

    for (; 0 < quata && 0 < cur_table.elem_cnt; --quata) {
        if (root_entry == nullptr) {
            root_entry = static_cast<void**>(cur_table.FindFirst(idxes));
        }

        auto* trans_n{ ZETA_Core_MemberToStruct(Node, tn, *root_entry) };

        void* new_root{ rbtree::Extract(&trans_n->tn) };

        *root_entry = new_root;

        if (new_root == nullptr) {
            cur_table.Erase(idxes);
            root_entry = nullptr;
        }

        (InsertToTable_)(ght.nxt_salt, nxt_table, ght.nxt_bucket_size, trans_n,
                         hasher, cmptr, trans_n);
    }

    if (0 < cur_table.elem_cnt) {
        ght.cur_table_node_cnt = cur_table.elem_cnt;
        ght.cur_table_root = cur_table.root;

        ght.nxt_table_node_cnt = nxt_table.elem_cnt;
        ght.nxt_table_root = nxt_table.root;

        return;
    }

    if (nxt_table.elem_cnt == 0) {
        ght.cur_salt = random::GetRandomInt<unsigned long long>(
            meta::GetInstRef(ght.salt_random_engine));
        ght.cur_table_node_cnt = 0;
        ght.cur_bucket_size = min_bucket_size;
        ght.cur_table_root = nullptr;
    } else {
        ght.cur_salt = ght.nxt_salt;
        ght.cur_table_node_cnt = nxt_table.elem_cnt;
        ght.cur_bucket_size = nxt_bucket_size;
        ght.cur_table_root = nxt_table.root;
    }

    ght.nxt_table_node_cnt = 0;
    ght.nxt_bucket_size = 0;
    ght.nxt_table_root = nullptr;
}

constexpr void CheckRehasingConfig_(RehashingConfig const& rehashing_config) {
    ZETA_Core_DebugAssert(rehashing_config.center_load_ratio >=
                          min_center_load_ratio);
    ZETA_Core_DebugAssert(rehashing_config.center_load_ratio <=
                          max_center_load_ratio);
    ZETA_Core_DebugAssert(rehashing_config.drift_ratio >= min_drift_ratio);
    ZETA_Core_DebugAssert(rehashing_config.drift_ratio <= max_drift_ratio);
}

template <CntrTplParamList>
constexpr void CheckCntr_(Cntr<CntrTplArgList> const& ght) {
    size_t cur_bucket_size{ ght.cur_bucket_size };

    ZETA_Core_DebugAssert(cur_bucket_size != 0);
}

}  // namespace generic_hash_table::detail

template <CntrTplParamList>
template <typename NodeHashLikeInitArg, typename ComparatorInitArg,
          typename TableNodeAllocatorInitArg, typename SaltRandomEngineInitArg>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::Init(
    this Cntr& ght, RehashingConfig const& rehashing_config,
    NodeHashLikeInitArg&& hasher_init_arg, ComparatorInitArg&& cmptr_init_arg,
    SaltRandomEngineInitArg&& salt_random_engine_init_arg,
    TableNodeAllocatorInitArg&& table_node_alctr_init_arg) {
    lifecycle::Init(
        ght.table_node_alctr,
        meta::Forward<TableNodeAllocatorInitArg>(table_node_alctr_init_arg));

    lifecycle::Init(ght.hasher,
                    meta::Forward<NodeHashLikeInitArg>(hasher_init_arg));

    lifecycle::Init(ght.cmptr,
                    meta::Forward<ComparatorInitArg>(cmptr_init_arg));

    lifecycle::Init(
        ght.salt_random_engine,
        meta::Forward<SaltRandomEngineInitArg>(salt_random_engine_init_arg));
    random::CheckContract(meta::GetInstRef(ght.salt_random_engine));

    detail::CheckRehasingConfig_(rehashing_config);

    ght.rehashing_config = rehashing_config;

    ght.cur_salt = random::GetRandomInt<unsigned long long>(
        meta::GetInstRef(ght.salt_random_engine));

    ght.cur_table_root = nullptr;
    ght.nxt_table_root = nullptr;

    ght.cur_table_node_cnt = 0;
    ght.nxt_table_node_cnt = 0;

    ght.cur_bucket_size = min_bucket_size;
    ght.nxt_bucket_size = 0;

    ght.node_cnt = 0;
}

template <CntrTplParamList>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::Deinit(
    this Cntr& ght) {
    ght.ExtractAll();
}

template <CntrTplParamList>
constexpr size_t generic_hash_table::Cntr<CntrTplArgList>::GetNodeCnt(
    this Cntr const& ght) {
    detail::CheckCntr_(ght);

    return ght.node_cnt;
}

template <CntrTplParamList>
constexpr bool generic_hash_table::Cntr<CntrTplArgList>::Contain(
    this Cntr const& ght, Node const* n) {
    detail::CheckCntr_(ght);

    if (n == nullptr) { return false; }

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    TreeNode const* root{ bin_tree::GetMostP(&n->tn).first };

    void** root_entry{ static_cast<void**>(
        cur_table.Access(detail::BranchIdxesSource_{
            detail::GetBucketIdx_(n->hash_code, cur_bucket_size) })) };

    bool ret{ root_entry != nullptr && *root_entry == root };

    if (!ret && 0 < nxt_bucket_size) {
        root_entry =
            static_cast<void**>(nxt_table.Access(detail::BranchIdxesSource_{
                detail::GetBucketIdx_(n->hash_code, nxt_bucket_size) }));

        ret = root_entry != nullptr && *root_entry == root;
    }

    return ret;
}

template <CntrTplParamList>
template <typename Key, hash::CanHash<Key const*> KeyHasher,
          comparison::CanCompare<Key const*, generic_hash_table::Node const*>
              KeyElemComparator>
constexpr generic_hash_table::Node*
generic_hash_table::Cntr<CntrTplArgList>::Find(
    this Cntr const& ght, Key const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr) {
    detail::CheckCntr_(ght);

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    if (cur_bucket_size == 0) { return nullptr; }

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    if (0 < nxt_bucket_size) {
        Node* ret{ detail::FindInTable_(ght.nxt_salt, nxt_table,
                                        nxt_bucket_size, key, key_hasher,
                                        key_elem_cmptr) };

        if (ret != nullptr) { return ret; }
    }

    return detail::FindInTable_(ght.cur_salt, cur_table, cur_bucket_size, key,
                                key_hasher, key_elem_cmptr);
}

template <CntrTplParamList>
template <typename Key, hash::CanHash<Key const*> KeyHasher,
          comparison::CanCompare<Key const*, generic_hash_table::Node const*>
              KeyElemComparator>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::Insert(
    this Cntr& ght, Key const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, Node* n) {
    detail::CheckCntr_(ght);

    ZETA_Core_DebugAssert(n != nullptr);

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    if (0 < nxt_bucket_size) {
        detail::InsertToTable_(ght.nxt_salt, nxt_table, nxt_bucket_size, key,
                               key_hasher, key_elem_cmptr, n);
    } else {
        detail::InsertToTable_(ght.cur_salt, cur_table, cur_bucket_size, key,
                               key_hasher, key_elem_cmptr, n);
    }

    ++ght.node_cnt;

    detail::TryRunPending_(ght, cur_table, nxt_table, 4);
}

template <CntrTplParamList>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::Extract(this Cntr& ght,
                                                                 Node* n) {
    detail::CheckCntr_(ght);

    ZETA_Core_DebugAssert(n != nullptr);

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    auto const& hasher{ meta::GetInstRef(ght.hasher) };

    TreeNode* n_root{ bin_tree::GetMostP(&n->tn).first };

    if (!detail::TryExtractFromTable_(ght.cur_salt, cur_table, cur_bucket_size,
                                      n, n_root, hasher) &&
        !detail::TryExtractFromTable_(ght.nxt_salt, nxt_table, nxt_bucket_size,
                                      n, n_root, hasher)) {
        ZETA_Core_DebugAssert(false);
    }

    --ght.node_cnt;

    detail::TryRunPending_(ght, cur_table, nxt_table, 4);
}

template <CntrTplParamList>
constexpr generic_hash_table::Node*
generic_hash_table::Cntr<CntrTplArgList>::ExtractAny(this Cntr& ght) {
    detail::CheckCntr_(ght);

    if (ght.node_cnt == 0) { return nullptr; }

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    size_t idxes[max_level];

    void** root_entry{ static_cast<void**>(cur_table.FindFirst(idxes)) };

    auto* n{ ZETA_Core_MemberToStruct(Node, tn, *root_entry) };

    void* new_root{ rbtree::Extract(&n->tn) };

    *root_entry = new_root;

    if (new_root == nullptr) { cur_table.Erase(idxes); }

    --ght.node_cnt;

    detail::TryRunPending_(ght, cur_table, nxt_table, 4);

    return n;
}

template <CntrTplParamList>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::ExtractAll(
    this Cntr& ght) {
    detail::CheckCntr_(ght);

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    cur_table.Deinit();

    if (0 < nxt_bucket_size) { nxt_table.Deinit(); }

    ght.cur_salt = random::GetRandomInt<unsigned long long>(
        meta::GetInstRef(ght.salt_random_engine));

    ght.cur_table_root = nullptr;
    ght.nxt_table_root = nullptr;

    ght.cur_table_node_cnt = 0;
    ght.nxt_table_node_cnt = 0;

    ght.cur_bucket_size = min_bucket_size;

    ght.node_cnt = 0;
}

template <CntrTplParamList>
constexpr bool generic_hash_table::Cntr<CntrTplArgList>::RunPending(
    this Cntr& ght, size_t quata) {
    detail::CheckCntr_(ght);

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    if (nxt_bucket_size == 0) { return false; }

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    detail::TryRunPending_(ght, cur_table, nxt_table,
                           comparison_utils::BasicMax(4ULL, quata));

    return 0 < nxt_bucket_size;
}

namespace generic_hash_table::detail {

constexpr pair::Pair<unsigned long long, unsigned long long>
GetNodeCntAndDepthSum_(TreeNode* root) {
    constexpr size_t buffer_capacity{ rbtree::max_height + 8 };

    struct {
        size_t depth;
        TreeNode* n;
    } buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = {
        .depth = 1,
        .n = root,
    };

    size_t n_cnt{ 0 };
    unsigned long long depth_sum{ 0 };

    while (0 < buffer_i) {
        auto [depth, tn]{ buffer[--buffer_i] };

        ++n_cnt;
        depth_sum += depth;

        TreeNode* tnl{ bin_tree::GetL(tn) };
        TreeNode* tnr{ bin_tree::GetR(tn) };

        if (tnl != nullptr) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            buffer[buffer_i++] = {
                .depth = depth + 1,
                .n = tnl,
            };
        }

        if (tnr != nullptr) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);

            buffer[buffer_i++] = {
                .depth = depth + 1,
                .n = tnr,
            };
        }
    }

    return { n_cnt, depth_sum };
}

}  // namespace generic_hash_table::detail

template <CntrTplParamList>
constexpr auto generic_hash_table::Cntr<CntrTplArgList>::GetEffFactor(
    this Cntr const& ght) {
    detail::CheckCntr_(ght);

    if (ght.node_cnt == 0) { return 0; }

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    unsigned long long n_cnt{ 0 };
    unsigned long long depth_sum{ 0 };

    size_t idxes[max_level];

    {
        void** root_entry{ static_cast<void**>(cur_table.FindFirst(idxes)) };

        while (root_entry != nullptr) {
            auto [cur_n_cnt, cur_depth_sum]{ detail::GetNodeCntAndDepthSum_(
                static_cast<TreeNode*>(*root_entry)) };

            n_cnt += cur_n_cnt;
            depth_sum += cur_depth_sum;

            root_entry = static_cast<void**>(cur_table.FindNextExcl(idxes));
        }
    }

    if (0 < nxt_bucket_size) {
        void** root_entry{ static_cast<void**>(nxt_table.FindFirst(idxes)) };

        while (root_entry != nullptr) {
            auto [cur_n_cnt, cur_depth_sum]{ detail::GetNodeCntAndDepthSum_(
                static_cast<TreeNode*>(*root_entry)) };

            n_cnt += cur_n_cnt;
            depth_sum += cur_depth_sum;

            root_entry = static_cast<void**>(nxt_table.FindNextExcl(idxes));
        }
    }

    return fixed_point::FromFraction(depth_sum, n_cnt);
}

namespace generic_hash_table::detail {

struct SanitizeTreeRet_ {
    size_t cnt;
    TreeNode* most_l_tn;
    TreeNode* most_r_tn;
};

template <CntrTplParamList>
constexpr SanitizeTreeRet_ SanitizeTree_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& ght, mem_recorder::MemRecorder* dst_node,
     unsigned long long salt, size_t bucket_size, size_t bucket_idx,
     TreeNode* tn) {
    if (tn == nullptr) { return { 0, nullptr, nullptr }; }

    auto* n{ ZETA_Core_MemberToStruct(Node, tn, tn) };

    ZETA_Core_DebugAssert(
        (GetBucketIdx_)(hash::Hash(meta::GetInstRef(ght.hasher), n, salt),
                        bucket_size) == bucket_idx);

    if (dst_node != nullptr) {
        mem_recorder::Record(*dst_node, n, sizeof(Node));
    }

    TreeNode* tnl{ bin_tree::GetL(&n->tn) };
    TreeNode* tnr{ bin_tree::GetR(&n->tn) };

    SanitizeTreeRet_ l_ret{ SanitizeTree_(ght, dst_node, salt, bucket_size,
                                          bucket_idx, tnl) };

    SanitizeTreeRet_ r_ret{ SanitizeTree_(ght, dst_node, salt, bucket_size,
                                          bucket_idx, tnr) };

    SanitizeTreeRet_ ret{
        .cnt = l_ret.cnt + 1 + r_ret.cnt,
        .most_l_tn = tn,
        .most_r_tn = tn,
    };

    if (tnl != nullptr) {
        comparison::Ordering cmp{ comparison::Compare(
            meta::GetInstRef(ght.cmptr),
            meta::AutoValueWrapper<comparison::OpEnum::Order>{},
            ZETA_Core_MemberToStruct(Node, tn, l_ret.most_r_tn), n) };

        ZETA_Core_DebugAssert(cmp != comparison::Ordering::Greater);

        ret.most_l_tn = l_ret.most_l_tn;
    }

    if (tnr != nullptr) {
        comparison::Ordering cmp{ comparison::Compare(
            meta::GetInstRef(ght.cmptr),
            meta::AutoValueWrapper<comparison::OpEnum::Order>{}, n,
            ZETA_Core_MemberToStruct(Node, tn, r_ret.most_l_tn)) };

        ZETA_Core_DebugAssert(cmp != comparison::Ordering::Greater);

        ret.most_r_tn = r_ret.most_r_tn;
    }

    return ret;
}

}  // namespace generic_hash_table::detail

template <CntrTplParamList>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::Sanitize(
    this Cntr const& ght, mem_recorder::MemRecorder* dst_table_node,
    mem_recorder::MemRecorder* dst_node) {
    detail::CheckCntr_(ght);

    size_t cur_bucket_size{ ght.cur_bucket_size };
    size_t nxt_bucket_size{ ght.nxt_bucket_size };

    BuildCurMLPTTable;
    BuildNxtMLPTTable;

    cur_table.Sanitize(dst_table_node);

    if (0 < nxt_bucket_size) { nxt_table.Sanitize(dst_table_node); }

    size_t total_size{ 0 };

    size_t idxes[max_level];

    {
        void** root_entry{ static_cast<void**>(cur_table.FindFirst(idxes)) };

        while (root_entry != nullptr) {
            total_size += bin_tree::Count(static_cast<TreeNode*>(*root_entry));

            detail::SanitizeTree_(
                ght, dst_node, ght.cur_salt, cur_bucket_size,
                detail::BranchIdxesToBucketIdx_(cur_table.level, idxes),
                static_cast<TreeNode*>(*root_entry));

            root_entry =
                static_cast<void**>(cur_table.FindNextExcl(idxes, idxes));
        }
    }

    if (0 < nxt_bucket_size) {
        void** root_entry{ static_cast<void**>(nxt_table.FindFirst(idxes)) };

        while (root_entry != nullptr) {
            total_size += bin_tree::Count(static_cast<TreeNode*>(*root_entry));

            detail::SanitizeTree_(
                ght, dst_node, ght.nxt_salt, nxt_bucket_size,
                detail::BranchIdxesToBucketIdx_(nxt_table.level, idxes),
                static_cast<TreeNode*>(*root_entry));

            root_entry =
                static_cast<void**>(nxt_table.FindNextExcl(idxes, idxes));
        }
    }

    ZETA_Core_DebugAssert(ght.node_cnt == total_size);
}

}  // namespace zeta::core

#pragma pop_macro("BuildNxtMLPTTable")
#pragma pop_macro("BuildCurMLPTTable")
#pragma pop_macro("MLPT_CNTR")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")
