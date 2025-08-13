#include <zeta/core/debugger.h>
#include <zeta/core/multi_level_table_temp.h>
#include <zeta/core/utils.h>

#define max_level ZETA_Core_MultiLevelTable_max_level
#define min_branch_num ZETA_Core_MultiLevelTable_min_branch_num
#define max_branch_num ZETA_Core_MultiLevelTable_max_branch_num

#define NavNode Zeta_Core_MultiLevelTable_NavNode

#if DATA

#define Cntr Zeta_Core_MultiLevelDataTable

#else

#define Cntr Zeta_Core_MultiLevelPtrTable

#endif

#define Cntr_(x) ZETA_Core_Concat(Cntr, _, x)

#if ZETA_Core_EnableDebug

#define CheckCntr_(cntr) Cntr_(Check)(cntr)

#define CheckIdxes_(cntr, idxes) Cntr_(CheckIdxes)(cntr, idxes)

#else

#define CheckCntr_(cntr)

#define CheckIdxes_(cntr, idxes)

#endif

#if DATA

#define DataNodeSize_(stride, branch_num)                     \
    (ZETA_Core_IntRoundUp(                                    \
        (stride) * (branch_num) + sizeof(unsigned long long), \
        alignof(unsigned long long)))

#endif

#define TestActiveMap_(active_map, idx) \
    (((active_map) & ((1ULL) << (idx))) != 0)

static void* AllocateNavNode_(int branch_num,
                              Zeta_Core_Allocator nav_node_allocator) {
    NavNode* nav_node =
        ZETA_Core_Allocator_SafeAllocate(nav_node_allocator, alignof(NavNode),
                                         offsetof(NavNode, ptrs[branch_num]));

    nav_node->active_map = 0;

    return &nav_node->active_map;
}

static void DeallocateNavNode_(Zeta_Core_Allocator nav_node_allocator,
                               void* node) {
    ZETA_Core_Allocator_Deallocate(
        nav_node_allocator,
        ZETA_Core_MemberToStruct(NavNode, active_map, node));
}

#if DATA

static void* AllocateDatNode_(size_t stride, int branch_num,
                              Zeta_Core_Allocator dat_node_allocator) {
    size_t data_node_size = DataNodeSize_(stride, branch_num);

    void* dat_node =
        ZETA_Core_Allocator_SafeAllocate(
            dat_node_allocator, alignof(unsigned long long), data_node_size) +
        (data_node_size - sizeof(unsigned long long));

    *((unsigned long long*)dat_node) = 0;

    return dat_node;
}

static void DeallocateDatNode_(size_t stride, int branch_num,
                               Zeta_Core_Allocator dat_node_allocator,
                               void* node) {
    ZETA_Core_Allocator_Deallocate(dat_node_allocator,
                                   node - (DataNodeSize_(stride, branch_num) -
                                           sizeof(unsigned long long)));
}

#endif

void Cntr_(Init)(void* mlt_) {
    Cntr* mlt = mlt_;
    ZETA_Core_DebugAssert(mlt != NULL);

    int level = mlt->level;
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    unsigned short const* branch_nums = mlt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        int branch_num = branch_nums[level_i];

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }

#if DATA
    ZETA_Core_DebugAssert(0 < mlt->stride);
#endif

    mlt->size = 0;

    mlt->root = NULL;

    ZETA_Core_DebugAssert(mlt->nav_node_allocator.vtable != NULL);
    ZETA_Core_DebugAssert(mlt->nav_node_allocator.vtable->GetAlign != NULL);
    ZETA_Core_DebugAssert(
        ZETA_Core_Allocator_GetAlign(mlt->nav_node_allocator) %
            alignof(NavNode) ==
        0);

#if DATA
    ZETA_Core_DebugAssert(mlt->dat_node_allocator.vtable != NULL);
    ZETA_Core_DebugAssert(mlt->dat_node_allocator.vtable->GetAlign != NULL);
    ZETA_Core_DebugAssert(
        ZETA_Core_Allocator_GetAlign(mlt->dat_node_allocator) %
            alignof(unsigned long long) ==
        0);
