#pragma once

#include <zeta/core/allocator.ipp>
#include <zeta/core/array.hpp>
#include <zeta/core/basic_bin_tree_node.ipp>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/fixed_point.ipp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/hash.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lin_seq_endpoint.ipp>
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

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                             \
    typename NodeHashLike, typename NodeCompareLike, \
        typename SaltRandomEngineLike, typename TableNodeAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList \
    NodeHashLike, NodeCompareLike, SaltRandomEngineLike, TableNodeAllocatorLike

/*
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
    static_assert(true)

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
    static_assert(true)
*/

namespace zeta::core {

constexpr void generic_hash_table::Node::Construct() { this->tn.Construct(); }

namespace generic_hash_table::detail {

constexpr size_t GetBucketIdx_  // NOLINT(misc-use-internal-linkage)
    (unsigned long long hash_code, size_t bucket_size) {
    return utils::SimpleUnsignedIntegralHash(hash_code, 0ULL) % bucket_size;
}

constexpr void BucketIdxToBranchIdxes_  // NOLINT(misc-use-internal-linkage)
    (unsigned level, size_t bucket_idx,
     multi_level_ptr_table::BranchNum* branch_idxes) {
    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        branch_idxes[level_i] = bucket_idx % generic_hash_table::branch_num;
        bucket_idx /= generic_hash_table::branch_num;
    }
}

constexpr size_t BranchIdxesToBucketIdx_  // NOLINT(misc-use-internal-linkage)
    (unsigned level, multi_level_ptr_table::BranchNum const* branch_idxes) {
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

template <allocator::IsAllocator NodeAllocator>
struct MLPTHelper_ {
    using MLPT = multi_level_ptr_table::Cntr<
        ActiveMap,
        decltype(debug_utils::recording_allocator::TryMakeSubGroupAllocator(
            meta::Declval<NodeAllocator&>(), "mlpt.node_alctr"))>;

    char data[sizeof(MLPT)] __attribute__((aligned(alignof(MLPT))));

    constexpr MLPTHelper_(unsigned level, size_t elem_cnt, void* root,
                          NodeAllocator& table_node_alctr) {
        new (data) MLPT{
            lifecycle::DirectConstructTag{},
            level,
            branch_nums.elems,
            elem_cnt,
            root,
            debug_utils::recording_allocator::TryMakeSubGroupAllocator(
                table_node_alctr, "mlpt.node_alctr"),
        };
    }

    constexpr ~MLPTHelper_() {
        reinterpret_cast<MLPT*>(data)->DisownDestruct();
    }

    constexpr MLPT& GetMLPT() const {
        return *reinterpret_cast<MLPT const*>(data);
    }
};

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

    multi_level_ptr_table::BranchNum branch_idxes[max_level];

    (BucketIdxToBranchIdxes_)(
        table.level, (GetBucketIdx_)(key_hash_code, bucket_size), branch_idxes);

    void** tmp{ static_cast<void**>(
        table.Access(lin_seq_endpoint::provider::Provider{
            .data{ branch_idxes + (table.level - 1) },
            .elem_size{ sizeof(multi_level_ptr_table::BranchNum) },
            .elem_cnt{ table.level },
            .step{ -1 },
        })) };

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
            key_elem_cmptr, comparison::OpTags::Order{}, key,
            ZETA_Core_MemberToStruct(Node, tn, tn)) };

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

    auto p{ ({
        multi_level_ptr_table::BranchNum branch_idxes[max_level];

        detail::BucketIdxToBranchIdxes_(
            table.level, (GetBucketIdx_)(key_hash_code, bucket_size),
            branch_idxes);

        table.Insert(lin_seq_endpoint::provider::Provider{
            .data{ branch_idxes + (table.level - 1) },
            .elem_size{ sizeof(multi_level_ptr_table::BranchNum) },
            .elem_stride{ -static_cast<ptrdiff_t>(
                sizeof(multi_level_ptr_table::BranchNum)) },
            .elem_cnt{ table.level },
        });
    }) };

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
            key_elem_cmptr, comparison::OpTags::Order{}, n,
            ZETA_Core_MemberToStruct(Node, tn, tn)) };

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

