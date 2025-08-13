#include <zeta/core/debugger.h>
#include <zeta/core/multi_level_table.h>
#include <zeta/core/utils.h>

#if ZETA_Core_EnableDebug

#define CheckCntr_(cntr) Zeta_Core_MultiLevelTable_Check(cntr)

#define CheckIdxes_(cntr, idxes) \
    Zeta_Core_MultiLevelTable_CheckIdxes(cntr, idxes)

#else

#define CheckCntr_(cntr)

#define CheckIdxes_(cntr, idxes)

#endif

#define TestActiveMap_(active_map, idx) ((active_map) & ((1ULL) << (idx)) != 0)

#define DataNodeSize_(stride, seg_size)                                       \
    (ZETA_Core_IntRoundUp((stride) * (seg_size) + sizeof(unsigned long long), \
                          alignof(unsigned long long)))

void Zeta_Core_MultiLevelTable_Init(void* mlt_) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    ZETA_Core_DebugAssert(mlt != NULL);

    int level = mlt->level;
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= ZETA_Core_MultiLevelTable_max_level);

    size_t width = mlt->width;
    size_t stride = mlt->stride;
    size_t seg_size = mlt->seg_size;

    ZETA_Core_DebugAssert(0 < width);
    ZETA_Core_DebugAssert(width <= stride);
    ZETA_Core_DebugAssert(0 < seg_size);

    unsigned short const* branch_nums = mlt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        size_t branch_num = branch_nums[level_i];

        ZETA_Core_DebugAssert(ZETA_Core_MultiLevelTable_min_branch_num <=
                              branch_num);
        ZETA_Core_DebugAssert(branch_num <=
                              ZETA_Core_MultiLevelTable_max_branch_num);
    }

    mlt->root = NULL;

    ZETA_Core_DebugAssert(mlt->mlpt_node_allocator.vtable != NULL);
    ZETA_Core_DebugAssert(mlt->mlpt_node_allocator.vtable->GetAlign != NULL);
    ZETA_Core_DebugAssert(
        ZETA_Core_Allocator_GetAlign(mlt->mlpt_node_allocator) %
            alignof(Zeta_Core_MultiLevelPtrTable_Node) ==
        0);

    ZETA_Core_DebugAssert(mlt->data_node_allocator.vtable != NULL);
    ZETA_Core_DebugAssert(mlt->data_node_allocator.vtable->GetAlign != NULL);
}

void Zeta_Core_MultiLevelTable_Deinit(void* mlt) {
    Zeta_Core_MultiLevelTable_EraseAll(mlt);
}

size_t Zeta_Core_MultiLevelTable_GetCapacity(void* mlt_) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckCntr_(mlt);

    size_t ret = mlt->seg_size;

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        if (__builtin_umulll_overflow(ret, branch_nums[level_i], &ret)) {
            return ZETA_Core_size_max;
        }
    }

    return ret;
}

void* Zeta_Core_MultiLevelTable_Access(void* mlt_, size_t* idxes) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;

    Zeta_Core_MultiLevelPtrTable mlpt = {
        .level = level,
        .branch_nums = mlt->branch_nums,
        .root = mlt->root,
        .node_allocator = mlt->mlpt_node_allocator,
    };

    void** addr = Zeta_Core_MultiLevelPtrTable_Access(&mlpt, idxes);

    return addr == NULL ? NULL
                        : *addr + mlt->stride * (idxes[level] - mlt->seg_size);
}

