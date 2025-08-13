#include <zeta/core/circular_array.h>
#include <zeta/core/debugger.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core/memory.h>
#include <zeta/core/pool_allocator.h>
#include <zeta/core/ptr_utils.h>
#include <zeta/core/seg_staging_vector_temp.h>
#include <zeta/core/seg_utils.h>
#include <zeta/core/utils.h>

#if STAGING

#include <zeta/core/generic_hash_table.h>

#endif

#if STAGING

#define Cntr Zeta_Core_StagingVector

#define TreeNode Zeta_Core_OrdCnt3RBTreeNode

#else

#define Cntr Zeta_Core_SegVector

#define TreeNode Zeta_Core_OrdCntRBTreeNode

#endif

#define Cntr_(x) ZETA_Core_Concat(Cntr, _, x)

#define TreeNode_(x) ZETA_Core_Concat(TreeNode, _, x)

#if ZETA_Core_EnableDebug

#define CheckCntr_(cntr) Cntr_(Check)((cntr))

#define CheckCursor_(cntr, cursor) Cntr_(Cursor_Check)((cntr), (cursor))

#else

#define CheckCntr_(cntr) ZETA_Core_Unused((cntr));

#define CheckCursor_(cntr, cursor)  \
    {                               \
        ZETA_Core_Unused((cntr));   \
        ZETA_Core_Unused((cursor)); \
    }                               \
    ZETA_Core_StaticAssert(TRUE)

#endif

#if STAGING

#define GetWidth_(cntr) ZETA_Core_SeqCntr_GetWidth((cntr)->origin)

#define ref_color (ZETA_StagingVector_ref_color)
#define dat_color (ZETA_StagingVector_dat_color)

#define GetNColor__(tmp, n)                                          \
    ({                                                               \
        int tmp = TreeNode_(GetLColor)((n));                         \
        ZETA_Core_DebugAssert(tmp == ref_color || tmp == dat_color); \
        tmp;                                                         \
    })

#define DirectlySetNColor_(n, c) TreeNode_(SetLColor)((n), (c))

#define StagingThreeWay(cond, x, y) ((cond) ? (x) : (y))

#else

#define GetWidth_(cntr) (cntr)->width

#define ref_color 0
#define dat_color 1

#define GetNColor__(tmp, n) dat_color

#define DirectlySetNColor_(n, c)

#define StagingThreeWay(cond, x, y) (y)

#endif

#define GetNColor_(n) GetNColor__(ZETA_Core_TmpName, n)

#define NToSeg__(tmp_n, n_)                             \
    ({                                                  \
        ZETA_Core_AutoVar(tmp_n, n_);                   \
        ZETA_Core_DebugAssert(0 <= GetNColor_(tmp_n));  \
        ZETA_Core_MemberToStruct(Cntr_(Seg), n, tmp_n); \
    })

#define NToSeg_(n) NToSeg__(ZETA_Core_TmpName, (n))

#define SetNColor__(tmp_n, n, c)                       \
    {                                                  \
        ZETA_Core_AutoVar(tmp_n, n);                   \
        ZETA_Core_DebugAssert(0 <= GetNColor_(tmp_n)); \
        DirectlySetNColor_(tmp_n, (c));                \
    }                                                  \
    ZETA_Core_StaticAssert(TRUE)

#define SetNColor_(n, c) SetNColor__(ZETA_Core_TmpName, (n), (c))

#define GetAvgCnt__(tmp_x, tmp_seg_cnt, x)                                    \
    ({                                                                        \
        size_t tmp_x = (x);                                                   \
        size_t tmp_seg_cnt = ZETA_Core_UnsafeCeilIntDiv(tmp_x, seg_capacity); \
        (tmp_x + Zeta_Core_SimpleRandomRotate(&random_seed) % tmp_seg_cnt) /  \
            tmp_seg_cnt;                                                      \
    })

#define GetAvgCnt_(x) GetAvgCnt__(ZETA_Core_TmpName, ZETA_Core_TmpName, (x))

static void InitTree_(Cntr* cntr) {
    TreeNode_(Init)(cntr->lb);
    TreeNode_(Init)(cntr->rb);

    TreeNode_(SetAccSize)(cntr->lb, 1);
    TreeNode_(SetAccSize)(cntr->rb, 1);

    void* root = NULL;

    root = Zeta_Core_RBTree(TreeNode, InsertR)(root, cntr->lb);
    root = Zeta_Core_RBTree(TreeNode, InsertR)(root, cntr->rb);

    cntr->root = root;
}

static void* AllocateData_(Cntr* cntr, Zeta_Core_PoolAllocator* datas) {
    void* data = NULL;

    if (datas != NULL) { data = Zeta_Core_PoolAllocator_Allocate(datas, 1); }

    if (data == NULL) {
        data = ZETA_Core_Allocator_SafeAllocate(
            cntr->data_allocator, 1, cntr->stride * cntr->seg_capacity);
    }

    return data;
}

static void* AllocateSeg_(Cntr* cntr, Zeta_Core_PoolAllocator* segs) {
    Cntr_(Seg)* seg = NULL;

    if (segs != NULL) { seg = Zeta_Core_PoolAllocator_Allocate(segs, 1); }

    if (seg == NULL) {
        seg = ZETA_Core_Allocator_SafeAllocate(
            cntr->seg_allocator, alignof(Cntr_(Seg)), sizeof(Cntr_(Seg)));
    }

    TreeNode_(Init)(&seg->n);

    return seg;
}

#if STAGING

static Cntr_(Seg) * AllocateRefSeg_(Cntr* cntr) {
    Cntr_(Seg)* seg = AllocateSeg_(cntr, NULL);

    TreeNode_(Init)(&seg->n);
    TreeNode_(SetAccSize)(&seg->n, 0);

    DirectlySetNColor_(&seg->n, ref_color);

    seg->ref.beg = 0;
    seg->ref.size = 0;

    return seg;
}

#endif

static Cntr_(Seg) * AllocateDatSeg_(Cntr* cntr) {
    Cntr_(Seg)* seg = AllocateSeg_(cntr, NULL);

    TreeNode_(Init)(&seg->n);
    TreeNode_(SetAccSize)(&seg->n, 0);

    DirectlySetNColor_(&seg->n, dat_color);

    seg->dat.data = AllocateData_(cntr, NULL);

    seg->dat.offset = 0;
    seg->dat.size = 0;

    return seg;
}

#if STAGING

static void TransRefSegToDatSeg_(Cntr* cntr, Cntr_(Seg) * seg) {
    Zeta_Core_SeqCntr origin = cntr->origin;

    void* data = AllocateData_(cntr, NULL);

    size_t size = seg->ref.size;

    void* origin_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);

    ZETA_Core_SeqCntr_Access(origin, seg->ref.beg, origin_cursor, NULL);
    ZETA_Core_SeqCntr_Read(origin, origin_cursor, size, data, cntr->stride,
                           NULL);

    SetNColor_(&seg->n, dat_color);

    seg->dat.data = data;
    seg->dat.offset = 0;
    seg->dat.size = size;
}

#endif

static void EraseSeg_(Cntr* cntr, Cntr_(Seg) * seg) {
    int color = GetNColor_(&seg->n);
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);

    cntr->root = Zeta_Core_RBTree(TreeNode, Extract)(&seg->n);

    if (color == dat_color) {
        ZETA_Core_Allocator_Deallocate(cntr->data_allocator, seg->dat.data);
    }

    ZETA_Core_Allocator_Deallocate(cntr->seg_allocator, seg);
}

static void EraseAllSeg_(Cntr* cntr, void* n) {
    void* nl = TreeNode_(GetL)(n);
    void* nr = TreeNode_(GetR)(n);

    if (cntr->lb != n && cntr->rb != n) {
        Cntr_(Seg)* seg = NToSeg_(n);

        if (GetNColor_(n) == dat_color) {
            ZETA_Core_Allocator_Deallocate(cntr->data_allocator, seg->dat.data);
        }

        ZETA_Core_Allocator_Deallocate(cntr->seg_allocator, seg);
    }

    if (nl != NULL) { EraseAllSeg_(cntr, nl); }
    if (nr != NULL) { EraseAllSeg_(cntr, nr); }
}

#if STAGING

static void RefOrigin_(Cntr* cntr) {
    Zeta_Core_SeqCntr origin = cntr->origin;

    size_t origin_size = ZETA_Core_SeqCntr_GetSize(origin);

    if (origin_size == 0) { return; }

    Cntr_(Seg)* seg = AllocateRefSeg_(cntr);
    cntr->root =
        Zeta_Core_RBTree(TreeNode, Insert)(cntr->lb, cntr->rb, &seg->n);

    seg->ref.beg = 0;
    seg->ref.size = origin_size;

    Zeta_Core_BinTree(TreeNode, SetSize)(&seg->n, origin_size);
}

#else

#define RefOrigin_(cntr)

#endif

#if STAGING

static void PushRefL_(Cntr* cntr, Zeta_Core_CircularArray* ca, size_t beg,
                      size_t size) {
    Zeta_Core_SeqCntr origin = cntr->origin;

    Zeta_Core_CircularArray_Cursor ca_cursor;
    Zeta_Core_CircularArray_PeekL(ca, &ca_cursor, NULL);
    Zeta_Core_CircularArray_Insert(ca, &ca_cursor, size);

    void* origin_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);

    ZETA_Core_SeqCntr_Access(origin, beg, origin_cursor, NULL);

    Zeta_Core_CircularArray_AssignFromSeqCntr(ca, &ca_cursor, origin,
                                              origin_cursor, size);
}

static void PushRefR_(Cntr* cntr, Zeta_Core_CircularArray* ca, size_t beg,
                      size_t size) {
    Zeta_Core_SeqCntr origin = cntr->origin;

    Zeta_Core_CircularArray_Cursor ca_cursor;
    Zeta_Core_CircularArray_GetRBCursor(ca, &ca_cursor);
    Zeta_Core_CircularArray_Insert(ca, &ca_cursor, size);

    void* origin_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);

    ZETA_Core_SeqCntr_Access(origin, beg, origin_cursor, NULL);

    Zeta_Core_CircularArray_AssignFromSeqCntr(ca, &ca_cursor, origin,
                                              origin_cursor, size);
}

#else

#define PushRefL_(cntr, ca, beg, size)

#define PushRefR_(cntr, ca, beg, size)

#endif

static void Merge2_(Cntr* cntr, Cntr_(Seg) * a_seg, Cntr_(Seg) * b_seg) {
#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;

    int a_c = GetNColor_(&a_seg->n);
    int b_c = GetNColor_(&b_seg->n);
#endif

    size_t a_size =
        StagingThreeWay(a_c == ref_color, a_seg->ref.size, a_seg->dat.size);
    size_t b_size =
        StagingThreeWay(b_c == ref_color, b_seg->ref.size, b_seg->dat.size);

#if STAGING
    size_t total_size = a_size + b_size;
#endif

    size_t width = GetWidth_(cntr);
    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;

    unsigned long long random_seed = Zeta_Core_GetRandom();

#if STAGING
    void* origin_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);

    if (a_c == ref_color && b_c == ref_color) {
        void* data = AllocateData_(cntr, NULL);

        ZETA_Core_SeqCntr_Access(origin, a_seg->ref.beg, origin_cursor, NULL);
        ZETA_Core_SeqCntr_Read(origin, origin_cursor, a_size, data, stride,
                               NULL);

        ZETA_Core_SeqCntr_Access(origin, b_seg->ref.beg, origin_cursor, NULL);
        ZETA_Core_SeqCntr_Read(origin, origin_cursor, b_seg->ref.size,
                               data + stride * a_size, stride, NULL);

        a_seg->ref.size = 0;
        b_seg->ref.size = 0;

        Cntr_(Seg) * dst_seg;

        switch (Zeta_Core_SimpleRandomRotate(&random_seed) % 2) {
            case 0: dst_seg = a_seg; break;
            case 1: dst_seg = b_seg; break;
        }

        SetNColor_(&dst_seg->n, dat_color);
        dst_seg->dat.data = data;
        dst_seg->dat.size = total_size;

        return;
    }
#endif

    Zeta_Core_CircularArray a_ca;
    a_ca.data = a_seg->dat.data;
    a_ca.width = width;
    a_ca.stride = stride;
    a_ca.offset = a_seg->dat.offset;
    a_ca.size = a_size;
    a_ca.capacity = seg_capacity;

    Zeta_Core_CircularArray b_ca;
    b_ca.data = b_seg->dat.data;
    b_ca.width = width;
    b_ca.stride = stride;
    b_ca.offset = b_seg->dat.offset;
    b_ca.size = b_size;
    b_ca.capacity = seg_capacity;

    size_t merge_a_cost = StagingThreeWay(a_c == ref_color, a_size, 0) + b_size;
    size_t merge_b_cost = StagingThreeWay(b_c == ref_color, b_size, 0) + a_size;

    if (Zeta_Choose2(merge_a_cost <= merge_b_cost, merge_b_cost <= merge_a_cost,
                     &random_seed) == 0) {
#if STAGING
        if (b_c == ref_color) {
            PushRefR_(cntr, &a_ca, b_seg->ref.beg, b_size);
        } else
#endif
        {
            SegShoveL(&a_ca, &b_ca, 0, 0, b_size);
        }
    } else {
#if STAGING
        if (a_c == ref_color) {
            PushRefL_(cntr, &b_ca, a_seg->ref.beg, a_size);
        } else
#endif
        {
            SegShoveR(&a_ca, &b_ca, 0, 0, a_size);
        }
    }

#if STAGING
    if (a_c == ref_color) {
        a_seg->ref.size = 0;
    } else
#endif
    {
        a_seg->dat.offset = a_ca.offset;
        a_seg->dat.size = a_ca.size;
    }

#if STAGING
    if (b_c == ref_color) {
        b_seg->ref.size = 0;
    } else
#endif
    {
        b_seg->dat.offset = b_ca.offset;
        b_seg->dat.size = b_ca.size;
    }
}

#if STAGING

#define SetSegState_(seg_tmp, ca_tmp, vacant_tmp, seg, ca, vacant)             \
    {                                                                          \
        Cntr_(Seg)* seg_tmp = (seg);                                           \
        Zeta_Core_CircularArray* ca_tmp = (ca);                                \
        size_t* vacant_tmp = (vacant);                                         \
                                                                               \
        if (GetNColor_(&seg_tmp->n) == ref_color) {                            \
            ca_tmp->size = seg_tmp->ref.size;                                  \
            *vacant_tmp =                                                      \
                seg_capacity - ZETA_Core_GetMinOf(ca_tmp->size, seg_capacity); \
        } else {                                                               \
            ca_tmp->data = seg_tmp->dat.data;                                  \
            ca_tmp->offset = seg_tmp->dat.offset;                              \
            ca_tmp->size = seg_tmp->dat.size;                                  \
            *vacant_tmp = seg_capacity - ca_tmp->size;                         \
        }                                                                      \
    }

#else

#define SetSegState_(seg_tmp, ca_tmp, vacant_tmp, seg, ca, vacant) \
    {                                                              \
        Cntr_(Seg)* seg_tmp = (seg);                               \
        Zeta_Core_CircularArray* ca_tmp = (ca);                    \
        size_t* vacant_tmp = (vacant);                             \
                                                                   \
        ca_tmp->data = seg_tmp->dat.data;                          \
        ca_tmp->offset = seg_tmp->dat.offset;                      \
        ca_tmp->size = seg_tmp->dat.size;                          \
        *vacant_tmp = seg_capacity - ca_tmp->size;                 \
    }

#endif

#define SetSegState(seg, ca, vacant)                                           \
    SetSegState_(ZETA_Core_TmpName, ZETA_Core_TmpName, ZETA_Core_TmpName, seg, \
                 ca, vacant)

#if STAGING