template <allocator::IsAllocator TableNodeAllocator, CntrTplParamList>
void TryRunPending_(Cntr<CntrTplArgList> const& self_,
                    MLPTHelper_<TableNodeAllocator>& cur_table,
                    MLPTHelper_<TableNodeAllocator>& nxt_table, size_t quata) {
    auto& self{ const_cast<Cntr<CntrTplArgList>&>(self_) };

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    if (nxt_bucket_size == 0) {
        self.cur_table_node_cnt = cur_table.elem_cnt;
        self.cur_table_root = cur_table.root;

        auto center_capacity{ fixed_point::FromIntegral(cur_bucket_size) *
                              self.rehashing_config.center_load_ratio };

        auto lb_capacity{
            (center_capacity / self.rehashing_config.drift_ratio).Floor()
        };

        auto rb_capacity{
            (center_capacity * self.rehashing_config.drift_ratio).Ceil()
        };

        size_t nxt_bucket_size{ cur_bucket_size };

        if (cur_table.elem_cnt < lb_capacity) {
            nxt_bucket_size = (FindPrvBucketSize_)(static_cast<size_t>(
                (fixed_point::FromIntegral(cur_bucket_size) /
                 self.rehashing_config.drift_ratio)
                    .Floor()));
        } else if (rb_capacity < cur_table.elem_cnt) {
            nxt_bucket_size = (FindNxtBucketSize_)(static_cast<size_t>(
                (fixed_point::FromIntegral(cur_bucket_size) *
                 self.rehashing_config.drift_ratio)
                    .Ceil()));
        }

        if (cur_bucket_size != nxt_bucket_size) {
            self.nxt_bucket_size = nxt_bucket_size;
            self.nxt_salt = random::GetRandomInt<unsigned long long>(
                meta::GetInstRef(self.salt_random_engine));
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

        (InsertToTable_)(self.nxt_salt, nxt_table, self.nxt_bucket_size,
                         trans_n, meta::GetInstRef(self.hasher_like),
                         meta::GetInstRef(self.cmptr_like), trans_n);
    }

    if (0 < cur_table.elem_cnt) {
        self.cur_table_node_cnt = cur_table.elem_cnt;
        self.cur_table_root = cur_table.root;

        self.nxt_table_node_cnt = nxt_table.elem_cnt;
        self.nxt_table_root = nxt_table.root;

        return;
    }

    if (nxt_table.elem_cnt == 0) {
        self.cur_salt = random::GetRandomInt<unsigned long long>(
            meta::GetInstRef(self.salt_random_engine));
        self.cur_table_node_cnt = 0;
        self.cur_bucket_size = min_bucket_size;
        self.cur_table_root = nullptr;
    } else {
        self.cur_salt = self.nxt_salt;
        self.cur_table_node_cnt = nxt_table.elem_cnt;
        self.cur_bucket_size = nxt_bucket_size;
        self.cur_table_root = nxt_table.root;
    }

    self.nxt_table_node_cnt = 0;
    self.nxt_bucket_size = 0;
    self.nxt_table_root = nullptr;
}

constexpr void CheckRehasingConfig_(RehashingConfig const& rehashing_config) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(min_center_load_ratio <=
                                            rehashing_config.center_load_ratio);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        rehashing_config.center_load_ratio <= max_center_load_ratio);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(min_drift_ratio <=
                                            rehashing_config.drift_ratio);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rehashing_config.drift_ratio <=
                                            max_drift_ratio);
}

template <CntrTplParamList>
constexpr void CheckCntr_(Cntr<CntrTplArgList> const& self) {
    size_t cur_bucket_size{ self.cur_bucket_size };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cur_bucket_size != 0);
}

}  // namespace generic_hash_table::detail

template <CntrTplParamList>
template <typename NodeHashLikeConstructArg, typename ComparatorConstructArg,
          typename TableNodeAllocatorConstructArg,
          typename SaltRandomEngineConstructArg>