void** Zeta_Core_MultiLevelTable_FindFirst(void* mlt_, size_t* dst_idxes) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckCntr_(mlt);

    int level = mlt->level;

    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    for (int level_i = 0; level_i < level; ++level_i) { idxes[level_i] = 0; }

    void** ret = Zeta_Core_MultiLevelTable_FindNext(mlt, idxes, TRUE);

    if (dst_idxes != NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

void** Zeta_Core_MultiLevelTable_FindLast(void* mlt_, size_t* dst_idxes) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckCntr_(mlt);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

    size_t idxes[ZETA_Core_MultiLevelTable_max_level];

    for (int level_i = 0; level_i < level; ++level_i) {
        idxes[level_i] = branch_nums[level_i] - 1;
    }

    void** ret = Zeta_Core_MultiLevelTable_FindPrev(mlt, idxes, TRUE);

    if (dst_idxes != NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

void** Zeta_Core_MultiLevelTable_FindPrev(void* mlt_, size_t* idxes,
                                          bool_t included) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

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

    Zeta_Core_MultiLevelTable_Node* root = mlt->root;

    if (root == NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            idxes[level_i] = branch_nums[level_i] - 1;
        }

        return NULL;
    }

    Zeta_Core_MultiLevelTable_Node* nodes[ZETA_Core_MultiLevelTable_max_level];

    int level_i = level - 1;
    Zeta_Core_MultiLevelTable_Node* node = root;

    for (;; --level_i) {
        nodes[level_i] = node;
        if (!TestActiveMap_(node->active_map, idxes[level_i])) { break; }

        void** nxt_node = node->ptrs + idxes[level_i];
        if (level_i == 0) { return nxt_node; }

        node = *nxt_node;
    }

    for (; level_i < level; ++level_i) {
        node = nodes[level_i];

        int found_idx = FindPrevActive_(node->active_map, idxes[level_i] - 1);

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
            FindPrevActive_(node->active_map, branch_nums[level_i] - 1);
    }

    return nodes[0]->ptrs + idxes[0];
}

void** Zeta_Core_MultiLevelTable_FindNext(void* mlt_, size_t* idxes,
                                          bool_t included) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

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

    Zeta_Core_MultiLevelTable_Node* root = mlt->root;

    if (root == NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            idxes[level_i] = 0;
        }

        return NULL;
    }

    Zeta_Core_MultiLevelTable_Node* nodes[ZETA_Core_MultiLevelTable_max_level];

    int level_i = level - 1;
    Zeta_Core_MultiLevelTable_Node* node = root;

    for (;; --level_i) {
        nodes[level_i] = node;
        if (!TestActiveMap_(node->active_map, idxes[level_i])) { break; }

        void** nxt_node = node->ptrs + idxes[level_i];
        if (level_i == 0) { return nxt_node; }

        node = *nxt_node;
    }

    for (; level_i < level; ++level_i) {
        node = nodes[level_i];

        int found_idx = FindNextActive_(node->active_map, idxes[level_i] + 1);

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
        idxes[level_i] = FindNextActive_(node->active_map, 0);
    }

    return nodes[0]->ptrs + idxes[0];
}

static void* AllocateDataNode_(size_t stride, size_t seg_size,
                               Zeta_Core_Allocator data_node_allocator) {
    void* ret = ZETA_Core_Allocator_SafeAllocate(
                    data_node_allocator, alignof(unsigned long long),
                    DataNodeSize_(stride, seg_size)) -
                sizeof(unsigned long long);

    *((unsigned long long*)ret) = 0;

    return ret;
}

void** Zeta_Core_MultiLevelTable_Insert(void* mlt_, size_t* idxes) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;

    Zeta_Core_MultiLevelPtrTable mlpt = {
        .level = level,
        .branch_nums = mlt->branch_nums,
        .root = mlt->root,
        .node_allocator = mlt->mlpt_node_allocator,
    };

    void** addr = Zeta_Core_MultiLevelPtrTable_Access(&mlpt, idxes);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

    if (mlt->root == NULL) {
        mlt->root = AllocateNode_(branch_nums[0], mlt->node_allocator);
    }

    Zeta_Core_MultiLevelTable_Node* node = mlt->root;

    int level_i = level - 1;

    for (; 0 < level_i && TestActiveMap_(node->active_map, idxes[level_i]);
         --level_i) {
        node = node->ptrs[idxes[level_i]];
    }

    for (; 0 < level_i; --level_i) {
        node->active_map = node->active_map + (1ULL << idxes[level_i]);

        node = node->ptrs[idxes[level_i]] =
            AllocateNode_(branch_nums[level_i + 1], mlt->node_allocator);
    }

    void** ret = node->ptrs + idxes[0];

    if (TestActiveMap_(node->active_map, idxes[0])) { return ret; }

    ++mlt->size;

    node->active_map = node->active_map + (1ULL << idxes[0]);
    *ret = NULL;

    return ret;
}