#endif
}

void Cntr_(Deinit)(void* mlt) { Cntr_(EraseAll)(mlt); }

size_t Cntr_(GetSize)(void* mlt_) {
    Cntr* mlt = mlt_;
    CheckCntr_(mlt);

    return mlt->size;
}

size_t Cntr_(GetCapacity)(void* mlt_) {
    Cntr* mlt = mlt_;
    CheckCntr_(mlt);

    size_t ret = 1;

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        if (__builtin_umulll_overflow(ret, branch_nums[level_i], &ret)) {
            return ZETA_Core_max_capacity;
        }
    }

    return ret;
}

void* Cntr_(Access)(void* mlt_, size_t* idxes) {
    Cntr* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;

    void* node = mlt->root;

    if (node == NULL) { return NULL; }

    for (int level_i = level - 1; 0 < level_i; --level_i) {
        int cur_idx = idxes[level_i];

        if (!TestActiveMap_(*(unsigned long long*)node, cur_idx)) {
            return NULL;
        }

        node = ((NavNode*)node)->ptrs[cur_idx];
    }

    int last_idx = idxes[0];

    if (!TestActiveMap_(*(unsigned long long*)node, last_idx)) { return NULL; }

#if DATA
    return node + sizeof(unsigned long long) -
           DataNodeSize_(mlt->stride, mlt->branch_nums[0]) +
           mlt->stride * last_idx;
#else
    return ((NavNode*)node)->ptrs + last_idx;
#endif
}