constexpr generic_hash_table::Cntr<CntrTplArgList>::Cntr(
    RehashingConfig const& rehashing_config,
    NodeHashLikeConstructArg&& hasher_construct_arg,
    ComparatorConstructArg&& cmptr_construct_arg,
    SaltRandomEngineConstructArg&& salt_random_engine_construct_arg,
    TableNodeAllocatorConstructArg&& table_node_alctr_construct_arg)
    : hasher_like{ ZETA_Core_Lifecycle_UnpackConstructArg(
          HasherLike, NodeHashLikeConstructArg, hasher_construct_arg) },
      cmptr_like{ ZETA_Core_Lifecycle_UnpackConstructArg(
          ComparatorLike, ComparatorConstructArg, cmptr_construct_arg) },
      salt_random_engine_like{ ZETA_Core_Lifecycle_UnpackConstructArg(
          SaltRandomEngineLike, SaltRandomEngineConstructArg,
          salt_random_engine_construct_arg) },
      table_node_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArg(
          TableNodeAllocatorLike, TableNodeAllocatorConstructArg,
          table_node_alctr_construct_arg) } {
    detail::CheckRehasingConfig_(rehashing_config);

    this->rehashing_config = rehashing_config;

    this->cur_salt = random::GetRandomInt<unsigned long long>(
        meta::GetInstRef(this->salt_random_engine_like));

    this->cur_table_root = nullptr;
    this->nxt_table_root = nullptr;

    this->cur_table_node_cnt = 0;
    this->nxt_table_node_cnt = 0;

    this->cur_bucket_size = min_bucket_size;
    this->nxt_bucket_size = 0;

    this->node_cnt = 0;
}

template <CntrTplParamList>
constexpr generic_hash_table::Cntr<CntrTplArgList>::~Cntr() {
    this->ExtractAll();
}

template <CntrTplParamList>
constexpr size_t generic_hash_table::Cntr<CntrTplArgList>::GetNodeCnt(
    this Cntr const& self) {
    detail::CheckCntr_(self);

    return self.node_cnt;
}

template <CntrTplParamList>
constexpr bool generic_hash_table::Cntr<CntrTplArgList>::Contain(
    this Cntr const& self, Node const* n) {
    detail::CheckCntr_(self);

    if (n == nullptr) { return false; }

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    TreeNode const* root{ bin_tree::GetMostP(&n->tn).first };

    void** root_entry{ ({
        multi_level_ptr_table::BranchNum branch_idxes[max_level];

        detail::BucketIdxToBranchIdxes_(
            cur_table.level,
            detail::GetBucketIdx_(n->hash_code, cur_bucket_size), branch_idxes);

        static_cast<void**>(
            cur_table.Access(lin_seq_endpoint::provider::Provider{
                .data{ branch_idxes + (cur_table.level - 1) },
                .elem_size{ sizeof(multi_level_ptr_table::BranchNum) },
                .elem_stride{ -static_cast<ptrdiff_t>(
                    sizeof(multi_level_ptr_table::BranchNum)) },
                .elem_cnt{ cur_table.level },
            }));
    }) };

    bool ret{ root_entry != nullptr && *root_entry == root };

    if (!ret && 0 < nxt_bucket_size) {
        root_entry = ({
            multi_level_ptr_table::BranchNum branch_idxes[max_level];

            detail::BucketIdxToBranchIdxes_(
                nxt_table.level,
                detail::GetBucketIdx_(n->hash_code, nxt_bucket_size),
                branch_idxes);

            static_cast<void**>(
                nxt_table.Access(lin_seq_endpoint::provider::Provider{
                    .data{ branch_idxes + (nxt_table.level - 1) },
                    .elem_size{ sizeof(multi_level_ptr_table::BranchNum) },
                    .elem_stride{ -static_cast<ptrdiff_t>(
                        sizeof(multi_level_ptr_table::BranchNum)) },
                    .elem_cnt{ nxt_table.level },
                }));
        });

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
    this Cntr const& self, Key const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr) {
    detail::CheckCntr_(self);

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    if (cur_bucket_size == 0) { return nullptr; }

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    if (0 < nxt_bucket_size) {
        Node* ret{ detail::FindInTable_(self.nxt_salt, nxt_table,
                                        nxt_bucket_size, key, key_hasher,
                                        key_elem_cmptr) };

        if (ret != nullptr) { return ret; }
    }

    return detail::FindInTable_(self.cur_salt, cur_table, cur_bucket_size, key,
                                key_hasher, key_elem_cmptr);
}

template <CntrTplParamList>
template <typename Key, hash::CanHash<Key const*> KeyHasher,
          comparison::CanCompare<Key const*, generic_hash_table::Node const*>
              KeyElemComparator>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::Insert(
    this Cntr& self, Key const* key, KeyHasher const& key_hasher,
    KeyElemComparator const& key_elem_cmptr, Node* n) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(n != nullptr);

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    if (0 < nxt_bucket_size) {
        detail::InsertToTable_(self.nxt_salt, nxt_table, nxt_bucket_size, key,
                               key_hasher, key_elem_cmptr, n);
    } else {
        detail::InsertToTable_(self.cur_salt, cur_table, cur_bucket_size, key,
                               key_hasher, key_elem_cmptr, n);
    }

    ++self.node_cnt;

    detail::TryRunPending_(self, cur_table, nxt_table, 4);
}

