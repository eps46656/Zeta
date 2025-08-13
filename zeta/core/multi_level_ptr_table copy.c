#include <zeta/core/debugger.h>
#include <zeta/core/multi_level_ptr_table.h>

#if ZETA_Core_EnableDebug

#define CheckCntr_(cntr) Zeta_Core_MultiLevelPtrTable_Check(cntr)

#define CheckIdxes_(cntr, idxes) \
    Zeta_Core_MultiLevelPtrTable_CheckIdxes(cntr, idxes)

#else

#define CheckCntr_(cntr)

#define CheckIdxes_(cntr, idxes)

#endif

#define TestActiveMap_(active_map, idx) ((active_map) & ((1ULL) << (idx)) != 0)

/*
static int FindPrevActive_(unsigned long long active_map, int idx) {
    if (idx == -1) { return -1; }

    active_map = active_map << (ZETA_Core_ullong_width - 1 - idx) >>
                 (ZETA_Core_ullong_width - 1 - idx);

    return active_map == 0
               ? -1
               : ZETA_Core_ullong_width - 1 - __builtin_clzll(active_map);
}

static int FindNextActive_(unsigned long long active_map, int idx) {
    if (idx == ZETA_Core_ullong_width) { return ZETA_Core_ullong_width; }

    active_map = active_map >> idx << idx;

    return active_map == 0 ? ZETA_Core_ullong_width
                           : __builtin_ctzll(active_map);
}
*/

void Zeta_Core_MultiLevelPtrTable_Init(void* mlpt_) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    ZETA_Core_DebugAssert(mlpt != NULL);

    int level = mlpt->level;
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= ZETA_Core_MultiLevelPtrTable_max_level);

    unsigned short const* branch_nums = mlpt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        int branch_num = branch_nums[level_i];

        ZETA_Core_DebugAssert(ZETA_Core_MultiLevelPtrTable_min_branch_num <=
                              branch_num);
        ZETA_Core_DebugAssert(branch_num <=
                              ZETA_Core_MultiLevelPtrTable_max_branch_num);
    }

    mlpt->root = NULL;

    ZETA_Core_DebugAssert(mlpt->node_allocator.vtable != NULL);
    ZETA_Core_DebugAssert(mlpt->node_allocator.vtable->GetAlign != NULL);
    ZETA_Core_DebugAssert(ZETA_Core_Allocator_GetAlign(mlpt->node_allocator) %
                              alignof(Zeta_Core_MultiLevelPtrTable_Node) ==
                          0);
}

void Zeta_Core_MultiLevelPtrTable_Deinit(void* mlpt) {
    Zeta_Core_MultiLevelPtrTable_EraseAll(mlpt);
}

size_t Zeta_Core_MultiLevelPtrTable_GetCapacity(void* mlpt_) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckCntr_(mlpt);

    size_t ret = 1;

    int level = mlpt->level;
    unsigned short const* branch_nums = mlpt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        if (__builtin_umulll_overflow(ret, branch_nums[level_i], &ret)) {
            return ZETA_Core_max_capacity;
        }
    }

    return ret;
}

void** Zeta_Core_MultiLevelPtrTable_Access(void* mlpt_, size_t* idxes) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckIdxes_(mlpt, idxes);

    int level = mlpt->level;

    Zeta_Core_MultiLevelPtrTable_Node* node = mlpt->root;

    if (node == NULL) { return NULL; }

    for (int level_i = level - 1; 0 < level_i; --level_i) {
        int cur_idx = idxes[level_i];
        if (!TestActiveMap_(node->active_map, cur_idx)) { return NULL; }
        node = node->ptrs[cur_idx];
    }

    int last_idx = idxes[0];

    return TestActiveMap_(node->active_map, last_idx) ? node->ptrs + last_idx
                                                      : NULL;
}

