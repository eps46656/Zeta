#pragma once

#include <zeta/core/bin_tree_node_tpl.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/multi_level_ptr_table.ipp>
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

#define MLPT_CNTR                                    \
    multi_level_ptr_table::Cntr<decltype(GetInstPtr( \
        Declval<TableNodeAllocatorLike&>()))>

namespace zeta::core::generic_hash_table {

constexpr bool TreeNodeView::IsConst(type_wrapper::TypeWrapper<TreeNodeView*>) {
    return false;
}

constexpr bool TreeNodeView::IsConst(
    type_wrapper::TypeWrapper<TreeNodeView const*>) {
    return true;
}

constexpr bool TreeNodeView::IsAccSizeEnabled(
    type_wrapper::TypeWrapper<TreeNodeView const*>) {
    return false;
}

TreeNodeView* TreeNodeView::GetP(TreeNodeView* n) {
    return reinterpret_cast<TreeNodeView*>(
        reinterpret_cast<TreeNode*>(n)->GetPPtr());
}

TreeNodeView* TreeNodeView::GetL(TreeNodeView* n) {
    return reinterpret_cast<TreeNodeView*>(
        reinterpret_cast<TreeNode*>(n)->GetLPtr());
}

TreeNodeView* TreeNodeView::GetR(TreeNodeView* n) {
    return reinterpret_cast<TreeNodeView*>(
        reinterpret_cast<TreeNode*>(n)->GetRPtr());
}

TreeNodeView const* TreeNodeView::GetP(TreeNodeView const* n) {
    return reinterpret_cast<TreeNodeView const*>(
        reinterpret_cast<TreeNode const*>(n)->GetPPtr());
}

TreeNodeView const* TreeNodeView::GetL(TreeNodeView const* n) {
    return reinterpret_cast<TreeNodeView const*>(
        reinterpret_cast<TreeNode const*>(n)->GetLPtr());
}

TreeNodeView const* TreeNodeView::GetR(TreeNodeView const* n) {
    return reinterpret_cast<TreeNodeView const*>(
        reinterpret_cast<TreeNode const*>(n)->GetRPtr());
}

void TreeNodeView::SetP(TreeNodeView* n, TreeNodeView* m) {
    reinterpret_cast<TreeNode*>(n)->SetPPtr(reinterpret_cast<TreeNode*>(m));
}

void TreeNodeView::SetL(TreeNodeView* n, TreeNodeView* m) {
    reinterpret_cast<TreeNode*>(n)->SetLPtr(reinterpret_cast<TreeNode*>(m));
}

void TreeNodeView::SetR(TreeNodeView* n, TreeNodeView* m) {
    reinterpret_cast<TreeNode*>(n)->SetRPtr(reinterpret_cast<TreeNode*>(m));
}

unsigned int TreeNodeView::GetColor(TreeNodeView const* n) {
    return reinterpret_cast<TreeNode const*>(n)->GetPColor();
}

void TreeNodeView::SetColor(TreeNodeView* n, unsigned int color) {
    reinterpret_cast<TreeNode*>(n)->SetPColor(color);
}

void Node::Init() { this->n.Init(); }

namespace ops {

namespace detail {

inline size_t GetBucketIdx  // NOLINT(misc-use-internal-linkage)
    (unsigned long long hash_code, size_t capacity) {
    return ULLHash(hash_code, 0) % capacity;
}

inline size_t GetIdx  // NOLINT(misc-use-internal-linkage)
    (unsigned level, size_t const* idxes) {
    size_t idx{ 0 };

    for (unsigned level_i{ level }; 0 < level_i; --level_i) {
        idx = idx * branch_num + idxes[level_i - 1];
    }

    return idx;
}

inline void SetIdxes  // NOLINT(misc-use-internal-linkage)
    (unsigned level, size_t* idxes, size_t idx) {
    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        idxes[level_i] = idx % branch_num;
        idx /= branch_num;
    }
}

inline size_t FindPrvCapacity  // NOLINT(misc-use-internal-linkage)
    (size_t capacity) {
    int len{ sizeof(capacities) / sizeof(capacities[0]) };

    if (capacity <= capacities[0]) { return 0; }

    int lb{ 0 };
    int rb{ len - 1 };

    while (lb < rb) {
        int mb{ (lb + rb + 1) / 2 };

        if (capacities[mb] < capacity) {
            lb = mb;
        } else {
            rb = mb - 1;
        }
    }

    return capacities[lb];
}

inline size_t FindNxtCapacity  // NOLINT(misc-use-internal-linkage)
    (size_t capacity) {
    int len{ sizeof(capacities) / sizeof(capacities[0]) };

    int lb{ 0 };
    int rb{ len };

    while (lb < rb) {
        int mb{ (lb + rb) / 2 };

        if (capacity < capacities[mb]) {
            rb = mb;
        } else {
            lb = mb + 1;
        }
    }

    return lb < len ? capacities[lb] : 0;
}

template <typename TableNodeAllocatorLike, typename KeyHash,
          typename KeyNodeCompare>
Node* FindInTable  // NOLINT(misc-use-internal-linkage)
    (unsigned long long salt,
     multi_level_ptr_table::Cntr<TableNodeAllocatorLike>* table,
     size_t capacity, void const* key, KeyHash const& key_hash,
     KeyNodeCompare const& key_node_compare) {
    size_t idxes[max_level];

    SetIdxes(table->level, idxes, GetBucketIdx(key_hash(key, salt), capacity));

    TreeNode* target_n{ nullptr };

    void** tmp{ static_cast<void**>(
        multi_level_ptr_table::ops::Access(table, idxes)) };

    for (TreeNode* n{
             ({ tmp == nullptr ? nullptr : static_cast<TreeNode*>(*tmp); }) };
         n != nullptr;) {
        int cmp{ key_node_compare(key, ZETA_Core_MemberToStruct(Node, n, n)) };

        if (cmp == 0) { target_n = n; }

        if (cmp <= 0) {
            n = n->GetLPtr();
        } else {
            n = n->GetRPtr();
        }
    }

    return target_n == nullptr ? nullptr
                               : ZETA_Core_MemberToStruct(Node, n, target_n);
}

template <CntrTplParamList>
void InsertToTable  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* ght, unsigned long long salt, MLPT_CNTR* table,
     size_t capacity, Node* node) {
    size_t idxes[max_level];

    SetIdxes(table->level, idxes,
             GetBucketIdx(GetInstRef(ght->node_hash)(node, salt), capacity));

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
        int cmp{ GetInstRef(ght->node_compare)(
            node, ZETA_Core_MemberToStruct(Node, n, n)) };

        if (cmp < 0) {
            gt_n = n;
            n = n->GetLPtr();
        } else {
            le_n = n;
            n = n->GetRPtr();
        }
    }