void* Cntr_(FindFirst)(void* mlt_, size_t* dst_idxes) {
    Cntr* mlt = mlt_;
    CheckCntr_(mlt);

    int level = mlt->level;

    size_t idxes[max_level];

    for (int level_i = 0; level_i < level; ++level_i) { idxes[level_i] = 0; }

    void* ret = Cntr_(FindNext)(mlt, idxes, TRUE);

    if (dst_idxes != NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

void* Cntr_(FindLast)(void* mlt_, size_t* dst_idxes) {
    Cntr* mlt = mlt_;
    CheckCntr_(mlt);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

    size_t idxes[max_level];

    for (int level_i = 0; level_i < level; ++level_i) {
        idxes[level_i] = branch_nums[level_i] - 1;
    }

    void* ret = Cntr_(FindPrev)(mlt, idxes, TRUE);

    if (dst_idxes != NULL) {
        for (int level_i = 0; level_i < level; ++level_i) {
            dst_idxes[level_i] = idxes[level_i];
        }
    }

    return ret;
}

void* Cntr_(FindPrev)(void* mlt_, size_t* idxes, bool_t included) {
    Cntr* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

#if DATA
    size_t data_node_size = DataNodeSize_(mlt->stride, mlt->branch_nums[0]);
#endif

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

    void* root = mlt->root;

    if (root == NULL) { goto UNFOUND_RET; }

    void* nodes[max_level];

    int level_i = level - 1;
    void* node = root;

    for (;; --level_i) {
        nodes[level_i] = node;

        size_t cur_idx = idxes[level_i];

        if (!TestActiveMap_(*(unsigned long long*)node, cur_idx)) { break; }

#if DATA
        if (level == 0) {
            return node + sizeof(unsigned long long) - data_node_size +
                   mlt->stride * cur_idx;
        }
#endif

        void* addr = ((NavNode*)node)->ptrs + cur_idx;

#if !DATA
        if (level_i == 0) { return addr; }
#endif

        node = *(void**)addr;
    }

    for (;; ++level_i) {
        if (level_i == level) { goto UNFOUND_RET; }

        node = nodes[level_i];

        int found_idx =
            Zeta_Core_FindPrevOne(*(unsigned long long*)node, idxes[level_i]);

        if (0 <= found_idx) {
            idxes[level_i] = found_idx;
            break;
        }
    }

    while (0 < level_i) {
        node = ((NavNode*)node)->ptrs[idxes[level_i--]];
        nodes[level_i] = node;
        idxes[level_i] = Zeta_Core_FindPrevOne(*(unsigned long long*)node,
                                               branch_nums[level_i]);
    }

#if DATA
    return nodes[0] + sizeof(unsigned long long) -
           DataNodeSize_(mlt->stride, mlt->branch_nums[0]) +
           mlt->stride * idxes[0];
#else
    return ((NavNode*)nodes[0])->ptrs + idxes[0];
#endif

UNFOUND_RET:;

    for (int level_i = 0; level_i < level; ++level_i) {
        idxes[level_i] = branch_nums[level_i] - 1;
    }

    return NULL;
}

void* Cntr_(FindNext)(void* mlt_, size_t* idxes, bool_t included) {
    Cntr* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

#if DATA
    size_t data_node_size = DataNodeSize_(mlt->stride, mlt->branch_nums[0]);
#endif

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

    void* root = mlt->root;

    if (root == NULL) { goto UNFOUND_RET; }

    void* nodes[max_level];

    int level_i = level - 1;
    void* node = root;

    for (;; --level_i) {
        nodes[level_i] = node;

        size_t cur_idx = idxes[level_i];

        if (!TestActiveMap_(*(unsigned long long*)node, idxes[level_i])) {
            break;
        }

#if DATA
        if (level == 0) {
            return node + sizeof(unsigned long long) - data_node_size +
                   mlt->stride * cur_idx;
        }
#endif

        void* addr = ((NavNode*)node)->ptrs + cur_idx;

#if !DATA
        if (level_i == 0) { return addr; }
#endif

        node = *(void**)addr;
    }

    for (;; ++level_i) {
        if (level_i == level) { goto UNFOUND_RET; }

        node = nodes[level_i];

        int found_idx =
            Zeta_Core_FindNextOne(*(unsigned long long*)node, idxes[level_i]);

        if (0 <= found_idx) {
            idxes[level_i] = found_idx;
            break;
        }
    }

    while (0 < level_i) {
        node = ((NavNode*)node)->ptrs[idxes[level_i--]];
        nodes[level_i] = node;
        idxes[level_i] = Zeta_Core_FindNextOne(*(unsigned long long*)node, -1);
    }

#if DATA
    return nodes[0] + sizeof(unsigned long long) -
           DataNodeSize_(mlt->stride, mlt->branch_nums[0]) +
           mlt->stride * idxes[0];
#else
    return ((NavNode*)nodes[0])->ptrs + idxes[0];
#endif

UNFOUND_RET:;

    for (int level_i = 0; level_i < level; ++level_i) { idxes[level_i] = 0; }

    return NULL;
}

Zeta_Core_MultiLevelTable_InsertResult Cntr_(Insert)(void* mlt_,
                                                     size_t* idxes) {
    Cntr* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

    if (mlt->root == NULL) {
#if DATA
        if (level == 1) {
            mlt->root = AllocateDatNode_(mlt->stride, branch_nums[level - 1],
                                         mlt->dat_node_allocator);
        } else
#endif
        {
            mlt->root = AllocateNavNode_(branch_nums[level - 1],
                                         mlt->nav_node_allocator);
        }
    }

    void* node = mlt->root;

    int level_i = level - 1;

    for (; 0 < level_i &&
           TestActiveMap_(*(unsigned long long*)node, idxes[level_i]);
         --level_i) {
        node = ((NavNode*)node)->ptrs[idxes[level_i]];
    }

    for (; 0 < level_i; --level_i) {
        *(unsigned long long*)node += 1ULL << idxes[level_i];

        void* nxt_node;

#if DATA
        if (level_i == 1) {
            nxt_node = AllocateDatNode_(mlt->stride, branch_nums[0],
                                        mlt->dat_node_allocator);
        } else
#endif
        {
            nxt_node = AllocateNavNode_(branch_nums[level_i - 1],
                                        mlt->nav_node_allocator);
        }

        node = ((NavNode*)node)->ptrs[idxes[level_i]] = nxt_node;
    }

    int last_idx = idxes[0];

#if DATA
    void* addr = node + sizeof(unsigned long long) -
                 DataNodeSize_(mlt->stride, mlt->branch_nums[0]) +
                 mlt->stride * last_idx;
#else
    void* addr = ((NavNode*)node)->ptrs + last_idx;
#endif

    bool inserted = !TestActiveMap_(*(unsigned long long*)node, last_idx);

    if (inserted) {
        ++mlt->size;
        *(unsigned long long*)node += (1ULL << last_idx);
    }

    return (Zeta_Core_MultiLevelTable_InsertResult){ .addr = addr,
                                                     .inserted = inserted };
}

bool_t Cntr_(Erase)(void* mlt_, size_t* idxes) {
    Cntr* mlt = mlt_;
    CheckIdxes_(mlt, idxes);

    int level = mlt->level;

    void* node = mlt->root;

    if (node == NULL) { return FALSE; }

    void* nodes[max_level];

    for (int level_i = level - 1;; --level_i) {
        nodes[level_i] = node;

        if (level_i == 0) { break; }

        if (!TestActiveMap_(*(unsigned long long*)node, idxes[level_i])) {
            return FALSE;
        }

        node = ((NavNode*)node)->ptrs[idxes[level_i]];
    }

    if (!TestActiveMap_(*(unsigned long long*)node, idxes[0])) { return FALSE; }

    --mlt->size;

    for (int level_i = 0; level_i < level; ++level_i) {
        node = nodes[level_i];

        *(unsigned long long*)node -= 1ULL << idxes[level_i];

        if (*(unsigned long long*)node != 0) { return TRUE; }

#if DATA
        if (level_i == 0) {
            DeallocateDatNode_(mlt->stride, mlt->branch_nums[0],
                               mlt->dat_node_allocator, node);
        } else
#endif
        {
            DeallocateNavNode_(mlt->nav_node_allocator, node);
        }
    }

    mlt->root = NULL;

    return TRUE;
}

#if DATA
static void EraseAll_(int level, unsigned short const* branch_nums,
                      size_t stride, void* node,
                      Zeta_Core_Allocator nav_node_allocator,
                      Zeta_Core_Allocator dat_node_allocator)
#else
static void EraseAll_(int level, unsigned short const* branch_nums, void* node,
                      Zeta_Core_Allocator nav_node_allocator)
#endif
{
    if (level == 0) {
#if DATA
        DeallocateDatNode_(stride, branch_nums[0], dat_node_allocator, node);
#else
        DeallocateNavNode_(nav_node_allocator, node);
#endif

        return;
    }

    int branch_num = branch_nums[level];

    for (int idx = 0; idx < branch_num; ++idx) {
        if (TestActiveMap_(*(unsigned long long*)node, idx)) {
#if DATA
            EraseAll_(level - 1, branch_nums, stride,
                      ((NavNode*)node)->ptrs[idx], nav_node_allocator,
                      dat_node_allocator);
#else
            EraseAll_(level - 1, branch_nums, ((NavNode*)node)->ptrs[idx],
                      nav_node_allocator);
#endif
        }
    }

    DeallocateNavNode_(nav_node_allocator, node);
}

void Cntr_(EraseAll)(void* mlt_) {
    Cntr* mlt = mlt_;
    CheckCntr_(mlt);

    int level = mlt->level;

    void* n = mlt->root;
    if (n == NULL) { return; }

#if DATA
    EraseAll_(level - 1, mlt->branch_nums, mlt->stride, n,
              mlt->nav_node_allocator, mlt->dat_node_allocator);
#else
    EraseAll_(level - 1, mlt->branch_nums, n, mlt->nav_node_allocator);
#endif

    mlt->root = NULL;
}

void Cntr_(Check)(void* mlt_) {
    Cntr* mlt = mlt_;
    ZETA_Core_DebugAssert(mlt != NULL);

    int level = mlt->level;
    ZETA_Core_DebugAssert(0 < level);
    ZETA_Core_DebugAssert(level <= max_level);

    unsigned short const* branch_nums = mlt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        int branch_num = branch_nums[level_i];

        ZETA_Core_DebugAssert(min_branch_num <= branch_num);
        ZETA_Core_DebugAssert(branch_num <= max_branch_num);
    }
}

void Cntr_(CheckIdxes)(void* mlt_, size_t const* idxes) {
    Cntr* mlt = mlt_;
    ZETA_Core_DebugAssert(mlt != NULL);

    ZETA_Core_DebugAssert(idxes != NULL);

    int level = mlt->level;
    unsigned short const* branch_nums = mlt->branch_nums;

    for (int level_i = 0; level_i < level; ++level_i) {
        ZETA_Core_DebugAssert(idxes[level_i] < branch_nums[level_i]);
    }
}

#if DATA
static size_t Sanitize_(Zeta_Core_MemRecorder* dst_nav_node,
                        Zeta_Core_MemRecorder* dst_dat_node, int level_i,
                        unsigned short const* branch_nums, size_t stride,
                        void* node)
#else
static size_t Sanitize_(Zeta_Core_MemRecorder* dst_nav_node, int level_i,
                        unsigned short const* branch_nums, void* node)
#endif
{
    ZETA_Core_DebugAssert(*(unsigned long long*)node != 0);

#if DATA
    if (level_i == 0) {
        if (dst_dat_node != NULL) {
            size_t data_node_size = DataNodeSize_(stride, branch_nums[0]);

            Zeta_Core_MemRecorder_Record(
                dst_dat_node,
                node - (data_node_size - sizeof(unsigned long long)),
                data_node_size);
        }

        return __builtin_popcountll(*(unsigned long long*)node);
    }
#endif

    if (dst_nav_node != NULL) {
        Zeta_Core_MemRecorder_Record(
            dst_nav_node, ZETA_Core_MemberToStruct(NavNode, active_map, node),
            sizeof(NavNode));
    }

#if !DATA
    if (level_i == 0) {
        return __builtin_popcountll(*(unsigned long long*)node);
    }
#endif

    size_t size = 0;

    for (int idx = -1; (idx = Zeta_Core_FindNextOne(*(unsigned long long*)node,
                                                    idx)) != -1;) {
        ZETA_Core_DebugAssert(TestActiveMap_(*(unsigned long long*)node, idx));

#if DATA
        size += Sanitize_(dst_nav_node, dst_dat_node, level_i - 1, branch_nums,
                          stride, ((NavNode*)node)->ptrs[idx]);
#else
        size += Sanitize_(dst_nav_node, level_i - 1, branch_nums,
                          ((NavNode*)node)->ptrs[idx]);
#endif
    }

    return size;
}

#if DATA
void Cntr_(Sanitize)(void* mlt_, Zeta_Core_MemRecorder* dst_nav_node,
                     Zeta_Core_MemRecorder* dst_dat_node)
#else
void Cntr_(Sanitize)(void* mlt_, Zeta_Core_MemRecorder* dst_nav_node)
#endif
{
    Cntr* mlt = mlt_;
    CheckCntr_(mlt);

    size_t size;

    if (mlt->root == NULL) {
        size = 0;
    } else {
#if DATA
        size = Sanitize_(dst_nav_node, dst_dat_node, mlt->level - 1,
                         mlt->branch_nums, mlt->stride, mlt->root);
#else
        size = Sanitize_(dst_nav_node, mlt->level - 1, mlt->branch_nums,
                         mlt->root);
#endif
    }

    ZETA_Core_DebugAssert(size == mlt->size);
}