static void RefShoveL_(Cntr* cntr, Zeta_Core_CircularArray* l_ca,
                       Cntr_(Seg) * r_seg, size_t rl_size, size_t ins_cnt,
                       size_t shove_cnt) {
#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;
#endif

    size_t stride = l_ca->stride;

    size_t cnt_a = ZETA_Core_GetMinOf(rl_size, shove_cnt);
    size_t cnt_b = ZETA_Core_GetMinOf(ins_cnt, shove_cnt - cnt_a);
    size_t cnt_c = shove_cnt - cnt_a - cnt_b;

    size_t l_i = l_ca->size;

    Zeta_Core_CircularArray_Cursor l_cursor;
    void* r_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);

    Zeta_Core_CircularArray_GetRBCursor(l_ca, &l_cursor);

    Zeta_Core_CircularArray_Insert(l_ca, &l_cursor, shove_cnt);

    ZETA_Core_SeqCntr_Access(origin, r_seg->ref.beg, r_cursor, NULL);

    for (size_t i = cnt_a; 0 < i;) {
        size_t cur_i = ZETA_Core_GetMinOf(
            i, Zeta_Core_CircularArray_GetLongestContSucr(l_ca, l_i));
        i -= cur_i;

        ZETA_Core_SeqCntr_Read(
            origin, r_cursor, cur_i,
            Zeta_Core_CircularArray_Access(l_ca, l_i, NULL, NULL), stride,
            r_cursor);

        l_i += cur_i;
    }

    l_i += cnt_b;

    for (size_t i = cnt_c; 0 < i;) {
        size_t cur_i = ZETA_Core_GetMinOf(
            i, Zeta_Core_CircularArray_GetLongestContSucr(l_ca, l_i));
        i -= cur_i;

        ZETA_Core_SeqCntr_Read(
            origin, r_cursor, cur_i,
            Zeta_Core_CircularArray_Access(l_ca, l_i, NULL, NULL), stride,
            r_cursor);

        l_i += cur_i;
    }

    size_t new_r_size = r_seg->ref.size + ins_cnt - shove_cnt;

    if (cnt_b < ins_cnt) {
        void* data = AllocateData_(cntr, NULL);
        void* data_i = data;

        if (cnt_a < rl_size) {
            ZETA_Core_SeqCntr_Read(origin, r_cursor, rl_size - cnt_a, data_i,
                                   stride, r_cursor);
        }

        data_i += stride * (rl_size - cnt_a + ins_cnt - cnt_b);

        if (rl_size < r_seg->ref.size) {
            ZETA_Core_SeqCntr_Read(origin, r_cursor, r_seg->ref.size - rl_size,
                                   data_i, stride, r_cursor);
        }

        SetNColor_(&r_seg->n, dat_color);

        r_seg->dat.data = data;
        r_seg->dat.offset = 0;
        r_seg->dat.size = new_r_size;
    } else {
        r_seg->ref.beg += cnt_a + cnt_c;
        r_seg->ref.size = new_r_size;
    }
}

static void RefShoveR_(Cntr* cntr, Cntr_(Seg) * l_seg,
                       Zeta_Core_CircularArray* r_ca, size_t lr_size,
                       size_t ins_cnt, size_t shove_cnt) {
#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;
#endif

    size_t stride = r_ca->stride;

    size_t cnt_a = ZETA_Core_GetMinOf(lr_size, shove_cnt);
    size_t cnt_b = ZETA_Core_GetMinOf(ins_cnt, shove_cnt - cnt_a);
    size_t cnt_c = shove_cnt - cnt_a - cnt_b;

    size_t r_i = 0;

    void* l_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);

    Zeta_Core_CircularArray_Cursor r_cursor;

    Zeta_Core_CircularArray_PeekL(r_ca, &r_cursor, NULL);

    Zeta_Core_CircularArray_Insert(r_ca, &r_cursor, shove_cnt);

    ZETA_Core_SeqCntr_Access(origin,
                             l_seg->ref.beg + l_seg->ref.size - cnt_a - cnt_c,
                             l_cursor, NULL);

    for (size_t i = cnt_c; 0 < i;) {
        size_t cur_i = ZETA_Core_GetMinOf(
            i, Zeta_Core_CircularArray_GetLongestContSucr(r_ca, r_i));
        i -= cur_i;

        ZETA_Core_SeqCntr_Read(
            origin, l_cursor, cur_i,
            Zeta_Core_CircularArray_Access(r_ca, r_i, NULL, NULL), stride,
            l_cursor);

        r_i += cur_i;
    }

    r_i += cnt_b;

    for (size_t i = cnt_a; 0 < i;) {
        size_t cur_i = ZETA_Core_GetMinOf(
            i, Zeta_Core_CircularArray_GetLongestContSucr(r_ca, r_i));
        i -= cur_i;

        ZETA_Core_SeqCntr_Read(
            origin, l_cursor, cur_i,
            Zeta_Core_CircularArray_Access(r_ca, r_i, NULL, NULL), stride,
            l_cursor);

        r_i += cur_i;
    }

    size_t new_l_size = l_seg->ref.size + ins_cnt - shove_cnt;

    if (cnt_b < ins_cnt) {
        void* data = AllocateData_(cntr, NULL);
        void* data_i = data;

        ZETA_Core_SeqCntr_Access(origin, l_seg->ref.beg, l_cursor, NULL);

        if (lr_size < l_seg->ref.size) {
            ZETA_Core_SeqCntr_Read(origin, l_cursor, l_seg->ref.size - lr_size,
                                   data_i, stride, l_cursor);
        }

        data_i += stride * (l_seg->ref.size - lr_size + ins_cnt - cnt_b);

        if (cnt_a < lr_size) {
            ZETA_Core_SeqCntr_Read(origin, l_cursor, lr_size - cnt_a, data_i,
                                   stride, l_cursor);
        }

        SetNColor_(&l_seg->n, dat_color);

        l_seg->dat.data = data;
        l_seg->dat.offset = 0;
        l_seg->dat.size = new_l_size;
    } else {
        l_seg->ref.size = new_l_size;
    }
}

#else

#define RefShoveL_(cntr, l_ca, r_seg, rl_size, ins_cnt, shove_cnt)

#define RefShoveR_(cntr, l_seg, r_ca, lr_size, ins_cnt, shove_cnt)

#endif

void Cntr_(Init)(void* cntr_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    cntr->lb = ZETA_Core_Allocator_SafeAllocate(
        cntr->seg_allocator, alignof(TreeNode), sizeof(TreeNode));

    cntr->rb = ZETA_Core_Allocator_SafeAllocate(
        cntr->seg_allocator, alignof(TreeNode), sizeof(TreeNode));

    InitTree_(cntr);
    RefOrigin_(cntr);
}

void Cntr_(Deinit)(void* cntr_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    EraseAllSeg_(cntr, cntr->root);

    ZETA_Core_Allocator_Deallocate(cntr->seg_allocator, cntr->lb);
    ZETA_Core_Allocator_Deallocate(cntr->seg_allocator, cntr->rb);
}

size_t Cntr_(GetWidth)(void const* cntr_) {
    Cntr const* cntr = cntr_;
    CheckCntr_(cntr);

    return GetWidth_(cntr);
}

size_t Cntr_(GetSize)(void const* cntr_) {
    Cntr const* cntr = cntr_;
    CheckCntr_(cntr);

    return TreeNode_(GetAccSize)(cntr->root) - 2;
}

size_t Cntr_(GetCapacity)(void const* cntr_) {
    Cntr const* cntr = cntr_;
    CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

void Cntr_(GetLBCursor)(void const* cntr_, void* dst_cursor_) {
    Cntr const* cntr = cntr_;
    CheckCntr_(cntr);

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    if (dst_cursor == NULL) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = -1;
    dst_cursor->n = cntr->lb;
    dst_cursor->seg_idx = 0;
    dst_cursor->ref = NULL;
}

void Cntr_(GetRBCursor)(void const* cntr_, void* dst_cursor_) {
    Cntr const* cntr = cntr_;
    CheckCntr_(cntr);

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    if (dst_cursor == NULL) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = Cntr_(GetSize)(cntr);
    dst_cursor->n = cntr->rb;
    dst_cursor->seg_idx = 0;
    dst_cursor->ref = NULL;
}

void* Cntr_(PeekL)(void* cntr_, void* dst_cursor_, void* dst_elem) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;
#endif

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    size_t width = GetWidth_(cntr);
    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;

    void* n = Zeta_Core_BinTree(TreeNode, StepR)(cntr->lb);

    if (cntr->rb == n) {
        if (dst_cursor != NULL) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = 0;
            dst_cursor->n = n;
            dst_cursor->seg_idx = 0;
            dst_cursor->ref = NULL;
        }

        return NULL;
    }

    Cntr_(Seg)* seg = NToSeg_(n);

    void* ref = NULL;

#if STAGING
    if (GetNColor_(n) == ref_color) {
        ZETA_Core_SeqCntr_Access(origin, seg->ref.beg, NULL, dst_elem);
    } else
#endif
    {
        Zeta_Core_CircularArray ca;
        ca.data = seg->dat.data;
        ca.width = width;
        ca.stride = stride;
        ca.offset = seg->dat.offset;
        ca.size = seg->dat.size;
        ca.capacity = seg_capacity;

        ref = Zeta_Core_CircularArray_PeekL(&ca, NULL, dst_elem);
    }

    if (dst_cursor != NULL) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
        dst_cursor->n = n;
        dst_cursor->seg_idx = 0;
        dst_cursor->ref = ref;
    }

    return ref;
}

void* Cntr_(PeekR)(void* cntr_, void* dst_cursor_, void* dst_elem) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;
#endif

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    size_t width = GetWidth_(cntr);
    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;
    size_t size = Cntr_(GetSize)(cntr);

    void* n = Zeta_Core_BinTree(TreeNode, StepL)(cntr->rb);

    if (cntr->lb == n) {
        if (dst_cursor != NULL) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = -1;
            dst_cursor->n = n;
            dst_cursor->seg_idx = 0;
            dst_cursor->ref = NULL;
        }

        return NULL;
    }

    Cntr_(Seg)* seg = NToSeg_(n);

    void* ref = NULL;

    size_t seg_size;

#if STAGING
    if (GetNColor_(n) == ref_color) {
        seg_size = seg->ref.size;

        ZETA_Core_SeqCntr_Access(origin, seg->ref.beg + seg_size - 1, NULL,
                                 dst_elem);
    } else
#endif
    {
        seg_size = seg->dat.size;

        Zeta_Core_CircularArray ca;
        ca.data = seg->dat.data;
        ca.width = width;
        ca.stride = stride;
        ca.offset = seg->dat.offset;
        ca.size = seg_size;
        ca.capacity = seg_capacity;

        ref = Zeta_Core_CircularArray_PeekR(&ca, NULL, dst_elem);
    }

    if (dst_cursor != NULL) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = size - 1;
        dst_cursor->n = n;
        dst_cursor->seg_idx = seg_size - 1;
        dst_cursor->ref = ref;
    }

    return ref;
}

void* Cntr_(Access)(void* cntr_, size_t idx, void* dst_cursor_,
                    void* dst_elem) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;
#endif

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    size_t width = GetWidth_(cntr);
    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;
    size_t size = Cntr_(GetSize)(cntr);

    ZETA_Core_DebugAssert(idx + 1 < size + 2);

    void* n;
    Cntr_(Seg) * seg;
    size_t seg_idx;

    Zeta_Core_BinTree(TreeNode, AccessL)(&n, &seg_idx, cntr->root, idx + 1);

    void* ref = NULL;

    if (idx < size) {
        seg = NToSeg_(n);

        if (GetNColor_(n) == dat_color) {
            Zeta_Core_CircularArray ca;
            ca.data = seg->dat.data;
            ca.width = width;
            ca.stride = stride;
            ca.offset = seg->dat.offset;
            ca.size = seg->dat.size;
            ca.capacity = seg_capacity;

            ref = Zeta_Core_CircularArray_Access(&ca, seg_idx, NULL, dst_elem);
        }
#if STAGING
        else if (dst_elem != NULL) {
            ZETA_Core_SeqCntr_Access(origin, seg->ref.beg + seg_idx, NULL,
                                     dst_elem);
        }
#endif
    }

    if (dst_cursor != NULL) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx;
        dst_cursor->n = n;
        dst_cursor->seg_idx = seg_idx;
        dst_cursor->ref = ref;
    }

    return ref;
}

void* Cntr_(Refer)(void* cntr_, void const* pos_cursor_) {
    Cntr* cntr = cntr_;
    Cntr_(Cursor) const* pos_cursor = pos_cursor_;
    Cntr_(Cursor_Check)(cntr, pos_cursor);

    return pos_cursor->ref;
}

void Cntr_(Read)(void const* cntr_, void const* pos_cursor_, size_t cnt,
                 void* dst, size_t dst_stride, void* dst_cursor_) {
    Cntr const* cntr = cntr_;
    Cntr_(Cursor) const* pos_cursor = pos_cursor_;
    Cntr_(Cursor_Check)(cntr, pos_cursor);

#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;
#endif

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    if (cnt == 0) {
        if (dst_cursor != NULL) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->n = pos_cursor->n;
            dst_cursor->seg_idx = pos_cursor->seg_idx;
            dst_cursor->ref = pos_cursor->ref;
        }

        return;
    }

    size_t width = GetWidth_(cntr);
    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;
    size_t size = Cntr_(GetSize)(cntr);

    size_t idx = pos_cursor->idx;

    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(cnt <= size - idx);

    size_t end = idx + cnt;

    ZETA_Core_DebugAssert(dst != NULL);

    void* n = pos_cursor->n;
    Cntr_(Seg) * seg;

    size_t seg_idx = pos_cursor->seg_idx;

#if STAGING
    void* origin_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);
#endif

    Zeta_Core_CircularArray ca;
    ca.width = width;
    ca.stride = stride;
    ca.capacity = seg_capacity;

    while (0 < cnt) {
        seg = NToSeg_(n);

        size_t cur_cnt;
        size_t seg_size;

#if STAGING
        if (GetNColor_(n) == ref_color) {
            seg_size = seg->ref.size;
            cur_cnt = ZETA_Core_GetMinOf(cnt, seg_size - seg_idx);

            ZETA_Core_SeqCntr_Access(origin, seg->ref.beg + seg_idx,
                                     origin_cursor, NULL);
            ZETA_Core_SeqCntr_Read(origin, origin_cursor, cur_cnt, dst,
                                   dst_stride, NULL);
        } else
#endif
        {
            seg_size = seg->dat.size;
            cur_cnt = ZETA_Core_GetMinOf(cnt, seg_size - seg_idx);

            ca.data = seg->dat.data;
            ca.offset = seg->dat.offset;
            ca.size = seg_size;

            Zeta_Core_CircularArray_Cursor ca_cursor;
            Zeta_Core_CircularArray_Access(&ca, seg_idx, &ca_cursor, NULL);
            Zeta_Core_CircularArray_Read(&ca, &ca_cursor, cur_cnt, dst,
                                         dst_stride, NULL);
        }

        dst += dst_stride * cur_cnt;

        seg_idx += cur_cnt;
        cnt -= cur_cnt;

        if (seg_idx == seg_size) {
            n = Zeta_Core_BinTree(TreeNode, StepR)(n);
            seg_idx = 0;
        }
    }

    if (dst_cursor == NULL) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = end;
    dst_cursor->n = n;
    dst_cursor->seg_idx = seg_idx;

    if (cntr->rb == n
#if STAGING
        || GetNColor_(n) == ref_color
#endif
    ) {
        dst_cursor->ref = NULL;
        return;
    }

    seg = NToSeg_(n);

    ca.data = seg->dat.data;
    ca.offset = seg->dat.offset;
    ca.size = seg->dat.size;

    dst_cursor->ref = Zeta_Core_CircularArray_Access(&ca, seg_idx, NULL, NULL);
}

void Cntr_(Write)(void* cntr_, void* pos_cursor_, size_t cnt, void const* src,
                  size_t src_stride, void* dst_cursor_) {
    Cntr* cntr = cntr_;
    Cntr_(Cursor)* pos_cursor = pos_cursor_;
    Cntr_(Cursor_Check)(cntr, pos_cursor);

#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;
#endif

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    if (cnt == 0) {
        if (dst_cursor != NULL) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->n = pos_cursor->n;
            dst_cursor->seg_idx = pos_cursor->seg_idx;
            dst_cursor->ref = pos_cursor->ref;
        }

        return;
    }

    size_t width = GetWidth_(cntr);
    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;
    size_t size = Cntr_(GetSize)(cntr);

    size_t idx = pos_cursor->idx;

    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(cnt <= size - idx);

    size_t end = idx + cnt;

    ZETA_Core_DebugAssert(src != NULL);

    size_t seg_idx = pos_cursor->seg_idx;

    void* n = pos_cursor->n;

#if STAGING
    void* l_n = Zeta_Core_BinTree(TreeNode, StepL)(n);
#endif

    Cntr_(Seg) * seg;

    Zeta_Core_CircularArray ca;
    ca.width = width;
    ca.stride = stride;
    ca.capacity = seg_capacity;

