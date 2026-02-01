#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

#define EnStaging 1

#include <iostream>
#include <zeta/core/allocator.hpp>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/bin_tree_node_tpl.ipp>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/compare.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/pool_allocator.hpp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/seg_utils.ipp>
#include <zeta/core/seg_vector.mpp.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

#pragma push_macro("NameSpace")
#pragma push_macro("CntrTplParamList")
#pragma push_macro("CntrTplArgList")
#pragma push_macro("EnStagingTernary")

#if EnStaging

#define NameSpace staging_seg_vector

#define CntrTplParamList \
    typename Origin, typename SegAllocatorLike, typename DataAllocatorLike

#define CntrTplArgList Origin, SegAllocatorLike, DataAllocatorLike

#define EnStagingTernary(cond, x, y) ((cond) ? (x) : (y))

#else

#define NameSpace seg_vector

#define CntrTplParamList typename SegAllocatorLike, typename DataAllocatorLike

#define CntrTplArgList SegAllocatorLike, DataAllocatorLike

#define EnStagingTernary(cond, x, y) (y)

#endif

// -----------------------------------------------------------------------------

namespace zeta::core::NameSpace {

constexpr bool TreeNodeView::IsConst(type_wrapper::TypeWrapper<TreeNode*>) {
    return false;
}

constexpr bool TreeNodeView::IsConst(
    type_wrapper::TypeWrapper<TreeNode const*>) {
    return true;
}

constexpr bool TreeNodeView::IsAccSizeEnabled(
    type_wrapper::TypeWrapper<TreeNode const*>) {
    return true;
}

constexpr size_t TreeNodeView::GetNullAccSize(
    type_wrapper::TypeWrapper<TreeNode const*>) {
    return 0;
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

unsigned TreeNodeView::GetColor(TreeNodeView const* n) {
    return reinterpret_cast<TreeNode const*>(n)->GetPColor();
}

void TreeNodeView::SetColor(TreeNodeView* n, unsigned color) {
    reinterpret_cast<TreeNode*>(n)->SetPColor(color);
}

size_t TreeNodeView::GetAccSize(TreeNodeView const* n) {
    return reinterpret_cast<TreeNode const*>(n)->GetAccSize();
}

void TreeNodeView::SetAccSize(TreeNodeView* n, size_t size) {
    reinterpret_cast<TreeNode*>(n)->SetAccSize(size);
}

// -----------------------------------------------------------------------------

namespace ops {

namespace detail {

template <CntrTplParamList>
void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

    size_t width{ cntr->width };

    ZETA_Core_DebugAssert(0 < width);

    size_t stride{ cntr->stride };
    ZETA_Core_DebugAssert(width <= stride);

    size_t seg_capacity{ cntr->seg_capacity };
    ZETA_Core_DebugAssert(0 < seg_capacity);
    ZETA_Core_DebugAssert(seg_capacity <= ZETA_Core_ushrt_max);
}

template <CntrTplParamList>
void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* cntr, Cursor const* cursor) {
    CheckCntr_(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    Cursor re_cursor;
    Access(cntr, cursor->idx, &re_cursor, nullptr);

    ZETA_Core_DebugAssert(*cursor == re_cursor);
}

size_t GetAvgCnt_  // NOLINT(misc-use-internal-linkage)
    (size_t cnt, size_t seg_capacity, unsigned long long& random_seed) {
    size_t seg_cnt{ UIntCeilDiv(cnt, seg_capacity) };

    return (cnt + SimpleRandomRotate(&random_seed) % seg_cnt) / seg_cnt;
}

#if EnStaging

inline unsigned GetNColor_  // NOLINT(misc-use-internal-linkage)
    (TreeNode const* n) {
    unsigned color{ n->GetLColor() };
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);
    return color;
}

inline void DirectlySetNColor_  // NOLINT(misc-use-internal-linkage)
    (TreeNode* n, unsigned color) {
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);

    n->SetLColor(color);
}

inline void SetNColor_  // NOLINT(misc-use-internal-linkage)
    (TreeNode* n, unsigned color) {
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);
    ZETA_Core_DebugAssert(0 <= GetNColor_(n));

    DirectlySetNColor_(n, color);
}

#endif

inline Seg* NToSeg_  // NOLINT(misc-use-internal-linkage)
    (TreeNode* n) {
    return ZETA_Core_MemberToStruct(Seg, n, n);
}

// -----------------------------------------------------------------------------

#if EnStaging

inline void GetRefSegState_  // NOLINT(misc-use-internal-linkage)
    (size_t seg_capacity, Seg* seg, CircularArray* dst_ca, size_t* dst_vacant) {
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == ref_color);

    size_t size{ seg->ref.size };

    dst_ca->size = size;

    *dst_vacant = seg_capacity - Min(size, seg_capacity);
}

#endif

inline void GetDatSegState_  // NOLINT(misc-use-internal-linkage)
    (size_t seg_capacity, Seg* seg, CircularArray* dst_ca, size_t* dst_vacant) {
#if EnStaging
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == dat_color);
#endif

    size_t size{ seg->dat.size };

    dst_ca->data = seg->dat.data;
    dst_ca->offset = seg->dat.offset;
    dst_ca->size = size;

    *dst_vacant = seg_capacity - size;
}

inline void GetSegState_  // NOLINT(misc-use-internal-linkage)
    (size_t seg_capacity, Seg* seg,
#if EnStaging
     unsigned* dst_color,
#endif
     CircularArray* dst_ca, size_t* dst_vacant) {
#if EnStaging
    unsigned color{ GetNColor_(&seg->n) };
    *dst_color = color;

    if (color == ref_color) {
        GetRefSegState_(seg_capacity, seg, dst_ca, dst_vacant);
    } else
#endif
    {
        GetDatSegState_(seg_capacity, seg, dst_ca, dst_vacant);
    }
}

template <CntrTplParamList>
Seg* AllocateSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, PoolAllocator* seg_pool_allocator) {
    Seg* seg{ seg_pool_allocator == nullptr
                  ? nullptr
                  : static_cast<Seg*>(
                        PoolAllocator::Allocate(seg_pool_allocator, 1)) };

    if (seg == nullptr) {
        seg = static_cast<Seg*>(allocator::SafeAllocate(
            &cntr->seg_allocator, alignof(Seg), sizeof(Seg)));
    }

    seg->n.Init();

    return seg;
}

template <CntrTplParamList>
void* AllocateData_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, PoolAllocator* data_pool_allocator) {
    void* data{ data_pool_allocator == nullptr
                    ? nullptr
                    : PoolAllocator::Allocate(data_pool_allocator, 1) };

    if (data == nullptr) {
        data = allocator::SafeAllocate(&cntr->data_allocator, 1,
                                       cntr->stride * cntr->seg_capacity);
    }

    return data;
}

#if EnStaging

template <CntrTplParamList>
Seg* AllocateRefSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr) {
    Seg* seg{ AllocateSeg_(cntr, nullptr) };

    DirectlySetNColor_(&seg->n, ref_color);

    seg->ref.beg = 0;
    seg->ref.size = 0;

    return seg;
}

#endif

template <CntrTplParamList>
Seg* AllocateDatSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr) {
    Seg* seg{ AllocateSeg_(cntr, nullptr) };

#if EnStaging
    DirectlySetNColor_(&seg->n, dat_color);
#endif

    seg->dat.data = AllocateData_(cntr, nullptr);
    seg->dat.offset = 0;
    seg->dat.size = 0;

    return seg;
}

#if EnStaging

template <CntrTplParamList>
void TransferRefSegToDatSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* seg) {
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == ref_color);

    Origin* origin{ cntr->origin };

    void* data{ AllocateData_(cntr, nullptr) };

    size_t size{ seg->ref.size };

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    Origin::Access(origin, seg->ref.beg, origin_cursor, nullptr);
    Origin::Read(origin, origin_cursor, size,
                 seq_cntr::MemReader{
                     .dst = data,
                     .dst_width = cntr->stride,
                     .dst_stride = cntr->stride,
                 },
                 nullptr);

    SetNColor_(&seg->n, dat_color);

    seg->dat.data = data;
    seg->dat.offset = 0;
    seg->dat.size = static_cast<unsigned short>(size);
}

template <CntrTplParamList>
void PushRefL_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, CircularArray* ca, size_t beg, size_t size) {
    Origin* origin{ cntr->origin };

    circular_array::ops::PushL(ca, size, [](void*, size_t, size_t) {}, nullptr);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    Origin::Access(origin, beg, origin_cursor, nullptr);

    circular_array::ops::AssignFromSeqCntr(ca, 0, origin, origin_cursor, size);
}

template <CntrTplParamList>
void PushRefR_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, CircularArray* ca, size_t beg, size_t size) {
    Origin* origin{ cntr->origin };

    circular_array::ops::PushR(ca, size, [](void*, size_t, size_t) {}, nullptr);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    Origin::Access(origin, beg, origin_cursor, nullptr);

    circular_array::ops::AssignFromSeqCntr(ca, ca->size - size, origin,
                                           origin_cursor, size);
}

template <CntrTplParamList, typename Writer>
void RefShoveL_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, CircularArray* l_ca, Seg* r_seg,
     size_t rl_size, size_t ins_cnt, size_t shove_cnt, Writer& writer) {
    Origin* origin{ cntr->origin };

    size_t stride{ l_ca->stride };

    size_t cnt_a{ Min(rl_size, shove_cnt) };
    size_t cnt_b{ Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_i{ l_ca->size };

    void* r_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::ops::PushR(
        l_ca, shove_cnt, [](void*, size_t, size_t) {}, nullptr);

    Origin::Access(origin, r_seg->ref.beg, r_cursor, nullptr);

    size_t l_ca_offset{ l_ca->offset };
    size_t l_ca_size{ l_ca->size };
    size_t l_ca_capacity{ l_ca->capacity };

    for (size_t i{ cnt_a }; 0 < i;) {
        size_t cur_i{ Min(i, circular_array::ops::GetLongestContSucr(
                                 l_ca_offset, l_i, l_ca_size, l_ca_capacity)) };

        i -= cur_i;

        Origin::MemRead(
            origin, r_cursor, cur_i,
            circular_array::ops::Access(l_ca, l_i, nullptr, nullptr), stride,
            r_cursor);

        l_i += cur_i;
    }

    circular_array::ops::IdxWrite(l_ca, l_i, cnt_b, writer);

    l_i += cnt_b;

    for (size_t i{ cnt_c }; 0 < i;) {
        size_t cur_i{ Min(i, circular_array::ops::GetLongestContSucr(
                                 l_ca_offset, l_i, l_ca_size, l_ca_capacity)) };

        i -= cur_i;

        Origin::MemRead(
            origin, r_cursor, cur_i,
            circular_array::ops::Access(l_ca, l_i, nullptr, nullptr), stride,
            r_cursor);

        l_i += cur_i;
    }

    size_t new_r_size{ r_seg->ref.size + ins_cnt - shove_cnt };

    if (ins_cnt <= cnt_b) {
        r_seg->ref.beg += cnt_a + cnt_c;
        r_seg->ref.size = new_r_size;
        return;
    }

    void* data{ AllocateData_(cntr, nullptr) };
    auto* data_i{ static_cast<char*>(data) };

    if (cnt_a < rl_size) {
        Origin::MemRead(origin, r_cursor, rl_size - cnt_a, data_i, stride,
                        r_cursor);

        data_i += stride * (rl_size - cnt_a);
    }

    writer(data_i, stride, ins_cnt - cnt_b);

    data_i += stride * (ins_cnt - cnt_b);

    if (rl_size < r_seg->ref.size) {
        Origin::MemRead(origin, r_cursor, r_seg->ref.size - rl_size, data_i,
                        stride, r_cursor);
    }

    SetNColor_(&r_seg->n, dat_color);

    r_seg->dat.data = data;
    r_seg->dat.offset = 0;
    r_seg->dat.size = static_cast<unsigned short>(new_r_size);
}

template <CntrTplParamList, typename Writer>
void RefShoveR_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* l_seg, CircularArray* r_ca,
     size_t lr_size, size_t ins_cnt, size_t shove_cnt, Writer& writer) {
    Origin* origin{ cntr->origin };

    size_t stride{ r_ca->stride };

    size_t cnt_a{ Min(lr_size, shove_cnt) };
    size_t cnt_b{ Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t r_i{ 0 };

    void* l_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::ops::PushL(
        r_ca, shove_cnt, [](void*, size_t, size_t) {}, nullptr);

    Origin::Access(origin, l_seg->ref.beg + l_seg->ref.size - cnt_a - cnt_c,
                   l_cursor, nullptr);

    size_t r_ca_offset{ r_ca->offset };
    size_t r_ca_size{ r_ca->size };
    size_t r_ca_capacity{ r_ca->capacity };

    for (size_t i{ cnt_c }; 0 < i;) {
        size_t cur_i{ Min(i, circular_array::ops::GetLongestContSucr(
                                 r_ca_offset, r_i, r_ca_size, r_ca_capacity)) };

        i -= cur_i;

        Origin::MemRead(
            origin, l_cursor, cur_i,
            circular_array::ops::Access(r_ca, r_i, nullptr, nullptr), stride,
            l_cursor);

        r_i += cur_i;
    }

    circular_array::ops::IdxWrite(r_ca, r_i, cnt_b, writer);

    r_i += cnt_b;

    for (size_t i{ cnt_a }; 0 < i;) {
        size_t cur_i{ Min(i, circular_array::ops::GetLongestContSucr(
                                 r_ca_offset, r_i, r_ca_size, r_ca_capacity)) };

        i -= cur_i;

        Origin::MemRead(
            origin, l_cursor, cur_i,
            circular_array::ops::Access(r_ca, r_i, nullptr, nullptr), stride,
            l_cursor);

        r_i += cur_i;
    }

    size_t new_l_size{ l_seg->ref.size + ins_cnt - shove_cnt };

    if (ins_cnt <= cnt_b) {
        l_seg->ref.size = new_l_size;
        return;
    }

    void* data{ AllocateData_(cntr, nullptr) };
    auto* data_i{ static_cast<char*>(data) };

    Origin::Access(origin, l_seg->ref.beg, l_cursor, nullptr);

    if (lr_size < l_seg->ref.size) {
        Origin::MemRead(origin, l_cursor, l_seg->ref.size - lr_size, data_i,
                        stride, l_cursor);

        data_i += stride * (l_seg->ref.size - lr_size);
    }

    writer(data_i, stride, ins_cnt - cnt_b);

    data_i += stride * (ins_cnt - cnt_b);

    if (cnt_a < lr_size) {
        Origin::MemRead(origin, l_cursor, lr_size - cnt_a, data_i, stride,
                        l_cursor);
    }

    SetNColor_(&l_seg->n, dat_color);

    l_seg->dat.data = data;
    l_seg->dat.offset = 0;
    l_seg->dat.size = static_cast<unsigned short>(new_l_size);
}

#endif

template <CntrTplParamList>
int Merge2_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* a_seg, Seg* b_seg, bool a_read_data,
     bool b_read_data) {
#if EnStaging
    Origin* origin{ cntr->origin };
#endif

    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };

#if EnStaging
    unsigned a_color{ GetNColor_(&a_seg->n) };
    unsigned b_color{ GetNColor_(&b_seg->n) };