void** Zeta_Core_MultiLevelPtrTable_FindFirst(void* mlpt_, size_t* dst_idxes) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckCntr_(mlpt);

    int level = mlpt->level;

    size_t idxes[ZETA_Core_MultiLevelPtrTable_max_level];

    for (int level_i = 0; level_i < level; ++level_i) { idxes[level_i] = 0; }

    void** ret = Zeta_Core_MultiLevelPtrTable_FindNext(mlpt, idxes, TRUE);

    if (dst_idxes != NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

void** Zeta_Core_MultiLevelPtrTable_FindLast(void* mlpt_, size_t* dst_idxes) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckCntr_(mlpt);

    int level = mlpt->level;
    unsigned short const* branch_nums = mlpt->branch_nums;

    size_t idxes[ZETA_Core_MultiLevelPtrTable_max_level];

    for (int level_i = 0; level_i < level; ++level_i) {
        idxes[level_i] = branch_nums[level_i] - 1;
    }

    void** ret = Zeta_Core_MultiLevelPtrTable_FindPrev(mlpt, idxes, TRUE);

    if (dst_idxes != NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

void** Zeta_Core_MultiLevelPtrTable_FindPrev(void* mlpt_, size_t* idxes,
                                             bool_t included) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckIdxes_(mlpt, idxes);

    int level = mlpt->level;
    unsigned short const* branch_nums = mlpt->branch_nums;

    if (!included) {
        for (int level_i = 0; level_i < level; ++level_i) {
            if (0 < idxes[level_i]) {
                --idxes[level_i];
                goto L1;
            }

            idxes[level_i] = branch_nums[level_i] - 1;
        }

        return NULL;
    }

L1:

    Zeta_Core_MultiLevelPtrTable_Node* root = mlpt->root;

    if (root == NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            idxes[level_i] = branch_nums[level_i] - 1;
        }

        return NULL;
    }

    Zeta_Core_MultiLevelPtrTable_Node*
        nodes[ZETA_Core_MultiLevelPtrTable_max_level];

    int level_i = level - 1;
    Zeta_Core_MultiLevelPtrTable_Node* node = root;

    for (;; --level_i) {
        nodes[level_i] = node;
        if (!TestActiveMap_(node->active_map, idxes[level_i])) { break; }

        void** nxt_node = node->ptrs + idxes[level_i];
        if (level_i == 0) { return nxt_node; }

        node = *nxt_node;
    }

    for (; level_i < level; ++level_i) {
        node = nodes[level_i];

        int found_idx = Zeta_Core_FindPrevOne(node->active_map, idxes[level_i]);

        if (0 <= found_idx) {
            idxes[level_i] = found_idx;
            break;
        }
    }

    if (level_i == level) {
        for (int level_i = 0; level_i < level; ++level_i) {
            idxes[level_i] = branch_nums[level_i] - 1;
        }

        return NULL;
    }

    while (0 < level_i) {
        node = node->ptrs[idxes[level_i]];
        --level_i;
        nodes[level_i] = node;
        idxes[level_i] =
            Zeta_Core_FindPrevOne(node->active_map, branch_nums[level_i]);
    }

    return nodes[0]->ptrs + idxes[0];
}

void** Zeta_Core_MultiLevelPtrTable_FindNext(void* mlpt_, size_t* idxes,
                                             bool_t included) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckIdxes_(mlpt, idxes);

    int level = mlpt->level;
    unsigned short const* branch_nums = mlpt->branch_nums;

    if (!included) {
        for (int level_i = 0; level_i < level; ++level_i) {
            if (idxes[level_i] < branch_nums[level_i] - 1) {
                ++idxes[level_i];
                goto L1;
            }

            idxes[level_i] = 0;
        }

        return NULL;
    }