#if STAGING
    void* origin_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);

    unsigned long long random_seed = Zeta_Core_GetRandom();

    if (GetNColor_(n) == ref_color && 0 < seg_idx) {
        seg = NToSeg_(n);

        size_t seg_size = seg->ref.size;

        size_t seg_cnt = ZETA_Core_UnsafeCeilIntDiv(seg_size, seg_capacity);

        size_t k = (seg_idx * seg_cnt + seg_cnt - 1) / seg_size;

        size_t l_seg_size = k * seg_size / seg_cnt;

        size_t avg_seg_size = (k + 1) * seg_size / seg_cnt - l_seg_size;

        seg_idx -= l_seg_size;

        if (0 < l_seg_size) {
            Cntr_(Seg)* new_seg = AllocateRefSeg_(cntr);
            cntr->root =
                Zeta_Core_RBTree(TreeNode, Insert)(l_n, n, &new_seg->n);

            new_seg->ref.beg = seg->ref.beg;
            new_seg->ref.size = l_seg_size;

            seg->ref.beg += l_seg_size;
            seg->ref.size -= l_seg_size;

            Zeta_Core_BinTree(TreeNode, SetSize)(&new_seg->n,
                                                 new_seg->ref.size);

            l_n = &new_seg->n;
        }

        if (seg_idx == 0) { goto WRITE_INTO_REF_SEG_END; }

        size_t cnt_a = seg_idx;
        size_t cnt_b = ZETA_Core_GetMinOf(cnt, avg_seg_size - seg_idx);
        size_t cnt_c = avg_seg_size - cnt_a - cnt_b;

        cnt -= cnt_b;

        if (avg_seg_size == seg->ref.size) {
            void* data = AllocateData_(cntr, NULL);

            ZETA_Core_SeqCntr_Access(origin, seg->ref.beg, origin_cursor, NULL);

            ZETA_Core_SeqCntr_Read(origin, origin_cursor, cnt_a, data, stride,
                                   NULL);

            Zeta_Core_ElemCopy(data + stride * cnt_a, src, width, stride,
                               src_stride, cnt_b);

            if (0 < cnt_c) {
                ZETA_Core_SeqCntr_Access(origin, seg->ref.beg + cnt_a + cnt_b,
                                         origin_cursor, NULL);

                ZETA_Core_SeqCntr_Read(origin, origin_cursor, cnt_c,
                                       data + stride * (cnt_a + cnt_b), stride,
                                       NULL);
            }

            SetNColor_(n, dat_color);

            seg->dat.data = data;
            seg->dat.offset = 0;
            seg->dat.size = avg_seg_size;

            seg_idx += cnt_b;

            src += src_stride * cnt_b;

            if (seg_idx == avg_seg_size) {
                Zeta_Core_BinTree(TreeNode, SetSize)(n, avg_seg_size);

                l_n = n;
                n = Zeta_Core_BinTree(TreeNode, StepR)(n);

                seg_idx = 0;
            }
        } else {
            Cntr_(Seg)* new_seg = AllocateDatSeg_(cntr);
            cntr->root =
                Zeta_Core_RBTree(TreeNode, Insert)(l_n, n, &new_seg->n);

            new_seg->dat.size = avg_seg_size;

            void* data = new_seg->dat.data;

            ZETA_Core_SeqCntr_Access(origin, seg->ref.beg, origin_cursor, NULL);

            ZETA_Core_SeqCntr_Read(origin, origin_cursor, cnt_a, data, stride,
                                   NULL);

            Zeta_Core_ElemCopy(data + stride * cnt_a, src, width, stride,
                               src_stride, cnt_b);

            if (0 < cnt_c) {
                ZETA_Core_SeqCntr_Access(origin, seg->ref.beg + cnt_a + cnt_b,
                                         origin_cursor, NULL);

                ZETA_Core_SeqCntr_Read(origin, origin_cursor, cnt_c,
                                       data + stride * (cnt_a + cnt_b), stride,
                                       NULL);
            }

            seg->ref.beg += avg_seg_size;
            seg->ref.size -= avg_seg_size;

            seg_idx += cnt_b;

            src += src_stride * cnt_b;

            if (seg_idx < new_seg->dat.size) {
                Zeta_Core_BinTree(TreeNode, SetSize)(n, seg->ref.size);

                n = &new_seg->n;
            } else {
                Zeta_Core_BinTree(TreeNode, SetSize)(&new_seg->n,
                                                     new_seg->dat.size);

                l_n = &new_seg->n;
                seg_idx = 0;
            }
        }
    }

WRITE_INTO_REF_SEG_END:

#endif

    while (0 < cnt) {
        seg = NToSeg_(n);

        if (GetNColor_(n) == dat_color) {
            ca.data = seg->dat.data;
            ca.offset = seg->dat.offset;
            ca.size = seg->dat.size;

            size_t cur_cnt = ZETA_Core_GetMinOf(cnt, ca.size - seg_idx);

            Zeta_Core_CircularArray_Cursor ca_cursor;
            Zeta_Core_CircularArray_Access(&ca, seg_idx, &ca_cursor, NULL);
            Zeta_Core_CircularArray_Write(&ca, &ca_cursor, cur_cnt, src,
                                          src_stride, NULL);

            src += src_stride * cur_cnt;

            seg_idx += cur_cnt;
            cnt -= cur_cnt;

            if (seg_idx == seg->dat.size) {
#if STAGING
                l_n = n;
#endif
                n = Zeta_Core_BinTree(TreeNode, StepR)(n);
                seg_idx = 0;
            }

            continue;
        }

#if STAGING
        void* r_n = NULL;

        size_t seg_size = seg->ref.size;

        if (seg_capacity < seg_size) {
            size_t split_cnt = GetAvgCnt_(seg_size);

            Cntr_(Seg)* new_seg = AllocateRefSeg_(cntr);
            cntr->root =
                Zeta_Core_RBTree(TreeNode, Insert)(l_n, n, &new_seg->n);

            new_seg->ref.beg = seg->ref.beg;
            new_seg->ref.size = split_cnt;

            seg->ref.beg += split_cnt;
            seg->ref.size -= split_cnt;

            r_n = n;

            seg = new_seg;
            n = &new_seg->n;

            seg_size = seg->ref.size;
        }

        Zeta_Core_BinTree(TreeNode, SetSize)(&seg->n, seg_size);

        size_t cur_cnt = ZETA_Core_GetMinOf(cnt, seg_size);

        void* data = AllocateData_(cntr, NULL);

        Zeta_Core_ElemCopy(data, src, width, stride, src_stride, cur_cnt);

        src += src_stride * cur_cnt;

        SetNColor_(n, dat_color);

        if (cur_cnt == seg_size) {
            seg->dat.data = data;
            seg->dat.offset = 0;
            seg->dat.size = seg_size;

            cnt -= cur_cnt;
            l_n = n;
            n = r_n == NULL ? Zeta_Core_BinTree(TreeNode, StepR)(n) : r_n;
            continue;
        }

        ZETA_Core_SeqCntr_Access(origin, seg->ref.beg + cur_cnt, origin_cursor,
                                 NULL);

        ZETA_Core_SeqCntr_Read(origin, origin_cursor, seg_size - cur_cnt,
                               data + stride * cur_cnt, stride, NULL);

        seg->dat.data = data;
        seg->dat.offset = 0;
        seg->dat.size = seg_size;

        if (r_n != NULL) {
            Cntr_(Seg)* r_seg = NToSeg_(r_n);

            Zeta_Core_BinTree(TreeNode, SetSize)(
                r_n, GetNColor_(r_n) == ref_color ? r_seg->ref.size
                                                  : r_seg->dat.size);
        }

        seg_idx = cur_cnt;

        break;
#endif
    }

    if (cntr->rb != n) {
        seg = NToSeg_(n);

        Zeta_Core_BinTree(TreeNode, SetSize)(
            n, StagingThreeWay(GetNColor_(n) == ref_color, seg->ref.size,
                               seg->dat.size));
    }

    if (dst_cursor == NULL) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = end;
    dst_cursor->n = n;
    dst_cursor->seg_idx = seg_idx;

    if (cntr->rb == n || GetNColor_(n) == ref_color) {
        dst_cursor->ref = NULL;
        return;
    }

    seg = NToSeg_(n);

    ca.data = seg->dat.data;
    ca.offset = seg->dat.offset;
    ca.size = seg->dat.size;

    dst_cursor->ref = Zeta_Core_CircularArray_Access(&ca, seg_idx, NULL, NULL);
}

void* Cntr_(PushL)(void* cntr_, size_t cnt, void* dst_cursor_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    Cntr_(Cursor) pos_cursor;
    Cntr_(PeekL)(cntr, &pos_cursor, NULL);

    Cntr_(Insert)(cntr, &pos_cursor, cnt);

    if (dst_cursor != NULL) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = pos_cursor.idx;
        dst_cursor->n = pos_cursor.n;
        dst_cursor->seg_idx = pos_cursor.seg_idx;
        dst_cursor->ref = pos_cursor.ref;
    }

    return pos_cursor.ref;
}

void* Cntr_(PushR)(void* cntr_, size_t cnt, void* dst_cursor_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    Cntr_(Cursor)* dst_cursor = dst_cursor_;

    Cntr_(Cursor) pos_cursor;
    Cntr_(GetRBCursor)(cntr, &pos_cursor);

    Cntr_(Insert)(cntr, &pos_cursor, cnt);

    if (dst_cursor != NULL) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = pos_cursor.idx;
        dst_cursor->n = pos_cursor.n;
        dst_cursor->seg_idx = pos_cursor.seg_idx;
        dst_cursor->ref = pos_cursor.ref;
    }

    return pos_cursor.ref;
}