void* Zeta_Core_MultiLevelTable_Erase(void* mlt_, size_t* idxes) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;

    Zeta_Core_MultiLevelTable_Node* node = mlt->root;
    if (node == NULL) { return NULL; }

    Zeta_Core_MultiLevelTable_Node* nodes[ZETA_Core_MultiLevelTable_max_level];

    for (int level_i = level - 1;; --level_i) {
        nodes[level_i] = node;
        if (level_i == 0) { break; }
        if (!TestActiveMap_(node->active_map, idxes[level_i])) { return NULL; }
        node = node->ptrs[idxes[level_i]];
    }

    if (!TestActiveMap_(node->active_map, idxes[0])) { return NULL; }

    void* ret = node->ptrs[idxes[0]];

    --mlt->size;

    for (int level_i = 0; level_i < level; ++level_i) {
        node = nodes[level_i];

        node->active_map = node->active_map - (1ULL << idxes[level_i]);

        if (node->active_map != 0) { return ret; }

        ZETA_Core_Allocator_Deallocate(mlt->node_allocator, node);
    }

    mlt->root = NULL;

    return ret;
}

static void EraseAll_(int level, unsigned short const* branch_nums,
                      Zeta_Core_MultiLevelTable_Node* node,
                      Zeta_Core_Allocator node_allocator) {
    if (level == 0) {
        ZETA_Core_Allocator_Deallocate(node_allocator, node);
        return;
    }

    size_t branch_num = branch_nums[level];

    for (size_t idx = 0; idx < branch_num; ++idx) {
        if (TestActiveMap_(node->active_map, idx)) {
            EraseAll_(level - 1, branch_nums, node->ptrs[idx], node_allocator);
        }
    }

    ZETA_Core_Allocator_Deallocate(node_allocator, node);
}

void Zeta_Core_MultiLevelTable_EraseAll(void* mlt_) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckCntr_(mlt);

    int level = mlt->level;

    Zeta_Core_MultiLevelTable_Node* n = mlt->root;
    if (n == NULL) { return; }

    EraseAll_(level - 1, mlt->branch_nums, n, mlt->node_allocator);

    mlt->size = 0;
    mlt->root = NULL;
}

void Zeta_Core_MultiLevelTable_Check(void* mlt_) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    ZETA_Core_DebugAssert(mlt != NULL);

    int level = mlt->level;
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= ZETA_Core_MultiLevelTable_max_level);

    unsigned short const* branch_nums = mlt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        size_t branch_num = branch_nums[level_i];

        ZETA_Core_DebugAssert(ZETA_Core_MultiLevelTable_min_branch_num <=
                              branch_num);
        ZETA_Core_DebugAssert(branch_num <=
                              ZETA_Core_MultiLevelTable_max_branch_num);
    }
}

void Zeta_Core_MultiLevelTable_CheckIdxes(void* mlt_, size_t const* idxes) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    ZETA_Core_DebugAssert(mlt != NULL);

    ZETA_Core_DebugAssert(idxes != NULL);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        ZETA_Core_DebugAssert(idxes[level_i] < branch_nums[level_i]);
    }
}

static size_t Sanitize_(Zeta_Core_MemRecorder* dst_node, int level_i,
                        unsigned short const* branch_nums,
                        Zeta_Core_MultiLevelTable_Node* node) {
    ZETA_Core_DebugAssert(node->active_map != 0);

    size_t branch_num = branch_nums[level_i];

    if (dst_node != NULL) {
        Zeta_Core_MemRecorder_Record(
            dst_node, node,
            offsetof(Zeta_Core_MultiLevelTable_Node, ptrs[branch_num]));
    }

    if (level_i == 0) {
        size_t size = 0;

        for (size_t idx = 0; idx < branch_num; ++idx) {
            if (TestActiveMap_(node->active_map, idx)) { ++size; }
        }

        return size;
    }

    size_t size = 0;

    for (size_t idx = 0; idx < branch_num;) {
        idx = FindNextActive_(node->active_map, idx);

        if (idx == (size_t)(-1)) { break; }

        ZETA_Core_DebugAssert(TestActiveMap_(node->active_map, idx));

        size += Sanitize_(dst_node, level_i - 1, branch_nums, node->ptrs[idx]);

        ++idx;
    }

    return size;
}

void Zeta_Core_MultiLevelTable_Sanitize(void* mlt_,
                                        Zeta_Core_MemRecorder* dst_node) {
    Zeta_Core_MultiLevelTable* mlt = mlt_;
    CheckCntr_(mlt);

    if (mlt->root == NULL) {
        ZETA_Core_DebugAssert(mlt->size == 0);
    } else {
        size_t size =
            Sanitize_(dst_node, mlt->level - 1, mlt->branch_nums, mlt->root);
        ZETA_Core_DebugAssert(mlt->size == size);
    }
}
