#pragma once

#include <zeta/core/basic_bin_tree_node.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/multi_level_ptr_table.ipp>
#include <zeta/core/percent_prime_table.hpp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")
#pragma push_macro("MLPT_CNTR")

#define CntrTplParamList                             \
    typename NodeHashLike, typename NodeCompareLike, \
        typename TableNodeAllocatorLike

#define CntrTplArgList NodeHashLike, NodeCompareLike, TableNodeAllocatorLike

#define MLPT_CNTR                                           \
    multi_level_ptr_table::Cntr<decltype(utils::GetInstPtr( \
        meta::Declval<TableNodeAllocatorLike&>()))>

namespace zeta::core {

constexpr bool bin_tree::Traits<generic_hash_table::TreeNode>::IsConst() {
    return false;
}

constexpr bool bin_tree::Traits<generic_hash_table::TreeNode const>::IsConst() {
    return true;
}

constexpr bool
bin_tree::Traits<generic_hash_table::TreeNode const>::HasAccSize() {
    return false;
}

inline generic_hash_table::TreeNode* bin_tree::Traits<
    generic_hash_table::TreeNode>::GetP(generic_hash_table::TreeNode* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<generic_hash_table::TreeNode*>(n->GetPPtr());
}

inline generic_hash_table::TreeNode* bin_tree::Traits<
    generic_hash_table::TreeNode>::GetL(generic_hash_table::TreeNode* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<generic_hash_table::TreeNode*>(n->GetLPtr());
}

inline generic_hash_table::TreeNode* bin_tree::Traits<
    generic_hash_table::TreeNode>::GetR(generic_hash_table::TreeNode* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<generic_hash_table::TreeNode*>(n->GetRPtr());
}

inline generic_hash_table::TreeNode const*
bin_tree::Traits<generic_hash_table::TreeNode const>::GetP(
    generic_hash_table::TreeNode const* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<generic_hash_table::TreeNode const*>(n->GetPPtr());
}

inline generic_hash_table::TreeNode const*
bin_tree::Traits<generic_hash_table::TreeNode const>::GetL(
    generic_hash_table::TreeNode const* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<generic_hash_table::TreeNode const*>(n->GetLPtr());
}

inline generic_hash_table::TreeNode const*
bin_tree::Traits<generic_hash_table::TreeNode const>::GetR(
    generic_hash_table::TreeNode const* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<generic_hash_table::TreeNode const*>(n->GetRPtr());
}

inline void bin_tree::Traits<generic_hash_table::TreeNode>::SetP(
    generic_hash_table::TreeNode* n, generic_hash_table::TreeNode* m) {
    n->SetPPtr(m);
}

inline void bin_tree::Traits<generic_hash_table::TreeNode>::SetL(
    generic_hash_table::TreeNode* n, generic_hash_table::TreeNode* m) {
    n->SetLPtr(m);
}

inline void bin_tree::Traits<generic_hash_table::TreeNode>::SetR(
    generic_hash_table::TreeNode* n, generic_hash_table::TreeNode* m) {
    n->SetRPtr(m);
}

inline unsigned rbtree::Traits<generic_hash_table::TreeNode const>::GetColor(
    generic_hash_table::TreeNode const* n) {
    return n->GetPColor();
}

inline void rbtree::Traits<generic_hash_table::TreeNode>::SetColor(
    generic_hash_table::TreeNode* n, unsigned color) {
    n->SetPColor(color);
}

inline void generic_hash_table::Node::Init() { this->n.Init(); }

namespace generic_hash_table::ops::detail {

inline size_t GetBucketIdx_  // NOLINT(misc-use-internal-linkage)
    (unsigned long long hash_code, size_t bucket_size) {
    return utils::ULLHash(hash_code, 0) % bucket_size;
}

inline size_t GetIdx_  // NOLINT(misc-use-internal-linkage)
    (unsigned level, size_t const* idxes) {
    size_t idx{ 0 };

    for (unsigned level_i{ level }; 0 < level_i; --level_i) {
        idx = idx * branch_num + idxes[level_i - 1];
    }

    return idx;
}

inline void SetIdxes_  // NOLINT(misc-use-internal-linkage)
    (unsigned level, size_t* idxes, size_t idx) {
    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        idxes[level_i] = idx % branch_num;
        idx /= branch_num;
    }
}

inline size_t FindPrvBucketSize_  // NOLINT(misc-use-internal-linkage)
    (size_t bucket_size) {
    bucket_size = utils::Max(bucket_size, min_bucket_size);

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

    return utils::Max(min_bucket_size, percent_prime_table::table[lb]);
}

inline size_t FindNxtBucketSize_  // NOLINT(misc-use-internal-linkage)
    (size_t bucket_size) {
    bucket_size = utils::Min(bucket_size, max_bucket_size);

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

    return utils::Min(percent_prime_table::table[lb], max_bucket_size);
}

template <typename TableNodeAllocatorLike, typename KeyHash,
          typename KeyNodeCompare>
Node* FindInTable_  // NOLINT(misc-use-internal-linkage)
    (unsigned long long salt,
     multi_level_ptr_table::Cntr<TableNodeAllocatorLike>* table,
     size_t bucket_size, void const* key, KeyHash const& key_hash,
     KeyNodeCompare const& key_node_compare) {
    size_t idxes[max_level];

    SetIdxes_(table->level, idxes,
              GetBucketIdx_(key_hash(key, salt), bucket_size));

    TreeNode* target_n{ nullptr };

    void** tmp{ static_cast<void**>(
        multi_level_ptr_table::ops::Access(table, idxes)) };

    for (TreeNode* n{
             ({ tmp == nullptr ? nullptr : static_cast<TreeNode*>(*tmp); }) };
         n != nullptr;) {
        int cmp{ key_node_compare(key, ZETA_Core_MemberToStruct(Node, n, n)) };

        if (cmp == 0) { target_n = n; }

        if (cmp <= 0) {
            n = bin_tree::ops::GetL(n);
        } else {
            n = bin_tree::ops::GetR(n);
        }
    }

    return target_n == nullptr ? nullptr
                               : ZETA_Core_MemberToStruct(Node, n, target_n);
}

template <CntrTplParamList>
void InsertToTable_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* ght, unsigned long long salt, MLPT_CNTR* table,
     size_t bucket_size, Node* node) {
    size_t idxes[max_level];

    (SetIdxes_)(table->level, idxes,
                (GetBucketIdx_)(utils::GetInstRef(ght->node_hash)(node, salt),
                                bucket_size));

    auto p{ multi_level_ptr_table::ops::Insert(table, idxes) };

    void** root_entry{ static_cast<void**>(p.first) };

    if (p.second) {
        *root_entry = &node->n;
        return;
    }

    TreeNode* n{ static_cast<TreeNode*>(*root_entry) };

    TreeNode* le_n{ nullptr };
    TreeNode* gt_n{ nullptr };

    while (n != nullptr) {
        int cmp{ utils::GetInstRef(ght->node_compare)(
            node, ZETA_Core_MemberToStruct(Node, n, n)) };

        if (cmp < 0) {
            gt_n = n;
            n = bin_tree::ops::GetL(n);
        } else {
            le_n = n;
            n = bin_tree::ops::GetR(n);
        }
    }

    *root_entry = rbtree::ops::Insert(le_n, gt_n, &node->n);
}