template <CntrTplParamList>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::Extract(
    this Cntr& self, Node* n) {
    detail::CheckCntr_(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(n != nullptr);

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    auto const& hasher{ meta::GetInstRef(self.hasher_like) };

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    TreeNode* n_root{ bin_tree::GetMostP(&n->tn).first };

    if (!detail::TryExtractFromTable_(self.cur_salt, cur_table, cur_bucket_size,
                                      n, n_root, hasher) &&
        !detail::TryExtractFromTable_(self.nxt_salt, nxt_table, nxt_bucket_size,
                                      n, n_root, hasher)) {
        ZETA_Core_DebugUtils_Diag_Unreachable();
    }

    --self.node_cnt;

    detail::TryRunPending_(self, cur_table, nxt_table, 4);
}

template <CntrTplParamList>
constexpr generic_hash_table::Node*
generic_hash_table::Cntr<CntrTplArgList>::ExtractAny(this Cntr& self) {
    detail::CheckCntr_(self);

    if (self.node_cnt == 0) { return nullptr; }

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    size_t idxes[max_level];

    void** root_entry{ static_cast<void**>(cur_table.FindFirst(idxes)) };

    auto* n{ ZETA_Core_MemberToStruct(Node, tn, *root_entry) };

    void* new_root{ rbtree::Extract(&n->tn) };

    *root_entry = new_root;

    if (new_root == nullptr) { cur_table.Erase(idxes); }

    --self.node_cnt;

    detail::TryRunPending_(self, cur_table, nxt_table, 4);

    return n;
}

template <CntrTplParamList>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::ExtractAll(
    this Cntr& self) {
    detail::CheckCntr_(self);

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    cur_table.Deconstruct();

    if (0 < nxt_bucket_size) { nxt_table.Deconstruct(); }

    self.cur_salt = random::GetRandomInt<unsigned long long>(
        meta::GetInstRef(self.salt_random_engine_like));

    self.cur_table_root = nullptr;
    self.nxt_table_root = nullptr;

    self.cur_table_node_cnt = 0;
    self.nxt_table_node_cnt = 0;

    self.cur_bucket_size = min_bucket_size;

    self.node_cnt = 0;
}

template <CntrTplParamList>
constexpr bool generic_hash_table::Cntr<CntrTplArgList>::RunPending(
    this Cntr& self, size_t quata) {
    detail::CheckCntr_(self);

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    if (nxt_bucket_size == 0) { return false; }

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::TryRunPending_(self, cur_table, nxt_table,
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
            ZETA_Core_DebugUtils_Diag_PromiseAssert(buffer_i < buffer_capacity);

            buffer[buffer_i++] = {
                .depth = depth + 1,
                .n = tnl,
            };
        }

        if (tnr != nullptr) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(buffer_i < buffer_capacity);

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
    this Cntr const& self) {
    detail::CheckCntr_(self);

    if (self.node_cnt == 0) { return 0; }

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

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
    (Cntr<CntrTplArgList> const& self, mem_recorder::MemRecorder* dst_node,
     unsigned long long salt, size_t bucket_size, size_t bucket_idx,
     TreeNode* tn) {
    if (tn == nullptr) { return { 0, nullptr, nullptr }; }
    auto* n{ ZETA_Core_MemberToStruct(Node, tn, tn) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        (GetBucketIdx_)(hash::Hash(meta::GetInstRef(self.hasher_like), n, salt),
                        bucket_size) == bucket_idx);

    if (dst_node != nullptr) { dst_node->Record(n, sizeof(Node)); }

    TreeNode* tnl{ bin_tree::GetL(&n->tn) };
    TreeNode* tnr{ bin_tree::GetR(&n->tn) };

    SanitizeTreeRet_ l_ret{ SanitizeTree_(self, dst_node, salt, bucket_size,
                                          bucket_idx, tnl) };

    SanitizeTreeRet_ r_ret{ SanitizeTree_(self, dst_node, salt, bucket_size,
                                          bucket_idx, tnr) };

    SanitizeTreeRet_ ret{
        .cnt = l_ret.cnt + 1 + r_ret.cnt,
        .most_l_tn = tn,
        .most_r_tn = tn,
    };

    if (tnl != nullptr) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(comparison::Compare(
            meta::GetInstRef(self.cmptr_like), comparison::OpTags::LessEqual{},
            ZETA_Core_MemberToStruct(Node, tn, l_ret.most_r_tn), n));

        ret.most_l_tn = l_ret.most_l_tn;
    }

    if (tnr != nullptr) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(comparison::Compare(
            meta::GetInstRef(self.cmptr_like), comparison::OpTags::LessEqual{},
            n, ZETA_Core_MemberToStruct(Node, tn, r_ret.most_l_tn)));

        ret.most_r_tn = r_ret.most_r_tn;
    }

    return ret;
}

}  // namespace generic_hash_table::detail