#endif

    size_t a_size{ EnStagingTernary(a_color == ref_color, a_seg->ref.size,
                                    a_seg->dat.size) };
    size_t b_size{ EnStagingTernary(b_color == ref_color, b_seg->ref.size,
                                    b_seg->dat.size) };

    unsigned long long random_seed{ GetRandom() };

#if EnStaging
    if (a_color == ref_color && b_color == ref_color) {
        size_t total_size{ a_size + b_size };

        void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

        char* data{ static_cast<char*>(AllocateData_(cntr, nullptr)) };

        if (a_read_data) {
            Origin::Access(origin, a_seg->ref.beg, origin_cursor, nullptr);
            Origin::MemRead(origin, origin_cursor, a_size, data, stride,
                            nullptr);
        }

        if (b_read_data) {
            Origin::Access(origin, b_seg->ref.beg, origin_cursor, nullptr);
            Origin::MemRead(origin, origin_cursor, b_seg->ref.size,
                            data + stride * a_size, stride, nullptr);
        }

        a_seg->ref.size = 0;
        b_seg->ref.size = 0;

        Seg* dst_seg;

        int side{ static_cast<int>(SimpleRandomRotate(&random_seed) % 2) };

        switch (side) {
        case 0: dst_seg = a_seg; break;
        case 1: dst_seg = b_seg; break;
        }

        SetNColor_(&dst_seg->n, dat_color);
        dst_seg->dat.data = data;
        dst_seg->dat.size = static_cast<unsigned short>(total_size);

        return side;
    }
#endif

    CircularArray a_ca{
        .data = a_seg->dat.data,
        .width = width,
        .stride = stride,
        .offset = a_seg->dat.offset,
        .size = a_size,
        .capacity = seg_capacity,
    };

    CircularArray b_ca{
        .data = b_seg->dat.data,
        .width = width,
        .stride = stride,
        .offset = b_seg->dat.offset,
        .size = b_size,
        .capacity = seg_capacity,
    };

    size_t merge_a_cost{ EnStagingTernary(a_color == ref_color, a_size + b_size,
                                          b_size) };
    size_t merge_b_cost{ EnStagingTernary(b_color == ref_color, a_size + b_size,
                                          a_size) };

    int side{ Choose2(merge_a_cost <= merge_b_cost,
                      merge_b_cost <= merge_a_cost, &random_seed) };

    switch (side) {
    case 0:
        if (!b_read_data) {
            circular_array::ops::PushR(
                &a_ca, b_size, [](void*, size_t, size_t) {}, nullptr);
        }
#if EnStaging
        else if (b_color == ref_color) {
            PushRefR_(cntr, &a_ca, b_seg->ref.beg, b_size);
        }
#endif
        else {
            seg_utils::SegShoveL(&a_ca, &b_ca, 0, 0, b_size,
                                 [](void*, size_t, size_t) {});
        }

        break;

    case 1:
        if (!a_read_data) {
            circular_array::ops::PushL(
                &b_ca, a_size, [](void*, size_t, size_t) {}, nullptr);
        }
#if EnStaging
        else if (a_color == ref_color) {
            PushRefL_(cntr, &b_ca, a_seg->ref.beg, a_size);
        }
#endif
        else {
            seg_utils::SegShoveR(&a_ca, &b_ca, 0, 0, a_size,
                                 [](void*, size_t, size_t) {});
        }

        break;
    }

#if EnStaging
    if (a_color == ref_color) {
        a_seg->ref.size = 0;
    } else
#endif
    {
        a_seg->dat.offset = static_cast<unsigned short>(a_ca.offset);
        a_seg->dat.size = static_cast<unsigned short>(a_ca.size);
    }

#if EnStaging
    if (b_color == ref_color) {
        b_seg->ref.size = 0;
    } else
#endif
    {
        b_seg->dat.offset = static_cast<unsigned short>(b_ca.offset);
        b_seg->dat.size = static_cast<unsigned short>(b_ca.size);
    }

    return side;
}

// -----------------------------------------------------------------------------

template <CntrTplParamList, bool EnRead, typename ReaderWriterCore>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
     ReaderWriterCore& reader_writer_core, Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor));

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->n = pos_cursor->n;
            dst_cursor->seg_idx = pos_cursor->seg_idx;
            dst_cursor->ref = pos_cursor->ref;
        }

        return;
    }

    ZETA_Core_DebugAssert(
        seq_cntr::IsDereferable(pos_cursor->idx, cnt, GetSize(cntr)));

#if EnStaging
    Origin* origin{ cntr->origin };
#endif

    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };
    size_t size{ GetSize(cntr) };

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(idx <= size);
    ZETA_Core_DebugAssert(cnt <= size - idx);

    size_t end{ idx + cnt };

    size_t seg_idx{ pos_cursor->seg_idx };

    TreeNode* n{ pos_cursor->n };
    Seg* seg;

#if EnStaging
    TreeNode* l_n{ bin_tree::StepL(n) };

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    unsigned long long random_seed{ GetRandom() };

    if (GetNColor_(n) == ref_color && 0 < seg_idx) {
        seg = NToSeg_(n);

        size_t seg_size{ seg->ref.size };

        size_t seg_cnt{ UIntCeilDiv(seg_size, seg_capacity) };

        size_t k{ (seg_idx * seg_cnt + seg_cnt - 1) / seg_size };

        size_t l_seg_size{ k * seg_size / seg_cnt };

        size_t avg_seg_size{ (k + 1) * seg_size / seg_cnt - l_seg_size };

        pos_cursor->seg_idx = seg_idx -= l_seg_size;

        if (0 < l_seg_size) {
            Seg* new_seg{ AllocateRefSeg_(cntr) };

            cntr->root = rbtree::Insert(l_n, n, &new_seg->n);

            new_seg->ref.beg = seg->ref.beg;
            new_seg->ref.size = l_seg_size;

            seg->ref.beg += l_seg_size;
            seg->ref.size -= l_seg_size;

            bin_tree::SetSize(&new_seg->n, new_seg->ref.size);

            l_n = &new_seg->n;
        }

        if (seg_idx == 0) { goto WRITE_INTO_REF_SEG_END; }

        size_t cnt_a{ seg_idx };
        size_t cnt_b{ Min(cnt, avg_seg_size - seg_idx) };
        size_t cnt_c{ avg_seg_size - cnt_a - cnt_b };

        cnt -= cnt_b;

        if (avg_seg_size == seg->ref.size) {
            char* data{ static_cast<char*>(AllocateData_(cntr, nullptr)) };

            Origin::Access(origin, seg->ref.beg, origin_cursor, nullptr);

            if constexpr (EnRead) {
                Origin::MemRead(origin, origin_cursor, cnt_a + cnt_b + cnt_c,
                                data, stride, nullptr);
            } else {
                Origin::MemRead(origin, origin_cursor, cnt_a, data, stride,
                                nullptr);

                if (0 < cnt_c) {
                    Origin::Access(origin, seg->ref.beg + cnt_a + cnt_b,
                                   origin_cursor, nullptr);

                    Origin::MemRead(origin, origin_cursor, cnt_c,
                                    data + stride * (cnt_a + cnt_b), stride,
                                    nullptr);
                }
            }

            reader_writer_core.ReadWriteDat(data + stride * cnt_a, width,
                                            stride, cnt_b);

            SetNColor_(n, dat_color);

            seg->dat.data = data;
            seg->dat.offset = 0;
            seg->dat.size = static_cast<unsigned short>(avg_seg_size);

            pos_cursor->ref = data + stride * seg_idx;

            l_n = n;
            n = bin_tree::StepR(n);

            seg_idx = 0;
        } else {
            Seg* new_seg{ AllocateDatSeg_(cntr) };

            cntr->root = rbtree::Insert(l_n, n, &new_seg->n);

            new_seg->dat.size = static_cast<unsigned short>(avg_seg_size);

            char* data{ static_cast<char*>(new_seg->dat.data) };

            Origin::Access(origin, seg->ref.beg, origin_cursor, nullptr);

            if constexpr (EnRead) {
                Origin::MemRead(origin, origin_cursor, cnt_a + cnt_b + cnt_c,
                                data, stride, nullptr);
            } else {
                Origin::MemRead(origin, origin_cursor, cnt_a, data, stride,
                                nullptr);

                if (0 < cnt_c) {
                    Origin::Access(origin, seg->ref.beg + cnt_a + cnt_b,
                                   origin_cursor, nullptr);

                    Origin::MemRead(origin, origin_cursor, cnt_c,
                                    data + stride * (cnt_a + cnt_b), stride,
                                    nullptr);
                }
            }

            reader_writer_core.ReadWriteDat(data, 0, cnt_a, cnt_b);

            seg->ref.beg += avg_seg_size;
            seg->ref.size -= avg_seg_size;

            pos_cursor->n = &new_seg->n;
            pos_cursor->ref = data + stride * seg_idx;

            seg_idx += cnt_b;

            if (seg_idx < new_seg->dat.size) {
                bin_tree::SetSize(n, seg->ref.size);

                n = &new_seg->n;
            } else {
                bin_tree::SetSize(&new_seg->n, new_seg->dat.size);

                l_n = &new_seg->n;
                seg_idx = 0;
            }
        }
    }

WRITE_INTO_REF_SEG_END:;
#endif

    while (0 < cnt) {
        seg = NToSeg_(n);

#if EnStaging
        if (GetNColor_(n) == ref_color) {
            TreeNode* r_n{ nullptr };

            size_t seg_size{ seg->ref.size };

            if (seg_capacity < seg_size) {
                size_t split_cnt{ GetAvgCnt_(seg_size, seg_capacity,
                                             random_seed) };

                Seg* new_seg{ AllocateRefSeg_(cntr) };

                cntr->root = rbtree::Insert(l_n, n, &new_seg->n);

                new_seg->ref.beg = seg->ref.beg;
                new_seg->ref.size = split_cnt;

                seg->ref.beg += split_cnt;
                seg->ref.size -= split_cnt;

                r_n = n;

                n = &new_seg->n;
                seg = new_seg;

                seg_size = seg->ref.size;
            }

            bin_tree::SetSize(&seg->n, seg_size);

            size_t cur_cnt{ Min(cnt, seg_size) };

            char* data{ static_cast<char*>(AllocateData_(cntr, nullptr)) };

            if constexpr (EnRead) {
                Origin::Access(origin, seg->ref.beg, origin_cursor, nullptr);

                Origin::MemRead(origin, origin_cursor, seg_size, data, stride,
                                nullptr);
            } else if (cur_cnt < seg_size) {
                Origin::Access(origin, seg->ref.beg + cur_cnt, origin_cursor,
                               nullptr);

                Origin::MemRead(origin, origin_cursor, seg_size - cur_cnt,
                                data + stride * cur_cnt, stride, nullptr);
            }

            reader_writer_core.ReadWriteDat(data, 0, 0, cur_cnt);

            SetNColor_(n, dat_color);

            seg->dat.data = data;
            seg->dat.offset = 0;
            seg->dat.size = static_cast<unsigned short>(seg_size);

            if (cur_cnt < seg_size) {
                if (r_n != nullptr) {
                    Seg* r_seg{ NToSeg_(r_n) };

                    bin_tree::SetSize(r_n, GetNColor_(r_n) == ref_color
                                               ? r_seg->ref.size
                                               : r_seg->dat.size);
                }

                seg_idx = cur_cnt;

                break;
            }

            l_n = n;
            n = r_n == nullptr ? bin_tree::StepR(n) : r_n;

            cnt -= cur_cnt;
        } else
#endif
        {
            size_t cur_cnt{ Min(cnt, seg->dat.size - seg_idx) };

            reader_writer_core.ReadWriteDat(seg->dat.data, seg->dat.offset,
                                            seg_idx, cur_cnt);

            seg_idx += cur_cnt;

            if (seg_idx == seg->dat.size) {
#if EnStaging
                l_n = n;
#endif
                n = bin_tree::StepR(n);
                seg_idx = 0;
            }

            cnt -= cur_cnt;
        }
    }

    if (cntr->rb != n) {
        seg = NToSeg_(n);

        bin_tree::SetSize(n, EnStagingTernary(GetNColor_(n) == ref_color,
                                              seg->ref.size, seg->dat.size));
    }

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = end;
    dst_cursor->n = n;
    dst_cursor->seg_idx = seg_idx;

    if (cntr->rb == n
#if EnStaging
        || GetNColor_(n) == ref_color
#endif
    ) {
        dst_cursor->ref = nullptr;
        return;
    }

    seg = NToSeg_(n);

    CircularArray ca{
        .data = seg->dat.data,
        .width = width,
        .stride = stride,
        .offset = seg->dat.offset,
        .size = seg->dat.size,
        .capacity = seg_capacity,
    };

    dst_cursor->ref =
        circular_array::ops::Access(&ca, seg_idx, nullptr, nullptr);
}

#if EnStaging

template <CntrTplParamList>
void EraseRefSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* seg) {
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == ref_color);

    cntr->root = rbtree::Extract(&seg->n);

    SegAllocatorLike::Deallocate(&cntr->seg_allocator, seg);
}

#endif

template <CntrTplParamList>
void EraseDatSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* seg) {
#if EnStaging
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == dat_color);
#endif

    cntr->root = rbtree::Extract(&seg->n);

    DataAllocatorLike::Deallocate(&cntr->data_allocator, seg->dat.data);

    SegAllocatorLike::Deallocate(&cntr->seg_allocator, seg);
}

template <CntrTplParamList>
void EraseSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* seg) {
#if EnStaging
    if (GetNColor_(&seg->n) == ref_color) {
        EraseRefSeg_(cntr, seg);
    } else
#endif
    {
        EraseDatSeg_(cntr, seg);
    }
}

template <typename SegAllocatorLike, typename DataAllocatorLike>
void EraseAllSegsRecursive_  // NOLINT(misc-use-internal-linkage)
    (TreeNode* lb, TreeNode* rb, TreeNode* n, SegAllocatorLike* seg_alctr,
     DataAllocatorLike* data_alctr) {
    do {
        TreeNode* nl{ n->GetLPtr() };
        TreeNode* nr{ n->GetRPtr() };

        if (lb != n && rb != n) {
            Seg* seg{ NToSeg_(n) };

#if EnStaging
            if (GetNColor_(n) == dat_color)
#endif
            {
                DataAllocatorLike::Deallocate(data_alctr, seg->dat.data);
            }

            SegAllocatorLike::Deallocate(seg_alctr, seg);
        }

        if (nr == nullptr) {
            if (nl == nullptr) { break; }

            nr = nl;
            nl = nullptr;
        }

        if (nl != nullptr) {
            EraseAllSegsRecursive_(lb, rb, nl, seg_alctr, data_alctr);
        }

        n = nr;
    } while (n != nullptr);
}

template <CntrTplParamList>
void EraseAllSegs_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, SegAllocatorLike* seg_alctr,
     DataAllocatorLike* data_alctr) {
    EraseAllSegsRecursive_(cntr->lb, cntr->rb, cntr->root, seg_alctr,
                           data_alctr);
}

template <CntrTplParamList>
void InitTree_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr) {
    cntr->lb->Init();
    cntr->rb->Init();

    cntr->lb->SetAccSize(1);
    cntr->rb->SetAccSize(1);

    TreeNode* root{ nullptr };

    root = rbtree::InsertR(root, cntr->lb);
    root = rbtree::InsertR(root, cntr->rb);

    cntr->root = root;
}