template <CntrTplParamList>
bool TryExtractFromTable_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* ght, unsigned long long salt, MLPT_CNTR* table,
     size_t bucket_size, Node* node, TreeNode* root) {
    if (bucket_size == 0) { return false; }

    size_t idxes[max_level];

    (SetIdxes_)(
        table->level, idxes,
        detail::GetBucketIdx_(utils::GetInstRef(ght->node_hash)(node, salt),
                              bucket_size));

    void** root_entry{ static_cast<void**>(
        multi_level_ptr_table::ops::Access(table, idxes)) };

    if (root_entry == nullptr || *root_entry != root) { return false; }

    void* new_root{ rbtree::ops::Extract(&node->n) };

    *root_entry = new_root;

    if (new_root == nullptr) {
        multi_level_ptr_table::ops::Erase(table, idxes);
    }

    return true;
}

template <CntrTplParamList>
void TryRunPending_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* ght_, MLPT_CNTR* cur_table,
     MLPT_CNTR* nxt_table, size_t quata) {
    auto ght{ const_cast<Cntr<CntrTplArgList>*>(ght_) };

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    if (nxt_bucket_size == 0) {
        ght->cur_table_size = cur_table->size;
        ght->cur_table_root = cur_table->root;

        auto center_capacity{ fixed_point::ops::FromIntegral(cur_bucket_size) *
                              ght->rehashing_config.center_load_ratio };

        auto lb_capacity{ fixed_point::ops::Floor(
            center_capacity / ght->rehashing_config.drift_ratio) };

        auto rb_capacity{ fixed_point::ops::Ceil(
            center_capacity * ght->rehashing_config.drift_ratio) };

        size_t nxt_bucket_size{ cur_bucket_size };

        if (cur_table->size < lb_capacity) {
            nxt_bucket_size = (FindPrvBucketSize_)(static_cast<size_t>(
                fixed_point::ops::Floor(
                    fixed_point::ops::FromIntegral(cur_bucket_size) /
                    ght->rehashing_config.drift_ratio)));
        } else if (rb_capacity < cur_table->size) {
            nxt_bucket_size =
                (FindNxtBucketSize_)(static_cast<size_t>(fixed_point::ops::Ceil(
                    fixed_point::ops::FromIntegral(cur_bucket_size) *
                    ght->rehashing_config.drift_ratio)));
        }

        if (cur_bucket_size != nxt_bucket_size) {
            ght->nxt_bucket_size = nxt_bucket_size;
            ght->nxt_salt = utils::GetRandom();
        }

        return;
    }

    size_t idxes[max_level];

    void** root_entry{ nullptr };

    for (; 0 < quata && 0 < cur_table->size; --quata) {
        if (root_entry == nullptr) {
            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindFirst(cur_table, idxes));
        }

        auto* trans_node{ ZETA_Core_MemberToStruct(Node, n, *root_entry) };

        void* new_root{ rbtree::ops::Extract(&trans_node->n) };

        *root_entry = new_root;

        if (new_root == nullptr) {
            multi_level_ptr_table::ops::Erase(cur_table, idxes);
            root_entry = nullptr;
        }

        InsertToTable_(const_cast<Cntr<CntrTplArgList>*>(ght), ght->nxt_salt,
                       nxt_table, ght->nxt_bucket_size, trans_node);
    }

    if (0 < cur_table->size) {
        ght->cur_table_size = cur_table->size;
        ght->cur_table_root = cur_table->root;

        ght->nxt_table_size = nxt_table->size;
        ght->nxt_table_root = nxt_table->root;

        return;
    }

    if (nxt_table->size == 0) {
        ght->cur_salt = utils::GetRandom();
        ght->cur_table_size = 0;
        ght->cur_bucket_size = min_bucket_size;
        ght->cur_table_root = nullptr;
    } else {
        ght->cur_salt = ght->nxt_salt;
        ght->cur_table_size = nxt_table->size;
        ght->cur_bucket_size = nxt_bucket_size;
        ght->cur_table_root = nxt_table->root;
    }

    ght->nxt_table_size = 0;
    ght->nxt_bucket_size = 0;
    ght->nxt_table_root = nullptr;
}