void* Cntr_(Insert)(void* cntr_, void* pos_cursor_, size_t cnt) {
    Cntr* cntr = cntr_;
    Cntr_(Cursor)* pos_cursor = pos_cursor_;
    Cntr_(Cursor_Check)(cntr, pos_cursor);

    if (cnt == 0) { return pos_cursor->ref; }

    ZETA_Core_SeqCntr_CheckInsertable(
        pos_cursor->idx, cnt, Cntr_(GetSize)(cntr), Cntr_(GetCapacity)(cntr));

    size_t width = GetWidth_(cntr);
    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;

    void* m_n = pos_cursor->n;
    size_t seg_idx = pos_cursor->seg_idx;

    ZETA_Core_DebugAssert(cntr->lb != m_n);

    unsigned long long random_seed = Zeta_Core_GetRandom();

    void* l_n;
    void* r_n;

    Cntr_(Seg) * l_seg;
    Cntr_(Seg) * m_seg;
    Cntr_(Seg) * r_seg;

    size_t l_vacant;
    size_t m_vacant;
    size_t r_vacant;

    Zeta_Core_CircularArray l_ca;
    l_ca.width = width;
    l_ca.stride = stride;
    l_ca.capacity = seg_capacity;

    Zeta_Core_CircularArray m_ca;
    m_ca.width = width;
    m_ca.stride = stride;
    m_ca.capacity = seg_capacity;

    Zeta_Core_CircularArray r_ca;
    r_ca.width = width;
    r_ca.stride = stride;
    r_ca.capacity = seg_capacity;

    Zeta_Core_CircularArray_Cursor ca_cursor;

    if (seg_idx == 0) {
        l_n = Zeta_Core_BinTree(TreeNode, StepL)(m_n);

        r_n = m_n;

        m_n = NULL;
        m_seg = NULL;

        if (cntr->lb == l_n) {
            l_ca.size = 0;
            l_vacant = 0;
        } else {
            l_seg = NToSeg_(l_n);
            SetSegState(l_seg, &l_ca, &l_vacant);
        }

        if (cntr->rb == r_n) {
            r_ca.size = 0;
            r_vacant = 0;
        } else {
            r_seg = NToSeg_(r_n);
            SetSegState(r_seg, &r_ca, &r_vacant);
        }
    } else {
        m_seg = NToSeg_(m_n);
        SetSegState(m_seg, &m_ca, &m_vacant);

        if (cnt <= m_vacant) {
            void* ref;

#if STAGING
            if (GetNColor_(m_n) == ref_color) {
                m_ca.data = AllocateData_(cntr, NULL);
                m_ca.offset = 0;
                m_ca.size = 0;

                RefShoveL_(cntr, &m_ca, m_seg, seg_idx, cnt,
                           m_seg->ref.size + cnt);

                SetNColor_(&m_seg->n, dat_color);

                m_seg->dat.data = m_ca.data;

                ref =
                    Zeta_Core_CircularArray_Access(&m_ca, seg_idx, NULL, NULL);
            } else
#endif
            {
                Zeta_Core_CircularArray_Access(&m_ca, seg_idx, &ca_cursor,
                                               NULL);
                ref = Zeta_Core_CircularArray_Insert(&m_ca, &ca_cursor, cnt);
            }

            m_seg->dat.offset = m_ca.offset;
            m_seg->dat.size = m_ca.size;
            Zeta_Core_BinTree(TreeNode, SetSize)(m_n, m_ca.size);

            pos_cursor->n = m_n;
            pos_cursor->seg_idx = seg_idx;
            pos_cursor->ref = ref;

            return ref;
        }

        l_n = Zeta_Core_BinTree(TreeNode, StepL)(m_n);
        r_n = Zeta_Core_BinTree(TreeNode, StepR)(m_n);

        if (cntr->lb == l_n) {
            l_ca.size = 0;
            l_vacant = 0;
        } else {
            l_seg = NToSeg_(l_n);
            SetSegState(l_seg, &l_ca, &l_vacant);
        }

        if (cntr->rb == r_n) {
            r_ca.size = 0;
            r_vacant = 0;
        } else {
            r_seg = NToSeg_(r_n);
            SetSegState(r_seg, &r_ca, &r_vacant);
        }

        size_t ml_size = seg_idx;
        size_t mr_size = m_ca.size - seg_idx;

        size_t new_l_m_size = l_ca.size + m_ca.size + cnt;
        size_t new_m_r_size = m_ca.size + r_ca.size + cnt;

        size_t l_m_vacant = l_vacant + m_vacant;
        size_t m_r_vacant = m_vacant + r_vacant;

        bool_t l_m_ok = new_l_m_size <= seg_capacity * 2 && cnt <= l_m_vacant;

        bool_t m_r_ok = new_m_r_size <= seg_capacity * 2 && cnt <= m_r_vacant;

        if (!l_m_ok && !m_r_ok) { goto SPLIT; }

        int shove_dir = Zeta_Choose2(
            l_m_ok && (!m_r_ok || m_r_vacant <= l_m_vacant),
            m_r_ok && (!l_m_ok || l_m_vacant <= m_r_vacant), &random_seed);

        if (shove_dir == 0) {
#if STAGING
            if (GetNColor_(l_n) == ref_color) {
                TransRefSegToDatSeg_(cntr, l_seg);
                l_ca.data = l_seg->dat.data;
                l_ca.offset = l_seg->dat.offset;
            }
#endif

            size_t ret_seg_idx = l_ca.size + ml_size;

            size_t new_l_size = (new_l_m_size + (m_ca.size <= l_ca.size)) / 2;

#if STAGING
            if (GetNColor_(m_n) == ref_color) {
                RefShoveL_(cntr, &l_ca, m_seg, ml_size, cnt,
                           new_l_size - l_ca.size);

                if (GetNColor_(m_n) == ref_color) {
                    m_ca.size = m_seg->ref.size;
                } else {
                    m_ca.data = m_seg->dat.data;
                    m_ca.offset = m_seg->dat.offset;
                    m_ca.size = m_seg->dat.size;
                }
            } else
#endif
            {
                SegShoveL(&l_ca, &m_ca, ml_size, cnt, new_l_size - l_ca.size);

                m_seg->dat.offset = m_ca.offset;
                m_seg->dat.size = m_ca.size;
            }

            Zeta_Core_BinTree(TreeNode, SetSize)(m_n, m_ca.size);

            l_seg->dat.offset = l_ca.offset;
            l_seg->dat.size = l_ca.size;
            Zeta_Core_BinTree(TreeNode, SetSize)(l_n, l_ca.size);

            if (ret_seg_idx < l_ca.size) {
                pos_cursor->n = l_n;
                pos_cursor->seg_idx = ret_seg_idx;
                pos_cursor->ref = Zeta_Core_CircularArray_Access(
                    &l_ca, ret_seg_idx, NULL, NULL);
            } else {
                ret_seg_idx -= l_ca.size;

                pos_cursor->n = m_n;
                pos_cursor->seg_idx = ret_seg_idx;
                pos_cursor->ref =
                    StagingThreeWay(GetNColor_(m_n) == ref_color, NULL,
                                    Zeta_Core_CircularArray_Access(
                                        &m_ca, ret_seg_idx, NULL, NULL));
            }
        } else {
#if STAGING
            if (GetNColor_(r_n) == ref_color) {
                TransRefSegToDatSeg_(cntr, r_seg);
                r_ca.data = r_seg->dat.data;
                r_ca.offset = r_seg->dat.offset;
            }
#endif

            size_t ret_seg_idx = seg_idx;

            size_t new_r_size = (new_m_r_size + (m_ca.size <= r_ca.size)) / 2;

#if STAGING
            if (GetNColor_(m_n) == ref_color) {
                RefShoveR_(cntr, m_seg, &r_ca, mr_size, cnt,
                           new_r_size - r_ca.size);

                if (GetNColor_(m_n) == ref_color) {
                    m_ca.size = m_seg->ref.size;
                } else {
                    m_ca.data = m_seg->dat.data;
                    m_ca.offset = m_seg->dat.offset;
                    m_ca.size = m_seg->dat.size;
                }
            } else
#endif
            {
                SegShoveR(&m_ca, &r_ca, mr_size, cnt, new_r_size - r_ca.size);

                m_seg->dat.offset = m_ca.offset;
                m_seg->dat.size = m_ca.size;
            }

            Zeta_Core_BinTree(TreeNode, SetSize)(m_n, m_ca.size);

            r_seg->dat.offset = r_ca.offset;
            r_seg->dat.size = r_ca.size;
            Zeta_Core_BinTree(TreeNode, SetSize)(r_n, r_ca.size);

            if (ret_seg_idx < m_ca.size) {
                pos_cursor->n = m_n;
                pos_cursor->seg_idx = ret_seg_idx;
                pos_cursor->ref =
                    StagingThreeWay(GetNColor_(m_n) == ref_color, NULL,
                                    Zeta_Core_CircularArray_Access(
                                        &m_ca, ret_seg_idx, NULL, NULL));
            } else {
                ret_seg_idx -= m_ca.size;

                pos_cursor->n = r_n;
                pos_cursor->seg_idx = ret_seg_idx;
                pos_cursor->ref = Zeta_Core_CircularArray_Access(
                    &r_ca, ret_seg_idx, NULL, NULL);
            }
        }

        return pos_cursor->ref;

    SPLIT:

        if (ml_size <= l_vacant) {
#if STAGING
            if (GetNColor_(l_n) == ref_color) {
                TransRefSegToDatSeg_(cntr, l_seg);

                l_ca.data = l_seg->dat.data;
                l_ca.offset = l_seg->dat.offset;
            }
#endif

#if STAGING
            if (GetNColor_(m_n) == ref_color) {
                PushRefR_(cntr, &l_ca, m_seg->ref.beg, ml_size);

                m_seg->ref.beg += ml_size;
                m_ca.size -= ml_size;
            } else
#endif
            {
                SegShoveL(&l_ca, &m_ca, 0, 0, ml_size);
            }

            ml_size = 0;
            l_vacant = seg_capacity - l_ca.size;
        }

        if (mr_size <= r_vacant) {
#if STAGING
            if (GetNColor_(r_n) == ref_color) {
                TransRefSegToDatSeg_(cntr, r_seg);

                r_ca.data = r_seg->dat.data;
                r_ca.offset = r_seg->dat.offset;
            }
#endif

#if STAGING
            if (GetNColor_(m_n) == ref_color) {
                PushRefL_(cntr, &r_ca, m_seg->ref.beg + ml_size, mr_size);

                m_ca.size -= mr_size;
            } else
#endif
            {
                SegShoveR(&m_ca, &r_ca, 0, 0, mr_size);
            }

            mr_size = 0;
            r_vacant = seg_capacity - r_ca.size;
        }

        if (ml_size == 0 && mr_size == 0) { goto INS; }

        if (ml_size == 0) {
            r_n = m_n;
            r_seg = m_seg;
            r_ca.data = m_ca.data;
            r_ca.offset = m_ca.offset;
            r_ca.size = m_ca.size;
            r_vacant =
                seg_capacity - ZETA_Core_GetMinOf(r_ca.size, seg_capacity);

            m_n = NULL;
            m_seg = NULL;

            goto INS;
        }

        if (mr_size == 0) {
            l_n = m_n;
            l_seg = m_seg;
            l_ca.data = m_ca.data;
            l_ca.offset = m_ca.offset;
            l_ca.size = m_ca.size;
            l_vacant =
                seg_capacity - ZETA_Core_GetMinOf(l_ca.size, seg_capacity);

            m_n = NULL;
            m_seg = NULL;

            goto INS;
        }

        int split_dir =
            Zeta_Choose2(ml_size <= mr_size, mr_size <= ml_size, &random_seed);

        if (split_dir == 0) {
#if STAGING
            if (GetNColor_(m_n) == ref_color) {
                Cntr_(Seg)* new_l_seg = AllocateRefSeg_(cntr);
                cntr->root =
                    Zeta_Core_RBTree(TreeNode, Insert)(l_n, m_n, &new_l_seg->n);

                l_n = &new_l_seg->n;
                l_seg = new_l_seg;
                l_seg->ref.beg = m_seg->ref.beg;
                l_ca.size = ml_size;
                l_vacant =
                    seg_capacity - ZETA_Core_GetMinOf(l_ca.size, seg_capacity);

                r_n = m_n;
                r_seg = m_seg;
                r_seg->ref.beg += ml_size;
                r_ca.size = mr_size;
                r_vacant =
                    seg_capacity - ZETA_Core_GetMinOf(r_ca.size, seg_capacity);
            } else
#endif
            {
                Cntr_(Seg)* new_l_seg = AllocateDatSeg_(cntr);
                cntr->root =
                    Zeta_Core_RBTree(TreeNode, Insert)(l_n, m_n, &new_l_seg->n);

                l_n = &new_l_seg->n;
                l_seg = new_l_seg;
                l_ca.data = new_l_seg->dat.data;
                l_ca.offset = 0;
                l_ca.size = 0;

                r_n = m_n;
                r_seg = m_seg;
                r_ca.data = m_ca.data;
                r_ca.offset = m_ca.offset;
                r_ca.size = m_ca.size;

                SegShoveL(&l_ca, &r_ca, 0, 0, ml_size);

                l_vacant = seg_capacity - l_ca.size;
                r_vacant = seg_capacity - r_ca.size;
            }
        } else {
#if STAGING
            if (GetNColor_(m_n) == ref_color) {
                Cntr_(Seg)* new_r_seg = AllocateRefSeg_(cntr);
                cntr->root =
                    Zeta_Core_RBTree(TreeNode, Insert)(m_n, r_n, &new_r_seg->n);

                l_n = m_n;
                l_seg = m_seg;
                l_ca.size = ml_size;
                l_vacant =
                    seg_capacity - ZETA_Core_GetMinOf(l_ca.size, seg_capacity);

                r_n = &new_r_seg->n;
                r_seg = new_r_seg;
                r_seg->ref.beg = m_seg->ref.beg + ml_size;
                r_ca.size = mr_size;
                r_vacant =
                    seg_capacity - ZETA_Core_GetMinOf(r_ca.size, seg_capacity);
            } else
#endif
            {
                Cntr_(Seg)* new_r_seg = AllocateDatSeg_(cntr);
                cntr->root =
                    Zeta_Core_RBTree(TreeNode, Insert)(m_n, r_n, &new_r_seg->n);

                l_n = m_n;
                l_seg = m_seg;
                l_ca.data = m_ca.data;
                l_ca.offset = m_ca.offset;
                l_ca.size = m_ca.size;

                r_n = &new_r_seg->n;
                r_seg = new_r_seg;
                r_ca.data = r_seg->dat.data;
                r_ca.offset = 0;
                r_ca.size = 0;

                SegShoveR(&l_ca, &r_ca, 0, 0, mr_size);

                l_vacant = seg_capacity - l_ca.size;
                r_vacant = seg_capacity - r_ca.size;
            }
        }

        m_n = NULL;
        m_seg = NULL;
    }

INS:

#if STAGING
    bool_t l_is_ref = cntr->lb != l_n && GetNColor_(l_n) == ref_color;
    bool_t r_is_ref = cntr->rb != r_n && GetNColor_(r_n) == ref_color;
#endif

    bool_t l_shove;
    bool_t r_shove;

    {
        size_t res_size =
            seg_capacity - 1 - (cnt + seg_capacity - 1) % seg_capacity;

        if (l_ca.size + r_ca.size <= res_size) {
            l_shove = TRUE;
            r_shove = TRUE;
        } else if (Zeta_Choose2(l_ca.size <= r_ca.size, r_ca.size <= l_ca.size,
                                &random_seed) == 0) {
            l_shove = l_ca.size <= res_size;
            r_shove = FALSE;
        } else {
            l_shove = FALSE;
            r_shove = r_ca.size <= res_size;
        }

        if (l_ca.size == 0) { l_shove = FALSE; }
        if (r_ca.size == 0) { r_shove = FALSE; }
    }

#if STAGING
    if (l_is_ref && l_shove) {
        l_seg->ref.size = l_ca.size;

        TransRefSegToDatSeg_(cntr, l_seg);

        l_ca.data = l_seg->dat.data;
        l_ca.offset = 0;
    }
#endif

#if STAGING
    if (r_is_ref && r_shove) {
        r_seg->ref.size = r_ca.size;

        TransRefSegToDatSeg_(cntr, r_seg);

        r_ca.data = r_seg->dat.data;
        r_ca.offset = 0;
    }
#endif

    size_t total_size =
        (l_shove ? l_ca.size : 0) + cnt + (r_shove ? r_ca.size : 0);

    size_t ret_seg_idx = l_ca.size;

    if (l_shove && r_shove && total_size <= seg_capacity) {
        if (Zeta_Choose2(r_ca.size <= l_ca.size, l_ca.size <= r_ca.size,
                         &random_seed) == 0) {
            SegShoveL(&l_ca, &r_ca, 0, cnt, cnt + r_ca.size);

            EraseSeg_(cntr, r_seg);

            l_seg->dat.offset = l_ca.offset;
            l_seg->dat.size = l_ca.size;
            Zeta_Core_BinTree(TreeNode, SetSize)(l_n, l_ca.size);

            pos_cursor->n = l_n;
            pos_cursor->seg_idx = ret_seg_idx;
            pos_cursor->ref =
                Zeta_Core_CircularArray_Access(&l_ca, ret_seg_idx, NULL, NULL);
        } else {
            SegShoveR(&l_ca, &r_ca, 0, cnt, l_ca.size + cnt);

            EraseSeg_(cntr, l_seg);

            r_seg->dat.offset = r_ca.offset;
            r_seg->dat.size = r_ca.size;
            Zeta_Core_BinTree(TreeNode, SetSize)(r_n, r_ca.size);

            pos_cursor->n = r_n;
            pos_cursor->seg_idx = ret_seg_idx;
            pos_cursor->ref =
                Zeta_Core_CircularArray_Access(&r_ca, ret_seg_idx, NULL, NULL);
        }

        return pos_cursor->ref;
    }

    size_t new_l_size;
    size_t new_r_size;

    if (l_shove) {
        new_l_size = GetAvgCnt_(total_size);
        total_size -= new_l_size;
    } else {
        new_l_size = l_ca.size;
    }

    if (r_shove) {
        new_r_size = GetAvgCnt_(total_size);
        total_size -= new_r_size;
    } else {
        new_r_size = r_ca.size;
    }

    if (l_ca.size < new_l_size) {
        Zeta_Core_CircularArray_Cursor ca_cursor;
        Zeta_Core_CircularArray_GetRBCursor(&l_ca, &ca_cursor);
        Zeta_Core_CircularArray_Insert(&l_ca, &ca_cursor,
                                       new_l_size - l_ca.size);
    }

    if (r_ca.size < new_r_size) {
        Zeta_Core_CircularArray_Cursor ca_cursor;
        Zeta_Core_CircularArray_PeekL(&r_ca, &ca_cursor, NULL);
        Zeta_Core_CircularArray_Insert(&r_ca, &ca_cursor,
                                       new_r_size - r_ca.size);
    }

    if (cntr->lb != l_n) {
#if STAGING
        if (GetNColor_(l_n) == ref_color) {
            l_seg->ref.size = l_ca.size;
        } else
#endif
        {
            l_seg->dat.offset = l_ca.offset;
            l_seg->dat.size = l_ca.size;
        }

        Zeta_Core_BinTree(TreeNode, SetSize)(l_n, l_ca.size);
    }

    if (cntr->rb != r_n) {
#if STAGING
        if (GetNColor_(r_n) == ref_color) {
            r_seg->ref.size = r_ca.size;
        } else
#endif
        {
            r_seg->dat.offset = r_ca.offset;
            r_seg->dat.size = r_ca.size;
        }

        Zeta_Core_BinTree(TreeNode, SetSize)(r_n, r_ca.size);
    }

    if (0 < total_size && m_n != NULL) {
        size_t cur_size = GetAvgCnt_(total_size);
        total_size -= cur_size;

#if STAGING
        if (GetNColor_(m_n) == ref_color) {
            SetNColor_(m_n, dat_color);
            m_seg->dat.data = AllocateData_(cntr, NULL);
        }
#endif

        m_seg->dat.offset = 0;
        m_seg->dat.size = cur_size;
        Zeta_Core_BinTree(TreeNode, SetSize)(m_n, cur_size);

        r_n = m_n;
        r_seg = m_seg;

        m_n = NULL;
        m_seg = NULL;
    }

    while (0 < total_size) {
        size_t cur_size = GetAvgCnt_(total_size);
        total_size -= cur_size;

        Cntr_(Seg)* new_r_seg = AllocateDatSeg_(cntr);

        cntr->root =
            Zeta_Core_RBTree(TreeNode, Insert)(l_n, r_n, &new_r_seg->n);

        new_r_seg->dat.size = cur_size;
        Zeta_Core_BinTree(TreeNode, SetSize)(&new_r_seg->n, cur_size);

        r_n = &new_r_seg->n;
        r_seg = new_r_seg;
    }

    if (m_n != NULL) { EraseSeg_(cntr, m_seg); }

    if (ret_seg_idx < l_ca.size) {
        pos_cursor->n = l_n;
        pos_cursor->seg_idx = ret_seg_idx;

#if STAGING
        if (GetNColor_(l_n) == ref_color) {
            pos_cursor->ref = NULL;
        } else
#endif
        {
            l_ca.data = l_seg->dat.data;
            l_ca.offset = l_seg->dat.offset;
            l_ca.size = l_seg->dat.size;

            pos_cursor->ref =
                Zeta_Core_CircularArray_Access(&l_ca, ret_seg_idx, NULL, NULL);
        }

        return pos_cursor->ref;
    }

    ret_seg_idx -= l_ca.size;

    pos_cursor->n = r_n;
    pos_cursor->seg_idx = ret_seg_idx;

#if STAGING
    if (GetNColor_(r_n) == ref_color) {
        pos_cursor->ref = NULL;
    } else
#endif
    {
        r_ca.data = r_seg->dat.data;
        r_ca.offset = r_seg->dat.offset;
        r_ca.size = r_seg->dat.size;

        pos_cursor->ref =
            Zeta_Core_CircularArray_Access(&r_ca, ret_seg_idx, NULL, NULL);
    }

    return pos_cursor->ref;
}

void Cntr_(PopL)(void* cntr, size_t cnt) {
    Cntr_(Cursor) pos_cursor;
    Cntr_(PeekL)(cntr, &pos_cursor, NULL);

    Cntr_(Erase)(cntr, &pos_cursor, cnt);
}

void Cntr_(PopR)(void* cntr, size_t cnt) {
    size_t size = Cntr_(GetSize)(cntr);

    ZETA_Core_DebugAssert(cnt <= size);

    Cntr_(Cursor) pos_cursor;
    Cntr_(Access)(cntr, size - cnt, &pos_cursor, NULL);

    Cntr_(Erase)(cntr, &pos_cursor, cnt);
}