#if EnStaging

template <CntrTplParamList>
void RefOrigin_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr) {
    Origin* origin{ cntr->origin };

    size_t origin_size{ Origin::GetSize(origin) };

    if (origin_size == 0) { return; }

    Seg* seg{ AllocateRefSeg_(cntr) };

    cntr->root = rbtree::Insert(cntr->lb, cntr->rb, &seg->n);

    seg->ref.beg = 0;
    seg->ref.size = origin_size;

    bin_tree::SetSize(&seg->n, origin_size);
}

#endif

}  // namespace detail

// -----------------------------------------------------------------------------

template <CntrTplParamList>
void PrintState_(Cntr<CntrTplArgList>* cntr, TreeNode* n) {
    TreeNode* l_n{ n->GetLPtr() };
    TreeNode* r_n{ n->GetRPtr() };

    if (l_n != nullptr) { PrintState_(cntr, l_n); }

    if (cntr->lb != n && cntr->rb != n) {
        Seg* seg{ NToSeg_(n) };

#if EnStaging
        if (GetNColor_(&seg->n) == ref_color) {
            std::cout << "ref " << static_cast<size_t>(seg->ref.size) << " ["
                      << static_cast<size_t>(seg->ref.beg) << ", "
                      << static_cast<size_t>(seg->ref.beg + seg->ref.size)
                      << "]\n";
        } else
#endif
        {
            std::cout << "dat " << static_cast<size_t>(seg->dat.size) << "\n";
        }
    }

    if (r_n != nullptr) { PrintState_(cntr, r_n); }
}

template <CntrTplParamList>
Stats GetStats_(Cntr<CntrTplArgList> const* cntr, TreeNode* n) {
    Stats ret{
#if EnStaging
        .ref_seg_cnt = 0,
#endif
        .dat_seg_cnt = 0,
#if EnStaging
        .ref_size = 0,
#endif
        .dat_size = 0,
    };

    for (;;) {
        if (cntr->lb != n && cntr->rb != n) {
            Seg* seg{ NToSeg_(n) };

#if EnStaging
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

        TreeNode* l_n{ n->GetLPtr() };
        TreeNode* r_n{ n->GetRPtr() };

        if (l_n == nullptr) { Swap(l_n, r_n); }

        if (l_n == nullptr) { break; }

        if (r_n == nullptr) {
            n = l_n;
            continue;
        }

        Stats l_stats{ GetStats_(cntr, l_n) };

#if EnStaging
        ret.ref_seg_cnt += l_stats.ref_seg_cnt;
#endif
        ret.dat_seg_cnt += l_stats.dat_seg_cnt;
#if EnStaging
        ret.ref_size += l_stats.ref_size;
#endif
        ret.dat_size += l_stats.dat_size;

        n = r_n;
    }

    return ret;
}

struct SanitizeRet {
    bool b;
    size_t ref_beg;
    size_t ref_end;
};

template <CntrTplParamList>
SanitizeRet Sanitize_(Cntr<CntrTplArgList> const* cntr, MemRecorder* dst_seg,
                      MemRecorder* dst_data, TreeNode* n) {
#if EnStaging
    auto* origin{ cntr->origin };
    size_t origin_size{ Origin::GetSize(origin) };
#endif

    size_t stride{ cntr->stride };

    TreeNode* l_n{ bin_tree::StepL(n) };
    TreeNode* r_n{ bin_tree::StepR(n) };

    if (cntr->lb == n) {
        ZETA_Core_DebugAssert(l_n == nullptr);
        ZETA_Core_DebugAssert(r_n != nullptr);

        ZETA_Core_DebugAssert(bin_tree::GetSize(n) == 1);

        if (dst_seg != nullptr) {
            MemRecorder::Record(dst_seg, cntr->lb, sizeof(TreeNode));
        }
    } else if (cntr->rb == n) {
        ZETA_Core_DebugAssert(l_n != nullptr);
        ZETA_Core_DebugAssert(r_n == nullptr);

        ZETA_Core_DebugAssert(bin_tree::GetSize(n) == 1);

        if (dst_seg != nullptr) {
            MemRecorder::Record(dst_seg, cntr->rb, sizeof(TreeNode));
        }
    }
#if EnStaging
    else if (GetNColor_(n) == ref_color) {
        ZETA_Core_DebugAssert(l_n != nullptr);
        ZETA_Core_DebugAssert(r_n != nullptr);

        Seg* seg{ NToSeg_(n) };

        if (dst_seg != nullptr) {
            MemRecorder::Record(dst_seg, seg, sizeof(Seg));
        }

        ZETA_Core_DebugAssert(0 < seg->ref.size);

        ZETA_Core_DebugAssert(seg->ref.beg < origin_size);
        ZETA_Core_DebugAssert(seg->ref.beg + seg->ref.size <= origin_size);

        size_t size{ seg->ref.size };

        ZETA_Core_DebugAssert(size == bin_tree::GetSize(n));

        if (cntr->lb != l_n) {
            Seg* l_seg{ NToSeg_(l_n) };

            if (GetNColor_(&l_seg->n) == ref_color) {
                ZETA_Core_DebugAssert(l_seg->ref.beg + l_seg->ref.size <
                                      seg->ref.beg);
            }
        }

        if (cntr->rb != r_n) {
            Seg* r_seg{ NToSeg_(r_n) };

            if (GetNColor_(&r_seg->n) == ref_color) {
                ZETA_Core_DebugAssert(seg->ref.beg + seg->ref.size <
                                      r_seg->ref.beg);
            }
        }
    }
#endif
    else {

#if EnStaging
        ZETA_Core_DebugAssert(GetNColor_(n) == dat_color);
#endif

        ZETA_Core_DebugAssert(l_n != nullptr);
        ZETA_Core_DebugAssert(r_n != nullptr);

        Seg* seg{ NToSeg_(n) };

        if (dst_seg != nullptr) {
            MemRecorder::Record(dst_seg, seg, sizeof(Seg));
        }

        size_t size{ seg->dat.size };

        ZETA_Core_DebugAssert(size == bin_tree::GetSize(n));

        ZETA_Core_DebugAssert(0 < size);
        ZETA_Core_DebugAssert(size <= cntr->seg_capacity);
        ZETA_Core_DebugAssert(seg->dat.offset < cntr->seg_capacity);

        if (dst_data != nullptr) {
            MemRecorder::Record(dst_data, seg->dat.data,
                                stride * cntr->seg_capacity);
        }

        size_t l_vacant;
        size_t r_vacant;

        if (cntr->lb == l_n) {
            l_vacant = 0;
        } else {
            Seg* l_seg{ NToSeg_(l_n) };

            l_vacant =
                cntr->seg_capacity -
                EnStagingTernary(GetNColor_(l_n) == ref_color,
                                 Min(l_seg->ref.size, cntr->seg_capacity),
                                 l_seg->dat.size);
        }

        if (cntr->rb == r_n) {
            r_vacant = 0;
        } else {
            Seg* r_seg{ NToSeg_(r_n) };

            r_vacant =
                cntr->seg_capacity -
                EnStagingTernary(GetNColor_(r_n) == ref_color,
                                 Min(r_seg->ref.size, cntr->seg_capacity),
                                 r_seg->dat.size);
        }

        ZETA_Core_DebugAssert(0 <= l_vacant);
        ZETA_Core_DebugAssert(0 <= r_vacant);
    }

    TreeNode* sub_l_n{ n->GetLPtr() };
    TreeNode* sub_r_n{ n->GetRPtr() };

    SanitizeRet l_check_ret{ false, 0, 0 };
    SanitizeRet r_check_ret{ false, 0, 0 };

    if (sub_l_n != nullptr) {
        l_check_ret = Sanitize_(cntr, dst_seg, dst_data, sub_l_n);
    }

    if (sub_r_n != nullptr) {
        r_check_ret = Sanitize_(cntr, dst_seg, dst_data, sub_r_n);
    }

    SanitizeRet ret{ false, 0, 0 };

    if (l_check_ret.b && r_check_ret.b) {
        ZETA_Core_DebugAssert(l_check_ret.ref_end <= r_check_ret.ref_beg);

        ret.b = true;
        ret.ref_beg = l_check_ret.ref_beg;
        ret.ref_end = r_check_ret.ref_end;
    } else if (l_check_ret.b) {
        ret = l_check_ret;
    } else if (r_check_ret.b) {
        ret = r_check_ret;
    }

    if (cntr->lb == n || cntr->rb == n) { return ret; }

#if EnStaging
    if (GetNColor_(n) == ref_color) {
        Seg* seg{ NToSeg_(n) };

        size_t cur_ref_beg{ seg->ref.beg };
        size_t cur_ref_end{ seg->ref.beg + seg->ref.size };

        if (l_check_ret.b) {
            ZETA_Core_DebugAssert(l_check_ret.ref_end <= cur_ref_beg);
        }
        if (r_check_ret.b) {
            ZETA_Core_DebugAssert(cur_ref_end <= r_check_ret.ref_beg);
        }

        if (ret.b) {
            ret.ref_beg = Min(ret.ref_beg, cur_ref_beg);
            ret.ref_end = Max(ret.ref_end, cur_ref_end);
        } else {
            ret.b = true;
            ret.ref_beg = cur_ref_beg;
            ret.ref_end = cur_ref_end;
        }
    }
#endif

    return ret;
}

// -----------------------------------------------------------------------------

template <CntrTplParamList>
void Init(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    cntr->lb = static_cast<TreeNode*>(allocator::SafeAllocate(
        &cntr->seg_allocator, alignof(TreeNode), sizeof(TreeNode)));

    cntr->rb = static_cast<TreeNode*>(allocator::SafeAllocate(
        &cntr->seg_allocator, alignof(TreeNode), sizeof(TreeNode)));

    InitTree_(cntr);

#if EnStaging
    RefOrigin_(cntr);
#endif
}

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    EraseAllSegs_(cntr, &cntr->seg_allocator, &cntr->data_allocator);

    SegAllocatorLike::Deallocate(&cntr->seg_allocator, cntr->lb);
    SegAllocatorLike::Deallocate(&cntr->seg_allocator, cntr->rb);
}

template <CntrTplParamList>
constexpr size_t GetCursorSize(void const*) {
    return sizeof(Cursor);
}

template <CntrTplParamList>
size_t GetWidth(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    return cntr->width;
}

template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    return cntr->root->GetAccSize() - 2;
}

template <CntrTplParamList>
size_t GetCapacity(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void GetLBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->n = cntr->lb;
    dst_cursor->seg_idx = 0;
    dst_cursor->ref = nullptr;
}

template <CntrTplParamList>
void GetRBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = GetSize(cntr);
    dst_cursor->n = cntr->rb;
    dst_cursor->seg_idx = 0;
    dst_cursor->ref = nullptr;
}

template <CntrTplParamList>
void* PeekL(Cntr<CntrTplArgList>* cntr, Cursor* dst_cursor, bool lazy_copy_elem,
            void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

#if EnStaging
    auto* origin{ cntr->origin };
#endif

    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };

    TreeNode* n{ bin_tree::StepR(cntr->lb) };

    if (cntr->rb == n) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = 0;
            dst_cursor->n = n;
            dst_cursor->seg_idx = 0;
            dst_cursor->ref = nullptr;
        }

        return nullptr;
    }

    Seg* seg{ detail::NToSeg_(n) };

    void* ref;

#if EnStaging
    if (detail::GetNColor_(n) == ref_color) {
        Origin::Access(origin, seg->ref.beg, nullptr, dst_elem);
        ref = nullptr;
    } else
#endif
    {
        CircularArray ca{
            .data = seg->dat.data,
            .width = width,
            .stride = stride,
            .offset = seg->dat.offset,
            .size = seg->dat.size,
            .capacity = seg_capacity,
        };

        ref = circular_array::ops::PeekL(&ca, nullptr, dst_elem);
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
        dst_cursor->n = n;
        dst_cursor->seg_idx = 0;
        dst_cursor->ref = ref;
    }

    return ref;
}

template <CntrTplParamList>
void const* PeekL(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor,
                  bool lazy_copy_elem, void* dst_elem) {
    return PeekL(const_cast<Cntr<CntrTplArgList>*>(cntr), dst_cursor,
                 lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
void* PeekR(Cntr<CntrTplArgList>* cntr, Cursor* dst_cursor, bool lazy_copy_elem,
            void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

#if EnStaging
    auto* origin{ cntr->origin };
#endif

    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };
    size_t size{ GetSize(cntr) };

    TreeNode* n{ bin_tree::StepL(cntr->rb) };

    if (cntr->lb == n) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = static_cast<size_t>(-1);
            dst_cursor->n = n;
            dst_cursor->seg_idx = 0;
            dst_cursor->ref = nullptr;
        }

        return nullptr;
    }

    Seg* seg{ NToSeg_(n) };

    void* ref;

    size_t seg_size;

#if EnStaging
    if (GetNColor_(n) == ref_color) {
        seg_size = seg->ref.size;

        Origin::Access(origin, seg->ref.beg + seg_size - 1, nullptr, dst_elem);

        ref = nullptr;
    } else
#endif
    {
        seg_size = seg->dat.size;

        CircularArray ca{
            .data = seg->dat.data,
            .width = width,
            .stride = stride,
            .offset = seg->dat.offset,
            .size = seg_size,
            .capacity = seg_capacity,
        };

        ref = circular_array::ops::PeekR(&ca, nullptr, dst_elem);
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = size - 1;
        dst_cursor->n = n;
        dst_cursor->seg_idx = seg_size - 1;
        dst_cursor->ref = ref;
    }

    return ref;
}

template <CntrTplParamList>
void const* PeekR(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor,
                  bool lazy_copy_elem, void* dst_elem) {
    return PeekR(const_cast<Cntr<CntrTplArgList>*>(cntr), dst_cursor,
                 lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
void* Access(Cntr<CntrTplArgList>* cntr, size_t idx, Cursor* dst_cursor,
             void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

#if EnStaging
    auto* origin{ cntr->origin };
#endif

    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };
    size_t size{ GetSize(cntr) };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, size));

    auto [n, seg_idx]{ bin_tree::AccessL(cntr->root, idx + 1) };

    void* ref{ nullptr };

    if (idx < size) {
        Seg* seg{ NToSeg_(n) };

#if EnStaging
        if (GetNColor_(n) == ref_color) {
            if (dst_elem != nullptr) {
                Origin::Access(origin, seg->ref.beg + seg_idx, nullptr,
                               dst_elem);
            }
        } else
#endif
        {
            CircularArray ca{
                .data = seg->dat.data,
                .width = width,
                .stride = stride,
                .offset = seg->dat.offset,
                .size = seg->dat.size,
                .capacity = seg_capacity,
            };

            ref = circular_array::ops::Access(&ca, seg_idx, nullptr, dst_elem);
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx;
        dst_cursor->n = n;
        dst_cursor->seg_idx = seg_idx;
        dst_cursor->ref = ref;
    }

    return ref;
}

template <CntrTplParamList>
void const* Access(Cntr<CntrTplArgList> const* cntr, size_t idx,
                   Cursor* dst_cursor, void* dst_elem) {
    return Access(const_cast<Cntr<CntrTplArgList>*>(cntr), idx, dst_cursor,
                  dst_elem);
}