template <CntrTplParamList>
void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* ght) {
    ZETA_Core_DebugAssert(ght != nullptr);

    size_t cur_bucket_size{ ght->cur_bucket_size };

    ZETA_Core_DebugAssert(cur_bucket_size != 0);
}

}  // namespace generic_hash_table::ops::detail

template <CntrTplParamList>
void generic_hash_table::ops::Init(Cntr<CntrTplArgList>* ght) {
    ZETA_Core_DebugAssert(ght != nullptr);

    ght->cur_salt = utils::GetRandom();

    ght->cur_table_root = nullptr;
    ght->nxt_table_root = nullptr;

    ght->cur_table_size = 0;
    ght->nxt_table_size = 0;

    ght->cur_bucket_size = min_bucket_size;
    ght->nxt_bucket_size = 0;

    ght->size = 0;
}

template <CntrTplParamList>
void generic_hash_table::ops::Deinit(Cntr<CntrTplArgList>* ght) {
    ExtractAll(ght);
}

template <CntrTplParamList>
size_t generic_hash_table::ops::GetSize(Cntr<CntrTplArgList> const* ght) {
    detail::CheckCntr_(ght);

    return ght->size;
}

template <CntrTplParamList>
bool generic_hash_table::ops::Contain(Cntr<CntrTplArgList> const* ght,
                                      Node const* node) {
    detail::CheckCntr_(ght);

    if (node == nullptr) { return false; }

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    TreeNode const* root{ bin_tree::ops::GetMostP(&node->n).first };

    size_t idxes[max_level];

    detail::SetIdxes_(cur_table.level, idxes,
                      detail::GetBucketIdx_(utils::GetInstRef(ght->node_hash)(
                                                node, ght->cur_salt),
                                            cur_bucket_size));

    void** root_entry{ static_cast<void**>(
        multi_level_ptr_table::ops::Access(&cur_table, idxes)) };

    bool ret{ root_entry != nullptr && *root_entry == root };

    if (!ret && 0 < nxt_bucket_size) {
        detail::SetIdxes_(
            nxt_table.level, idxes,
            detail::GetBucketIdx_(
                utils::GetInstRef(ght->node_hash)(node, ght->nxt_salt),
                nxt_bucket_size));

        root_entry = static_cast<void**>(
            multi_level_ptr_table::ops::Access(&nxt_table, idxes));

        ret = root_entry != nullptr && *root_entry == root;
    }

    detail::TryRunPending_(ght, &cur_table, &nxt_table, 4);

    return ret;
}