void Cntr_(Erase)(void* cntr_, void* pos_cursor_, size_t cnt) {
    Cntr* cntr = cntr_;
    Cntr_(Cursor)* pos_cursor = pos_cursor_;
    Cntr_(Cursor_Check)(cntr, pos_cursor);

    if (cnt == 0) { return; }

    size_t width = GetWidth_(cntr);
    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;
    size_t size = Cntr_(GetSize)(cntr);

    ZETA_Core_SeqCntr_CheckErasable(pos_cursor->idx, cnt, size);

    void* m_n = pos_cursor->n;
    size_t seg_idx = pos_cursor->seg_idx;

    void* l_n = Zeta_Core_BinTree(TreeNode, StepL)(m_n);

    Cntr_(Seg)* m_seg = NToSeg_(m_n);

    Cntr_(Seg)* first_seg = m_seg;
    size_t first_seg_idx = seg_idx;

    Cntr_(Seg)* last_seg = m_n;

    Zeta_Core_CircularArray m_ca;
    m_ca.width = width;
    m_ca.stride = stride;
    m_ca.capacity = seg_capacity;

#if STAGING
    if (GetNColor_(m_n) == ref_color && 0 < seg_idx) {
        m_seg = NToSeg_(m_n);

        size_t ref_beg = m_seg->ref.beg;
        size_t ref_size = m_seg->ref.size;

        if (ref_size - seg_idx <= cnt) {
            m_seg->ref.size = seg_idx;
            cnt -= ref_size - seg_idx;
            m_n = Zeta_Core_BinTree(TreeNode, StepR)(m_n);
        } else {
            Cntr_(Seg)* pre_m_seg = AllocateRefSeg_(cntr);
            cntr->root = Zeta_Core_RBTree(TreeNode, Insert)(l_n, &m_seg->n,
                                                            &pre_m_seg->n);

            first_seg = pre_m_seg;

            pre_m_seg->ref.beg = ref_beg;
            pre_m_seg->ref.size = seg_idx;

            m_seg->ref.beg = ref_beg + seg_idx + cnt;
            m_seg->ref.size = ref_size - seg_idx - cnt;

            cnt = 0;
        }

        seg_idx = 0;
    }
#endif

    while (0 < cnt) {
        m_seg = NToSeg_(m_n);
        last_seg = m_seg;

#if STAGING
        if (GetNColor_(m_n) == ref_color) {
            if (m_seg->ref.size <= cnt) {
                cnt -= m_seg->ref.size;
                void* nxt_m_n = Zeta_Core_BinTree(TreeNode, StepR)(m_n);
                EraseSeg_(cntr, m_seg);
                m_n = nxt_m_n;

                continue;
            }

            m_seg->ref.beg += cnt;
            m_seg->ref.size -= cnt;

            break;
        }
#endif

        if (seg_idx == 0 && m_seg->dat.size <= cnt) {
            cnt -= m_seg->dat.size;
            void* nxt_m_n = Zeta_Core_BinTree(TreeNode, StepR)(m_n);
            EraseSeg_(cntr, m_seg);
            m_n = nxt_m_n;

            continue;
        }

        size_t cur_cnt = ZETA_Core_GetMinOf(cnt, m_seg->dat.size - seg_idx);

        m_ca.data = m_seg->dat.data;
        m_ca.offset = m_seg->dat.offset;
        m_ca.size = m_seg->dat.size;

        Zeta_Core_CircularArray_Cursor ca_cursor;
        Zeta_Core_CircularArray_Access(&m_ca, seg_idx, &ca_cursor, NULL);
        Zeta_Core_CircularArray_Erase(&m_ca, &ca_cursor, cur_cnt);

        m_seg->dat.offset = m_ca.offset;
        m_seg->dat.size = m_ca.size;

        cnt -= cur_cnt;

        if (seg_idx == m_seg->dat.size) {
            m_n = Zeta_Core_BinTree(TreeNode, StepR)(m_n);
            seg_idx = 0;
        }
    }

    void* ret_n = m_n;
    size_t ret_seg_idx = seg_idx;

    unsigned long long random_seed = Zeta_Core_GetRandom();

    bool_t first_exist = first_seg_idx != 0 || &first_seg->n == m_n;
    bool_t last_exist = &last_seg->n == m_n;

    Cntr_(Seg) * a_seg;
    Cntr_(Seg) * b_seg;
    Cntr_(Seg) * c_seg;
    Cntr_(Seg) * d_seg;

    size_t a_size;
    size_t b_size;
    size_t c_size;
    size_t d_size;

    size_t a_vacant;
    size_t b_vacant;
    size_t c_vacant;
    size_t d_vacant;

    void* r_n = last_exist ? Zeta_Core_BinTree(TreeNode, StepR)(m_n) : m_n;

    {
        Cntr_(Seg) * segs_buffer[4] = { NULL, NULL, NULL, NULL };
        size_t segs_buffer_i = 0;

        if (cntr->lb != l_n) { segs_buffer[segs_buffer_i++] = NToSeg_(l_n); }

        if (first_exist) { segs_buffer[segs_buffer_i++] = first_seg; }

        if (last_exist && first_seg != last_seg) {
            segs_buffer[segs_buffer_i++] = last_seg;
        }

        if (cntr->rb != r_n) { segs_buffer[segs_buffer_i++] = NToSeg_(r_n); }

        a_seg = segs_buffer[0];
        b_seg = segs_buffer[1];
        c_seg = segs_buffer[2];
        d_seg = segs_buffer[3];
    }

    if (a_seg == NULL) {
    }
#if STAGING
    else if (GetNColor_(&a_seg->n) == ref_color) {
        a_size = a_seg->ref.size;
        a_vacant = seg_capacity - ZETA_Core_GetMinOf(a_size, seg_capacity);
    }
#endif
    else {
        a_size = a_seg->dat.size;
        a_vacant = seg_capacity - a_size;
    }

    if (b_seg == NULL) {
    }
#if STAGING
    else if (GetNColor_(&b_seg->n) == ref_color) {
        b_size = b_seg->ref.size;
        b_vacant = seg_capacity - ZETA_Core_GetMinOf(b_size, seg_capacity);
    }
#endif
    else {
        b_size = b_seg->dat.size;
        b_vacant = seg_capacity - b_size;
    }

    if (c_seg == NULL) {
    }
#if STAGING
    else if (GetNColor_(&c_seg->n) == ref_color) {
        c_size = c_seg->ref.size;
        c_vacant = seg_capacity - ZETA_Core_GetMinOf(c_size, seg_capacity);
    }
#endif
    else {
        c_size = c_seg->dat.size;
        c_vacant = seg_capacity - c_size;
    }

    if (d_seg == NULL) {
    }
#if STAGING
    else if (GetNColor_(&d_seg->n) == ref_color) {
        d_size = d_seg->ref.size;
        d_vacant = seg_capacity - ZETA_Core_GetMinOf(d_size, seg_capacity);
    }
#endif
    else {
        d_size = d_seg->dat.size;
        d_vacant = seg_capacity - d_size;
    }

    if (a_seg == NULL) { goto UPDATE; }

    if (ret_n == &a_seg->n) { ret_n = NULL; }

    if (b_seg == NULL) { goto UPDATE; }

    if (ret_n == &b_seg->n) {
        ret_n = NULL;
        ret_seg_idx += a_size;
    }

#if STAGING
    if (GetNColor_(&a_seg->n) == ref_color &&
        GetNColor_(&b_seg->n) == ref_color &&
        a_seg->ref.beg + a_seg->ref.size == b_seg->ref.beg) {
        a_seg->ref.size += b_seg->ref.size;

        EraseSeg_(cntr, b_seg);
        b_seg = NULL;

        if (c_seg == NULL) { goto UPDATE; }

        b_seg = c_seg;
        c_seg = d_seg;
        d_seg = NULL;
    }
#endif

    if (c_seg == NULL) { goto MERGE_2; }

    if (ret_n == &c_seg->n) {
        ret_n = NULL;
        ret_seg_idx += a_size + b_size;
    }

#if STAGING
    if (GetNColor_(&b_seg->n) == ref_color &&
        GetNColor_(&c_seg->n) == ref_color &&
        b_seg->ref.beg + b_seg->ref.size == c_seg->ref.beg) {
        b_seg->ref.size += c_seg->ref.size;

        EraseSeg_(cntr, c_seg);
        c_seg = NULL;

        if (d_seg == NULL) { goto MERGE_2; }

        c_seg = d_seg;
        d_seg = NULL;

        goto MERGE_3;
    }
#endif

    if (d_seg == NULL) { goto MERGE_3; }

    if (ret_n == &d_seg->n) {
        ret_n = NULL;
        ret_seg_idx += a_size + b_size + c_size;
    }

#if STAGING
    if (GetNColor_(&c_seg->n) == ref_color &&
        GetNColor_(&d_seg->n) == ref_color &&
        c_seg->ref.beg + c_seg->ref.size == d_seg->ref.beg) {
        c_seg->ref.size += d_seg->ref.size;

        EraseSeg_(cntr, d_seg);
        d_seg = NULL;

        goto MERGE_3;
    }
#endif

    goto MERGE_4;

MERGE_2: {
    size_t ab_vacant = a_vacant + b_vacant;

    if (seg_capacity <= ab_vacant) { Merge2_(cntr, a_seg, b_seg); }

    goto UPDATE;
}

MERGE_3: {
    size_t ab_vacant = a_vacant + b_vacant;
    size_t bc_vacant = b_vacant + c_vacant;

    size_t max_vacant = ZETA_Core_GetMaxOf(ab_vacant, bc_vacant);

    if (seg_capacity < max_vacant) {
        if (Zeta_Choose2(bc_vacant <= ab_vacant, ab_vacant <= bc_vacant,
                         &random_seed) == 0) {
            Merge2_(cntr, a_seg, b_seg);
        } else {
            Merge2_(cntr, b_seg, c_seg);
        }
    }

    goto UPDATE;
}

MERGE_4: {
    size_t ab_vacant = a_vacant + b_vacant;
    size_t bc_vacant = b_vacant + c_vacant;
    size_t cd_vacant = c_vacant + d_vacant;

    if (seg_capacity <= ab_vacant && seg_capacity <= cd_vacant) {
        Merge2_(cntr, a_seg, b_seg);
        Merge2_(cntr, c_seg, d_seg);
    } else {
        size_t max_vacant = ZETA_Core_GetMaxOf(ab_vacant, bc_vacant, cd_vacant);

        if (seg_capacity <= max_vacant) {
            switch (Zeta_Choose3(ab_vacant == max_vacant,
                                 bc_vacant == max_vacant,
                                 cd_vacant == max_vacant, &random_seed)) {
                case 0: Merge2_(cntr, a_seg, b_seg); break;
                case 1: Merge2_(cntr, b_seg, c_seg); break;
                case 2: Merge2_(cntr, c_seg, d_seg); break;
            }
        }
    }

    goto UPDATE;
}

UPDATE: {
    if (a_seg == NULL) { goto UPDATE_END; }

    a_size = StagingThreeWay(GetNColor_(&a_seg->n) == ref_color,
                             a_seg->ref.size, a_seg->dat.size);

    if (a_size == 0) {
        EraseSeg_(cntr, a_seg);
    } else {
        Zeta_Core_BinTree(TreeNode, SetSize)(&a_seg->n, a_size);
    }

    if (ret_n != NULL) {
    } else if (ret_seg_idx < a_size) {
        ret_n = &a_seg->n;
    } else {
        ret_seg_idx -= a_size;
    }

    if (b_seg == NULL) { goto UPDATE_END; }

    b_size = StagingThreeWay(GetNColor_(&b_seg->n) == ref_color,
                             b_seg->ref.size, b_seg->dat.size);

    if (b_size == 0) {
        EraseSeg_(cntr, b_seg);
    } else {
        Zeta_Core_BinTree(TreeNode, SetSize)(&b_seg->n, b_size);
    }

    if (ret_n != NULL) {
    } else if (ret_seg_idx < b_size) {
        ret_n = &b_seg->n;
    } else {
        ret_seg_idx -= b_size;
    }

    if (c_seg == NULL) { goto UPDATE_END; }

    c_size = StagingThreeWay(GetNColor_(&c_seg->n) == ref_color,
                             c_seg->ref.size, c_seg->dat.size);

    if (c_size == 0) {
        EraseSeg_(cntr, c_seg);
    } else {
        Zeta_Core_BinTree(TreeNode, SetSize)(&c_seg->n, c_size);
    }

    if (ret_n != NULL) {
    } else if (ret_seg_idx < c_size) {
        ret_n = &c_seg->n;
    } else {
        ret_seg_idx -= c_size;
    }

    if (d_seg == NULL) { goto UPDATE_END; }

    d_size = StagingThreeWay(GetNColor_(&d_seg->n) == ref_color,
                             d_seg->ref.size, d_seg->dat.size);

    if (d_size == 0) {
        EraseSeg_(cntr, d_seg);
    } else {
        Zeta_Core_BinTree(TreeNode, SetSize)(&d_seg->n, d_size);
    }

    if (ret_n == NULL) { ret_n = &d_seg->n; }
}

UPDATE_END:

    pos_cursor->n = ret_n;
    pos_cursor->seg_idx = ret_seg_idx;

    if (cntr->rb == pos_cursor->n
#if STAGING
        || GetNColor_(pos_cursor->n) == ref_color
#endif
    ) {
        pos_cursor->ref = NULL;
        return;
    }

    Cntr_(Seg)* seg = NToSeg_(pos_cursor->n);

    m_ca.data = seg->dat.data;
    m_ca.offset = seg->dat.offset;
    m_ca.size = seg->dat.size;

    pos_cursor->ref =
        Zeta_Core_CircularArray_Access(&m_ca, pos_cursor->seg_idx, NULL, NULL);
}

void Cntr_(EraseAll)(void* cntr_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    EraseAllSeg_(cntr, cntr->root);

    InitTree_(cntr);
}

void Cntr_(Reset)(void* cntr_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    EraseAllSeg_(cntr, cntr->root);

    InitTree_(cntr);
    RefOrigin_(cntr);
}

static void CollectAllResources_(Cntr* cntr, void* n,
                                 Zeta_Core_PoolAllocator* dst_segs,
                                 Zeta_Core_PoolAllocator* dst_datas) {
    void* nl = TreeNode_(GetL)(n);
    void* nr = TreeNode_(GetR)(n);

    if (nl != NULL) { CollectAllResources_(cntr, nl, dst_segs, dst_datas); }
    if (nr != NULL) { CollectAllResources_(cntr, nr, dst_segs, dst_datas); }

    if (cntr->lb == n || cntr->rb == n) { return; }

    Cntr_(Seg)* seg = NToSeg_(n);

    if (GetNColor_(n) == dat_color) {
        if (dst_datas == NULL) {
            ZETA_Core_Allocator_Deallocate(cntr->data_allocator, seg->dat.data);
        } else {
            Zeta_Core_PoolAllocator_Deallocate(dst_datas, seg->dat.data);
        }
    }

    Zeta_Core_PoolAllocator_Deallocate(dst_segs, seg);
}

static inline void* CopySegs_(Cntr* cntr, Cntr* src_cntr, void* src_n,
                              Zeta_Core_PoolAllocator* segs,
                              Zeta_Core_PoolAllocator* datas) {
    void* src_nl = TreeNode_(GetL)(src_n);
    void* src_nr = TreeNode_(GetR)(src_n);

    void* nl =
        src_nl == NULL ? NULL : CopySegs_(cntr, src_cntr, src_nl, segs, datas);

    void* nr =
        src_nr == NULL ? NULL : CopySegs_(cntr, src_cntr, src_nr, segs, datas);

    void* n = NULL;

    if (src_cntr->lb == src_n) {
        n = cntr->lb;
        TreeNode_(Init)(n);
        Zeta_Core_BinTree(TreeNode, SetSize)(n, 1);
    } else if (src_cntr->rb == src_n) {
        n = cntr->rb;
        TreeNode_(Init)(n);
        Zeta_Core_BinTree(TreeNode, SetSize)(n, 1);
    } else {
        Cntr_(Seg)* seg = AllocateSeg_(cntr, segs);
        n = &seg->n;

        TreeNode_(Init)(n);

        Cntr_(Seg)* src_seg = NToSeg_(src_n);

#if STAGING
        int color = GetNColor_(src_n);
        DirectlySetNColor_(n, color);

        if (color == ref_color) {
            seg->ref.beg = src_seg->ref.beg;
            seg->ref.size = src_seg->ref.size;

            Zeta_Core_BinTree(TreeNode, SetSize)(n, seg->ref.size);
        } else
#endif
        {
            seg->dat.data = AllocateData_(cntr, datas);
            seg->dat.offset = 0;
            seg->dat.size = src_seg->dat.size;

            Zeta_Core_CircularArray ca;
            ca.data = src_seg->dat.data;
            ca.width = GetWidth_(cntr);
            ca.stride = cntr->stride;
            ca.offset = src_seg->dat.offset;
            ca.size = src_seg->dat.size;
            ca.capacity = src_cntr->seg_capacity;

            Zeta_Core_CircularArray_Cursor ca_cursor;

            Zeta_Core_CircularArray_PeekL(&ca, &ca_cursor, NULL);

            Zeta_Core_CircularArray_Read(&ca, &ca_cursor, ca.size,
                                         seg->dat.data, cntr->stride, NULL);

            Zeta_Core_BinTree(TreeNode, SetSize)(n, ca.size);
        }
    }

    TreeNode_(SetPColor)(n, TreeNode_(GetPColor)(src_n));

    Zeta_Core_BinTree(TreeNode, AttatchL)(n, nl);
    Zeta_Core_BinTree(TreeNode, AttatchR)(n, nr);

    return n;
}