template <CntrTplParamList>
void* Derefer(Cntr<CntrTplArgList>* cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor));

    void* elem{ pos_cursor->ref };

    // TODO
}

template <CntrTplParamList>
void const* Derefer(Cntr<CntrTplArgList> const* cntr, void const* pos_cursor,
                    bool lazy_copy_elem, void* dst_elem) {
    return Derefer(const_cast<Cntr<CntrTplArgList>*>(cntr), pos_cursor,
                   lazy_copy_elem, dst_elem);
}

template <CntrTplParamList, typename Reader>
void Read(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
          size_t cnt,
          Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
          Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor));

    ZETA_Core_DebugAssert(
        seq_cntr::IsDereferable(pos_cursor->idx, cnt, GetSize(cntr)));

#if EnStaging
    auto* origin{ cntr->origin };
#endif

    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };

    TreeNode* n{ pos_cursor->n };
    Seg* seg;

    size_t seg_idx{ pos_cursor->seg_idx };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = pos_cursor->idx + cnt;
    }

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->n = n;
            dst_cursor->seg_idx = seg_idx;
            dst_cursor->ref = pos_cursor->ref;
        }

        return;
    }

#if EnStaging
    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };
#endif

    CircularArray ca{
        .data = {},
        .width = width,
        .stride = stride,
        .offset = {},
        .size = seg_capacity,
        .capacity = seg_capacity,
    };

    seq_cntr::FnReader fn_reader{ reader };

    while (0 < cnt) {
        seg = NToSeg_(n);

        size_t cur_cnt;
        size_t seg_size;

#if EnStaging
        if (GetNColor_(n) == ref_color) {
            seg_size = seg->ref.size;
            cur_cnt = Min(cnt, seg_size - seg_idx);

            Origin::ConstAccess(origin, seg->ref.beg + seg_idx, origin_cursor,
                                nullptr);

            Origin::FnRead(origin, origin_cursor, cur_cnt, fn_reader, nullptr);
        } else
#endif
        {
            seg_size = seg->dat.size;
            cur_cnt = Min(cnt, seg_size - seg_idx);

            ca.data = seg->dat.data;
            ca.offset = seg->dat.offset;

            circular_array::ops::IdxRead(&ca, seg_idx, cur_cnt, reader);
        }

        seg_idx += cur_cnt;

        if (seg_idx == seg_size) {
            n = bin_tree::StepR(n);
            seg_idx = 0;
        }

        cnt -= cur_cnt;
    }

    if (dst_cursor == nullptr) { return; }

    dst_cursor->n = n;
    dst_cursor->seg_idx = seg_idx;

    if (cntr->rb == n
#if EnStaging
        || GetNColor_(n) == ref_color
#endif
    ) {
        dst_cursor->ref = nullptr;
        return;
    }

    seg = NToSeg_(n);

    ca.data = seg->dat.data;
    ca.offset = seg->dat.offset;

    dst_cursor->ref =
        circular_array::ops::Access(&ca, seg_idx, nullptr, nullptr);
}

template <CntrTplParamList, typename Writer>
void Write(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
           Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
           Cursor* dst_cursor) {
    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };

    struct {
        Writer&&
            writer;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        seq_cntr::FnWriter fn_writer;

        CircularArray ca;

        void ReadWriteDat(void* data, size_t offset, size_t idx, size_t cnt) {
            this->ca.data = data;
            this->ca.offset = offset;

            circular_array::ops::IdxWrite(&this->ca, idx, cnt, this->writer);
        }
    } reader_core{
        .writer = writer,
        .fn_writer = writer,

        .ca = {
            .data = {},
            .width = width,
            .stride = stride,
            .offset = {},
            .size = seg_capacity,
            .capacity = seg_capacity,
        },
    };

    CoreReadWrite_<false>(cntr, pos_cursor, cnt, reader_core, dst_cursor);
}

template <CntrTplParamList, typename ReaderWriter>
void ReadWrite(
    Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };

    struct {
        ReaderWriter&&
            reader_writer;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        seq_cntr::FnReaderWriter fn_reader_writer;

        CircularArray ca;

        void ReadWriteDat(void* data, size_t offset, size_t idx, size_t cnt) {
            this->ca.data = data;
            this->ca.offset = offset;

            circular_array::ops::IdxWrite(&this->ca, idx, cnt,
                                          this->reader_writer);
        }
    } reader_writer_core{
        .reader_writer = reader_writer,
        .fn_reader_writer = reader_writer,

        .ca = {
            .data = {},
            .width = width,
            .stride = stride,
            .offset = {},
            .size = seg_capacity,
            .capacity = seg_capacity,
        },
    };

    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

    CoreReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer_core, dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* PushL(Cntr<CntrTplArgList>* cntr, size_t cnt,
            Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
            Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    Cursor pos_cursor;
    PeekL(cntr, &pos_cursor, nullptr);

    return Insert(cntr, &pos_cursor, cnt, writer, dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* PushR(Cntr<CntrTplArgList>* cntr, size_t cnt,
            Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
            Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    Cursor pos_cursor;
    GetRBCursor(cntr, &pos_cursor);

    return Insert(cntr, &pos_cursor, cnt, writer, dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* Insert(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
             Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
             Cursor* dst_cursor) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor));

    ZETA_Core_DebugAssert(seq_cntr::IsInsertable(
        pos_cursor->idx, cnt, GetSize(cntr), GetCapacity(cntr)));

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->n = pos_cursor->n;
            dst_cursor->seg_idx = pos_cursor->seg_idx;
            dst_cursor->ref = pos_cursor->ref;
        }

        return pos_cursor->ref;
    }

    size_t width{ cntr->width };
    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };

    size_t end_idx{ pos_cursor->idx + cnt };

    TreeNode* m_n{ pos_cursor->n };
    size_t seg_idx{ pos_cursor->seg_idx };

    ZETA_Core_DebugAssert(cntr->lb != m_n);

    unsigned long long random_seed{ GetRandom() };

    TreeNode* l_n;
    TreeNode* r_n;

    Seg* l_seg;
    Seg* m_seg;
    Seg* r_seg;

#if EnStaging
    unsigned l_color;
    unsigned m_color;
    unsigned r_color;