    *root_entry = rbtree::Insert(reinterpret_cast<TreeNodeView*>(le_n),
                                 reinterpret_cast<TreeNodeView*>(gt_n),
                                 reinterpret_cast<TreeNodeView*>(&node->n));
}

template <CntrTplParamList>
bool TryExtractFromTable  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* ght, unsigned long long salt, MLPT_CNTR* table,
     size_t capacity, Node* node, TreeNode* root) {
    if (capacity == 0) { return false; }

    size_t idxes[max_level];

    detail::SetIdxes(
        table->level, idxes,
        detail::GetBucketIdx(GetInstRef(ght->node_hash)(node, salt), capacity));

    void** root_entry{ static_cast<void**>(
        multi_level_ptr_table::ops::Access(table, idxes)) };

    if (root_entry == nullptr || *root_entry != root) { return false; }

    void* new_root{ rbtree::Extract(
        reinterpret_cast<TreeNodeView*>(&node->n)) };

    *root_entry = new_root;

    if (new_root == nullptr) {
        multi_level_ptr_table::ops::Erase(table, idxes);
    }

    return true;
}

template <CntrTplParamList>
void TryTransfer  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* ght, MLPT_CNTR* cur_table,
     MLPT_CNTR* nxt_table, size_t quata) {
    if (cur_table->level == 0 || nxt_table->level == 0 ||
        cur_table->size == 0) {
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

        void* new_root{ rbtree::Extract(
            reinterpret_cast<TreeNodeView*>(&trans_node->n)) };

        *root_entry = new_root;

        if (new_root == nullptr) {
            multi_level_ptr_table::ops::Erase(cur_table, idxes);
            root_entry = nullptr;
        }

        InsertToTable(const_cast<Cntr<CntrTplArgList>*>(ght), ght->nxt_salt,
                      nxt_table, ght->nxt_capacity, trans_node);
    }
}