void Cntr_(Copy)(void* cntr_, void* src_cntr_) {
    Cntr* cntr = cntr_;
    Cntr* src_cntr = src_cntr_;

    CheckCntr_(cntr);
    CheckCntr_(src_cntr);

    if (cntr == src_cntr) { return; }

    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;

    size_t src_stride = src_cntr->stride;
    size_t src_seg_capacity = cntr->seg_capacity;

    bool_t same_data_size =
        stride * seg_capacity == src_stride * src_seg_capacity;

    Zeta_Core_PoolAllocator segs;
    Zeta_Core_PoolAllocator datas;

    Zeta_Core_PoolAllocator_Init(&segs);
    Zeta_Core_PoolAllocator_Init(&datas);

    CollectAllResources_(cntr, cntr->root, &segs,
                         same_data_size ? &datas : NULL);

#if STAGING
    cntr->origin = src_cntr->origin;
#endif

    cntr->seg_capacity = src_seg_capacity;

    cntr->root = CopySegs_(cntr, src_cntr, src_cntr->root, &segs,
                           same_data_size ? &datas : NULL);

    for (;;) {
        void* seg = Zeta_Core_PoolAllocator_Allocate(&segs, 1);
        if (seg == NULL) { break; }
        ZETA_Core_Allocator_Deallocate(cntr->seg_allocator, seg);
    }

    for (;;) {
        void* data = Zeta_Core_PoolAllocator_Allocate(&datas, 1);
        if (data == NULL) { break; }
        ZETA_Core_Allocator_Deallocate(cntr->data_allocator, data);
    }
}

#if STAGING

void Cntr_(Collapse)(void* cntr_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    ZETA_Core_DebugAssert(
        cntr->origin.vtable ==
        &Zeta_Core_DynamicVector_seq_cntr_vtablestaging_vector_seq_cntr_vtable);

    Cntr* origin_cntr = cntr->origin.context;

    cntr->origin = origin_cntr->origin;

    size_t stride = cntr->stride;
    size_t seg_capacity = cntr->seg_capacity;

    TreeNode* n = Zeta_Core_BinTree(TreeNode, StepR)(cntr->lb);

    Cntr_(Cursor) origin_cursor;
    Cntr_(Access)(origin_cntr, 0, &origin_cursor, NULL);

    unsigned long long random_seed = Zeta_Core_GetRandom();

    Zeta_Core_CircularArray origin_ca;
    origin_ca.width = ZETA_Core_SeqCntr_GetWidth(origin_cntr->origin);
    origin_ca.stride = stride;
    origin_ca.capacity = origin_cntr->seg_capacity;

    Zeta_Core_CircularArray_Cursor origin_ca_cursor;

    while (n != cntr->rb) {
        Cntr_(Seg)* seg = NToSeg_(n);

        if (GetNColor_(n) == dat_color) { goto NEXT; }

        Cntr_(Cursor_AdvanceR)(origin_cntr, &origin_cursor,
                               seg->ref.beg - origin_cursor.idx);

        TreeNode* origin_n = origin_cursor.n;
        size_t origin_seg_idx = origin_cursor.seg_idx;

        Cntr_(Seg)* origin_seg = NToSeg_(origin_n);

        int origin_seg_color = GetNColor_(&origin_seg->n);

        size_t origin_seg_size = origin_seg_color == ref_color
                                     ? origin_seg->ref.size
                                     : origin_seg->dat.size;

        size_t origin_res_size = origin_seg_size - origin_seg_idx;

        if (origin_seg_color == ref_color) {
            if (seg->ref.size <= origin_res_size) {
                seg->ref.beg = origin_seg->ref.beg + origin_seg_idx;
                goto NEXT;
            }

            Cntr_(Seg)* new_ref_seg = AllocateRefSeg_(cntr);

            new_ref_seg->ref.beg = origin_seg->ref.beg + origin_seg_idx;
            new_ref_seg->ref.size = origin_res_size;

            Zeta_Core_BinTree(TreeNode, SetSize)(&new_ref_seg->n,
                                                 new_ref_seg->ref.size);

            seg->ref.beg += origin_res_size;
            seg->ref.size -= origin_res_size;

            Zeta_Core_BinTree(TreeNode, SetSize)(seg, seg->ref.size);

            cntr->root =
                Zeta_Core_RBTree(TreeNode, InsertL)(n, &new_ref_seg->n);

            continue;
        }

        size_t cur_cnt =
            GetAvgCnt_(ZETA_Core_GetMinOf(seg->ref.size, origin_res_size));

        if (cur_cnt == seg->ref.size) {
            SetNColor_(n, dat_color);
            seg->dat.data = AllocateData_(cntr, NULL);
            seg->dat.offset = 0;
            seg->dat.size = cur_cnt;
        } else {
            Cntr_(Seg)* new_dat_seg = AllocateDatSeg_(cntr);

            new_dat_seg->dat.size = cur_cnt;

            Zeta_Core_BinTree(TreeNode, SetSize)(&new_dat_seg->n, cur_cnt);

            seg->ref.beg += origin_res_size;
            seg->ref.size -= origin_res_size;

            Zeta_Core_BinTree(TreeNode, SetSize)(n, seg->ref.size);

            cntr->root =
                Zeta_Core_RBTree(TreeNode, InsertL)(n, &new_dat_seg->n);

            n = &new_dat_seg->n;
            seg = new_dat_seg;
        }

        origin_ca.data = origin_seg->dat.data;
        origin_ca.offset = origin_seg->dat.offset;
        origin_ca.size = origin_seg_size;

        Zeta_Core_CircularArray_Access(&origin_ca, origin_seg_idx,
                                       &origin_ca_cursor, NULL);

        Zeta_Core_CircularArray_Read(&origin_ca, &origin_ca_cursor, cur_cnt,
                                     seg->dat.data, stride, NULL);

    NEXT: { n = Zeta_Core_BinTree(TreeNode, StepR)(n); }
    }
}

#endif

#if STAGING

ZETA_Core_DeclareStruct(WBSeg);

struct WBSeg {
    size_t beg;
    size_t size;

    size_t dst_idx;
    size_t acc_ref;

    void* data;
    unsigned short offset;
};

#endif

#if STAGING

ZETA_Core_DeclareStruct(ToWBSegRet);

struct ToWBSegRet {
    size_t ref_segs_cnt;
    size_t dat_segs_cnt;
};

static inline ToWBSegRet ToWBSeg_(Cntr* cntr, TreeNode* n, WBSeg* dst) {
    ToWBSegRet ret = { 0, 0 };

    while (n != NULL) {
        TreeNode* nl = TreeNode_(GetL)(n);
        TreeNode* nr = TreeNode_(GetR)(n);

        ToWBSegRet l_ret = ToWBSeg_(cntr, nl, dst);
        dst += l_ret.ref_segs_cnt + l_ret.dat_segs_cnt;

        ret.ref_segs_cnt += l_ret.ref_segs_cnt;
        ret.dat_segs_cnt += l_ret.dat_segs_cnt;

        if (cntr->lb == n || cntr->rb == n) {
            n = nr;
            continue;
        }

        Cntr_(Seg)* seg = NToSeg_(n);

        if (GetNColor_(n) == ref_color) {
            ++ret.ref_segs_cnt;

            dst->beg = seg->ref.beg;
            dst->size = seg->ref.size;
        } else {
            ++ret.dat_segs_cnt;

            dst->beg = ZETA_Core_size_max;
            dst->size = seg->dat.size;
            dst->data = seg->dat.data;
            dst->offset = seg->dat.offset;
        }

        ZETA_Core_Allocator_Deallocate(cntr->seg_allocator, seg);

        WBSeg* prv = dst - 1;

        dst->dst_idx = prv->dst_idx + prv->size;

        dst->acc_ref = prv->beg == ZETA_Core_size_max ? prv->acc_ref
                                                      : prv->beg + prv->size;

        ++dst;

        n = nr;
    }

    return ret;
}

#endif

#if STAGING

static inline void WriteWBSeg_(Cntr* cntr, Zeta_Core_CircularArray* ca,
                               WBSeg* wb_segs, size_t segs_cnt,
                               size_t dst_offset, size_t ref_offset) {
    if (segs_cnt == 0) { return; }

    Zeta_Core_SeqCntr origin = cntr->origin;

    for (;;) {
        if (segs_cnt == 1) {
            if (wb_segs->beg == ZETA_Core_size_max) {
                ca->data = wb_segs->data;
                ca->offset = wb_segs->offset;
                ca->size = wb_segs->size;

                Zeta_Core_SeqCntr_RangeAssign(
                    origin,
                    (Zeta_Core_SeqCntr){
                        .vtable = &Zeta_Core_CircularArray_seq_cntr_vtable,
                        .context = ca,
                    },
                    dst_offset + wb_segs->dst_idx, 0, wb_segs->size);

                ZETA_Core_Allocator_Deallocate(cntr->data_allocator,

                                               ca->data);

                return;
            }

            size_t cur_dst_idx = dst_offset + wb_segs->dst_idx;
            size_t cur_ref_beg = ref_offset + wb_segs->beg;

            if (cur_dst_idx != cur_ref_beg) {
                Zeta_Core_SeqCntr_RangeAssign(origin, origin, cur_dst_idx,
                                              cur_ref_beg, wb_segs->size);
            }

            return;
        }

        size_t half_segs_cnt = segs_cnt / 2;

        WBSeg* mid_wb_segs = wb_segs + half_segs_cnt;

        size_t dst_pivot = dst_offset + mid_wb_segs->dst_idx;
        size_t src_pivot = ref_offset + mid_wb_segs->acc_ref;

        if (dst_pivot <= src_pivot) {
            WriteWBSeg_(cntr, ca, wb_segs, half_segs_cnt, dst_offset,
                        ref_offset);
            wb_segs = mid_wb_segs;
            segs_cnt -= half_segs_cnt;
        } else {
            WriteWBSeg_(cntr, ca, mid_wb_segs, segs_cnt - half_segs_cnt,
                        dst_offset, ref_offset);
            segs_cnt = half_segs_cnt;
        }
    }
}

#endif

#if STAGING

ZETA_Core_DeclareStruct(OffsetCntNode);

struct OffsetCntNode {
    Zeta_Core_GenericHashTable_Node ghtn;

    size_t offset;
    size_t cnt;
};

static unsigned long long OffsetCntNodeHash(void const* context,
                                            void const* ghtn,
                                            unsigned long long salt) {
    ZETA_Core_Unused(context);

    OffsetCntNode* node = ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, ghtn);

    return Zeta_Core_ULLHash(node->offset, salt);
}

static int OffsetCntNodeCompare(void const* context, void const* a_ghtn,
                                void const* b_ghtn) {
    ZETA_Core_Unused(context);

    OffsetCntNode* a_node =
        ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, a_ghtn);

    OffsetCntNode* b_node =
        ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, b_ghtn);

    return ZETA_Core_ThreeWayCompare(a_node->offset, b_node->offset);
}

static unsigned long long OffsetHash(void const* context, void const* offset,
                                     unsigned long long salt) {
    ZETA_Core_Unused(context);

    return Zeta_Core_ULLHash(*(unsigned long long*)offset, salt);
}

static int OffsetOffsetCntNodeCompare(void const* context, void const* offset,
                                      void const* ghtn) {
    ZETA_Core_Unused(context);

    OffsetCntNode* node = ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, ghtn);

    return ZETA_Core_ThreeWayCompare(*(unsigned long long*)offset,
                                     node->offset);
}

#endif

#if STAGING

static size_t RecordOffset_(Cntr* cntr, TreeNode* n, size_t dst_idx,
                            Zeta_Core_GenericHashTable* ght) {
    while (n != NULL) {
        TreeNode* nl = TreeNode_(GetL)(n);
        TreeNode* nr = TreeNode_(GetR)(n);

        if (nl != NULL) { dst_idx = RecordOffset_(cntr, nl, dst_idx, ght); }

        if (cntr->lb == n || cntr->rb == n) {
            n = nr;
            continue;
        }

        if (GetNColor_(n) == dat_color) {
            Cntr_(Seg)* seg = NToSeg_(n);
            dst_idx += seg->dat.size;
            n = nr;
            continue;
        }

        Cntr_(Seg)* seg = NToSeg_(n);

        size_t offset = seg->ref.beg - dst_idx;

        Zeta_Core_GenericHashTable_Node* ghtn = Zeta_Core_GenericHashTable_Find(
            ght, &offset, OffsetHash, NULL, OffsetOffsetCntNodeCompare, NULL);

        OffsetCntNode* offset_cnt_node;

        if (ghtn == NULL) {
            offset_cnt_node = ZETA_Core_Allocator_SafeAllocate(
                Zeta_Core_Memory_weak_lifo_allocator, alignof(OffsetCntNode),
                sizeof(OffsetCntNode));

            ghtn = &offset_cnt_node->ghtn;

            offset_cnt_node->offset = offset;
            offset_cnt_node->cnt = 0;

            Zeta_Core_GenericHashTable_Node_Init(ghtn);

            Zeta_Core_GenericHashTable_Insert(ght, ghtn);
        } else {
            offset_cnt_node =
                ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, ghtn);
        }

        offset_cnt_node->cnt += seg->ref.size;

        dst_idx += seg->ref.size;

        n = nr;
    }

    return dst_idx;
}

#endif

#if STAGING

static void WriteBack_LR_(Cntr* cntr, int write_back_strategy,
                          unsigned long long cost_coeff_read,
                          unsigned long long cost_coeff_write,
                          unsigned long long cost_coeff_insert,
                          unsigned long long cost_coeff_erase) {
    size_t stride = cntr->stride;

    size_t size = Cntr_(GetSize)(cntr);

    Zeta_Core_SeqCntr origin = cntr->origin;

    size_t origin_size = ZETA_Core_SeqCntr_GetSize(origin);

    size_t del_l_cnt;
    size_t del_r_cnt;

    switch (write_back_strategy) {
        case ZETA_StagingVector_WriteBackStrategy_L:
            del_l_cnt = size - origin_size;
            del_r_cnt = 0;
            break;

        case ZETA_StagingVector_WriteBackStrategy_R:
            del_l_cnt = 0;
            del_r_cnt = size - origin_size;
            break;

        case ZETA_StagingVector_WriteBackStrategy_LR: {
            Zeta_Core_GenericHashTable ght;

            ght.NodeHash = OffsetCntNodeHash;
            ght.node_hash_context = NULL;

            ght.NodeCompare = OffsetCntNodeCompare;
            ght.node_cmp_context = NULL;

            ght.table_node_allocator = Zeta_Core_Memory_weak_lifo_allocator;

            Zeta_Core_GenericHashTable_Init(&ght);

            RecordOffset_(cntr, cntr->root, 0, &ght);

            unsigned long long best_cost = ZETA_Core_ullong_max;

            del_l_cnt = 0;
            del_r_cnt = size - origin_size;

            for (;;) {
                Zeta_Core_GenericHashTable_Node* ghtn =
                    Zeta_Core_GenericHashTable_ExtractAny(&ght);

                if (ghtn == NULL) { break; }

                OffsetCntNode* offset_cnt_node =
                    ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, ghtn);

                size_t cur_offset = offset_cnt_node->offset;

                size_t cur_del_l_cnt = 0;
                size_t cur_del_r_cnt = size - origin_size;

                if (cur_offset <= ZETA_Core_size_max / 2) {
                    cur_del_l_cnt -= cur_offset;
                    cur_del_r_cnt += cur_offset;
                } else {
                    cur_del_l_cnt += cur_offset;
                    cur_del_r_cnt -= cur_offset;
                }

                unsigned long long cur_cost =
                    -((cost_coeff_read + cost_coeff_write) *
                      offset_cnt_node->cnt);

                if (cur_del_l_cnt <= ZETA_Core_size_max / 2) {
                    cur_cost += cost_coeff_insert * cur_del_l_cnt;
                } else {
                    cur_cost += cost_coeff_erase * -cur_del_l_cnt;
                }

                if (cur_del_r_cnt <= ZETA_Core_size_max / 2) {
                    cur_cost += cost_coeff_insert * cur_del_r_cnt;
                } else {
                    cur_cost += cost_coeff_erase * -cur_del_r_cnt;
                }

                if (cur_cost - best_cost < ZETA_Core_ullong_max / 2) {}

                if (cur_cost < best_cost ||
                    (cur_cost == best_cost && cur_del_l_cnt < del_l_cnt)) {
                    best_cost = cur_cost;
                    del_l_cnt = cur_del_l_cnt;
                    del_r_cnt = cur_del_r_cnt;
                }

                ZETA_Core_Allocator_Deallocate(
                    Zeta_Core_Memory_weak_lifo_allocator,

                    offset_cnt_node);
            }

            break;
        }
    }

    ZETA_Core_SanitizeAssert(origin_size + del_l_cnt + del_r_cnt == size);

    size_t push_l_cnt;
    size_t pop_l_cnt;

    size_t push_r_cnt;
    size_t pop_r_cnt;

    if (del_l_cnt <= ZETA_Core_size_max / 2) {
        push_l_cnt = del_l_cnt;
        pop_l_cnt = 0;
    } else {
        push_l_cnt = 0;
        pop_l_cnt = -del_l_cnt;
    }

    if (del_r_cnt <= ZETA_Core_size_max / 2) {
        push_r_cnt = del_r_cnt;
        pop_r_cnt = 0;
    } else {
        push_r_cnt = 0;
        pop_r_cnt = -del_r_cnt;
    }

    if (0 < push_l_cnt) { ZETA_Core_SeqCntr_PushL(origin, push_l_cnt, NULL); }

    if (0 < push_r_cnt) { ZETA_Core_SeqCntr_PushR(origin, push_r_cnt, NULL); }

    size_t segs_cnt = Zeta_Core_BinTree(TreeNode, Count)(cntr->root) - 2;

    WBSeg* wb_segs = (WBSeg*)ZETA_Core_Allocator_SafeAllocate(
                         Zeta_Core_Memory_weak_lifo_allocator, alignof(WBSeg),
                         sizeof(WBSeg) * (segs_cnt + 1)) +
                     1;

    WBSeg* lb_wb_seg = wb_segs - 1;

    lb_wb_seg->beg = 0;
    lb_wb_seg->size = 0;

    lb_wb_seg->dst_idx = pop_l_cnt;
    lb_wb_seg->acc_ref = pop_l_cnt;

    ToWBSeg_(cntr, cntr->root, wb_segs);

    Zeta_Core_CircularArray ca;
    ca.data = &ca;
    ca.width = GetWidth_(cntr);
    ca.stride = stride;
    ca.offset = 0;
    ca.size = 0;
    ca.capacity = cntr->seg_capacity;

    WriteWBSeg_(cntr, &ca, wb_segs, segs_cnt, 0, push_l_cnt);

    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,
                                   lb_wb_seg);

    if (0 < pop_l_cnt) { ZETA_Core_SeqCntr_PopL(origin, pop_l_cnt); }

    if (0 < pop_r_cnt) { ZETA_Core_SeqCntr_PopR(origin, pop_r_cnt); }

    InitTree_(cntr);
    RefOrigin_(cntr);
}