#endif

    size_t l_vacant;
    size_t m_vacant;
    size_t r_vacant;

    CircularArray l_ca{
        .data = {},
        .width = width,
        .stride = stride,
        .offset = {},
        .size = {},
        .capacity = seg_capacity,
    };

    CircularArray m_ca{
        .data = {},
        .width = width,
        .stride = stride,
        .offset = {},
        .size = {},
        .capacity = seg_capacity,
    };

    CircularArray r_ca{
        .data = {},
        .width = width,
        .stride = stride,
        .offset = {},
        .size = {},
        .capacity = seg_capacity,
    };

    if (seg_idx == 0) {
        /*

        if the insertion point is between segments, the state has already be a
        nice form to insert (optionally) new segments to store more data.

        */

        l_n = bin_tree::StepL(m_n);

        r_n = m_n;

        m_n = nullptr;
        m_seg = nullptr;

        if (cntr->lb == l_n) {
            l_seg = nullptr;
            l_ca.size = 0;
            l_vacant = 0;
        } else {
            l_seg = NToSeg_(l_n);
            GetSegState_(seg_capacity, l_seg,
#if EnStaging
                         &l_color,
#endif
                         &l_ca, &l_vacant);
        }

        if (cntr->rb == r_n) {
            r_seg = nullptr;
            r_ca.size = 0;
            r_vacant = 0;
        } else {
            r_seg = NToSeg_(r_n);
            GetSegState_(seg_capacity, r_seg,
#if EnStaging
                         &r_color,
#endif
                         &r_ca, &r_vacant);
        }
    } else {
        m_seg = NToSeg_(m_n);
        GetSegState_(seg_capacity, m_seg,
#if EnStaging
                     &m_color,
#endif
                     &m_ca, &m_vacant);

        if (cnt <= m_vacant) {
            /*

            if the insertion segment can hold all old + new data, just insert
            into it.

            */

            void* ref;

#if EnStaging
            if (m_color == ref_color) {
                m_ca.data = AllocateData_(cntr, nullptr);
                m_ca.offset = 0;
                m_ca.size = 0;

                RefShoveL_(cntr, &m_ca, m_seg, seg_idx, cnt,
                           m_seg->ref.size + cnt, writer);

                SetNColor_(&m_seg->n, dat_color);

                m_seg->dat.data = m_ca.data;

                ref = circular_array::ops::Access(&m_ca, seg_idx, nullptr,
                                                  nullptr);
            } else
#endif
            {
                ref =
                    circular_array::ops::IdxInsert(&m_ca, seg_idx, cnt, writer);
            }

            m_seg->dat.offset = static_cast<unsigned short>(m_ca.offset);
            m_seg->dat.size = static_cast<unsigned short>(m_ca.size);

            bin_tree::SetSize(m_n, m_ca.size);

            pos_cursor->n = m_n;
            pos_cursor->seg_idx = seg_idx;
            pos_cursor->ref = ref;

            if (dst_cursor != nullptr) {
                dst_cursor->cntr = cntr;
                dst_cursor->idx = end_idx;
                dst_cursor->n = m_n;
                dst_cursor->seg_idx = seg_idx + cnt;
                dst_cursor->ref = circular_array::ops::Access(
                    &m_ca, seg_idx + cnt, nullptr, nullptr);
            }

            return ref;
        }

        l_n = reinterpret_cast<TreeNode*>(
            bin_tree::StepL(reinterpret_cast<TreeNodeView*>(m_n)));
        r_n = reinterpret_cast<TreeNode*>(
            bin_tree::StepR(reinterpret_cast<TreeNodeView*>(m_n)));

        if (cntr->lb == l_n) {
            l_seg = nullptr;
            l_ca.size = 0;
            l_vacant = 0;
        } else {
            l_seg = NToSeg_(l_n);
            GetSegState_(seg_capacity, l_seg,
#if EnStaging
                         &l_color,
#endif
                         &l_ca, &l_vacant);
        }

        if (cntr->rb == r_n) {
            r_seg = nullptr;
            r_ca.size = 0;
            r_vacant = 0;
        } else {
            r_seg = NToSeg_(r_n);
            GetSegState_(seg_capacity, r_seg,
#if EnStaging
                         &r_color,
#endif
                         &r_ca, &r_vacant);
        }

        size_t ml_size{ seg_idx };
        size_t mr_size{ m_ca.size - seg_idx };

        size_t new_l_m_size{ l_ca.size + m_ca.size + cnt };
        size_t new_m_r_size{ m_ca.size + r_ca.size + cnt };

        size_t l_m_vacant{ l_vacant + m_vacant };
        size_t m_r_vacant{ m_vacant + r_vacant };

        bool l_m_ok{ cnt <= l_m_vacant };
        bool m_r_ok{ cnt <= m_r_vacant };

        if (!l_m_ok && !m_r_ok) {
            /*

            if neither l + m and m + r can hold all old + new data, it means it
            is too crowded near this section, we do not try to use l + m + r to
            hold all old + new data, instead we jump to split logic to transform
            the insertion point to be between segments.

            */

            goto SPLIT;
        }

        // choose the better side to insert

        switch (Choose2(l_m_ok && m_r_vacant <= l_m_vacant,
                        m_r_ok && l_m_vacant <= m_r_vacant, &random_seed)) {
        case 0: {
            // use l + m to hold all old + new data

#if EnStaging
            if (l_color == ref_color) {
                TransferRefSegToDatSeg_(cntr, l_seg);
                l_color = dat_color;
                l_ca.data = l_seg->dat.data;
                l_ca.offset = l_seg->dat.offset;
            }
#endif

            size_t ret_seg_idx{ l_ca.size + ml_size };

            size_t new_l_size{ (new_l_m_size + (m_ca.size <= l_ca.size)) / 2 };

#if EnStaging
            if (m_color == ref_color) {
                RefShoveL_(cntr, &l_ca, m_seg, ml_size, cnt,
                           new_l_size - l_ca.size, writer);

                if (m_color == ref_color) {
                    m_ca.size = m_seg->ref.size;
                } else {
                    m_ca.data = m_seg->dat.data;
                    m_ca.offset = m_seg->dat.offset;
                    m_ca.size = m_seg->dat.size;
                }
            } else
#endif
            {
                seg_utils::SegShoveL(&l_ca, &m_ca, ml_size, cnt,
                                     new_l_size - l_ca.size, writer);

                m_seg->dat.offset = static_cast<unsigned short>(m_ca.offset);
                m_seg->dat.size = static_cast<unsigned short>(m_ca.size);
            }

            bin_tree::SetSize(m_n, m_ca.size);

            l_seg->dat.offset = static_cast<unsigned short>(l_ca.offset);
            l_seg->dat.size = static_cast<unsigned short>(l_ca.size);

            bin_tree::SetSize(l_n, l_ca.size);

            if (ret_seg_idx < l_ca.size) {
                pos_cursor->n = l_n;
                pos_cursor->seg_idx = ret_seg_idx;
                pos_cursor->ref = circular_array::ops::Access(
                    &l_ca, ret_seg_idx, nullptr, nullptr);
            } else {
                ret_seg_idx -= l_ca.size;

                pos_cursor->n = m_n;
                pos_cursor->seg_idx = ret_seg_idx;

#if EnStaging
                if (m_color == ref_color) {
                    pos_cursor->ref = nullptr;
                } else
#endif
                {
                    pos_cursor->ref = circular_array::ops::Access(
                        &m_ca, ret_seg_idx, nullptr, nullptr);
                }
            }

            if (dst_cursor != nullptr) {
                dst_cursor->cntr = cntr;
                dst_cursor->idx = end_idx;
                dst_cursor->n = m_n;
                dst_cursor->seg_idx = m_ca.size - mr_size;
                dst_cursor->ref = EnStagingTernary(
                    m_color == ref_color, nullptr,
                    circular_array::ops::Access(&m_ca, m_ca.size - mr_size,
                                                nullptr, nullptr));
            }

            break;
        }

        case 1: {
            // use m + r to hold all old + new data

#if EnStaging
            if (r_color == ref_color) {
                TransferRefSegToDatSeg_(cntr, r_seg);
                r_color = dat_color;
                r_ca.data = r_seg->dat.data;
                r_ca.offset = r_seg->dat.offset;
            }
#endif

            size_t ret_seg_idx{ seg_idx };

            size_t old_r_size{ r_ca.size };
            size_t new_r_size{ (new_m_r_size + (m_ca.size <= r_ca.size)) / 2 };

#if EnStaging
            if (m_color == ref_color) {
                RefShoveR_(cntr, m_seg, &r_ca, mr_size, cnt,
                           new_r_size - r_ca.size, writer);

                m_ca.size = m_seg->ref.size;
            } else
#endif
            {
                seg_utils::SegShoveR(&m_ca, &r_ca, mr_size, cnt,
                                     new_r_size - r_ca.size, writer);

                m_seg->dat.offset = static_cast<unsigned short>(m_ca.offset);
                m_seg->dat.size = static_cast<unsigned short>(m_ca.size);
            }

            bin_tree::SetSize(m_n, m_ca.size);

            r_seg->dat.offset = static_cast<unsigned short>(r_ca.offset);
            r_seg->dat.size = static_cast<unsigned short>(r_ca.size);

            bin_tree::SetSize(r_n, r_ca.size);

            if (ret_seg_idx < m_ca.size) {
                pos_cursor->n = m_n;
                pos_cursor->seg_idx = ret_seg_idx;

#if EnStaging
                if (m_color == ref_color) {
                    pos_cursor->ref = nullptr;
                } else
#endif
                {
                    pos_cursor->ref = circular_array::ops::Access(
                        &m_ca, ret_seg_idx, nullptr, nullptr);
                }
            } else {
                ret_seg_idx -= m_ca.size;

                pos_cursor->n = r_n;
                pos_cursor->seg_idx = ret_seg_idx;
                pos_cursor->ref = circular_array::ops::Access(
                    &r_ca, ret_seg_idx, nullptr, nullptr);
            }

            if (dst_cursor != nullptr) {
                dst_cursor->cntr = cntr;
                dst_cursor->idx = end_idx;
                dst_cursor->n = r_n;
                dst_cursor->seg_idx = r_ca.size - old_r_size;
                dst_cursor->ref = circular_array::ops::Access(
                    &r_ca, r_ca.size - old_r_size, nullptr, nullptr);
            }

            break;
        }
        }

        return pos_cursor->ref;

    SPLIT: {
        /*

        the aim is to transform the insertion point to be between segments.

        case 0: push ml to l
        case 1: push mr to r
        case 2: insert a seg between l and m, push ml to new seg
        case 3: insert a seg between m and r, push mr to new seg

        */

        bool l_m_ok{ ml_size <= l_vacant };
        bool m_r_ok{ mr_size <= r_vacant };

        size_t new_l_size{ ml_size + l_ca.size };
        size_t new_r_size{ r_ca.size + mr_size };

        switch (l_m_ok || m_r_ok
                    ? Choose2(l_m_ok && (!m_r_ok || new_l_size <= new_r_size),
                              m_r_ok && (!l_m_ok || new_r_size <= new_l_size),
                              &random_seed)
                    : Choose2(ml_size <= mr_size, mr_size <= ml_size,
                              &random_seed) +
                          2) {
        case 0: {
            // push ml to l

#if EnStaging
            if (l_color == ref_color) {
                TransferRefSegToDatSeg_(cntr, l_seg);
                l_color = dat_color;
                l_ca.data = l_seg->dat.data;
                l_ca.offset = l_seg->dat.offset;
            }
#endif

#if EnStaging
            if (m_color == ref_color) {
                PushRefR_(cntr, &l_ca, m_seg->ref.beg, ml_size);

                m_seg->ref.beg += ml_size;
                m_ca.size -= ml_size;
            } else
#endif
            {
                seg_utils::SegShoveL(&l_ca, &m_ca, 0, 0, ml_size,
                                     [](void*, size_t, size_t) {});
            }

            ml_size = 0;
            l_vacant = seg_capacity - l_ca.size;

            r_n = m_n;
            r_seg = m_seg;
#if EnStaging
            r_color = m_color;
#endif
            r_ca.data = m_ca.data;
            r_ca.offset = m_ca.offset;
            r_ca.size = m_ca.size;
            r_vacant = seg_capacity - Min(r_ca.size, seg_capacity);

            m_n = nullptr;
            m_seg = nullptr;

            break;
        }

        case 1: {
            // push mr to r

#if EnStaging
            if (r_color == ref_color) {
                TransferRefSegToDatSeg_(cntr, r_seg);
                r_color = dat_color;
                r_ca.data = r_seg->dat.data;
                r_ca.offset = r_seg->dat.offset;
            }
#endif

#if EnStaging
            if (m_color == ref_color) {
                PushRefL_(cntr, &r_ca, m_seg->ref.beg + ml_size, mr_size);

                m_ca.size -= mr_size;
            } else
#endif
            {
                seg_utils::SegShoveR(&m_ca, &r_ca, 0, 0, mr_size,
                                     [](void*, size_t, size_t) {});
            }

            mr_size = 0;
            r_vacant = seg_capacity - r_ca.size;

            l_n = m_n;
            l_seg = m_seg;
#if EnStaging
            l_color = m_color;
#endif
            l_ca.data = m_ca.data;
            l_ca.offset = m_ca.offset;
            l_ca.size = m_ca.size;
            l_vacant = seg_capacity - Min(l_ca.size, seg_capacity);

            m_n = nullptr;
            m_seg = nullptr;

            break;
        }

        case 2: {
            // insert a seg between l and m, push ml to new seg

#if EnStaging
            if (m_color == ref_color) {
                Seg* new_l_seg{ AllocateRefSeg_(cntr) };

                cntr->root = rbtree::Insert(l_n, m_n, &new_l_seg->n);

                l_n = &new_l_seg->n;
                l_seg = new_l_seg;
                l_color = ref_color;
                l_seg->ref.beg = m_seg->ref.beg;
                l_ca.size = ml_size;
                l_vacant = seg_capacity - Min(l_ca.size, seg_capacity);

                r_n = m_n;
                r_seg = m_seg;
                r_color = ref_color;
                r_seg->ref.beg += ml_size;
                r_ca.size = mr_size;
                r_vacant = seg_capacity - Min(r_ca.size, seg_capacity);
            } else
#endif
            {
                Seg* new_l_seg{ AllocateDatSeg_(cntr) };

                cntr->root = rbtree::Insert(l_n, m_n, &new_l_seg->n);

                l_n = &new_l_seg->n;
                l_seg = new_l_seg;
#if EnStaging
                l_color = dat_color;
#endif
                l_ca.data = new_l_seg->dat.data;
                l_ca.offset = 0;
                l_ca.size = 0;

                r_n = m_n;
                r_seg = m_seg;
#if EnStaging
                r_color = dat_color;
#endif
                r_ca.data = m_ca.data;
                r_ca.offset = m_ca.offset;
                r_ca.size = m_ca.size;

                seg_utils::SegShoveL(&l_ca, &r_ca, 0, 0, ml_size,
                                     [](void*, size_t, size_t) {});

                l_vacant = seg_capacity - l_ca.size;
                r_vacant = seg_capacity - r_ca.size;
            }

            break;
        }

        case 3: {
            // insert a seg between m and r, push mr to new seg

#if EnStaging
            if (m_color == ref_color) {
                Seg* new_r_seg{ AllocateRefSeg_(cntr) };

                cntr->root = rbtree::Insert(m_n, r_n, &new_r_seg->n);

                l_n = m_n;
                l_seg = m_seg;
                l_color = m_color;
                l_ca.size = ml_size;
                l_vacant = seg_capacity - Min(l_ca.size, seg_capacity);

                r_n = &new_r_seg->n;
                r_seg = new_r_seg;
                r_color = ref_color;
                r_seg->ref.beg = m_seg->ref.beg + ml_size;
                r_ca.size = mr_size;
                r_vacant = seg_capacity - Min(r_ca.size, seg_capacity);
            } else
#endif
            {
                Seg* new_r_seg{ AllocateDatSeg_(cntr) };

                cntr->root = rbtree::Insert(m_n, r_n, &new_r_seg->n);

                l_n = m_n;
                l_seg = m_seg;
#if EnStaging
                l_color = m_color;
#endif
                l_ca.data = m_ca.data;
                l_ca.offset = m_ca.offset;
                l_ca.size = m_ca.size;

                r_n = &new_r_seg->n;
                r_seg = new_r_seg;
#if EnStaging
                r_color = dat_color;
#endif
                r_ca.data = new_r_seg->dat.data;
                r_ca.offset = 0;
                r_ca.size = 0;

                seg_utils::SegShoveR(&l_ca, &r_ca, 0, 0, mr_size,
                                     [](void*, size_t, size_t) {});

                l_vacant = seg_capacity - l_ca.size;
                r_vacant = seg_capacity - r_ca.size;
            }

            break;
        }
        }
    }
    }

    /*

    the insertion point is between segments now. try to insert new segments to
    hold more data if necessary. consider use l and r to hold partial data if
    segments usage can be reduced.

    */

    size_t extra_segs_cnt_00{ UIntCeilDiv(cnt, seg_capacity) };
    size_t extra_segs_cnt_01{ UIntCeilDiv(cnt - Min(r_vacant, cnt),
                                          seg_capacity) };
    size_t extra_segs_cnt_10{ UIntCeilDiv(cnt - Min(l_vacant, cnt),
                                          seg_capacity) };
    size_t extra_segs_cnt_11{ UIntCeilDiv(cnt - Min(l_vacant + r_vacant, cnt),
                                          seg_capacity) };

    size_t extra_segs_cnt{ Min(extra_segs_cnt_00, extra_segs_cnt_01,
                               extra_segs_cnt_10, extra_segs_cnt_11) };

    bool l_shove;
    bool r_shove;

    if (extra_segs_cnt == extra_segs_cnt_00) {
        l_shove = false;
        r_shove = false;
    } else if (extra_segs_cnt == extra_segs_cnt_01 &&
               extra_segs_cnt == extra_segs_cnt_10) {
        switch (
            Choose2(l_vacant <= r_vacant, r_vacant <= l_vacant, &random_seed)) {
        case 0:
            l_shove = false;
            r_shove = true;
            break;

        case 1:
            l_shove = true;
            r_shove = false;
            break;
        }
    } else if (extra_segs_cnt == extra_segs_cnt_01) {
        l_shove = false;
        r_shove = true;
    } else if (extra_segs_cnt == extra_segs_cnt_10) {
        l_shove = true;
        r_shove = false;
    } else {
        l_shove = true;
        r_shove = true;
    }

    size_t total_size{ (l_shove ? l_ca.size : 0) + cnt +
                       (r_shove ? r_ca.size : 0) };

    bool ref_is_set{ false };

    if (l_shove) {
#if EnStaging
        if (l_color == ref_color) {
            TransferRefSegToDatSeg_(cntr, l_seg);

            l_color = dat_color;
            l_ca.data = l_seg->dat.data;
            l_ca.offset = 0;
        }
#endif

        size_t new_l_size{ GetAvgCnt_(total_size, seg_capacity, random_seed) };

        total_size -= new_l_size;

        pos_cursor->n = l_n;
        pos_cursor->seg_idx = l_ca.size;
        pos_cursor->ref = circular_array::ops::PushR(
            &l_ca, new_l_size - l_ca.size, writer, nullptr);

        ref_is_set = true;
    }

    if (l_seg != nullptr) {
#if EnStaging
        if (l_color == ref_color) {
            l_seg->ref.size = l_ca.size;
        } else
#endif
        {
            l_seg->dat.offset = static_cast<unsigned short>(l_ca.offset);
            l_seg->dat.size = static_cast<unsigned short>(l_ca.size);
        }

        bin_tree::SetSize(l_n, l_ca.size);
    }

    for (size_t extra_seg_i{ 0 }; extra_seg_i < extra_segs_cnt; ++extra_seg_i) {
        size_t cur_size{ GetAvgCnt_(total_size, seg_capacity, random_seed) };

        Seg* new_l_seg{ AllocateDatSeg_(cntr) };

        cntr->root = rbtree::Insert(l_n, r_n, &new_l_seg->n);

        writer(new_l_seg->dat.data, stride, cur_size);

        new_l_seg->dat.size = static_cast<unsigned short>(cur_size);

        bin_tree::SetSize(&new_l_seg->n, cur_size);

        total_size -= cur_size;

        l_n = &new_l_seg->n;
        l_seg = new_l_seg;

        if (!ref_is_set) {
            pos_cursor->n = l_n;
            pos_cursor->seg_idx = 0;

            pos_cursor->ref = l_seg->dat.data;

            ref_is_set = true;
        }
    }

    size_t r_ins_cnt{ 0 };

    if (r_shove) {
#if EnStaging
        if (r_color == ref_color) {
            TransferRefSegToDatSeg_(cntr, r_seg);

            r_color = dat_color;
            r_ca.data = r_seg->dat.data;
            r_ca.offset = 0;
        }
#endif

        size_t new_r_size{ GetAvgCnt_(total_size, seg_capacity, random_seed) };

        total_size -= new_r_size;

        void* ref{ circular_array::ops::PushL(
            &r_ca, r_ins_cnt = new_r_size - r_ca.size, writer, nullptr) };

        if (!ref_is_set) {
            pos_cursor->n = r_n;
            pos_cursor->seg_idx = 0;
            pos_cursor->ref = ref;

            ref_is_set = true;
        }
    }

    if (r_seg != nullptr) {
#if EnStaging
        if (r_color == ref_color) {
            r_seg->ref.size = r_ca.size;
        } else
#endif
        {
            r_seg->dat.offset = static_cast<unsigned short>(r_ca.offset);
            r_seg->dat.size = static_cast<unsigned short>(r_ca.size);
        }

        bin_tree::SetSize(r_n, r_ca.size);
    }

    if (dst_cursor == nullptr) { return pos_cursor->ref; }

    dst_cursor->cntr = cntr;
    dst_cursor->n = r_n;
    dst_cursor->idx = end_idx;

    if (r_shove) {
        dst_cursor->seg_idx = r_ins_cnt;
        dst_cursor->ref =
            circular_array::ops::Access(&r_ca, r_ins_cnt, nullptr, nullptr);
    } else {
        dst_cursor->seg_idx = 0;

        dst_cursor->ref =
            r_seg == nullptr
#if EnStaging
                    || r_color == ref_color
#endif
                ? nullptr
                : circular_array::ops::PeekL(&r_ca, nullptr, nullptr);
    }

    return pos_cursor->ref;
}

template <CntrTplParamList>
void PopL(void* cntr, size_t cnt) {
    Cursor pos_cursor;
    PeekL(cntr, &pos_cursor, nullptr);

    Erase(cntr, &pos_cursor, cnt);
}

template <CntrTplParamList>
void PopR(void* cntr, size_t cnt) {
    size_t size{ GetSize(cntr) };

    ZETA_Core_DebugAssert(cnt <= size);

    Cursor pos_cursor;
    Access(cntr, size - cnt, &pos_cursor, nullptr);

    Erase(cntr, &pos_cursor, cnt);
}

template <CntrTplParamList>
void Erase(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, pos_cursor));

    if (cnt == 0) { return; }

#if EnStaging
    size_t width{ cntr->origin.width };
#else
    size_t width{ cntr->width };
#endif

    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };
    size_t size{ GetSize(cntr) };

    ZETA_Core_DebugAssert(seq_cntr::IsErasable(pos_cursor->idx, cnt, size));

    TreeNode* m_n{ pos_cursor->n };
    size_t seg_idx{ pos_cursor->seg_idx };

    TreeNode* l_n{ bin_tree::StepL(m_n) };

    Seg* m_seg{ NToSeg_(m_n) };

    Seg* first_seg{ m_seg };
    size_t first_seg_idx{ seg_idx };

    Seg* last_seg{ m_seg };

    CircularArray m_ca{
        .data = {},
        .width = width,
        .stride = stride,
        .offset = {},
        .size = {},
        .capacity = seg_capacity,
    };