template <CntrTplParamList>
void TryRunPending  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* ght_, MLPT_CNTR* cur_table,
     MLPT_CNTR* nxt_table, size_t quata) {
    auto ght{ const_cast<Cntr<CntrTplArgList>*>(ght_) };

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    TryTransfer(ght, cur_table, nxt_table, quata);

    if (nxt_capacity == 0) {
        ght->cur_table_size = cur_table->size;
        ght->cur_table_root = cur_table->root;

        if (cur_table->size * 2 < cur_capacity) {
            ght->nxt_capacity = FindPrvCapacity(cur_capacity);
            ght->nxt_salt = GetRandom();
        } else if (cur_capacity * 8 <= ght->size) {
            ght->nxt_capacity = FindNxtCapacity(cur_capacity);
            ght->nxt_salt = GetRandom();
        }

        return;
    }

    if (0 < cur_table->size) {
        ght->cur_table_size = cur_table->size;
        ght->cur_table_root = cur_table->root;

        ght->nxt_table_size = nxt_table->size;
        ght->nxt_table_root = nxt_table->root;

        return;
    }

    if (nxt_table->size == 0) {
        ght->cur_salt = GetRandom();
        ght->cur_table_size = 0;
        ght->cur_capacity = capacities[0];
        ght->cur_table_root = nullptr;
    } else {
        ght->cur_salt = ght->nxt_salt;
        ght->cur_table_size = nxt_table->size;
        ght->cur_capacity = nxt_capacity;
        ght->cur_table_root = nxt_table->root;
    }

    ght->nxt_table_size = 0;
    ght->nxt_capacity = 0;
    ght->nxt_table_root = nullptr;
}

template <CntrTplParamList>
void CheckCntr  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* ght) {
    ZETA_Core_DebugAssert(ght != nullptr);

    size_t cur_capacity{ ght->cur_capacity };

    ZETA_Core_DebugAssert(cur_capacity != 0);
}

}  // namespace detail

template <CntrTplParamList>
void Init(Cntr<CntrTplArgList>* ght) {
    ZETA_Core_DebugAssert(ght != nullptr);

    ght->cur_salt = GetRandom();

    ght->cur_table_root = nullptr;
    ght->nxt_table_root = nullptr;

    ght->cur_table_size = 0;
    ght->nxt_table_size = 0;

    ght->cur_capacity = capacities[0];
    ght->nxt_capacity = 0;

    ght->size = 0;
}

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* ght) {
    ExtractAll(ght);
}

template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList> const* ght) {
    detail::CheckCntr(ght);

    return ght->size;
}

template <CntrTplParamList>
bool Contain(Cntr<CntrTplArgList> const* ght, Node const* node) {
    detail::CheckCntr(ght);

    if (node == nullptr) { return false; }

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    TreeNode const* root{ reinterpret_cast<TreeNode const*>(
        bin_tree::GetMostP(reinterpret_cast<TreeNodeView const*>(&node->n))
            .first) };

    size_t idxes[max_level];

    detail::SetIdxes(
        cur_table.level, idxes,
        detail::GetBucketIdx(GetInstRef(ght->node_hash)(node, ght->cur_salt),
                             cur_capacity));

    void** root_entry{ static_cast<void**>(
        multi_level_ptr_table::ops::Access(&cur_table, idxes)) };

    bool ret{ root_entry != nullptr && *root_entry == root };

    if (!ret && 0 < nxt_capacity) {
        detail::SetIdxes(
            nxt_table.level, idxes,
            detail::GetBucketIdx(
                GetInstRef(ght->node_hash)(node, ght->nxt_salt), nxt_capacity));

        root_entry = static_cast<void**>(
            multi_level_ptr_table::ops::Access(&nxt_table, idxes));

        ret = root_entry != nullptr && *root_entry == root;
    }

    detail::TryRunPending(ght, &cur_table, &nxt_table, 4);

    return ret;
}

template <CntrTplParamList, typename KeyHash, typename KeyNodeCompare>
Node* Find(Cntr<CntrTplArgList> const* ght, void const* key,
           KeyHash const& key_hash, KeyNodeCompare const& key_node_compare) {
    detail::CheckCntr(ght);

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    Node* ret{ nxt_capacity == 0 ? nullptr
                                 : detail::FindInTable(
                                       ght->nxt_salt, &nxt_table, nxt_capacity,
                                       key, key_hash, key_node_compare) };

    if (ret == nullptr) {
        ret = detail::FindInTable(ght->cur_salt, &cur_table, cur_capacity, key,
                                  key_hash, key_node_compare);
    }

    detail::TryRunPending(ght, &cur_table, &nxt_table, 4);

    return ret;
}

