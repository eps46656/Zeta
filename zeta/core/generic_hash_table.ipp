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

namespace zeta::core {

void GenericHashTableNode::Init() { this->n.Init(); }

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::TreeNode*
GenericHashTable<NodeHash, NodeCompare,
                 TableNodeAllocator>::TreeNodeOperator::GetP(TreeNode* n)
    const {
    return n->GetPPtr();
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::TreeNode*
GenericHashTable<NodeHash, NodeCompare,
                 TableNodeAllocator>::TreeNodeOperator::GetL(TreeNode* n)
    const {
    return n->GetLPtr();
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::TreeNode*
GenericHashTable<NodeHash, NodeCompare,
                 TableNodeAllocator>::TreeNodeOperator::GetR(TreeNode* n)
    const {
    return n->GetRPtr();
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::TreeNode const*
GenericHashTable<NodeHash, NodeCompare,
                 TableNodeAllocator>::TreeNodeOperator::GetP(TreeNode const* n)
    const {
    return n->GetPPtr();
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::TreeNode const*
GenericHashTable<NodeHash, NodeCompare,
                 TableNodeAllocator>::TreeNodeOperator::GetL(TreeNode const* n)
    const {
    return n->GetLPtr();
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::TreeNode const*
GenericHashTable<NodeHash, NodeCompare,
                 TableNodeAllocator>::TreeNodeOperator::GetR(TreeNode const* n)
    const {
    return n->GetRPtr();
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare,
                      TableNodeAllocator>::TreeNodeOperator::SetP(TreeNode* n,
                                                                  TreeNode* m)
    const {
    n->SetPPtr(m);
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare,
                      TableNodeAllocator>::TreeNodeOperator::SetL(TreeNode* n,
                                                                  TreeNode* m)
    const {
    n->SetLPtr(m);
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare,
                      TableNodeAllocator>::TreeNodeOperator::SetR(TreeNode* n,
                                                                  TreeNode* m)
    const {
    n->SetRPtr(m);
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
unsigned int GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::
    TreeNodeOperator::GetColor(TreeNode* n) const {
    return n->GetPColor();
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::
    TreeNodeOperator::SetColor(TreeNode* n, unsigned int color) const {
    n->SetPColor(color);
}

// -----------------------------------------------------------------------------

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
size_t
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::GetBucketIdx_(
    unsigned long long hash_code, size_t capacity) {
    return ULLHash(hash_code, 0) % capacity;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
size_t GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::GetIdx_(
    unsigned level, size_t const* idxes) {
    size_t idx{ 0 };

    for (unsigned level_i{ level }; 0 < level_i; --level_i) {
        idx = idx * branch_num + idxes[level_i - 1];
    }

    return idx;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::SetIdxes_(
    unsigned level, size_t* idxes, size_t idx) {
    for (unsigned level_i{ 0 }; level_i < level; ++level_i) {
        idxes[level_i] = idx % branch_num;
        idx /= branch_num;
    }
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
size_t GenericHashTable<NodeHash, NodeCompare,
                        TableNodeAllocator>::FindPrvCapacity_(size_t capacity) {
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

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
size_t GenericHashTable<NodeHash, NodeCompare,
                        TableNodeAllocator>::FindNxtCapacity_(size_t capacity) {
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

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
template <typename KeyHash, typename KeyNodeCompare>
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Node*
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Find_(
    unsigned long long salt, MLPT* table, size_t capacity, void const* key,
    KeyHash const& key_hash, KeyNodeCompare const& key_node_compare) {
    size_t idxes[max_level];

    SetIdxes_(table->level, idxes,
              GetBucketIdx_(key_hash(key, salt), capacity));

    TreeNode* target_n{ nullptr };

    void** tmp{ static_cast<void**>(MLPT::Access(table, idxes)) };

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

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Insert_(
    GenericHashTable* ght, unsigned long long salt, MLPT* table,
    size_t capacity, Node* node) {
    size_t idxes[max_level];

    SetIdxes_(table->level, idxes,
              GetBucketIdx_(ght->node_hash(node, salt), capacity));

    auto p{ MLPT::Insert(table, idxes) };

    void** root_entry{ static_cast<void**>(p.first) };

    if (p.second) {
        *root_entry = &node->n;
        return;
    }

    TreeNode* n{ static_cast<TreeNode*>(*root_entry) };

    TreeNode* le_n{ nullptr };
    TreeNode* gt_n{ nullptr };

    while (n != nullptr) {
        int cmp{ ght->node_compare(node,
                                   ZETA_Core_MemberToStruct(Node, n, n)) };

        if (cmp < 0) {
            gt_n = n;
            n = n->GetLPtr();
        } else {
            le_n = n;
            n = n->GetRPtr();
        }
    }

    *root_entry = rbtree::Insert(tn_opr, le_n, gt_n, &node->n);
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
bool GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::TryExtract_(
    GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>* ght,
    unsigned long long salt, MLPT* table, size_t capacity, Node* node,
    TreeNode* root) {
    if (capacity == 0) { return false; }

    size_t idxes[max_level];

    SetIdxes_(table->level, idxes,
              GetBucketIdx_(ght->node_hash(node, salt), capacity));

    void** root_entry{ static_cast<void**>(MLPT::Access(table, idxes)) };

    if (root_entry == nullptr || *root_entry != root) { return false; }

    void* new_root{ rbtree::Extract(tn_opr, &node->n) };

    *root_entry = new_root;

    if (new_root == nullptr) { MLPT::Erase(table, idxes); }

    return true;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::TryTransfer_(
    GenericHashTable const* ght, MLPT* cur_table, MLPT* nxt_table,
    size_t quata) {
    if (cur_table->level == 0 || nxt_table->level == 0 ||
        cur_table->size == 0) {
        return;
    }

    size_t idxes[max_level];

    void** root_entry{ nullptr };

    for (; 0 < quata && 0 < cur_table->size; --quata) {
        if (root_entry == nullptr) {
            root_entry = static_cast<void**>(MLPT::FindFirst(cur_table, idxes));
        }

        auto* trans_node{ ZETA_Core_MemberToStruct(Node, n, *root_entry) };

        void* new_root{ rbtree::Extract(tn_opr, &trans_node->n) };

        *root_entry = new_root;

        if (new_root == nullptr) {
            MLPT::Erase(cur_table, idxes);
            root_entry = nullptr;
        }

        Insert_(const_cast<GenericHashTable*>(ght), ght->nxt_salt, nxt_table,
                ght->nxt_capacity, trans_node);
    }
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::
    TryRunPending_(GenericHashTable const* ght_, MLPT* cur_table,
                   MLPT* nxt_table, size_t quata) {
    auto ght{ const_cast<GenericHashTable*>(ght_) };

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    TryTransfer_(ght, cur_table, nxt_table, quata);

    if (nxt_capacity == 0) {
        ght->cur_table_size = cur_table->size;
        ght->cur_table_root = cur_table->root;

        if (cur_table->size * 2 < cur_capacity) {
            ght->nxt_capacity = FindPrvCapacity_(cur_capacity);
            ght->nxt_salt = GetRandom();
        } else if (cur_capacity * 8 <= ght->size) {
            ght->nxt_capacity = FindNxtCapacity_(cur_capacity);
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

// -----------------------------------------------------------------------------

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Init(
    void* ght_) {
    auto ght{ static_cast<GenericHashTable*>(ght_) };
    ZETA_Core_DebugAssert(ght != nullptr);

    ght->cur_salt = GetRandom();

    ght->cur_table_root = nullptr;
    ght->nxt_table_root = nullptr;

    ght->cur_table_size = 0;
    ght->nxt_table_size = 0;

    ght->cur_capacity = capacities[0];
    ght->nxt_capacity = 0;

    ght->size = 0;

    TableNodeAllocator::CheckAllocator(&ght->table_node_allocator);
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Deinit(
    void* ght) {
    ExtractAll(ght);
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
size_t GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::GetSize(
    void const* ght_) {
    auto ght{ static_cast<GenericHashTable const*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    return ght->size;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
bool GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Contain(
    void const* ght_, void const* node_) {
    auto ght{ static_cast<GenericHashTable const*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    auto const* node{ static_cast<Node const*>(node_) };

    if (node == nullptr) { return false; }

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    TreeNode const* root{ bin_tree::GetMostP(tn_opr, &node->n).first };

    size_t idxes[max_level];

    SetIdxes_(cur_table.level, idxes,
              GetBucketIdx_(ght->node_hash(node, ght->cur_salt), cur_capacity));

    void** root_entry{ static_cast<void**>(MLPT::Access(&cur_table, idxes)) };

    bool ret{ root_entry != nullptr && *root_entry == root };

    if (!ret && 0 < nxt_capacity) {
        SetIdxes_(
            nxt_table.level, idxes,
            GetBucketIdx_(ght->node_hash(node, ght->nxt_salt), nxt_capacity));

        root_entry = static_cast<void**>(MLPT::Access(&nxt_table, idxes));

        ret = root_entry != nullptr && *root_entry == root;
    }

    TryRunPending_(ght, &cur_table, &nxt_table, 4);

    return ret;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
template <typename KeyHash, typename KeyNodeCompare>
void* GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Find(
    void const* ght_, void const* key, KeyHash const& key_hash,
    KeyNodeCompare const& key_node_compare) {
    auto ght{ static_cast<GenericHashTable const*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    void* ret{ nxt_capacity == 0
                   ? nullptr
                   : Find_(ght->nxt_salt, &nxt_table, nxt_capacity, key,
                           key_hash, key_node_compare) };

    if (ret == nullptr) {
        ret = Find_(ght->cur_salt, &cur_table, cur_capacity, key, key_hash,
                    key_node_compare);
    }

    TryRunPending_(ght, &cur_table, &nxt_table, 4);

    return ret;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Insert(
    void* ght_, void* node_) {
    auto ght{ static_cast<GenericHashTable*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    auto* node{ static_cast<Node*>(node_) };
    ZETA_Core_DebugAssert(node != nullptr);

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    if (0 < nxt_capacity) {
        Insert_(ght, ght->nxt_salt, &nxt_table, nxt_capacity, node);
    } else {
        Insert_(ght, ght->cur_salt, &cur_table, cur_capacity, node);
    }

    ++ght->size;

    TryRunPending_(ght, &cur_table, &nxt_table, 4);
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Extract(
    void* ght_, void* node_) {
    auto ght{ static_cast<GenericHashTable*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    auto* node{ static_cast<Node*>(node_) };
    ZETA_Core_DebugAssert(node != nullptr);

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    TreeNode* root{ bin_tree::GetMostP(tn_opr, &node->n).first };

    if (!TryExtract_(ght, ght->cur_salt, &cur_table, cur_capacity, node,
                     root) &&
        !TryExtract_(ght, ght->nxt_salt, &nxt_table, nxt_capacity, node,
                     root)) {
        ZETA_Core_DebugAssert(false);
    }

    --ght->size;

    TryRunPending_(ght, &cur_table, &nxt_table, 4);
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void* GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::ExtractAny(
    void* ght_) {
    auto ght{ static_cast<GenericHashTable*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    if (ght->size == 0) { return nullptr; }

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    size_t idxes[max_level];

    void** root_entry{ static_cast<void**>(
        MLPT::FindFirst(&cur_table, idxes)) };

    auto* node{ ZETA_Core_MemberToStruct(Node, n, *root_entry) };

    void* new_root{ rbtree::Extract(tn_opr, &node->n) };

    *root_entry = new_root;

    if (new_root == nullptr) { MLPT::Erase(&cur_table, idxes); }

    --ght->size;

    TryRunPending_(ght, &cur_table, &nxt_table, 4);

    return node;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::ExtractAll(
    void* ght_) {
    auto ght{ static_cast<GenericHashTable*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT::Deinit(&cur_table);

    if (0 < nxt_capacity) { MLPT::Deinit(&nxt_table); }

    ght->cur_salt = GetRandom();

    ght->cur_table_root = nullptr;
    ght->nxt_table_root = nullptr;

    ght->cur_table_size = 0;
    ght->nxt_table_size = 0;

    ght->cur_capacity = capacities[0];

    ght->size = 0;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
bool GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::RunPending(
    void* ght_, size_t quata) {
    auto ght{ static_cast<GenericHashTable*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    if (nxt_capacity == 0) { return false; }

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    TryRunPending_(ght, &cur_table, &nxt_table, GetMaxOf(4ULL, quata));

    return 0 < nxt_capacity;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
unsigned long long
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::GetEffFactor(
    void const* ght_) {
    auto ght{ static_cast<GenericHashTable*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    if (ght->size == 0) { return 0; }

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    unsigned long long total_height{ 0 };

    size_t idxes[max_level];

    if (0 < cur_capacity) {
        void** root_entry{ static_cast<void**>(
            MLPT::FindFirst(&cur_table, idxes)) };

        while (root_entry != nullptr) {
            size_t tree_size{ bin_tree::Count(
                tn_opr, static_cast<TreeNode*>(*root_entry)) };

            total_height += CeilLog2(tree_size) * tree_size;

            root_entry =
                static_cast<void**>(MLPT::FindNext(&cur_table, idxes, false));
        }
    }

    if (0 < nxt_capacity) {
        void** root_entry{ static_cast<void**>(
            MLPT::FindFirst(&nxt_table, idxes)) };

        while (root_entry != nullptr) {
            size_t tree_size{ bin_tree::Count(
                tn_opr, static_cast<TreeNode*>(*root_entry)) };

            total_height += CeilLog2(tree_size) * tree_size;

            root_entry =
                static_cast<void**>(MLPT::FindNext(&nxt_table, idxes, false));
        }
    }

    return total_height * 1'000'000 / ght->size;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
bool GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::CheckCntr(
    void const* ght_) {
    auto ght{ static_cast<GenericHashTable const*>(ght_) };
    if (!(ght != nullptr)) { return false; }

    size_t cur_capacity{ ght->cur_capacity };

    if (!(cur_capacity != 0)) { return false; }

    if (!TableNodeAllocator::CheckAllocator(&ght->table_node_allocator)) {
        return false;
    }

    return true;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::SanitizeTreeRet
GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::SanitizeTree_(
    GenericHashTable const* ght, MemRecorder* dst_node, unsigned long long salt,
    size_t capacity, size_t bucket_idx, TreeNode* n) {
    if (n == nullptr) { return { 0, nullptr, nullptr }; }

    auto* node{ ZETA_Core_MemberToStruct(Node, n, n) };

    ZETA_Core_DebugAssert(GetBucketIdx_(ght->node_hash(node, salt), capacity) ==
                          bucket_idx);

    if (dst_node != nullptr) {
        MemRecorder::Record(dst_node, node, sizeof(Node));
    }

    TreeNode* nl{ node->n.GetLPtr() };
    TreeNode* nr{ node->n.GetRPtr() };

    SanitizeTreeRet l_ret{ SanitizeTree_(ght, dst_node, salt, capacity,
                                         bucket_idx, nl) };

    SanitizeTreeRet r_ret{ SanitizeTree_(ght, dst_node, salt, capacity,
                                         bucket_idx, nr) };

    SanitizeTreeRet ret{
        .cnt = l_ret.cnt + 1 + r_ret.cnt,
        .most_l_n = n,
        .most_r_n = n,
    };

    if (nl != nullptr) {
        int cmp{ ght->node_compare(
            ZETA_Core_MemberToStruct(Node, n, l_ret.most_r_n), node) };

        ZETA_Core_DebugAssert(cmp <= 0);

        ret.most_l_n = l_ret.most_l_n;
    }

    if (nr != nullptr) {
        int cmp{ ght->node_compare(
            node, ZETA_Core_MemberToStruct(Node, n, r_ret.most_l_n)) };

        ZETA_Core_DebugAssert(cmp <= 0);

        ret.most_r_n = r_ret.most_r_n;
    }

    return ret;
}

template <typename NodeHash, typename NodeCompare, typename TableNodeAllocator>
void GenericHashTable<NodeHash, NodeCompare, TableNodeAllocator>::Sanitize(
    void const* ght_, MemRecorder* dst_table_node, MemRecorder* dst_node) {
    auto ght{ static_cast<GenericHashTable const*>(ght_) };
    ZETA_Core_DebugAssert(CheckCntr(ght));

    size_t cur_capacity{ ght->cur_capacity };
    size_t nxt_capacity{ ght->nxt_capacity };

    MLPT cur_table{
        .level = CeilLog(cur_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->cur_table_size,
        .root = ght->cur_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    MLPT nxt_table{
        .level = CeilLog(nxt_capacity, branch_num),
        .branch_nums = branch_nums.elems,
        .size = ght->nxt_table_size,
        .root = ght->nxt_table_root,
        .nav_node_allocator = ght->table_node_allocator,
    };

    if (0 < cur_capacity) { MLPT::Sanitize(&cur_table, dst_table_node); }

    if (0 < nxt_capacity) { MLPT::Sanitize(&nxt_table, dst_table_node); }

    size_t total_size{ 0 };

    size_t idxes[max_level];

    if (0 < cur_capacity) {
        void** root_entry{ static_cast<void**>(
            MLPT::FindFirst(&cur_table, idxes)) };

        while (root_entry != nullptr) {
            total_size +=
                bin_tree::Count(tn_opr, static_cast<TreeNode*>(*root_entry));

            SanitizeTree_(ght, dst_node, ght->cur_salt, cur_capacity,
                          GetIdx_(cur_table.level, idxes),
                          static_cast<TreeNode*>(*root_entry));

            root_entry =
                static_cast<void**>(MLPT::FindNext(&cur_table, idxes, false));
        }
    }

    if (0 < nxt_capacity) {
        void** root_entry{ static_cast<void**>(
            MLPT::FindFirst(&nxt_table, idxes)) };

        while (root_entry != nullptr) {
            total_size +=
                bin_tree::Count(tn_opr, static_cast<TreeNode*>(*root_entry));

            SanitizeTree_(ght, dst_node, ght->nxt_salt, nxt_capacity,
                          GetIdx_(nxt_table.level, idxes),
                          static_cast<TreeNode*>(*root_entry));

            root_entry =
                static_cast<void**>(MLPT::FindNext(&nxt_table, idxes, false));
        }
    }

    ZETA_Core_DebugAssert(ght->size == total_size);
}

}  // namespace zeta::core