L1:

    Zeta_Core_MultiLevelPtrTable_Node* root = mlpt->root;

    if (root == NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            idxes[level_i] = 0;
        }

        return NULL;
    }

    Zeta_Core_MultiLevelPtrTable_Node*
        nodes[ZETA_Core_MultiLevelPtrTable_max_level];

    int level_i = level - 1;
    Zeta_Core_MultiLevelPtrTable_Node* node = root;

    for (;; --level_i) {
        nodes[level_i] = node;
        if (!TestActiveMap_(node->active_map, idxes[level_i])) { break; }

        void** nxt_node = node->ptrs + idxes[level_i];
        if (level_i == 0) { return nxt_node; }

        node = *nxt_node;
    }

    for (; level_i < level; ++level_i) {
        node = nodes[level_i];

        int found_idx = Zeta_Core_FindNextOne(node->active_map, idxes[level_i]);

        if (0 <= found_idx) {
            idxes[level_i] = found_idx;
            break;
        }
    }

    if (level_i == level) {
        for (int level_i = 0; level_i < level; ++level_i) {
            idxes[level_i] = 0;
        }

        return NULL;
    }

    while (0 < level_i) {
        node = node->ptrs[idxes[level_i]];
        --level_i;
        nodes[level_i] = node;
        idxes[level_i] = Zeta_Core_FindNextOne(node->active_map, -1);
    }

    return nodes[0]->ptrs + idxes[0];
}

static Zeta_Core_MultiLevelPtrTable_Node* AllocateNode_(
    int branch_num, Zeta_Core_Allocator node_allocator) {
    Zeta_Core_MultiLevelPtrTable_Node* node = ZETA_Core_Allocator_SafeAllocate(
        node_allocator, alignof(Zeta_Core_MultiLevelPtrTable_Node),
        offsetof(Zeta_Core_MultiLevelPtrTable_Node, ptrs[branch_num]));

    node->active_map = 0;

    return node;
}

Zeta_Core_MultiLevelPtrTable_InsertResult Zeta_Core_MultiLevelPtrTable_Insert(
    void* mlpt_, size_t* idxes) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckIdxes_(mlpt, idxes);

    int level = mlpt->level;
    unsigned short const* branch_nums = mlpt->branch_nums;

    if (mlpt->root == NULL) {
        mlpt->root = AllocateNode_(branch_nums[0], mlpt->node_allocator);
    }

    Zeta_Core_MultiLevelPtrTable_Node* node = mlpt->root;

    int level_i = level - 1;

    for (; 0 < level_i && TestActiveMap_(node->active_map, idxes[level_i]);
         --level_i) {
        node = node->ptrs[idxes[level_i]];
    }

    for (; 0 < level_i; --level_i) {
        node->active_map = node->active_map + (1ULL << idxes[level_i]);

        node = node->ptrs[idxes[level_i]] =
            AllocateNode_(branch_nums[level_i + 1], mlpt->node_allocator);
    }

    void** ret = node->ptrs + idxes[0];

    if (TestActiveMap_(node->active_map, idxes[0])) {
        return (Zeta_Core_MultiLevelPtrTable_InsertResult){ .addr = ret,
                                                            .inserted = FALSE };
    }

    node->active_map = node->active_map + (1ULL << idxes[0]);
    *ret = NULL;

    return (Zeta_Core_MultiLevelPtrTable_InsertResult){ .addr = ret,
                                                        .inserted = TRUE };
}

Zeta_Core_MultiLevelPtrTable_EraseResult Zeta_Core_MultiLevelPtrTable_Erase(
    void* mlpt_, size_t* idxes) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckIdxes_(mlpt, idxes);

    int level = mlpt->level;

    Zeta_Core_MultiLevelPtrTable_Node* node = mlpt->root;

    if (node == NULL) {
        return (Zeta_Core_MultiLevelPtrTable_EraseResult){ .ptr = NULL,
                                                           .erased = FALSE };
    }

    Zeta_Core_MultiLevelPtrTable_Node*
        nodes[ZETA_Core_MultiLevelPtrTable_max_level];

    for (int level_i = level - 1;; --level_i) {
        nodes[level_i] = node;

        if (level_i == 0) { break; }

        if (!TestActiveMap_(node->active_map, idxes[level_i])) {
            (Zeta_Core_MultiLevelPtrTable_EraseResult){ .ptr = NULL,
                                                        .erased = FALSE };
        }

        node = node->ptrs[idxes[level_i]];
    }

    if (!TestActiveMap_(node->active_map, idxes[0])) {
        (Zeta_Core_MultiLevelPtrTable_EraseResult){ .ptr = NULL,
                                                    .erased = FALSE };
    }

    void* ret = node->ptrs[idxes[0]];

    for (int level_i = 0; level_i < level; ++level_i) {
        node = nodes[level_i];

        node->active_map = node->active_map - (1ULL << idxes[level_i]);

        if (node->active_map != 0) {
            (Zeta_Core_MultiLevelPtrTable_EraseResult){ .ptr = ret,
                                                        .erased = TRUE };
        }

        ZETA_Core_Allocator_Deallocate(mlpt->node_allocator, node);
    }

    mlpt->root = NULL;

    return (Zeta_Core_MultiLevelPtrTable_EraseResult){ .ptr = ret,
                                                       .erased = TRUE };
}