template <CntrTplParamList>
void Insert(Cntr<CntrTplArgList>* ght, Node* node) {
    detail::CheckCntr(ght);

    ZETA_Core_DebugAssert(node != nullptr);

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    if (0 < nxt_capacity) {
        detail::InsertToTable(ght, ght->nxt_salt, &nxt_table, nxt_capacity,
                              node);
    } else {
        detail::InsertToTable(ght, ght->cur_salt, &cur_table, cur_capacity,
                              node);
    }

    ++ght->size;

    detail::TryRunPending(ght, &cur_table, &nxt_table, 4);
}

template <CntrTplParamList>
void Extract(Cntr<CntrTplArgList>* ght, Node* node) {
    detail::CheckCntr(ght);

    ZETA_Core_DebugAssert(node != nullptr);

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    TreeNode* root{ reinterpret_cast<TreeNode*>(
        bin_tree::GetMostP(reinterpret_cast<TreeNodeView*>(&node->n)).first) };

    if (!detail::TryExtractFromTable(ght, ght->cur_salt, &cur_table,
                                     cur_capacity, node, root) &&
        !detail::TryExtractFromTable(ght, ght->nxt_salt, &nxt_table,
                                     nxt_capacity, node, root)) {
        ZETA_Core_DebugAssert(false);
    }

    --ght->size;

    detail::TryRunPending(ght, &cur_table, &nxt_table, 4);
}

template <CntrTplParamList>
Node* ExtractAny(Cntr<CntrTplArgList>* ght) {
    detail::CheckCntr(ght);

    if (ght->size == 0) { return nullptr; }

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    size_t idxes[max_level];

    void** root_entry{ static_cast<void**>(
        multi_level_ptr_table::ops::FindFirst(&cur_table, idxes)) };

    auto* node{ ZETA_Core_MemberToStruct(Node, n, *root_entry) };

    void* new_root{ rbtree::Extract(&node->n) };

    *root_entry = new_root;

    if (new_root == nullptr) {
        multi_level_ptr_table::ops::Erase(&cur_table, idxes);
    }

    --ght->size;

    TryRunPending(ght, &cur_table, &nxt_table, 4);

    return node;
}

template <CntrTplParamList>
void ExtractAll(Cntr<CntrTplArgList>* ght) {
    detail::CheckCntr(ght);

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    multi_level_ptr_table::ops::Deinit(&cur_table);

    if (0 < nxt_capacity) { multi_level_ptr_table::ops::Deinit(&nxt_table); }

    ght->cur_salt = GetRandom();

    ght->cur_table_root = nullptr;
    ght->nxt_table_root = nullptr;

    ght->cur_table_size = 0;
    ght->nxt_table_size = 0;

    ght->cur_capacity = capacities[0];

    ght->size = 0;
}

template <CntrTplParamList>
bool RunPending(Cntr<CntrTplArgList>* ght, size_t quata) {
    detail::CheckCntr(ght);

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    if (nxt_capacity == 0) { return false; }

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    TryRunPending(ght, &cur_table, &nxt_table, GetMaxOf(4ULL, quata));

    return 0 < nxt_capacity;
}

template <CntrTplParamList>
unsigned long long GetEffFactor(Cntr<CntrTplArgList> const* ght) {
    detail::CheckCntr(ght);

    if (ght->size == 0) { return 0; }

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(ght->table_node_alctr),
    };

    unsigned long long total_height{ 0 };

    size_t idxes[max_level];

    if (0 < cur_capacity) {
        void** root_entry{ static_cast<void**>(
            multi_level_ptr_table::ops::FindFirst(&cur_table, idxes)) };

        while (root_entry != nullptr) {
            size_t tree_size{ bin_tree::Count(
                static_cast<TreeNodeView*>(*root_entry)) };

            total_height += CeilLog2(tree_size) * tree_size;

            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindNext(&cur_table, idxes, false));
        }
    }

    if (0 < nxt_capacity) {
        void** root_entry{ static_cast<void**>(
            multi_level_ptr_table::ops::FindFirst(&nxt_table, idxes)) };

        while (root_entry != nullptr) {
            size_t tree_size{ bin_tree::Count(
                static_cast<TreeNodeView*>(*root_entry)) };

            total_height += CeilLog2(tree_size) * tree_size;

            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindNext(&nxt_table, idxes, false));
        }
    }

    return total_height * 1'000'000 / ght->size;
}