template <CntrTplParamList, typename KeyHash, typename KeyNodeCompare>
generic_hash_table::Node* generic_hash_table::ops::Find(
    Cntr<CntrTplArgList> const* ght, void const* key, KeyHash const& key_hash,
    KeyNodeCompare const& key_node_compare) {
    detail::CheckCntr_(ght);

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    Node* ret{ nxt_bucket_size == 0
                   ? nullptr
                   : detail::FindInTable_(ght->nxt_salt, &nxt_table,
                                          nxt_bucket_size, key, key_hash,
                                          key_node_compare) };

    if (ret == nullptr) {
        ret = detail::FindInTable_(ght->cur_salt, &cur_table, cur_bucket_size,
                                   key, key_hash, key_node_compare);
    }

    detail::TryRunPending_(ght, &cur_table, &nxt_table, 4);

    return ret;
}

template <CntrTplParamList>
void generic_hash_table::ops::Insert(Cntr<CntrTplArgList>* ght, Node* node) {
    detail::CheckCntr_(ght);

    ZETA_Core_DebugAssert(node != nullptr);

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    if (0 < nxt_bucket_size) {
        detail::InsertToTable_(ght, ght->nxt_salt, &nxt_table, nxt_bucket_size,
                               node);
    } else {
        detail::InsertToTable_(ght, ght->cur_salt, &cur_table, cur_bucket_size,
                               node);
    }

    ++ght->size;

    detail::TryRunPending_(ght, &cur_table, &nxt_table, 4);
}

template <CntrTplParamList>
void generic_hash_table::ops::Extract(Cntr<CntrTplArgList>* ght, Node* node) {
    detail::CheckCntr_(ght);

    ZETA_Core_DebugAssert(node != nullptr);

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    TreeNode* root{ bin_tree::ops::GetMostP(&node->n).first };

    if (!detail::TryExtractFromTable_(ght, ght->cur_salt, &cur_table,
                                      cur_bucket_size, node, root) &&
        !detail::TryExtractFromTable_(ght, ght->nxt_salt, &nxt_table,
                                      nxt_bucket_size, node, root)) {
        ZETA_Core_DebugAssert(false);
    }

    --ght->size;

    detail::TryRunPending_(ght, &cur_table, &nxt_table, 4);
}