#if EnStaging
    if (GetNColor_(m_n) == ref_color && 0 < seg_idx) {
        m_seg = NToSeg_(m_n);

        size_t ref_beg{ m_seg->ref.beg };
        size_t ref_size{ m_seg->ref.size };

        if (ref_size - seg_idx <= cnt) {
            m_seg->ref.size = seg_idx;
            cnt -= ref_size - seg_idx;
            m_n = bin_tree::StepR(m_n);
        } else {
            Seg* pre_m_seg{ AllocateRefSeg_(cntr) };

            cntr->root = rbtree::Insert(l_n, &m_seg->n, &pre_m_seg->n);

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

#if EnStaging
        if (GetNColor_(m_n) == ref_color) {
            if (m_seg->ref.size <= cnt) {
                cnt -= m_seg->ref.size;
                TreeNode* nxt_m_n{ bin_tree::StepR(m_n) };
                EraseRefSeg_(cntr, m_seg);
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

            TreeNode* nxt_m_n{ bin_tree::StepR(m_n) };

            EraseDatSeg_(cntr, m_seg);

            m_n = nxt_m_n;

            continue;
        }

        size_t cur_cnt{ Min(cnt, m_seg->dat.size - seg_idx) };

        m_ca.data = m_seg->dat.data;
        m_ca.offset = m_seg->dat.offset;
        m_ca.size = m_seg->dat.size;

        circular_array::ops::IdxErase(&m_ca, seg_idx, cur_cnt);

        m_seg->dat.offset = static_cast<unsigned short>(m_ca.offset);
        m_seg->dat.size = static_cast<unsigned short>(m_ca.size);

        if (seg_idx == m_seg->dat.size) {
            m_n = bin_tree::StepR(m_n);
            seg_idx = 0;
        }

        cnt -= cur_cnt;
    }

    TreeNode* ret_n{ m_n };
    size_t ret_seg_idx{ seg_idx };

    unsigned long long random_seed{ GetRandom() };

    bool first_exist{ first_seg_idx != 0 || &first_seg->n == m_n };
    bool last_exist{ &last_seg->n == m_n };

    Seg* a_seg;
    Seg* b_seg;
    Seg* c_seg;
    Seg* d_seg;

    size_t a_size;
    size_t b_size;
    size_t c_size;
    size_t d_size;

    size_t a_vacant;
    size_t b_vacant;
    size_t c_vacant;
    size_t d_vacant;

    TreeNode* r_n{ last_exist ? bin_tree::StepR(m_n) : m_n };

    {
        Seg* segs_buffer[4]{ nullptr, nullptr, nullptr, nullptr };
        size_t segs_buffer_i{ 0 };

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

    if (a_seg == nullptr) {
    }
#if EnStaging
    else if (GetNColor_(&a_seg->n) == ref_color) {
        a_size = a_seg->ref.size;
        a_vacant = seg_capacity - Min(a_size, seg_capacity);
    }
#endif
    else {
        a_size = a_seg->dat.size;
        a_vacant = seg_capacity - a_size;
    }

    if (b_seg == nullptr) {
    }
#if EnStaging
    else if (GetNColor_(&b_seg->n) == ref_color) {
        b_size = b_seg->ref.size;
        b_vacant = seg_capacity - Min(b_size, seg_capacity);
    }
#endif
    else {
        b_size = b_seg->dat.size;
        b_vacant = seg_capacity - b_size;
    }

    if (c_seg == nullptr) {
    }
#if EnStaging
    else if (GetNColor_(&c_seg->n) == ref_color) {

        c_size = c_seg->ref.size;
        c_vacant = seg_capacity - Min(c_size, seg_capacity);
    }
#endif
    else {
        c_size = c_seg->dat.size;
        c_vacant = seg_capacity - c_size;
    }

    if (d_seg == nullptr) {
    }
#if EnStaging
    else if (GetNColor_(&d_seg->n) == ref_color) {
        d_size = d_seg->ref.size;
        d_vacant = seg_capacity - Min(d_size, seg_capacity);
    }
#endif
    else {
        d_size = d_seg->dat.size;
        d_vacant = seg_capacity - d_size;
    }

    if (a_seg == nullptr) { goto UPDATE; }

    if (ret_n == &a_seg->n) { ret_n = nullptr; }

    if (b_seg == nullptr) { goto UPDATE; }

    if (ret_n == &b_seg->n) {
        ret_n = nullptr;
        ret_seg_idx += a_size;
    }

#if EnStaging
    if (GetNColor_(&a_seg->n) == ref_color &&
        GetNColor_(&b_seg->n) == ref_color &&
        a_seg->ref.beg + a_seg->ref.size == b_seg->ref.beg) {
        a_seg->ref.size += b_seg->ref.size;

        EraseRefSeg_(cntr, b_seg);
        b_seg = nullptr;

        if (c_seg == nullptr) { goto UPDATE; }

        b_seg = c_seg;
        c_seg = d_seg;
        d_seg = nullptr;
    }
#endif

    if (c_seg == nullptr) { goto MERGE_2; }

    if (ret_n == &c_seg->n) {
        ret_n = nullptr;
        ret_seg_idx += a_size + b_size;
    }

#if EnStaging
    if (GetNColor_(&b_seg->n) == ref_color &&
        GetNColor_(&c_seg->n) == ref_color &&
        b_seg->ref.beg + b_seg->ref.size == c_seg->ref.beg) {
        b_seg->ref.size += c_seg->ref.size;

        EraseRefSeg_(cntr, c_seg);
        c_seg = nullptr;

        if (d_seg == nullptr) { goto MERGE_2; }

        c_seg = d_seg;
        d_seg = nullptr;

        goto MERGE_3;
    }
#endif

    if (d_seg == nullptr) { goto MERGE_3; }

    if (ret_n == &d_seg->n) {
        ret_n = nullptr;
        ret_seg_idx += a_size + b_size + c_size;
    }

#if EnStaging
    if (GetNColor_(&c_seg->n) == ref_color &&
        GetNColor_(&d_seg->n) == ref_color &&
        c_seg->ref.beg + c_seg->ref.size == d_seg->ref.beg) {
        c_seg->ref.size += d_seg->ref.size;

        EraseRefSeg_(cntr, d_seg);
        d_seg = nullptr;

        goto MERGE_3;
    }
#endif

    goto MERGE_4;

MERGE_2: {
    size_t ab_vacant{ a_vacant + b_vacant };

    if (seg_capacity <= ab_vacant) { Merge2_(cntr, a_seg, b_seg, true, true); }

    goto UPDATE;
}

MERGE_3: {
    size_t ab_vacant{ a_vacant + b_vacant };
    size_t bc_vacant{ b_vacant + c_vacant };

    size_t max_vacant = Max(ab_vacant, bc_vacant);

    if (seg_capacity < max_vacant) {
        switch (Choose2(bc_vacant <= ab_vacant, ab_vacant <= bc_vacant,
                        &random_seed)) {
        case 0: Merge2_(cntr, a_seg, b_seg, true, true); break;
        case 1: Merge2_(cntr, b_seg, c_seg, true, true); break;
        }
    }

    goto UPDATE;
}

MERGE_4: {
    size_t ab_vacant{ a_vacant + b_vacant };
    size_t bc_vacant{ b_vacant + c_vacant };
    size_t cd_vacant{ c_vacant + d_vacant };

    if (seg_capacity <= ab_vacant && seg_capacity <= cd_vacant) {
        Merge2_(cntr, a_seg, b_seg, true, true);
        Merge2_(cntr, c_seg, d_seg, true, true);
    } else {
        size_t max_vacant{ Max(ab_vacant, bc_vacant, cd_vacant) };

        if (seg_capacity <= max_vacant) {
            switch (Choose3(ab_vacant == max_vacant, bc_vacant == max_vacant,
                            cd_vacant == max_vacant, &random_seed)) {
            case 0: Merge2_(cntr, a_seg, b_seg, true, true); break;
            case 1: Merge2_(cntr, b_seg, c_seg, true, true); break;
            case 2: Merge2_(cntr, c_seg, d_seg, true, true); break;
            }
        }
    }

    goto UPDATE;
}

UPDATE: {
    if (a_seg == nullptr) { goto UPDATE_END; }

    a_size = EnStagingTernary(GetNColor_(&a_seg->n) == ref_color,
                              a_seg->ref.size, a_seg->dat.size);

    if (a_size == 0) {
        EraseSeg_(cntr, a_seg);
    } else {
        bin_tree::SetSize(&a_seg->n, a_size);
    }

    if (ret_n != nullptr) {
    } else if (ret_seg_idx < a_size) {
        ret_n = &a_seg->n;
    } else {
        ret_seg_idx -= a_size;
    }

    if (b_seg == nullptr) { goto UPDATE_END; }

    b_size = EnStagingTernary(GetNColor_(&b_seg->n) == ref_color,
                              b_seg->ref.size, b_seg->dat.size);

    if (b_size == 0) {
        EraseSeg_(cntr, b_seg);
    } else {
        bin_tree::SetSize(&b_seg->n, b_size);
    }

    if (ret_n != nullptr) {
    } else if (ret_seg_idx < b_size) {
        ret_n = &b_seg->n;
    } else {
        ret_seg_idx -= b_size;
    }

    if (c_seg == nullptr) { goto UPDATE_END; }

    c_size = EnStagingTernary(GetNColor_(&c_seg->n) == ref_color,
                              c_seg->ref.size, c_seg->dat.size);

    if (c_size == 0) {
        EraseSeg_(cntr, c_seg);
    } else {
        bin_tree::SetSize(&c_seg->n, c_size);
    }

    if (ret_n != nullptr) {
    } else if (ret_seg_idx < c_size) {
        ret_n = &c_seg->n;
    } else {
        ret_seg_idx -= c_size;
    }

    if (d_seg == nullptr) { goto UPDATE_END; }

    d_size = EnStagingTernary(GetNColor_(&d_seg->n) == ref_color,
                              d_seg->ref.size, d_seg->dat.size);

    if (d_size == 0) {
        EraseSeg_(cntr, d_seg);
    } else {
        bin_tree::SetSize(&d_seg->n, d_size);
    }

    if (ret_n == nullptr) { ret_n = &d_seg->n; }
}

UPDATE_END:

    pos_cursor->n = ret_n;
    pos_cursor->seg_idx = ret_seg_idx;

    if (cntr->rb == pos_cursor->n
#if EnStaging
        || GetNColor_(pos_cursor->n) == ref_color
#endif
    ) {
        pos_cursor->ref = nullptr;
        return;
    }

    Seg* seg{ NToSeg_(pos_cursor->n) };

    m_ca.data = seg->dat.data;
    m_ca.offset = seg->dat.offset;
    m_ca.size = seg->dat.size;

    pos_cursor->ref = circular_array::ops::Access(&m_ca, pos_cursor->seg_idx,
                                                  nullptr, nullptr);
}

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    detail::EraseAllSegs_(cntr, &cntr->seg_allocator, &cntr->data_allocator);

    detail::InitTree_(cntr);
}

template <CntrTplParamList>
void Reset(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    detail::EraseAllSegs_(cntr, &cntr->seg_allocator, &cntr->data_allocator);

    detail::InitTree_(cntr);

#if EnStaging
    detail::RefOrigin_(cntr);
#endif
}

template <CntrTplParamList>
void Copy(Cntr<CntrTplArgList>* cntr, void* src_sv_) {
    ZETA_Core_WhenEnableDebug(CheckCntr(cntr));

    auto src_sv{ static_cast<Cntr<CntrTplArgList>*>(src_sv_) };
    ZETA_Core_WhenEnableDebug(CheckCntr(src_sv));

    if (cntr == src_sv) { return; }

    size_t stride{ cntr->stride };
    size_t seg_capacity{ cntr->seg_capacity };
    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    size_t src_stride{ src_sv->stride };
    size_t src_seg_capacity{ cntr->seg_capacity };
    void* src_lb{ src_sv->lb };
    void* src_rb{ src_sv->rb };

    bool same_data_size{ stride * seg_capacity ==
                         src_stride * src_seg_capacity };

    PoolAllocator segs;
    PoolAllocator datas;

    PoolAllocator::Init(&segs);
    PoolAllocator::Init(&datas);

    if (same_data_size) {
        EraseAllSegs_(cntr, segs, datas);
    } else {
        EraseAllSegs_(cntr, segs, &cntr->data_allocator);
    }

#if EnStaging
    cntr->origin = src_sv->origin;
#endif

    cntr->seg_capacity = seg_capacity = src_seg_capacity;

    struct {
        TreeNode* p_n;
        TreeNode* src_n;
        int dir;
    } buffer[ZETA_Core_ullong_width * 3];

    size_t buffer_i{ 0 };

    buffer[++buffer_i] = { &cntr->root, src_sv->root };

    CircularArray ca{
        .data = {},
        .width = cntr->width,
        .stride = cntr->stride,
        .offset = {},
        .size = seg_capacity,
        .capacity = seg_capacity,
    };

    while (0 < buffer_i) {
        auto [p_n, src_n, dir]{ buffer[--buffer_i] };

        TreeNode* src_nl{ src_n->GetLPtr() };
        TreeNode* src_nr{ src_n->GetRPtr() };

        TreeNode* n{ nullptr };

        if (src_lb == src_n) {
            n = lb;
            n->Init();
            bin_tree::SetSize(n, 1);
        } else if (src_rb == src_n) {
            n = rb;
            n->Init();
            bin_tree::SetSize(n, 1);
        } else {
            Seg* seg{ AllocateSeg_(cntr, segs) };
            n = &seg->n;

            n->Init();

            Seg* src_seg{ NToSeg_(src_n) };

#if EnStaging
            unsigned color{ GetNColor_(src_n) };
            DirectlySetNColor_(n, color);
#endif

#if EnStaging
            if (color == ref_color) {
                seg->ref.beg = src_seg->ref.beg;
                seg->ref.size = src_seg->ref.size;

                bin_tree::SetSize(n, seg->ref.size);
            } else
#endif
            {
                seg->dat.data = AllocateData_(cntr, datas);
                seg->dat.offset = 0;
                seg->dat.size = src_seg->dat.size;

                ca.data = src_seg->dat.data;
                ca.offset = src_seg->dat.offset;

                circular_array::ops::IdxRead(&ca, 0, ca.size, seg->dat.data,
                                             cntr->stride, nullptr);

                bin_tree::SetSize(n, ca.size);
            }
        }

        n->SetPColor(src_n->GetPColor());

        if (p_n == nullptr) {
            cntr->root = n;
        } else {
            switch (dir) {
            case 0: bin_tree::AttatchL(p_n, n); break;
            case 1: bin_tree::AttatchR(p_n, n); break;
            }
        }

        if (src_nl != nullptr) { buffer[buffer_i++] = { n, src_nl, 0 }; }
        if (src_nr != nullptr) { buffer[buffer_i++] = { n, src_nr, 1 }; }
    }

    for (;;) {
        void* seg{ PoolAllocator::Allocate(&segs, 1) };
        if (seg == nullptr) { break; }
        SegAllocatorLike::Deallocate(&cntr->seg_allocator, seg);
    }

    for (;;) {
        void* data{ PoolAllocator::Allocate(&datas, 1) };
        if (data == nullptr) { break; }
        DataAllocatorLike::Deallocate(&cntr->data_allocator, data);
    }
}

#if EnStaging

template <CntrTplParamList>
void Collapse(Cntr<CntrTplArgList>* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    Cntr<CntrTplArgList>* origin_cntr{ cntr->origin.inst };

    cntr->origin = origin_cntr->origin;

    size_t stride{ cntr->stride };

    TreeNode* n{ bin_tree::StepR(cntr->lb) };

    Cursor origin_cursor;
    Access(origin_cntr, 0, &origin_cursor, nullptr);

    CircularArray origin_ca{
        .width = cntr->width,
        .stride = stride,
        .capacity = origin_cntr->seg_capacity,
    };

    while (n != cntr->rb) {
        Seg* seg{ NToSeg_(n) };

        if (GetNColor_(n) == dat_color) {
            n = bin_tree::StepR(n);
            continue;
        }

        CursorAdvanceR(origin_cntr, &origin_cursor,
                       seg->ref.beg - origin_cursor.idx);

        TreeNode* origin_n{ origin_cursor.n };
        size_t origin_seg_idx{ origin_cursor.seg_idx };

        Seg* origin_seg{ NToSeg_(origin_n) };

        unsigned origin_seg_color{ GetNColor_(&origin_seg->n) };

        size_t origin_seg_size{ origin_seg_color == ref_color
                                    ? origin_seg->ref.size
                                    : origin_seg->dat.size };

        size_t origin_res_size{ origin_seg_size - origin_seg_idx };

        if (origin_seg_color == ref_color) {
            if (seg->ref.size <= origin_res_size) {
                seg->ref.beg = origin_seg->ref.beg + origin_seg_idx;
                n = bin_tree::StepR(n);
                continue;
            }

            Seg* new_ref_seg{ AllocateRefSeg_(cntr) };

            new_ref_seg->ref.beg = origin_seg->ref.beg + origin_seg_idx;
            new_ref_seg->ref.size = origin_res_size;

            bin_tree::SetSize(&new_ref_seg->n, new_ref_seg->ref.size);

            seg->ref.beg += origin_res_size;
            seg->ref.size -= origin_res_size;

            bin_tree::SetSize(seg, seg->ref.size);

            cntr->root = rbtree::InsertL(n, &new_ref_seg->n);

            continue;
        }

        size_t cur_cnt{ GetAvgCnt_(Min(seg->ref.size, origin_res_size)) };

        if (cur_cnt == seg->ref.size) {
            SetNColor_(n, dat_color);
            seg->dat.data = AllocateData_(cntr, nullptr);
            seg->dat.offset = 0;
            seg->dat.size = cur_cnt;
        } else {
            Seg* new_dat_seg{ AllocateDatSeg_(cntr) };

            new_dat_seg->dat.size = cur_cnt;

            bin_tree::SetSize(&new_dat_seg->n, cur_cnt);

            seg->ref.beg += origin_res_size;
            seg->ref.size -= origin_res_size;

            bin_tree::SetSize(n, seg->ref.size);

            cntr->root = rbtree::InsertL(n, &new_dat_seg->n);

            n = &new_dat_seg->n;
            seg = new_dat_seg;
        }

        origin_ca.data = origin_seg->dat.data;
        origin_ca.offset = origin_seg->dat.offset;
        origin_ca.size = origin_seg_size;

        circular_array::ops::IdxRead(&origin_ca, origin_seg_idx, cur_cnt,
                                     seg->dat.data, stride, nullptr);

        n = bin_tree::StepR(n);
    }
}

namespace detail {

struct WBSeg {
    size_t beg;
    size_t size;

    size_t dst_idx;
    size_t acc_ref;

    void* data;
    unsigned short offset;
};

struct OffsetCntNode {
    generic_hash_table::Node ghtn;

    size_t offset;
    size_t cnt;
};

struct OffsetCntNodeHash {
    size_t operator()(generic_hash_table::Node const* ghtn,
                      unsigned long long salt) const {
        OffsetCntNode* node{ ZETA_Core_MemberToStruct(OffsetCntNode, ghtn,
                                                      ghtn) };

        return ULLHash(node->offset, salt);
    }
};

struct OffsetCntNodeCompare {
    int operator()(generic_hash_table::Node const* a_ghtn,
                   generic_hash_table::Node const* b_ghtn) const {
        return compare::Compare(
            ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, a_ghtn)->offset,
            ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, b_ghtn)->offset);
    }
};