namespace detail {

struct SanitizeTreeRet {
    size_t cnt;
    TreeNode* most_l_n;
    TreeNode* most_r_n;
};

template <CntrTplParamList>
SanitizeTreeRet SanitizeTree  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* ght, MemRecorder* dst_node,
     unsigned long long salt, size_t capacity, size_t bucket_idx, TreeNode* n) {
    if (n == nullptr) { return { 0, nullptr, nullptr }; }

    auto* node{ ZETA_Core_MemberToStruct(Node, n, n) };

    ZETA_Core_DebugAssert(GetBucketIdx(ght->node_hash(node, salt), capacity) ==
                          bucket_idx);

    if (dst_node != nullptr) {
        MemRecorder::Record(dst_node, node, sizeof(Node));
    }

    TreeNode* nl{ node->n.GetLPtr() };
    TreeNode* nr{ node->n.GetRPtr() };

    SanitizeTreeRet l_ret{ SanitizeTree(ght, dst_node, salt, capacity,
                                        bucket_idx, nl) };

    SanitizeTreeRet r_ret{ SanitizeTree(ght, dst_node, salt, capacity,
                                        bucket_idx, nr) };

    SanitizeTreeRet ret{
        .cnt = l_ret.cnt + 1 + r_ret.cnt,
        .most_l_n = n,
        .most_r_n = n,
    };

    if (nl != nullptr) {
        int cmp{ GetInstRef(ght->node_compare)(
            ZETA_Core_MemberToStruct(Node, n, l_ret.most_r_n), node) };

        ZETA_Core_DebugAssert(cmp <= 0);

        ret.most_l_n = l_ret.most_l_n;
    }

    if (nr != nullptr) {
        int cmp{ GetInstRef(ght->node_compare)(
            node, ZETA_Core_MemberToStruct(Node, n, r_ret.most_l_n)) };

        ZETA_Core_DebugAssert(cmp <= 0);

        ret.most_r_n = r_ret.most_r_n;
    }

    return ret;
}

}  // namespace detail

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList> const* ght, MemRecorder* dst_table_node,
              MemRecorder* dst_node) {
    detail::CheckCntr(ght);

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT_CNTR cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_alctr = GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    MLPT_CNTR nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_alctr = GetInstPtr(
            const_cast<Cntr<CntrTplArgList>*>(ght)->table_node_alctr),
    };

    if (0 < cur_capacity) {
        multi_level_ptr_table::ops::Sanitize(&cur_table, dst_table_node);
    }

    if (0 < nxt_capacity) {
        multi_level_ptr_table::ops::Sanitize(&nxt_table, dst_table_node);
    }

    size_t total_size{ 0 };

    size_t idxes[max_level];

    if (0 < cur_capacity) {
        void** root_entry{ static_cast<void**>(
            multi_level_ptr_table::ops::FindFirst(&cur_table, idxes)) };

        while (root_entry != nullptr) {
            total_size +=
                bin_tree::Count(static_cast<TreeNodeView*>(*root_entry));

            detail::SanitizeTree(ght, dst_node, ght->cur_salt, cur_capacity,
                                 detail::GetIdx(cur_table.level, idxes),
                                 static_cast<TreeNode*>(*root_entry));

            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindNext(&cur_table, idxes, false));
        }
    }

    if (0 < nxt_capacity) {
        void** root_entry{ static_cast<void**>(
            multi_level_ptr_table::ops::FindFirst(&nxt_table, idxes)) };

        while (root_entry != nullptr) {
            total_size +=
                bin_tree::Count(static_cast<TreeNodeView*>(*root_entry));

            detail::SanitizeTree(ght, dst_node, ght->nxt_salt, nxt_capacity,
                                 detail::GetIdx(nxt_table.level, idxes),
                                 static_cast<TreeNode*>(*root_entry));

            root_entry = static_cast<void**>(
                multi_level_ptr_table::ops::FindNext(&nxt_table, idxes, false));
        }
    }

    ZETA_Core_DebugAssert(ght->size == total_size);
}

}  // namespace ops

}  // namespace zeta::core::generic_hash_table

#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("MLPT_CNTR")