template <CntrTplParamList>
generic_hash_table::Node* generic_hash_table::ops::ExtractAny(
    Cntr<CntrTplArgList>* ght) {
    detail::CheckCntr_(ght);

    if (ght->size == 0) { return nullptr; }

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    size_t idxes[max_level];

    void** root_entry{ static_cast<void**>(
        multi_level_ptr_table::ops::FindFirst(&cur_table, idxes)) };

    auto* node{ ZETA_Core_MemberToStruct(Node, n, *root_entry) };

    void* new_root{ rbtree::ops::Extract(&node->n) };

    *root_entry = new_root;

    if (new_root == nullptr) {
        multi_level_ptr_table::ops::Erase(&cur_table, idxes);
    }

    --ght->size;

    TryRunPending(ght, &cur_table, &nxt_table, 4);

    return node;
}

template <CntrTplParamList>
void generic_hash_table::ops::ExtractAll(Cntr<CntrTplArgList>* ght) {
    detail::CheckCntr_(ght);

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    multi_level_ptr_table::ops::Deinit(&cur_table);

    if (0 < nxt_bucket_size) { multi_level_ptr_table::ops::Deinit(&nxt_table); }

    ght->cur_salt = utils::GetRandom();

    ght->cur_table_root = nullptr;
    ght->nxt_table_root = nullptr;

    ght->cur_table_size = 0;
    ght->nxt_table_size = 0;

    ght->cur_bucket_size = min_bucket_size;

    ght->size = 0;
}

template <CntrTplParamList>
bool generic_hash_table::ops::RunPending(Cntr<CntrTplArgList>* ght,
                                         size_t quata) {
    detail::CheckCntr_(ght);

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    if (nxt_bucket_size == 0) { return false; }

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    TryRunPending(ght, &cur_table, &nxt_table, GetMaxOf(4ULL, quata));

    return 0 < nxt_bucket_size;
}

template <CntrTplParamList>
unsigned long long generic_hash_table::ops::GetEffFactor(
    Cntr<CntrTplArgList> const* ght) {
    detail::CheckCntr_(ght);

    if (ght->size == 0) { return 0; }

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(ght->table_node_alctr),
    };

    unsigned long long total_height{ 0 };

    size_t idxes[max_level];

    if (0 < cur_bucket_size) {
        void** root_entry{ static_cast<void**>(
            multi_level_ptr_table::ops::FindFirst(&cur_table, idxes)) };

        while (root_entry != nullptr) {
            size_t tree_size{ bin_tree::ops::Count(
                static_cast<TreeNode*>(*root_entry)) };

            total_height += CeilLog2(tree_size) * tree_size;

            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindNext(&cur_table, idxes, false));
        }
    }

    if (0 < nxt_bucket_size) {
        void** root_entry{ static_cast<void**>(
            multi_level_ptr_table::ops::FindFirst(&nxt_table, idxes)) };

        while (root_entry != nullptr) {
            size_t tree_size{ bin_tree::ops::Count(
                static_cast<TreeNode*>(*root_entry)) };

            total_height += CeilLog2(tree_size) * tree_size;

            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindNext(&nxt_table, idxes, false));
        }
    }

    return total_height * 1'000'000 / ght->size;
}