template <CntrTplParamList>
unsigned long long OffsetHash_  // NOLINT(misc-use-internal-linkage)
    (size_t offset, unsigned long long salt) {
    return ULLHash(offset, salt);
}

template <CntrTplParamList>
int OffsetOffsetCntNodeCompare_  // NOLINT(misc-use-internal-linkage)
    (size_t offset, generic_hash_table::Node const* ghtn) {
    return compare::Compare(
        offset, ZETA_Core_MemberToStruct(OffsetCntNode, ghtn, ghtn)->offset);
}

using OffsetCntGenericHashTable =
    generic_hash_table::Cntr<OffsetCntNodeHash, OffsetCntNodeCompare,
                             allocator::Ref<value_wrapper::FalseType>>;

template <CntrTplParamList>
size_t RecordOffset_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, TreeNode* n, size_t dst_idx,
     OffsetCntGenericHashTable* ght) {
    while (n != nullptr) {
        TreeNode* nl{ n->GetLPtr() };
        TreeNode* nr{ n->GetRPtr() };

        if (nl != nullptr) { dst_idx = RecordOffset_(cntr, nl, dst_idx, ght); }

        if (cntr->lb == n || cntr->rb == n) {
            n = nr;
            continue;
        }

        Seg* seg{ NToSeg_(n) };

        if (GetNColor_(n) == dat_color) {
            dst_idx += seg->dat.size;
            n = nr;
            continue;
        }

        size_t offset{ seg->ref.beg - dst_idx };

        generic_hash_table::Node* ghtn{ generic_hash_table::ops::Find(
            ght, &offset, OffsetHash_, OffsetOffsetCntNodeCompare_) };

        OffsetCntNode* offset_cnt_node;

        if (ghtn == nullptr) {
            offset_cnt_node =
                static_cast<OffsetCntNode*>(allocator::SafeAllocate(
                    &allocator::weak_lifo_allocator, alignof(OffsetCntNode),
                    sizeof(OffsetCntNode)));

            ghtn = &offset_cnt_node->ghtn;

            offset_cnt_node->offset = offset;
            offset_cnt_node->cnt = 0;

            ghtn->Init();

            generic_hash_table::ops::Insert(ght, ghtn);
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

template <CntrTplParamList>
Pair<size_t, size_t> ToWBSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, TreeNode* n, WBSeg* dst) {
    Pair<size_t, size_t> ret{ 0, 0 };

    while (n != nullptr) {
        TreeNode* nl{ n->GetLPtr() };
        TreeNode* nr{ n->GetRPtr() };

        auto l_ret{ ToWBSeg_(cntr, nl, dst) };

        dst += l_ret.first + l_ret.second;

        ret.first += l_ret.first;
        ret.second += l_ret.second;

        if (cntr->lb == n || cntr->rb == n) {
            n = nr;
            continue;
        }

        Seg* seg{ NToSeg_(n) };

        if (GetNColor_(n) == ref_color) {
            ++ret.first;

            dst->beg = seg->ref.beg;
            dst->size = seg->ref.size;
        } else {
            ++ret.second;

            dst->beg = ZETA_Core_size_max;
            dst->size = seg->dat.size;
            dst->data = seg->dat.data;
            dst->offset = seg->dat.offset;
        }

        SegAllocatorLike::Deallocate(&cntr->seg_allocator, seg);

        WBSeg* prv{ dst - 1 };

        dst->dst_idx = prv->dst_idx + prv->size;

        dst->acc_ref = prv->beg == ZETA_Core_size_max ? prv->acc_ref
                                                      : prv->beg + prv->size;

        ++dst;

        n = nr;
    }

    return ret;
}

template <CntrTplParamList>
void WriteWBSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, CircularArray* ca, WBSeg* wb_segs,
     size_t wb_segs_cnt, size_t dst_offset, size_t ref_offset) {
    auto* origin{ &cntr->origin };
    DataAllocatorLike* data_allocator{ &cntr->data_allocator };

    if (wb_segs_cnt == 0) { return; }

    struct {
        size_t wb_seg_lb;
        size_t wb_seg_rb;
    } buffer[ZETA_Core_ullong_width * 2];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = { 0, wb_segs_cnt };

    while (0 < buffer_i) {
        auto [wb_seg_lb, wb_seg_rb]{ buffer[--buffer_i] };

        if (wb_seg_lb + 1 < wb_seg_rb) {
            size_t wb_seg_mb{ (wb_seg_lb + wb_seg_rb) / 2 };

            WBSeg* mid_wb_seg{ wb_segs + wb_seg_mb };

            size_t dst_pivot{ dst_offset + mid_wb_seg->dst_idx };
            size_t src_pivot{ ref_offset + mid_wb_seg->acc_ref };

            if (dst_pivot <= src_pivot) {
                buffer[buffer_i++] = { wb_seg_mb, wb_seg_rb };
                buffer[buffer_i++] = { wb_seg_lb, wb_seg_mb };
            } else {
                buffer[buffer_i++] = { wb_seg_lb, wb_seg_mb };
                buffer[buffer_i++] = { wb_seg_mb, wb_seg_rb };
            }

            continue;
        }

        if (wb_segs->beg == ZETA_Core_size_max) {
            ca->data = wb_segs->data;
            ca->offset = wb_segs->offset;
            ca->size = wb_segs->size;

            seq_cntr::RangeAssign(origin, &ca, dst_offset + wb_segs->dst_idx, 0,
                                  wb_segs->size);

            DataAllocatorLike::Deallocate(data_allocator, ca->data);
        } else {
            size_t cur_dst_idx{ dst_offset + wb_segs->dst_idx };
            size_t cur_ref_beg{ ref_offset + wb_segs->beg };

            if (cur_dst_idx != cur_ref_beg) {
                seq_cntr::RangeAssign(origin, origin, cur_dst_idx, cur_ref_beg,
                                      wb_segs->size);
            }
        }
    }
}

template <CntrTplParamList>
void WriteBack_LR_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, int write_back_strategy,
     unsigned long long cost_coeff_read, unsigned long long cost_coeff_write,
     unsigned long long cost_coeff_insert,
     unsigned long long cost_coeff_erase) {
    size_t stride{ cntr->stride };

    size_t size{ GetSize(cntr) };

    auto* origin{ &cntr->origin };

    size_t origin_size{ Origin::GetSize(origin) };

    size_t del_l_cnt;
    size_t del_r_cnt;

    switch (write_back_strategy) {
    case WriteBackStrategy::L:
        del_l_cnt = size - origin_size;
        del_r_cnt = 0;
        break;

    case WriteBackStrategy::R:
        del_l_cnt = 0;
        del_r_cnt = size - origin_size;
        break;

    case WriteBackStrategy::LR: {
        OffsetCntGenericHashTable ght;
        ght.table_node_alctr = allocator::weak_lifo_allocator;

        generic_hash_table::ops::Init(&ght);

        RecordOffset_(cntr, cntr->root, 0, &ght);

        unsigned long long best_cost{ ZETA_Core_ullong_max };

        del_l_cnt = 0;
        del_r_cnt = size - origin_size;

        for (;;) {
            generic_hash_table::Node* ghtn{ generic_hash_table::ops::ExtractAny(
                &ght) };

            if (ghtn == nullptr) { break; }

            auto* offset_cnt_node{ ZETA_Core_MemberToStruct(OffsetCntNode, ghtn,
                                                            ghtn) };

            size_t cur_offset{ offset_cnt_node->offset };

            size_t cur_del_l_cnt{ 0 };
            size_t cur_del_r_cnt{ size - origin_size };

            if (cur_offset <= ZETA_Core_size_max / 2) {
                cur_del_l_cnt -= cur_offset;
                cur_del_r_cnt += cur_offset;
            } else {
                cur_del_l_cnt += cur_offset;
                cur_del_r_cnt -= cur_offset;
            }

            unsigned long long cur_cost{ -(
                (cost_coeff_read + cost_coeff_write) * offset_cnt_node->cnt) };

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

            allocator::RefView<value_wrapper::FalseType>::Deallocate(
                reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
                    &allocator::weak_lifo_allocator),
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

    if (0 < push_l_cnt) { SeqCntrPushL(origin, push_l_cnt, nullptr); }

    if (0 < push_r_cnt) { SeqCntrPushR(origin, push_r_cnt, nullptr); }

    size_t segs_cnt{ bin_tree::Count(cntr->root) - 2 };

    auto* wb_segs{ static_cast<WBSeg*>(allocator::SafeAllocate(
                       &allocator::weak_lifo_allocator, alignof(WBSeg),
                       sizeof(WBSeg) * (segs_cnt + 1))) +
                   1 };

    WBSeg* lb_wb_seg{ wb_segs - 1 };

    lb_wb_seg->beg = 0;
    lb_wb_seg->size = 0;

    lb_wb_seg->dst_idx = pop_l_cnt;
    lb_wb_seg->acc_ref = pop_l_cnt;

    ToWBSeg_(cntr, cntr->root, wb_segs);

    CircularArray ca{
        .data = {},
        .width = origin->width,
        .stride = stride,
        .offset = 0,
        .size = 0,
        .capacity = cntr->seg_capacity,
    };

    WriteWBSeg_(cntr, &ca, wb_segs, segs_cnt, 0, push_l_cnt);

    allocator::RefView<value_wrapper::FalseType>::Deallocate(
        reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
            &allocator::weak_lifo_allocator),
        lb_wb_seg);

    if (0 < pop_l_cnt) { SeqCntrPopL(origin, pop_l_cnt); }

    if (0 < pop_r_cnt) { SeqCntrPopR(origin, pop_r_cnt); }

    InitTree_(cntr);
    RefOrigin_(cntr);
}

template <CntrTplParamList>
void WriteBack_Random_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, unsigned long long cost_coeff_read,
     unsigned long long cost_coeff_write, unsigned long long cost_coeff_insert,
     unsigned long long cost_coeff_erase) {
    auto* origin{ &cntr->origin };
    size_t origin_size{ SeqCntrGetSize(origin) };

    size_t stride{ cntr->stride };
    size_t size{ GetSize(cntr) };

    size_t segs_cnt{ bin_tree::Count(cntr->root) - 2 };

    auto* wb_segs{ static_cast<WBSeg*>(allocator::SafeAllocate(
                       &allocator::weak_lifo_allocator, alignof(WBSeg),
                       sizeof(WBSeg) * (segs_cnt + 2))) +
                   1 };

    WBSeg* lb_wb_seg{ wb_segs - 1 };

    lb_wb_seg->beg = 0;
    lb_wb_seg->size = 0;

    lb_wb_seg->dst_idx = 0;
    lb_wb_seg->acc_ref = 0;

    auto to_wb_seg_ret{ ToWBSeg_(cntr, cntr->root, wb_segs) };

    {
        size_t check_ref_segs_cnt{ 0 };
        size_t check_dat_segs_cnt{ 0 };

        for (size_t i{ 0 }; i < segs_cnt; ++i) {
            if (wb_segs[i].beg == ZETA_Core_size_max) {
                ++check_dat_segs_cnt;
            } else {
                ++check_ref_segs_cnt;
            }
        }

        ZETA_Core_SanitizeAssert(to_wb_seg_ret.first == check_ref_segs_cnt);
        ZETA_Core_SanitizeAssert(to_wb_seg_ret.second == check_dat_segs_cnt);
    }

    WBSeg* rb_wb_seg{ wb_segs + segs_cnt };

    rb_wb_seg->beg = origin_size;
    rb_wb_seg->size = 0;

    rb_wb_seg->dst_idx = size;
    rb_wb_seg->acc_ref = (rb_wb_seg - 1)->acc_ref;

    size_t ref_segs_cnt = to_wb_seg_ret.ref_segs_cnt;

    CircularArray ca{
        .data = {},
        .width = origin->width,
        .stride = stride,
        .offset = 0,
        .size = 0,
        .capacity = cntr->seg_capacity,
    };

    if (ref_segs_cnt == 0) {
        if (origin_size < size) {
            SeqCntrPopR(origin, size - origin_size);
        } else if (size < origin_size) {
            SeqCntrPushR(origin, origin_size - size, nullptr);
        }

        WriteWBSeg_(cntr, &ca, wb_segs, segs_cnt, 0, 0);

        allocator::RefView<value_wrapper::FalseType>::Deallocate(
            reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
                &allocator::weak_lifo_allocator),
            wb_segs - 1);

        InitTree_(cntr);
        RefOrigin_(cntr);

        return;
    }

    unsigned long long cost_coeff_read_write{ cost_coeff_read +
                                              cost_coeff_write };

    auto* ref_wb_segs{ static_cast<WBSeg**>(allocator::SafeAllocate(
                           &allocator::weak_lifo_allocator, alignof(WBSeg*),
                           sizeof(WBSeg*) * (ref_segs_cnt + 2))) +
                       1 };

    ref_wb_segs[-1] = lb_wb_seg;
    ref_wb_segs[ref_segs_cnt] = rb_wb_seg;

    for (size_t i{ 0 }, j{ 0 }; j < ref_segs_cnt; ++i) {
        ZETA_Core_SanitizeAssert(j < ref_segs_cnt);

        WBSeg* wb_seg{ wb_segs + i };

        if (wb_seg->beg != ZETA_Core_size_max) { ref_wb_segs[j++] = wb_seg; }
    }

    auto* acc_arr{ static_cast<size_t*>(allocator::SafeAllocate(
        &allocator::weak_lifo_allocator, alignof(size_t),
        sizeof(size_t) * (ref_segs_cnt + 2))) };

    auto* acc_brr{ static_cast<unsigned long long*>(allocator::SafeAllocate(
        &allocator::weak_lifo_allocator, alignof(unsigned long long),
        sizeof(unsigned long long) * (ref_segs_cnt + 1))) };

    acc_arr[0] = 0;
    acc_brr[0] = 0;

    size_t prv_ref_end{ 0 };
    size_t prv_dst_end{ 0 };

    for (size_t i{ 1 }; i <= ref_segs_cnt; ++i) {
        WBSeg* cur_ref_seg{ ref_wb_segs[i - 1] };

        size_t cur_beg{ cur_ref_seg->beg };
        size_t cur_size{ cur_ref_seg->size };
        size_t cur_dst_idx{ cur_ref_seg->dst_idx };

        acc_arr[i] = acc_arr[i - 1] + (cur_dst_idx - prv_dst_end) -
                     (cur_beg - prv_ref_end);
        acc_brr[i] = acc_brr[i - 1] + cost_coeff_read_write * cur_size;

        prv_ref_end = cur_beg + cur_size;
        prv_dst_end = cur_dst_idx + cur_size;
    }

    acc_arr[ref_segs_cnt + 1] = acc_arr[ref_segs_cnt] + (size - prv_dst_end) -
                                (origin_size - prv_ref_end);

#if ZETA_Core_EnableDebug
    auto* dp_best_cost{ static_cast<unsigned long long*>(
        allocator::SafeAllocate(
            &allocator::weak_lifo_allocator, alignof(unsigned long long),
            sizeof(unsigned long long) * (ref_segs_cnt + 2))) };

    dp_best_cost[0] = 0;
#endif

    auto* dp_cost{ static_cast<unsigned long long*>(allocator::SafeAllocate(
        &allocator::weak_lifo_allocator, alignof(unsigned long long),
        sizeof(unsigned long long) * (ref_segs_cnt + 2))) };

    auto* dp_prv{ static_cast<size_t*>(allocator::SafeAllocate(
        &allocator::weak_lifo_allocator, alignof(size_t),
        sizeof(size_t) * (ref_segs_cnt + 2))) };

    dp_cost[0] = 0;
    dp_prv[0] = ZETA_Core_size_max;

#if ZETA_Core_EnableDebug
    for (size_t i{ 1 }; i <= ref_segs_cnt + 1; ++i) {
        unsigned long long ans_cost{ ZETA_Core_ullong_max };

        for (size_t j{ i }, j_end{ 0 }; j_end < j--;) {
            size_t sum_arr{ acc_arr[i] - acc_arr[j] };

            unsigned long long cur_cost{ dp_best_cost[j] +
                                         (sum_arr <= ZETA_Core_size_max / 2
                                              ? cost_coeff_erase * sum_arr
                                              : cost_coeff_insert * -sum_arr) +
                                         acc_brr[i - 1] - acc_brr[j] };

            ans_cost = Min(ans_cost, cur_cost);
        }

        dp_best_cost[i] = ans_cost;
    }
#endif

    for (size_t i{ 1 }; i <= ref_segs_cnt + 1; ++i) {
        unsigned long long ans_cost{ ZETA_Core_ullong_max };
        unsigned long long ans_prv{ 0 };

        for (size_t j{ i }, j_end{ Max(8ULL, i) - 8 }; j_end < j--;) {
            size_t sum_arr{ acc_arr[i] - acc_arr[j] };

            unsigned long long cur_cost{ dp_cost[j] +
                                         (sum_arr <= ZETA_Core_size_max / 2
                                              ? cost_coeff_erase * sum_arr
                                              : cost_coeff_insert * -sum_arr) +
                                         acc_brr[i - 1] - acc_brr[j] };

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
        unsigned long long best_cost{ dp_best_cost[ref_segs_cnt + 1] };
        unsigned long long better_cost{ dp_cost[ref_segs_cnt + 1] };

        ZETA_Core_DebugAssert(best_cost <= 512 || better_cost <= best_cost * 2);
    }
#endif

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    unsigned long long check_cost{ 0 };

    for (size_t cur_end{ ref_segs_cnt + 1 }; 0 < cur_end;) {
        size_t cur_beg{ dp_prv[cur_end] };

        WBSeg* beg_wb_seg{ (ref_wb_segs - 1)[cur_beg] };
        WBSeg* end_wb_seg{ (ref_wb_segs - 1)[cur_end] };

        size_t src_beg{ beg_wb_seg->beg + beg_wb_seg->size };
        size_t src_end{ end_wb_seg->beg };

        size_t dst_beg{ beg_wb_seg->dst_idx + beg_wb_seg->size };
        size_t dst_end{ end_wb_seg->dst_idx };

        size_t dst_size{ dst_end - dst_beg };
        size_t src_size{ src_end - src_beg };

        {
            WBSeg* segs_{ beg_wb_seg + 1 };
            size_t segs_cnt_{ static_cast<size_t>(end_wb_seg - beg_wb_seg) -
                              1 };

            size_t check_dst_size{ 0 };

            for (size_t i{ 0 }; i < segs_cnt_; ++i) {
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
            size_t diff_size{ dst_size - src_size };

            SeqCntrAccess(origin, src_end, origin_cursor, nullptr);
            SeqCntrInsert(origin, origin_cursor, diff_size);

            src_end += diff_size;
            src_size += diff_size;
        }

        WriteWBSeg_(cntr, &ca, beg_wb_seg + 1, end_wb_seg - beg_wb_seg - 1,
                    src_beg - dst_beg, 0);

        if (dst_size < src_size) {
            size_t diff_size{ src_size - dst_size };

            SeqCntrAccess(origin, src_end - diff_size, origin_cursor, nullptr);

            SeqCntrErase(origin, origin_cursor, diff_size);
        }

        cur_end = cur_beg;
    }

    ZETA_Core_SanitizeAssert(check_cost == dp_cost[ref_segs_cnt + 1]);

    allocator::RefView<value_wrapper::FalseType>::Deallocate(
        reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
            &allocator::weak_lifo_allocator),
        wb_segs - 1);
    allocator::RefView<value_wrapper::FalseType>::Deallocate(
        reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
            &allocator::weak_lifo_allocator),
        static_cast<void*>(ref_wb_segs - 1));

    allocator::RefView<value_wrapper::FalseType>::Deallocate(
        reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
            &allocator::weak_lifo_allocator),
        acc_arr);
    allocator::RefView<value_wrapper::FalseType>::Deallocate(
        reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
            &allocator::weak_lifo_allocator),
        acc_brr);