#endif

#if STAGING

static void WriteBack_Random_(Cntr* cntr, unsigned long long cost_coeff_read,
                              unsigned long long cost_coeff_write,
                              unsigned long long cost_coeff_insert,
                              unsigned long long cost_coeff_erase) {
    size_t stride = cntr->stride;
    size_t size = Cntr_(GetSize)(cntr);

    Zeta_Core_SeqCntr origin = cntr->origin;

    size_t origin_size = ZETA_Core_SeqCntr_GetSize(origin);

    size_t segs_cnt = Zeta_Core_BinTree(TreeNode, Count)(cntr->root) - 2;

    WBSeg* wb_segs = (WBSeg*)ZETA_Core_Allocator_SafeAllocate(
                         Zeta_Core_Memory_weak_lifo_allocator, alignof(WBSeg),
                         sizeof(WBSeg) * (segs_cnt + 2)) +
                     1;

    WBSeg* lb_wb_seg = wb_segs - 1;

    lb_wb_seg->beg = 0;
    lb_wb_seg->size = 0;

    lb_wb_seg->dst_idx = 0;
    lb_wb_seg->acc_ref = 0;

    ToWBSegRet to_wb_seg_ret = ToWBSeg_(cntr, cntr->root, wb_segs);

    {
        size_t check_ref_segs_cnt = 0;
        size_t check_dat_segs_cnt = 0;

        for (size_t i = 0; i < segs_cnt; ++i) {
            if (wb_segs[i].beg == ZETA_Core_size_max) {
                ++check_dat_segs_cnt;
            } else {
                ++check_ref_segs_cnt;
            }
        }

        ZETA_Core_SanitizeAssert(to_wb_seg_ret.ref_segs_cnt ==
                                 check_ref_segs_cnt);
        ZETA_Core_SanitizeAssert(to_wb_seg_ret.dat_segs_cnt ==
                                 check_dat_segs_cnt);
    }

    WBSeg* rb_wb_seg = wb_segs + segs_cnt;

    rb_wb_seg->beg = origin_size;
    rb_wb_seg->size = 0;

    rb_wb_seg->dst_idx = size;
    rb_wb_seg->acc_ref = (rb_wb_seg - 1)->acc_ref;

    size_t ref_segs_cnt = to_wb_seg_ret.ref_segs_cnt;

    Zeta_Core_CircularArray ca;
    ca.data = &ca;
    ca.width = GetWidth_(cntr);
    ca.stride = stride;
    ca.offset = 0;
    ca.size = 0;
    ca.capacity = cntr->seg_capacity;

    if (ref_segs_cnt == 0) {
        if (origin_size < size) {
            ZETA_Core_SeqCntr_PopR(origin, size - origin_size);
        } else if (size < origin_size) {
            ZETA_Core_SeqCntr_PushR(origin, origin_size - size, NULL);
        }

        WriteWBSeg_(cntr, &ca, wb_segs, segs_cnt, 0, 0);

        ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,

                                       wb_segs - 1);

        InitTree_(cntr);
        RefOrigin_(cntr);

        return;
    }

    unsigned long long cost_coeff_read_write =
        cost_coeff_read + cost_coeff_write;

    WBSeg** ref_wb_segs =
        (WBSeg**)ZETA_Core_Allocator_SafeAllocate(
            Zeta_Core_Memory_weak_lifo_allocator, alignof(WBSeg*),
            sizeof(WBSeg*) * (ref_segs_cnt + 2)) +
        1;

    ref_wb_segs[-1] = lb_wb_seg;
    ref_wb_segs[ref_segs_cnt] = rb_wb_seg;

    for (size_t i = 0, j = 0; j < ref_segs_cnt; ++i) {
        ZETA_Core_SanitizeAssert(j < ref_segs_cnt);

        WBSeg* wb_seg = wb_segs + i;

        if (wb_seg->beg != ZETA_Core_size_max) { ref_wb_segs[j++] = wb_seg; }
    }

    size_t* acc_arr = ZETA_Core_Allocator_SafeAllocate(
        Zeta_Core_Memory_weak_lifo_allocator, alignof(size_t),
        sizeof(size_t) * (ref_segs_cnt + 2));

    unsigned long long* acc_brr = ZETA_Core_Allocator_SafeAllocate(
        Zeta_Core_Memory_weak_lifo_allocator, alignof(unsigned long long),
        sizeof(unsigned long long) * (ref_segs_cnt + 1));

    acc_arr[0] = 0;
    acc_brr[0] = 0;

    size_t prv_ref_end = 0;
    size_t prv_dst_end = 0;

    for (size_t i = 1; i <= ref_segs_cnt; ++i) {
        WBSeg* cur_ref_seg = ref_wb_segs[i - 1];

        size_t cur_beg = cur_ref_seg->beg;
        size_t cur_size = cur_ref_seg->size;
        size_t cur_dst_idx = cur_ref_seg->dst_idx;

        acc_arr[i] = acc_arr[i - 1] + (cur_dst_idx - prv_dst_end) -
                     (cur_beg - prv_ref_end);
        acc_brr[i] = acc_brr[i - 1] + cost_coeff_read_write * cur_size;

        prv_ref_end = cur_beg + cur_size;
        prv_dst_end = cur_dst_idx + cur_size;
    }

    acc_arr[ref_segs_cnt + 1] = acc_arr[ref_segs_cnt] + (size - prv_dst_end) -
                                (origin_size - prv_ref_end);

#if ZETA_Core_EnableDebug
    unsigned long long* dp_best_cost = ZETA_Core_Allocator_SafeAllocate(
        Zeta_Core_Memory_weak_lifo_allocator, alignof(unsigned long long),
        sizeof(unsigned long long) * (ref_segs_cnt + 2));

    dp_best_cost[0] = 0;
#endif

    unsigned long long* dp_cost = ZETA_Core_Allocator_SafeAllocate(
        Zeta_Core_Memory_weak_lifo_allocator, alignof(unsigned long long),
        sizeof(unsigned long long) * (ref_segs_cnt + 2));

    size_t* dp_prv = ZETA_Core_Allocator_SafeAllocate(
        Zeta_Core_Memory_weak_lifo_allocator, alignof(size_t),
        sizeof(size_t) * (ref_segs_cnt + 2));

    dp_cost[0] = 0;
    dp_prv[0] = ZETA_Core_size_max;

#if ZETA_Core_EnableDebug
    for (size_t i = 1; i <= ref_segs_cnt + 1; ++i) {
        unsigned long long ans_cost = ZETA_Core_ullong_max;

        for (size_t j = i, j_end = 0; j_end < j--;) {
            size_t sum_arr = acc_arr[i] - acc_arr[j];

            unsigned long long cur_cost = dp_best_cost[j] +
                                          (sum_arr <= ZETA_Core_size_max / 2
                                               ? cost_coeff_erase * sum_arr
                                               : cost_coeff_insert * -sum_arr) +
                                          acc_brr[i - 1] - acc_brr[j];

            if (cur_cost < ans_cost) { ans_cost = cur_cost; }
        }

        dp_best_cost[i] = ans_cost;
    }
#endif

    for (size_t i = 1; i <= ref_segs_cnt + 1; ++i) {
        unsigned long long ans_cost = ZETA_Core_ullong_max;
        unsigned long long ans_prv = 0;

        for (size_t j = i, j_end = ZETA_Core_GetMaxOf(8ULL, i) - 8;
             j_end < j--;) {
            size_t sum_arr = acc_arr[i] - acc_arr[j];

            unsigned long long cur_cost = dp_cost[j] +
                                          (sum_arr <= ZETA_Core_size_max / 2
                                               ? cost_coeff_erase * sum_arr
                                               : cost_coeff_insert * -sum_arr) +
                                          acc_brr[i - 1] - acc_brr[j];

            if (cur_cost < ans_cost) {
                ans_cost = cur_cost;
                ans_prv = j;
            }
        }

        dp_cost[i] = ans_cost;
        dp_prv[i] = ans_prv;
    }

#if ZETA_Core_EnableDebug
    {
        unsigned long long best_cost = dp_best_cost[ref_segs_cnt + 1];
        unsigned long long better_cost = dp_cost[ref_segs_cnt + 1];

        ZETA_Core_DebugAssert(best_cost <= 512 || better_cost <= best_cost * 2);
    }
#endif

    void* origin_cursor = ZETA_Core_SeqCntr_AllocaCursor(origin);

    unsigned long long check_cost = 0;

    for (size_t cur_end = ref_segs_cnt + 1; 0 < cur_end;) {
        size_t cur_beg = dp_prv[cur_end];

        WBSeg* beg_wb_seg = (ref_wb_segs - 1)[cur_beg];
        WBSeg* end_wb_seg = (ref_wb_segs - 1)[cur_end];

        size_t src_beg = beg_wb_seg->beg + beg_wb_seg->size;
        size_t src_end = end_wb_seg->beg;

        size_t dst_beg = beg_wb_seg->dst_idx + beg_wb_seg->size;
        size_t dst_end = end_wb_seg->dst_idx;

        size_t dst_size = dst_end - dst_beg;
        size_t src_size = src_end - src_beg;

        {
            WBSeg* segs_ = beg_wb_seg + 1;
            size_t segs_cnt_ = end_wb_seg - beg_wb_seg - 1;

            size_t check_dst_size = 0;

            for (size_t i = 0; i < segs_cnt_; ++i) {
                check_dst_size += segs_[i].size;

                if (segs_[i].beg == ZETA_Core_size_max) { continue; }

                check_cost += cost_coeff_read_write * segs_[i].size;
            }

            ZETA_Core_SanitizeAssert(dst_size == check_dst_size);

            if (src_size < dst_size) {
                check_cost += cost_coeff_insert * (dst_size - src_size);
            }

            if (dst_size < src_size) {
                check_cost += cost_coeff_erase * (src_size - dst_size);
            }
        }

        if (src_size < dst_size) {
            size_t diff_size = dst_size - src_size;

            ZETA_Core_SeqCntr_Access(origin, src_end, origin_cursor, NULL);
            ZETA_Core_SeqCntr_Insert(origin, origin_cursor, diff_size);

            src_end += diff_size;
            src_size += diff_size;
        }

        WriteWBSeg_(cntr, &ca, beg_wb_seg + 1, end_wb_seg - beg_wb_seg - 1,
                    src_beg - dst_beg, 0);

        if (dst_size < src_size) {
            size_t diff_size = src_size - dst_size;

            ZETA_Core_SeqCntr_Access(origin, src_end - diff_size, origin_cursor,
                                     NULL);

            ZETA_Core_SeqCntr_Erase(origin, origin_cursor, diff_size);
        }

        cur_end = cur_beg;
    }

    ZETA_Core_SanitizeAssert(check_cost == dp_cost[ref_segs_cnt + 1]);

    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,

                                   wb_segs - 1);
    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,

                                   ref_wb_segs - 1);
    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,
                                   acc_arr);
    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,
                                   acc_brr);

#if ZETA_Core_EnableDebug
    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,

                                   dp_best_cost);
#endif

    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,
                                   dp_cost);
    ZETA_Core_Allocator_Deallocate(Zeta_Core_Memory_weak_lifo_allocator,
                                   dp_prv);

    InitTree_(cntr);
    RefOrigin_(cntr);
}

#endif

#if STAGING

void Cntr_(WriteBack)(void* cntr_, int write_back_strategy,
                      unsigned long long cost_coeff_read,
                      unsigned long long cost_coeff_write,
                      unsigned long long cost_coeff_insert,
                      unsigned long long cost_coeff_erase) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    Zeta_Core_SeqCntr origin = cntr->origin;

    ZETA_Core_DebugAssert(
        write_back_strategy == ZETA_StagingVector_WriteBackStrategy_L ||    //
        write_back_strategy == ZETA_StagingVector_WriteBackStrategy_R ||    //
        write_back_strategy == ZETA_StagingVector_WriteBackStrategy_LR ||   //
        write_back_strategy == ZETA_StagingVector_WriteBackStrategy_Random  //
    );

    if (Cntr_(GetSize)(cntr) == 0) {
        ZETA_Core_SeqCntr_EraseAll(origin);
        return;
    }

    if (write_back_strategy != ZETA_StagingVector_WriteBackStrategy_Random) {
        WriteBack_LR_(cntr, write_back_strategy, cost_coeff_read,
                      cost_coeff_write, cost_coeff_insert, cost_coeff_erase);
    } else {
        WriteBack_Random_(cntr, cost_coeff_read, cost_coeff_write,
                          cost_coeff_insert, cost_coeff_erase);
    }
}

#endif

void Cntr_(Check)(void const* cntr_) {
    Cntr const* cntr = cntr_;
    ZETA_Core_DebugAssert(cntr != NULL);

#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;

    ZETA_Core_DebugAssert(origin.vtable != NULL);
    ZETA_Core_DebugAssert(origin.vtable->GetWidth != NULL);
    ZETA_Core_DebugAssert(origin.vtable->GetSize != NULL);
    ZETA_Core_DebugAssert(origin.vtable->Access != NULL);
    ZETA_Core_DebugAssert(origin.vtable->Read != NULL);
#endif

    size_t width = GetWidth_(cntr);
    ZETA_Core_DebugAssert(0 < width);

    size_t stride = cntr->stride;
    ZETA_Core_DebugAssert(width <= stride);

    size_t seg_capacity = cntr->seg_capacity;
    ZETA_Core_DebugAssert(0 < seg_capacity);
    ZETA_Core_DebugAssert(seg_capacity <= ZETA_Core_ushrt_max);

    ZETA_Core_Allocator_Check(cntr->seg_allocator, alignof(Cntr_(Seg)));
    ZETA_Core_Allocator_Check(cntr->data_allocator, 1);
}

#if ZETA_Core_EnableDebug

ZETA_Core_DeclareStruct(SanitizeRet);

struct SanitizeRet {
    bool_t b;
    size_t ref_beg;
    size_t ref_end;
};