namespace generic_hash_table::ops::detail {

struct SanitizeTreeRet_ {
    size_t cnt;
    TreeNode* most_l_n;
    TreeNode* most_r_n;
};

template <CntrTplParamList>
SanitizeTreeRet_ SanitizeTree_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* ght, mem_recorder::MemRecorder* dst_node,
     unsigned long long salt, size_t bucket_size, size_t bucket_idx,
     TreeNode* n) {
    if (n == nullptr) { return { 0, nullptr, nullptr }; }

    auto* node{ ZETA_Core_MemberToStruct(Node, n, n) };

    ZETA_Core_DebugAssert(
        (GetBucketIdx_)(ght->node_hash(node, salt), bucket_size) == bucket_idx);

    if (dst_node != nullptr) {
        mem_recorder::ops::Record(dst_node, node, sizeof(Node));
    }

    TreeNode* nl{ bin_tree::ops::GetL(node->n) };
    TreeNode* nr{ bin_tree::ops::GetR(node->n) };

    SanitizeTreeRet_ l_ret{ SanitizeTree_(ght, dst_node, salt, bucket_size,
                                          bucket_idx, nl) };

    SanitizeTreeRet_ r_ret{ SanitizeTree_(ght, dst_node, salt, bucket_size,
                                          bucket_idx, nr) };

    SanitizeTreeRet_ ret{
        .cnt = l_ret.cnt + 1 + r_ret.cnt,
        .most_l_n = n,
        .most_r_n = n,
    };

    if (nl != nullptr) {
        int cmp{ utils::GetInstRef(ght->node_compare)(
            ZETA_Core_MemberToStruct(Node, n, l_ret.most_r_n), node) };

        ZETA_Core_DebugAssert(cmp <= 0);

        ret.most_l_n = l_ret.most_l_n;
    }

    if (nr != nullptr) {
        int cmp{ utils::GetInstRef(ght->node_compare)(
            node, ZETA_Core_MemberToStruct(Node, n, r_ret.most_l_n)) };

        ZETA_Core_DebugAssert(cmp <= 0);

        ret.most_r_n = r_ret.most_r_n;
    }

    return ret;
}

}  // namespace generic_hash_table::ops::detail

template <CntrTplParamList>
void generic_hash_table::ops::Sanitize(
    Cntr<CntrTplArgList> const* ght, mem_recorder::MemRecorder* dst_table_node,
    mem_recorder::MemRecorder* dst_node) {
    detail::CheckCntr_(ght);

    size_t cur_bucket_size{ ght->cur_bucket_size };
    size_t nxt_bucket_size{ ght->nxt_bucket_size };

    MLPT_CNTR cur_table{
        .level = utils::CeilLog(cur_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = utils::GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = utils::CeilLog(nxt_bucket_size, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = utils::GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    if (0 < cur_bucket_size) {
        multi_level_ptr_table::ops::Sanitize(&cur_table, dst_table_node);
    }

    if (0 < nxt_bucket_size) {
        multi_level_ptr_table::ops::Sanitize(&nxt_table, dst_table_node);
    }

    size_t total_size{ 0 };

    size_t idxes[max_level];

    if (0 < cur_bucket_size) {
        void** root_entry{ static_cast<void**>(
            multi_level_ptr_table::ops::FindFirst(&cur_table, idxes)) };

        while (root_entry != nullptr) {
            total_size +=
                bin_tree::ops::Count(static_cast<TreeNode*>(*root_entry));

            detail::SanitizeTree_(ght, dst_node, ght->cur_salt, cur_bucket_size,
                                  detail::GetIdx_(cur_table.level, idxes),
                                  static_cast<TreeNode*>(*root_entry));

            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindNext(&cur_table, idxes, false));
        }
    }

    if (0 < nxt_bucket_size) {
        void** root_entry{ static_cast<void**>(
            multi_level_ptr_table::ops::FindFirst(&nxt_table, idxes)) };

        while (root_entry != nullptr) {
            total_size +=
                bin_tree::ops::Count(static_cast<TreeNode*>(*root_entry));

            detail::SanitizeTree_(ght, dst_node, ght->nxt_salt, nxt_bucket_size,
                                  detail::GetIdx_(nxt_table.level, idxes),
                                  static_cast<TreeNode*>(*root_entry));

            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindNext(&nxt_table, idxes, false));
        }
    }

    ZETA_Core_DebugAssert(ght->size == total_size);
}

}  // namespace zeta::core

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("MLPT_CNTR")