static void EraseAll_(int level, unsigned short const* branch_nums,
                      Zeta_Core_MultiLevelPtrTable_Node* node,
                      Zeta_Core_Allocator node_allocator) {
    if (level == 0) {
        ZETA_Core_Allocator_Deallocate(node_allocator, node);
        return;
    }

    int branch_num = branch_nums[level];

    for (int idx = 0; idx < branch_num; ++idx) {
        if (TestActiveMap_(node->active_map, idx)) {
            EraseAll_(level - 1, branch_nums, node->ptrs[idx], node_allocator);
        }
    }

    ZETA_Core_Allocator_Deallocate(node_allocator, node);
}

void Zeta_Core_MultiLevelPtrTable_EraseAll(void* mlpt_) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckCntr_(mlpt);

    int level = mlpt->level;

    Zeta_Core_MultiLevelPtrTable_Node* n = mlpt->root;
    if (n == NULL) { return; }

    EraseAll_(level - 1, mlpt->branch_nums, n, mlpt->node_allocator);

    mlpt->root = NULL;
}

void Zeta_Core_MultiLevelPtrTable_Check(void* mlpt_) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    ZETA_Core_DebugAssert(mlpt != NULL);

    int level = mlpt->level;
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= ZETA_Core_MultiLevelPtrTable_max_level);

    unsigned short const* branch_nums = mlpt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        int branch_num = branch_nums[level_i];

        ZETA_Core_DebugAssert(ZETA_Core_MultiLevelPtrTable_min_branch_num <=
                              branch_num);
        ZETA_Core_DebugAssert(branch_num <=
                              ZETA_Core_MultiLevelPtrTable_max_branch_num);
    }
}

void Zeta_Core_MultiLevelPtrTable_CheckIdxes(void* mlpt_, size_t const* idxes) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    ZETA_Core_DebugAssert(mlpt != NULL);

    ZETA_Core_DebugAssert(idxes != NULL);

    int level = mlpt->level;
    unsigned short const* branch_nums = mlpt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        ZETA_Core_DebugAssert(idxes[level_i] < branch_nums[level_i]);
    }
}

static void Sanitize_(Zeta_Core_MemRecorder* dst_node, int level_i,
                      unsigned short const* branch_nums,
                      Zeta_Core_MultiLevelPtrTable_Node* node) {
    ZETA_Core_DebugAssert(node->active_map != 0);

    int branch_num = branch_nums[level_i];

    if (dst_node != NULL) {
        Zeta_Core_MemRecorder_Record(
            dst_node, node,
            offsetof(Zeta_Core_MultiLevelPtrTable_Node, ptrs[branch_num]));
    }

    if (level_i == 0) { return; }

    for (int idx = -1;
         (idx = Zeta_Core_FindNextOne(node->active_map, idx)) != -1;) {
        ZETA_Core_DebugAssert(TestActiveMap_(node->active_map, idx));
        Sanitize_(dst_node, level_i - 1, branch_nums, node->ptrs[idx]);
    }
}

void Zeta_Core_MultiLevelPtrTable_Sanitize(void* mlpt_,
                                           Zeta_Core_MemRecorder* dst_node) {
    Zeta_Core_MultiLevelPtrTable* mlpt = mlpt_;
    CheckCntr_(mlpt);

    if (mlpt->root != NULL) {
        Sanitize_(dst_node, mlpt->level - 1, mlpt->branch_nums, mlpt->root);
    }
}