#if ZETA_Core_EnableDebug
    allocator::RefView<value_wrapper::FalseType>::Deallocate(
        reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
            &allocator::weak_lifo_allocator),
        dp_best_cost);
#endif

    allocator::RefView<value_wrapper::FalseType>::Deallocate(
        reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
            &allocator::weak_lifo_allocator),
        dp_cost);
    allocator::RefView<value_wrapper::FalseType>::Deallocate(
        reinterpret_cast<allocator::RefView<value_wrapper::FalseType>*>(
            &allocator::weak_lifo_allocator),
        dp_prv);

    InitTree_(cntr);
    RefOrigin_(cntr);
}

}  // namespace detail

template <CntrTplParamList>
void WriteBack(Cntr<CntrTplArgList>* cntr, int write_back_strategy,
               unsigned long long cost_coeff_read,
               unsigned long long cost_coeff_write,
               unsigned long long cost_coeff_insert,
               unsigned long long cost_coeff_erase) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    auto* origin{ &cntr->origin };

    ZETA_Core_DebugAssert(write_back_strategy == WriteBackStrategy::L ||    //
                          write_back_strategy == WriteBackStrategy::R ||    //
                          write_back_strategy == WriteBackStrategy::LR ||   //
                          write_back_strategy == WriteBackStrategy::Random  //
    );

    if (GetSize(cntr) == 0) {
        SeqCntrEraseAll(origin);
        return;
    }

    if (write_back_strategy != WriteBackStrategy::Random) {
        WriteBack_LR_(cntr, write_back_strategy, cost_coeff_read,
                      cost_coeff_write, cost_coeff_insert, cost_coeff_erase);
    } else {
        WriteBack_Random_(cntr, cost_coeff_read, cost_coeff_write,
                          cost_coeff_insert, cost_coeff_erase);
    }
}

#endif

template <CntrTplParamList>
void CopyCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor,
                Cursor const* src_cursor) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, src_cursor));
    ZETA_Core_DebugAssert(CheckCursor(cntr, dst_cursor));

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_a) == GetCursorIdx(cntr, cursor_b);
}

template <CntrTplParamList>
int CompareCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                  Cursor const* cursor_b) {
    return compare::Compare(GetCursorIdx(cntr, cursor_a) + 1,
                            GetCursorIdx(cntr, cursor_b) + 1);
}

template <CntrTplParamList>
size_t GetCursorDist(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                     Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_b) - GetCursorIdx(cntr, cursor_a);
}

template <CntrTplParamList>
size_t GetCursorIdx(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor));

    return cursor->idx;
}

template <CntrTplParamList>
void CursorStepL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor) {
    CursorAdvanceL(cntr, cursor, 1);
}

template <CntrTplParamList>
void CursorStepR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor) {
    CursorAdvanceR(cntr, cursor, 1);
}

template <CntrTplParamList>
void CursorAdvanceL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor,
                    size_t step) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor));

    if (step == 0) { return; }

#if EnStaging
    size_t width{ cntr->origin.width };
#else
    size_t width{ cntr->width };
#endif

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    TreeNode* n{ cursor->n };

    size_t n_size;

    {
        Seg* seg{ NToSeg_(n) };

        n_size = cntr->lb == n || cntr->rb == n
                     ? 1
                     : EnStagingTernary(GetNColor_(n) == ref_color,
                                        seg->ref.size, seg->dat.size);
    }

    auto [dst_n, dst_seg_idx]{ bin_tree::AdvanceL(
        n, n_size - 1 - cursor->seg_idx + step) };

    ZETA_Core_DebugAssert(dst_n != nullptr);

    cursor->idx -= step;
    cursor->n = dst_n;

    if (cntr->lb == dst_n) {
        cursor->seg_idx = 0;
        cursor->ref = nullptr;
        return;
    }

    Seg* dst_seg{ NToSeg_(dst_n) };

    cursor->seg_idx = EnStagingTernary(GetNColor_(dst_n) == ref_color,
                                       dst_seg->ref.size, dst_seg->dat.size) -
                      1 - dst_seg_idx;

#if EnStaging
    if (GetNColor_(dst_n) == ref_color) {
        cursor->ref = nullptr;
        return;
    }
#endif

    CircularArray ca{
        .data = dst_seg->dat.data,
        .width = width,
        .stride = cntr->stride,
        .offset = dst_seg->dat.offset,
        .size = dst_seg->dat.size,
        .capacity = cntr->seg_capacity,
    };

    cursor->ref =
        circular_array::ops::Access(&ca, cursor->seg_idx, nullptr, nullptr);
}

template <CntrTplParamList>
void CursorAdvanceR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor,
                    size_t step) {
    ZETA_Core_DebugAssert(CheckCursor(cntr, cursor));

    if (step == 0) { return; }

#if EnStaging
    size_t width{ cntr->origin.width };
#else
    size_t width{ cntr->width };
#endif

    size_t size{ GetSize(cntr) };

    ZETA_Core_DebugAssert(step <= size - cursor->idx);

    auto [dst_n,
          dst_seg_idx]{ bin_tree::AdvanceR(cursor->n, cursor->seg_idx + step) };

    ZETA_Core_DebugAssert(dst_n != nullptr);

    cursor->idx += step;
    cursor->n = dst_n;
    cursor->seg_idx = dst_seg_idx;

    if (cntr->rb == dst_n) {
        cursor->ref = nullptr;
        return;
    }

#if EnStaging
    if (GetNColor_(dst_n) == ref_color) {
        cursor->ref = nullptr;
        return;
    }
#endif

    Seg* dst_seg{ NToSeg_(dst_n) };

    CircularArray ca{
        .data = dst_seg->dat.data,
        .width = width,
        .stride = cntr->stride,
        .offset = dst_seg->dat.offset,
        .size = dst_seg->dat.size,
        .capacity = cntr->seg_capacity,
    };

    cursor->ref =
        circular_array::ops::Access(&ca, cursor->seg_idx, nullptr, nullptr);
}

// -----------------------------------------------------------------------------

template <CntrTplParamList>
void PrintState(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    PrintState_(cntr, cntr->root);
}

template <CntrTplParamList>
Stats GetStats(Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    TreeNode* root{ cntr->root };

    if (root == nullptr) { return { 0, 0, 0, 0 }; }

    return GetStats_(cntr, cntr->root);
}

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList> const* cntr, MemRecorder* dst_seg,
              MemRecorder* dst_data) {
    ZETA_Core_DebugAssert(CheckCntr(cntr));

    bin_tree::Sanitize(cntr->root);
    rbtree::Sanitize(nullptr, cntr->root);

#if ZETA_Core_EnableDebug
    Sanitize_(cntr, dst_seg, dst_data, cntr->root);
#else
    ZETA_Core_Unused(dst_seg);
    ZETA_Core_Unused(dst_data);
#endif
}

}  // namespace ops

}  // namespace zeta::core::NameSpace

#pragma pop_macro("Cntr")
#pragma pop_macro("EnStagingTernary")

// NOLINTEND(cppcoreguidelines-pro-type-union-access)