static SanitizeRet Sanitize_(Cntr const* cntr, Zeta_Core_MemRecorder* dst_seg,
                             Zeta_Core_MemRecorder* dst_data, void* n) {
    size_t stride = cntr->stride;

#if STAGING
    Zeta_Core_SeqCntr origin = cntr->origin;

    size_t origin_size = ZETA_Core_SeqCntr_GetSize(origin);
#endif

    void* l_n = Zeta_Core_BinTree(TreeNode, StepL)(n);
    void* r_n = Zeta_Core_BinTree(TreeNode, StepR)(n);

    if (cntr->lb == n) {
        ZETA_Core_DebugAssert(l_n == NULL);
        ZETA_Core_DebugAssert(r_n != NULL);

        ZETA_Core_DebugAssert(Zeta_Core_BinTree(TreeNode, GetSize)(n) == 1);

        if (dst_seg != NULL) {
            Zeta_Core_MemRecorder_Record(dst_seg, cntr->lb, sizeof(TreeNode));
        }
    } else if (cntr->rb == n) {
        ZETA_Core_DebugAssert(l_n != NULL);
        ZETA_Core_DebugAssert(r_n == NULL);

        ZETA_Core_DebugAssert(Zeta_Core_BinTree(TreeNode, GetSize)(n) == 1);

        if (dst_seg != NULL) {
            Zeta_Core_MemRecorder_Record(dst_seg, cntr->rb, sizeof(TreeNode));
        }
    }
#if STAGING
    else if (GetNColor_(n) == ref_color) {
        ZETA_Core_DebugAssert(l_n != NULL);
        ZETA_Core_DebugAssert(r_n != NULL);

        Cntr_(Seg)* seg = NToSeg_(n);

        if (dst_seg != NULL) {
            Zeta_Core_MemRecorder_Record(dst_seg, seg, sizeof(Cntr_(Seg)));
        }

        ZETA_Core_DebugAssert(0 < seg->ref.size);

        ZETA_Core_DebugAssert(seg->ref.beg < origin_size);
        ZETA_Core_DebugAssert(seg->ref.beg + seg->ref.size <= origin_size);

        size_t size = seg->ref.size;

        ZETA_Core_DebugAssert(size == Zeta_Core_BinTree(TreeNode, GetSize)(n));

        if (cntr->lb != l_n) {
            Cntr_(Seg)* l_seg = NToSeg_(l_n);

            if (GetNColor_(&l_seg->n) == ref_color) {
                ZETA_Core_DebugAssert(l_seg->ref.beg + l_seg->ref.size <
                                      seg->ref.beg);
            }
        }

        if (cntr->rb != r_n) {
            Cntr_(Seg)* r_seg = NToSeg_(r_n);

            if (GetNColor_(&r_seg->n) == ref_color) {
                ZETA_Core_DebugAssert(seg->ref.beg + seg->ref.size <
                                      r_seg->ref.beg);
            }
        }
    }
#endif
    else {
        ZETA_Core_DebugAssert(GetNColor_(n) == dat_color);

        ZETA_Core_DebugAssert(l_n != NULL);
        ZETA_Core_DebugAssert(r_n != NULL);

        Cntr_(Seg)* seg = NToSeg_(n);

        if (dst_seg != NULL) {
            Zeta_Core_MemRecorder_Record(dst_seg, seg, sizeof(Cntr_(Seg)));
        }

        size_t size = seg->dat.size;

        ZETA_Core_DebugAssert(size == Zeta_Core_BinTree(TreeNode, GetSize)(n));

        ZETA_Core_DebugAssert(0 < size);
        ZETA_Core_DebugAssert(size <= cntr->seg_capacity);
        ZETA_Core_DebugAssert(seg->dat.offset < cntr->seg_capacity);

        if (dst_data != NULL) {
            Zeta_Core_MemRecorder_Record(dst_data, seg->dat.data,
                                         stride * cntr->seg_capacity);
        }

        size_t l_vacant;
        size_t r_vacant;

        if (cntr->lb == l_n) {
            l_vacant = 0;
        } else {
            Cntr_(Seg)* l_seg = NToSeg_(l_n);

            l_vacant = cntr->seg_capacity -
                       StagingThreeWay(GetNColor_(l_n) == ref_color,
                                       ZETA_Core_GetMinOf(l_seg->ref.size,
                                                          cntr->seg_capacity),
                                       l_seg->dat.size);
        }

        if (cntr->rb == r_n) {
            r_vacant = 0;
        } else {
            Cntr_(Seg)* r_seg = NToSeg_(r_n);

            r_vacant = cntr->seg_capacity -
                       StagingThreeWay(GetNColor_(r_n) == ref_color,
                                       ZETA_Core_GetMinOf(r_seg->ref.size,
                                                          cntr->seg_capacity),
                                       r_seg->dat.size);
        }

        ZETA_Core_DebugAssert(0 <= l_vacant);
        ZETA_Core_DebugAssert(0 <= r_vacant);
    }

    void* sub_l_n = TreeNode_(GetL)(n);
    void* sub_r_n = TreeNode_(GetR)(n);

    SanitizeRet l_check_ret = (SanitizeRet){ FALSE, 0, 0 };
    SanitizeRet r_check_ret = (SanitizeRet){ FALSE, 0, 0 };

    if (sub_l_n != NULL) {
        l_check_ret = Sanitize_(cntr, dst_seg, dst_data, sub_l_n);
    }

    if (sub_r_n != NULL) {
        r_check_ret = Sanitize_(cntr, dst_seg, dst_data, sub_r_n);
    }

    SanitizeRet ret = (SanitizeRet){ FALSE, 0, 0 };

    if (l_check_ret.b && r_check_ret.b) {
        ZETA_Core_DebugAssert(l_check_ret.ref_end <= r_check_ret.ref_beg);

        ret.b = TRUE;
        ret.ref_beg = l_check_ret.ref_beg;
        ret.ref_end = r_check_ret.ref_end;
    } else if (l_check_ret.b) {
        ret = l_check_ret;
    } else if (r_check_ret.b) {
        ret = r_check_ret;
    }

    if (cntr->lb == n || cntr->rb == n) { return ret; }

#if STAGING
    Cntr_(Seg)* seg = NToSeg_(n);

    if (GetNColor_(n) == dat_color) { return ret; }

    size_t cur_ref_beg = seg->ref.beg;
    size_t cur_ref_end = seg->ref.beg + seg->ref.size;

    if (l_check_ret.b) {
        ZETA_Core_DebugAssert(l_check_ret.ref_end <= cur_ref_beg);
    }
    if (r_check_ret.b) {
        ZETA_Core_DebugAssert(cur_ref_end <= r_check_ret.ref_beg);
    }

    if (ret.b) {
        ret.ref_beg = ZETA_Core_GetMinOf(ret.ref_beg, cur_ref_beg);
        ret.ref_end = ZETA_Core_GetMaxOf(ret.ref_end, cur_ref_end);
    } else {
        ret.b = TRUE;
        ret.ref_beg = cur_ref_beg;
        ret.ref_end = cur_ref_end;
    }
#endif

    return ret;
}

#endif

void Cntr_(Sanitize)(void const* cntr_, Zeta_Core_MemRecorder* dst_seg,
                     Zeta_Core_MemRecorder* dst_data) {
    Cntr const* cntr = cntr_;
    CheckCntr_(cntr);

#if !ZETA_Core_EnableDebug
    ZETA_Core_Unused(dst_seg);
    ZETA_Core_Unused(dst_data);
#else
    Sanitize_(cntr, dst_seg, dst_data, cntr->root);
#endif
}

static void PrintState_(Cntr* cntr, void* n) {
    void* l_n = TreeNode_(GetL)(n);
    void* r_n = TreeNode_(GetR)(n);

    if (l_n != NULL) { PrintState_(cntr, l_n); }

    if (cntr->lb != n && cntr->rb != n) {
        Cntr_(Seg)* seg = NToSeg_(n);

#if STAGING
        if (GetNColor_(&seg->n) == ref_color) {
            printf("ref %8zu [%8zu, %8zu]\n", seg->ref.size, seg->ref.beg,
                   seg->ref.beg + seg->ref.size);
        } else
#endif
        {
            printf("dat %8zu\n", (size_t)seg->dat.size);
        }
    }

    if (r_n != NULL) { PrintState_(cntr, r_n); }
}

void Cntr_(PrintState)(void* cntr_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    PrintState_(cntr, cntr->root);
}

static Cntr_(Stats) GetStats_(Cntr* cntr, void* n) {
    Cntr_(Stats) ret;
    ret.ref_seg_cnt = 0;
    ret.dat_seg_cnt = 0;
    ret.ref_size = 0;
    ret.dat_size = 0;

    if (n == NULL) { return ret; }

    void* l_n = TreeNode_(GetL)(n);
    void* r_n = TreeNode_(GetR)(n);

    Cntr_(Stats) l_stats = GetStats_(cntr, l_n);
    Cntr_(Stats) r_stats = GetStats_(cntr, r_n);

    if (cntr->lb != n && cntr->rb != n) {
        Cntr_(Seg)* seg = NToSeg_(n);

#if STAGING
        if (GetNColor_(n) == ref_color) {
            ++ret.ref_seg_cnt;
            ret.ref_size += seg->ref.size;
        } else
#endif
        {
            ++ret.dat_seg_cnt;
            ret.dat_size += seg->dat.size;
        }
    }

    ret.ref_seg_cnt += l_stats.ref_seg_cnt + r_stats.ref_seg_cnt;
    ret.dat_seg_cnt += l_stats.dat_seg_cnt + r_stats.dat_seg_cnt;
    ret.ref_size += l_stats.ref_size + r_stats.ref_size;
    ret.dat_size += l_stats.dat_size + r_stats.dat_size;

    return ret;
}

Cntr_(Stats) Cntr_(GetStats)(void* cntr_) {
    Cntr* cntr = cntr_;
    CheckCntr_(cntr);

    return GetStats_(cntr, cntr->root);
}

bool_t Cntr_(Cursor_AreEqual)(void const* cntr_, void const* cursor_a_,
                              void const* cursor_b_) {
    Cntr const* cntr = cntr_;

    Cntr_(Cursor) const* cursor_a = cursor_a_;
    Cntr_(Cursor) const* cursor_b = cursor_b_;

    CheckCursor_(cntr, cursor_a);
    CheckCursor_(cntr, cursor_b);

    return cursor_a->idx == cursor_b->idx;
}

int Cntr_(Cursor_Compare)(void const* cntr_, void const* cursor_a_,
                          void const* cursor_b_) {
    Cntr const* cntr = cntr_;

    Cntr_(Cursor) const* cursor_a = cursor_a_;
    Cntr_(Cursor) const* cursor_b = cursor_b_;

    CheckCursor_(cntr, cursor_a);
    CheckCursor_(cntr, cursor_b);

    return ZETA_Core_ThreeWayCompare(cursor_a->idx + 1, cursor_b->idx + 1);
}

size_t Cntr_(Cursor_GetDist)(void const* cntr_, void const* cursor_a_,
                             void const* cursor_b_) {
    Cntr const* cntr = cntr_;

    Cntr_(Cursor) const* cursor_a = cursor_a_;
    Cntr_(Cursor) const* cursor_b = cursor_b_;

    CheckCursor_(cntr, cursor_a);
    CheckCursor_(cntr, cursor_b);

    return cursor_b->idx - cursor_a->idx;
}

size_t Cntr_(Cursor_GetIdx)(void const* cntr_, void const* cursor_) {
    Cntr const* cntr = cntr_;
    Cntr_(Cursor) const* cursor = cursor_;
    CheckCursor_(cntr, cursor);

    return cursor->idx;
}

void Cntr_(Cursor_StepL)(void const* cntr, void* cursor) {
    Cntr_(Cursor_AdvanceL)(cntr, cursor, 1);
}

void Cntr_(Cursor_StepR)(void const* cntr, void* cursor) {
    Cntr_(Cursor_AdvanceR)(cntr, cursor, 1);
}

void Cntr_(Cursor_AdvanceL)(void const* cntr_, void* cursor_, size_t step) {
    Cntr const* cntr = cntr_;
    Cntr_(Cursor)* cursor = cursor_;
    CheckCursor_(cntr, cursor);

    if (step == 0) { return; }

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    void* dst_n;
    size_t dst_seg_idx;

    void* n = cursor->n;

    size_t n_size = n == cntr->rb ? 1 : ({
        Cntr_(Seg)* seg = NToSeg_(n);

        (cntr->lb == n || cntr->rb == n)
            ? 1
            : StagingThreeWay(GetNColor_(n) == ref_color, seg->ref.size,
                              seg->dat.size);
    });

    Zeta_Core_BinTree(TreeNode, AdvanceL)(&dst_n, &dst_seg_idx, n,
                                          n_size - 1 - cursor->seg_idx + step);

    ZETA_Core_DebugAssert(dst_n != NULL);

    cursor->idx -= step;
    cursor->n = dst_n;

    if (cntr->lb == dst_n) {
        cursor->seg_idx = 0;
        cursor->ref = 0;
        return;
    }

    Cntr_(Seg)* dst_seg = NToSeg_(dst_n);

    cursor->seg_idx = StagingThreeWay(GetNColor_(dst_n) == ref_color,
                                      dst_seg->ref.size, dst_seg->dat.size) -
                      1 - dst_seg_idx;

#if STAGING
    if (GetNColor_(dst_n) == ref_color) {
        cursor->ref = NULL;
        return;
    }
#endif

    Zeta_Core_CircularArray ca;
    ca.data = dst_seg->dat.data;
    ca.width = GetWidth_(cntr);
    ca.stride = cntr->stride;
    ca.offset = dst_seg->dat.offset;
    ca.size = dst_seg->dat.size;
    ca.capacity = cntr->seg_capacity;

    cursor->ref =
        Zeta_Core_CircularArray_Access(&ca, cursor->seg_idx, NULL, NULL);
}

void Cntr_(Cursor_AdvanceR)(void const* cntr_, void* cursor_, size_t step) {
    Cntr const* cntr = cntr_;
    Cntr_(Cursor)* cursor = cursor_;
    CheckCursor_(cntr, cursor);

    if (step == 0) { return; }

    size_t size = Cntr_(GetSize)(cntr);

    ZETA_Core_DebugAssert(step <= size - cursor->idx);

    void* dst_n;
    size_t dst_seg_idx;

    Zeta_Core_BinTree(TreeNode, AdvanceR)(&dst_n, &dst_seg_idx, cursor->n,
                                          cursor->seg_idx + step);

    ZETA_Core_DebugAssert(dst_n != NULL);

    cursor->idx += step;
    cursor->n = dst_n;
    cursor->seg_idx = dst_seg_idx;

    if (cntr->rb == dst_n
#if STAGING
        || GetNColor_(dst_n) == ref_color
#endif
    ) {
        cursor->ref = NULL;
        return;
    }

    Cntr_(Seg)* dst_seg = NToSeg_(dst_n);

    Zeta_Core_CircularArray ca;
    ca.data = dst_seg->dat.data;
    ca.width = GetWidth_(cntr);
    ca.stride = cntr->stride;
    ca.offset = dst_seg->dat.offset;
    ca.size = dst_seg->dat.size;
    ca.capacity = cntr->seg_capacity;

    cursor->ref =
        Zeta_Core_CircularArray_Access(&ca, cursor->seg_idx, NULL, NULL);
}

void Cntr_(Cursor_Check)(void const* cntr_, void const* cursor_) {
    Cntr const* cntr = cntr_;
    CheckCntr_(cntr);

    Cntr_(Cursor) const* cursor = cursor_;
    ZETA_Core_DebugAssert(cursor != NULL);

    Cntr_(Cursor) re_cursor;
    Cntr_(Access)((void*)cntr, cursor->idx, &re_cursor, NULL);

    ZETA_Core_DebugAssert(re_cursor.cntr == cursor->cntr);
    ZETA_Core_DebugAssert(re_cursor.idx == cursor->idx);
    ZETA_Core_DebugAssert(re_cursor.n == cursor->n);
    ZETA_Core_DebugAssert(re_cursor.seg_idx == cursor->seg_idx);
    ZETA_Core_DebugAssert(re_cursor.ref == cursor->ref);
}

#if STAGING

Zeta_Core_SeqCntr_VTable const
    Zeta_Core_DynamicVector_seq_cntr_vtablestaging_vector_seq_cntr_vtable = {

#else

Zeta_Core_SeqCntr_VTable const
    Zeta_Core_DynamicVector_seq_cntr_vtableseg_vector_seq_cntr_vtable = {

#endif

        .cursor_size = sizeof(Cntr_(Cursor)),

        .GetWidth = Cntr_(GetWidth),

        .GetSize = Cntr_(GetSize),

        .GetCapacity = Cntr_(GetCapacity),

        .GetLBCursor = Cntr_(GetLBCursor),

        .GetRBCursor = Cntr_(GetRBCursor),

        .PeekL = Cntr_(PeekL),

        .PeekR = Cntr_(PeekR),

        .Access = Cntr_(Access),

        .Refer = Cntr_(Refer),

        .Read = Cntr_(Read),

        .Write = Cntr_(Write),

        .PushL = Cntr_(PushL),

        .PushR = Cntr_(PushR),

        .Insert = Cntr_(Insert),

        .PopL = Cntr_(PopL),

        .PopR = Cntr_(PopR),

        .Erase = Cntr_(Erase),

        .EraseAll = Cntr_(EraseAll),

        .Cursor_AreEqual = Cntr_(Cursor_AreEqual),

        .Cursor_Compare = Cntr_(Cursor_Compare),

        .Cursor_GetDist = Cntr_(Cursor_GetDist),

        .Cursor_GetIdx = Cntr_(Cursor_GetIdx),

        .Cursor_StepL = Cntr_(Cursor_StepL),

        .Cursor_StepR = Cntr_(Cursor_StepR),

        .Cursor_AdvanceL = Cntr_(Cursor_AdvanceL),

        .Cursor_AdvanceR = Cntr_(Cursor_AdvanceR),
    };