template <CntrTplParamList>
constexpr void generic_hash_table::Cntr<CntrTplArgList>::SanityCheck(
    this Cntr const& self, debug_utils::sanity::SanityCheckScope scope) {
    detail::CheckCntr_(self);

    size_t cur_bucket_size{ self.cur_bucket_size };
    size_t nxt_bucket_size{ self.nxt_bucket_size };

    auto& table_node_alctr{ meta::GetInstRef(self.table_node_alctr_like) };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> cur_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(cur_bucket_size, branch_num)),
        .elem_cnt = self.cur_table_node_cnt,
        .root = self.cur_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    detail::MLPTHelper_<meta::RemoveRef<decltype(table_node_alctr)>> nxt_table{
        .level = static_cast<unsigned>(
            integral_math::CeilLog(nxt_bucket_size, branch_num)),
        .elem_cnt = self.nxt_table_node_cnt,
        .root = self.nxt_table_root,
        .nav_node_alctr = table_node_alctr,
    };

    debug_utils::sanity::SanityCheck(cur_table, scope);

    if (0 < nxt_bucket_size) {
        debug_utils::sanity::SanityCheck(nxt_table, scope);
    }

    if (scope == debug_utils::sanity::SanityCheckScope::Basic) { return; }

    size_t total_size{ 0 };

    size_t idxes[max_level];

    {
        void** root_entry{ static_cast<void**>(cur_table.FindFirst(idxes)) };

        while (root_entry != nullptr) {
            total_size += bin_tree::Count(static_cast<TreeNode*>(*root_entry));

            detail::SanitizeTree_(
                self, dst_node, self.cur_salt, cur_bucket_size,
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
                self, dst_node, self.nxt_salt, nxt_bucket_size,
                detail::BranchIdxesToBucketIdx_(nxt_table.level, idxes),
                static_cast<TreeNode*>(*root_entry));

            root_entry =
                static_cast<void**>(nxt_table.FindNextExcl(idxes, idxes));
        }
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.node_cnt == total_size);
}

}  // namespace zeta::core

#pragma pop_macro("BuildNxtMLPTTable")
#pragma pop_macro("BuildCurMLPTTable")
#pragma pop_macro("MLPT_CNTR")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("CntrTplParamList")
