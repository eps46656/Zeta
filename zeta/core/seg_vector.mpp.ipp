#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

#include <iostream>
#include <zeta/core/allocator.hpp>
#include <zeta/core/basic_bin_tree_node.ipp>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/compare.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/pool_allocator.hpp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/seg_utils.ipp>
#include <zeta/core/seg_vector.mpp.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

#if EnStaging

#pragma push_macro("Namespace")
#define Namespace staging_seg_vector

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList \
    typename OriginLike, typename SegAllocatorLike, typename DataAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList OriginLike, SegAllocatorLike, DataAllocatorLike

#pragma push_macro("EnStagingTernary")
#define EnStagingTernary(cond, x, y) ((cond) ? (x) : (y))

#else

#pragma push_macro("Namespace")
#define Namespace seg_vector

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename SegAllocatorLike, typename DataAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList SegAllocatorLike, DataAllocatorLike

#pragma push_macro("EnStagingTernary")
#define EnStagingTernary(cond, x, y) (y)

#endif

namespace zeta::core {

namespace Namespace::detail {

template <CntrTplParamList>
void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& cntr) {
#if EnStaging
    auto& origin{ meta::GetInstRef(cntr.origin) };
    seq_cntr::CheckContract(origin);
#endif

    size_t elem_size{ cntr.elem_size };

    ZETA_Core_DebugAssert(0 < elem_size);

#if EnStaging
    ZETA_Core_DebugAssert(elem_size == seq_cntr::GetElemSize(origin));
#endif

    size_t elem_stride{ cntr.elem_stride };
    ZETA_Core_DebugAssert(elem_size <= elem_stride);

    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    ZETA_Core_DebugAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugAssert(seg_elem_slot_cnt <= max_seg_elem_slot_cnt);
}

template <CntrTplParamList>
void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& cntr, Cursor const* cursor) {
    (CheckCntr_)(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    Cursor re_cursor;
    (Access)(cntr, cursor->idx, true, &re_cursor, nullptr);

    ZETA_Core_DebugAssert(*cursor == re_cursor);
}

#if EnStaging

inline unsigned GetNColor_  // NOLINT(misc-use-internal-linkage)
    (Node const* n) {
    unsigned color{ n->GetLColor() };
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);
    return color;
}

inline void DirectlySetNColor_  // NOLINT(misc-use-internal-linkage)
    (Node* n, unsigned color) {
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);

    n->SetLColor(color);
}

inline void SetNColor_  // NOLINT(misc-use-internal-linkage)
    (Node* n, unsigned color) {
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);
    ZETA_Core_DebugAssert(0 <= GetNColor_(n));

    (DirectlySetNColor_)(n, color);
}

#endif

inline size_t CalcDataSize_  // NOLINT(misc-use-internal-linkage)
    (size_t elem_size, size_t elem_stride, size_t seg_elem_slot_cnt) {
    return elem_stride * (seg_elem_slot_cnt - 1) + elem_size;
}

inline Seg* NToSeg_  // NOLINT(misc-use-internal-linkage)
    (Node* n) {
    return ZETA_Core_MemberToStruct(Seg, n, n);
}

template <
#if EnStaging
    typename Origin,
#endif
    typename SegAllocator, typename DataAllocator>
struct CntrWork_ {
    size_t elem_size;
    size_t elem_stride;
    size_t seg_elem_slot_cnt;
    size_t data_size;

    size_t elem_cnt;

#if EnStaging
    Origin& origin;
#endif

    SegAllocator& seg_alctr;
    DataAllocator& data_alctr;
};

template <typename CntrType>
auto MakeCntrWork_(CntrType& cntr) {
#if EnStaging
    auto& origin{ meta::GetInstRef(cntr.origin) };
#endif

    auto& seg_alctr{ meta::GetInstRef(cntr.seg_alctr) };
    auto& data_alctr{ meta::GetInstRef(cntr.data_alctr) };

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };

    size_t elem_cnt{ cntr.root->GetAccSize() - 2 };

    return CntrWork_<
#if EnStaging
        meta::RemoveRef<decltype(origin)>,
#endif
        meta::RemoveRef<decltype(seg_alctr)>,
        meta::RemoveRef<decltype(data_alctr)>>{
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .seg_elem_slot_cnt = seg_elem_slot_cnt,
        .data_size = (CalcDataSize_)(elem_size, elem_stride, seg_elem_slot_cnt),

        .elem_cnt = elem_cnt,

#if EnStaging
        .origin = origin,
#endif

        .seg_alctr = seg_alctr,
        .data_alctr = data_alctr,
    };
}

struct SegWork_ {
    bool is_null;

    bool is_dirty;

    Seg* seg;

#if EnStaging
    unsigned color;
#endif

#if EnStaging
    size_t ref_beg;
#endif

    CircularArray ca;

    size_t elem_vac;

    void SetBasics(size_t elem_size, size_t elem_stride,
                   size_t seg_elem_slot_cnt) {
        this->ca.elem_size = elem_size;
        this->ca.elem_stride = elem_stride;
        this->ca.slot_cnt = seg_elem_slot_cnt;
    }

    template <typename CntrWorkType>
    void SetBasics(CntrWorkType const& cntr_work) {
        this->SetBasics(cntr_work.elem_size, cntr_work.elem_stride,
                        cntr_work.seg_elem_slot_cnt);
    }

    void SetNull() {
        this->is_null = true;
        this->is_dirty = false;
        this->ca.elem_cnt = 0;
        this->elem_vac = 0;
    }

    void CopyFromSameBasics(SegWork_ const& other) {
        ZETA_Core_DebugAssert(this->ca.elem_size == other.ca.elem_size);
        ZETA_Core_DebugAssert(this->ca.elem_stride == other.ca.elem_stride);
        ZETA_Core_DebugAssert(this->ca.slot_cnt == other.ca.slot_cnt);

        this->is_null = other.is_null;
        this->is_dirty = other.is_dirty;
        this->seg = other.seg;

#if EnStaging
        this->color = other.color;

        if (this->color == ref_color) {
            this->ref_beg = other.ref_beg;
        } else
#endif

        {
            this->ca.data = other.ca.data;
            this->ca.rot = other.ca.rot;
        }

        this->ca.elem_cnt = other.ca.elem_cnt;
        this->elem_vac = other.elem_vac;
    }

    void Load(Seg* seg) {
        this->is_null = false;

        this->is_dirty = false;

        this->seg = seg;

#if EnStaging
        unsigned color{ GetNColor_(&seg->n) };
#endif

#if EnStaging
        this->color = color;

        if (color == ref_color) {
            this->ref_beg = seg->ref.beg;
            this->ca.elem_cnt = seg->ref.elem_cnt;
            this->elem_vac = ca.slot_cnt - compare_utils::BasicMin(
                                               seg->ref.elem_cnt, ca.slot_cnt);
        } else
#endif
        {
            this->ca.data = seg->dat.data;
            this->ca.elem_cnt = seg->dat.elem_cnt;
            this->ca.rot = seg->dat.rot;
            this->elem_vac = ca.slot_cnt - this->ca.elem_cnt;
        }
    }

    void Store(Seg* seg) const {
        ZETA_Core_DebugAssert(seg != nullptr);
        ZETA_Core_DebugAssert(!this->is_null);

        bin_tree::AddDiffSize(
            &seg->n,
            this->ca.elem_cnt -
                EnStagingTernary((GetNColor_)(&seg->n) == ref_color,
                                 seg->ref.elem_cnt, seg->dat.elem_cnt));

#if EnStaging
        (SetNColor_)(&seg->n, this->color);

        if (this->color == ref_color) {
            seg->ref.beg = this->ref_beg;
            seg->ref.elem_cnt = this->ca.elem_cnt;
        } else
#endif
        {
            seg->dat.data = this->ca.data;
            seg->dat.elem_cnt = static_cast<unsigned short>(this->ca.elem_cnt);
            seg->dat.rot = static_cast<unsigned short>(this->ca.rot);
        }
    }

    void Store() const { this->Store(this->seg); }

    template <typename DataAllocator>
    void TryDeallocateData(DataAllocator& data_alctr) {
        ZETA_Core_DebugAssert(!this->is_null);

#if EnStaging
        if (this->color == dat_color)
#endif
        {
            allocator::Deallocate(data_alctr, this->ca.data);
        }
    }
};

template <typename CntrWorkType>
void CheckCntrSegWorkMatched_(CntrWorkType& cntr_work,
                              SegWork_ const& seg_work) {
    ZETA_Core_DebugAssert(seg_work.ca.elem_size == cntr_work.elem_size);
    ZETA_Core_DebugAssert(seg_work.ca.elem_stride == cntr_work.elem_stride);
    ZETA_Core_DebugAssert(seg_work.ca.slot_cnt == cntr_work.seg_elem_slot_cnt);
}

template <typename SegAllocator>
Seg* AllocateSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator& seg_alctr) {
    Seg* seg{ static_cast<Seg*>(
        allocator::SafeAllocate(seg_alctr, alignof(Seg), sizeof(Seg))) };

    seg->n.Init(0);

    return seg;
}

template <typename DataAllocator>
void* AllocateData_  // NOLINT(misc-use-internal-linkage)
    (size_t data_size, DataAllocator& data_alctr) {
    return allocator::SafeAllocate(data_alctr, 1, data_size);
}

#if EnStaging

template <typename SegAllocator>
Seg* AllocateRefSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator& seg_alctr) {
    Seg* seg{ (AllocateSeg_)(seg_alctr) };

    (DirectlySetNColor_)(&seg->n, ref_color);

    seg->ref.beg = 0;
    seg->ref.elem_cnt = 0;

    return seg;
}

#endif

template <typename SegAllocator, typename DataAllocator>
Seg* AllocateDatSeg_  // NOLINT(misc-use-internal-linkage)
    (size_t data_size, SegAllocator& seg_alctr, DataAllocator& data_alctr) {
    Seg* seg{ (AllocateSeg_)(seg_alctr) };

#if EnStaging
    (DirectlySetNColor_)(&seg->n, dat_color);
#endif

    seg->dat.data = (AllocateData_)(data_size, data_alctr);
    seg->dat.elem_cnt = 0;
    seg->dat.rot = 0;

    return seg;
}

#if EnStaging

template <typename SegAllocator>
void DeallocateRefSeg_  // NOLINT(misc-use-internal-linkage)
    (Seg* seg, SegAllocator& seg_alctr) {
    ZETA_Core_DebugAssert((GetNColor_)(&seg->n) == ref_color);

    allocator::Deallocate(seg_alctr, seg);
}

#endif

template <typename SegAllocator, typename DataAllocator>
void DeallocateDatSeg_  // NOLINT(misc-use-internal-linkage)
    (Seg* seg, SegAllocator& seg_alctr, DataAllocator& data_alctr) {
#if EnStaging
    ZETA_Core_DebugAssert((GetNColor_)(&seg->n) == dat_color);
#endif

    allocator::Deallocate(data_alctr, seg->dat.data);
    allocator::Deallocate(seg_alctr, seg);
}

template <typename SegAllocator, typename DataAllocator>
void DeallocateSeg_  // NOLINT(misc-use-internal-linkage)
    (Seg* seg, SegAllocator& seg_alctr, DataAllocator& data_alctr) {
#if EnStaging
    unsigned color{ (GetNColor_)(&seg->n) };
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);
#endif

#if EnStaging
    if (color == ref_color) {
        (DeallocateRefSeg_)(seg, seg_alctr);
    } else
#endif
    {
        (DeallocateDatSeg_)(seg, seg_alctr, data_alctr);
    }
}

#if EnStaging

template <typename CntrWorkType>
void MaterializeRefSeg_(CntrWorkType& cntr_work, SegWork_& seg_work) {
    ZETA_Core_DebugAssert(!seg_work.is_null);
    ZETA_Core_DebugAssert(seg_work.color == ref_color);
    ZETA_Core_DebugAssert(seg_work.ca.elem_cnt <= cntr_work.seg_elem_slot_cnt);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

    void* data{ (AllocateData_)(cntr_work.data_size, cntr_work.data_alctr) };

    seq_cntr::Access(cntr_work.origin, seg_work.ref_beg, true, origin_cursor,
                     nullptr);
    seq_cntr::Read(cntr_work.origin, origin_cursor, seg_work.ca.elem_cnt,
                   seq_cntr::MemReader{
                       .data = data,
                       .elem_size = cntr_work.elem_size,
                       .elem_stride = cntr_work.elem_stride,
                   },
                   nullptr);

    seg_work.is_dirty = true;
    seg_work.color = dat_color;
    seg_work.ca.data = data;
    seg_work.ca.rot = 0;
}

template <typename CntrWorkType, typename Writer>
void AugMaterializeRefSeg_(CntrWorkType& cntr_work, SegWork_& seg_work,
                           size_t l_cnt, size_t ins_cnt, size_t r_cnt,
                           Writer&& writer) {
    ZETA_Core_DebugAssert(!seg_work.is_null);
    ZETA_Core_DebugAssert(seg_work.color == ref_color);
    ZETA_Core_DebugAssert(l_cnt + ins_cnt + r_cnt <= seg_work.ca.slot_cnt);
    ZETA_Core_DebugAssert(l_cnt + r_cnt <= seg_work.ca.elem_cnt);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

    void* data{ (AllocateData_)(cntr_work.data_size, cntr_work.data_alctr) };

    if (0 < l_cnt) {
        seq_cntr::Access(cntr_work.origin, seg_work.ref_beg, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(cntr_work.origin, origin_cursor, l_cnt,
                       seq_cntr::MemReader{
                           .data = data,
                           .elem_size = cntr_work.elem_size,
                           .elem_stride = cntr_work.elem_stride,
                       },
                       nullptr);
    }

    if (0 < ins_cnt) {
        writer(static_cast<char*>(data) + cntr_work.elem_stride * l_cnt,
               cntr_work.elem_stride, ins_cnt);
    }

    if (0 < r_cnt) {
        seq_cntr::Access(cntr_work.origin,
                         seg_work.ref_beg + seg_work.ca.elem_cnt - r_cnt, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(cntr_work.origin, origin_cursor, r_cnt,
                       seq_cntr::MemReader{
                           .data = static_cast<char*>(data) +
                                   cntr_work.elem_stride * (l_cnt + ins_cnt),
                           .elem_size = cntr_work.elem_size,
                           .elem_stride = cntr_work.elem_stride,
                       },
                       nullptr);
    }

    seg_work.is_dirty = true;
    seg_work.color = dat_color;
    seg_work.ca.data = data;
    seg_work.ca.elem_cnt = l_cnt + ins_cnt + r_cnt;
    seg_work.ca.rot = 0;
    seg_work.elem_vac = seg_work.ca.slot_cnt - seg_work.ca.elem_cnt;
}

#endif

template <typename CntrWorkType>
void SegShoveL_(CntrWorkType& cntr_work, SegWork_& l_seg_work,
                SegWork_& r_seg_work, size_t shove_cnt) {
    (CheckCntrSegWorkMatched_)(cntr_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(cntr_work, r_seg_work);

    ZETA_Core_DebugAssert(!l_seg_work.is_null);
    ZETA_Core_DebugAssert(!r_seg_work.is_null);

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= r_seg_work.ca.elem_cnt);
    ZETA_Core_DebugAssert(l_seg_work.ca.elem_cnt + shove_cnt <=
                          cntr_work.seg_elem_slot_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (l_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(cntr_work, l_seg_work);
    }
#endif

#if EnStaging
    if (r_seg_work.color == ref_color) {
        void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

        seq_cntr::Access(cntr_work.origin, r_seg_work.ref_beg, true,
                         origin_cursor, nullptr);

        circular_array::PushR(l_seg_work.ca, shove_cnt, seq_cntr::EmptyWriter{},
                              nullptr);

        circular_array::AssignFromSeqCntr(
            l_seg_work.ca, l_seg_work.ca.elem_cnt - shove_cnt, cntr_work.origin,
            origin_cursor, shove_cnt);

        r_seg_work.ref_beg += shove_cnt;
        r_seg_work.ca.elem_cnt -= shove_cnt;
        r_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt -
            compare_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                    cntr_work.seg_elem_slot_cnt);
    } else
#endif
    {
        seg_utils::SegShoveL(l_seg_work.ca, r_seg_work.ca, shove_cnt);

        r_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
    }

    l_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
}

template <typename CntrWorkType>
void SegShoveR_(CntrWorkType& cntr_work, SegWork_& l_seg_work,
                SegWork_& r_seg_work, size_t shove_cnt) {
    (CheckCntrSegWorkMatched_)(cntr_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(cntr_work, r_seg_work);

    ZETA_Core_DebugAssert(!l_seg_work.is_null);
    ZETA_Core_DebugAssert(!r_seg_work.is_null);

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= l_seg_work.ca.elem_cnt);
    ZETA_Core_DebugAssert(r_seg_work.ca.elem_cnt + shove_cnt <=
                          cntr_work.seg_elem_slot_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (r_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(cntr_work, r_seg_work);
    }
#endif

#if EnStaging
    if (l_seg_work.color == ref_color) {
        void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

        seq_cntr::Access(
            cntr_work.origin,
            l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - shove_cnt, true,
            origin_cursor, nullptr);

        circular_array::PushL(r_seg_work.ca, shove_cnt, seq_cntr::EmptyWriter{},
                              nullptr);

        circular_array::AssignFromSeqCntr(r_seg_work.ca, 0, cntr_work.origin,
                                          origin_cursor, shove_cnt);

        l_seg_work.ca.elem_cnt -= shove_cnt;
        l_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt -
            compare_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                    cntr_work.seg_elem_slot_cnt);
    } else
#endif
    {
        seg_utils::SegShoveR(l_seg_work.ca, r_seg_work.ca, shove_cnt);

        l_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
    }

    r_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
}

template <typename CntrWorkType, typename Writer>
void SegInsertShoveL_(
    CntrWorkType& cntr_work, SegWork_& l_seg_work, SegWork_& r_seg_work,
    size_t rl_cnt, size_t ins_cnt, size_t shove_cnt,
    Writer&& writer  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    (CheckCntrSegWorkMatched_)(cntr_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(cntr_work, r_seg_work);

    ZETA_Core_DebugAssert(!l_seg_work.is_null);
    ZETA_Core_DebugAssert(!r_seg_work.is_null);

    size_t l_vac{ cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt };
    size_t r_vac{ cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt };

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(rl_cnt <= r_seg_work.ca.elem_cnt);
    ZETA_Core_DebugAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_seg_work.ca.elem_cnt + ins_cnt);

    ZETA_Core_DebugAssert(rl_cnt + ins_cnt <= shove_cnt ||
                          r_seg_work.ca.elem_cnt + ins_cnt - shove_cnt <=
                              r_seg_work.ca.slot_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (l_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(cntr_work, l_seg_work);
    }
#endif

#if EnStaging
    if (r_seg_work.color == dat_color)
#endif
    {
        seg_utils::SegInsertShoveL(l_seg_work.ca, r_seg_work.ca, rl_cnt,
                                   ins_cnt, shove_cnt, writer);

        l_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

        r_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

#if EnStaging
    size_t cnt_a{ compare_utils::BasicMin(rl_cnt, shove_cnt) };
    size_t cnt_b{ compare_utils::BasicMin(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_elem_cnt{ l_seg_work.ca.elem_cnt };

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

    circular_array::PushR(l_seg_work.ca, shove_cnt, seq_cntr::EmptyWriter{},
                          nullptr);

    if (0 < cnt_a) {
        seq_cntr::Access(cntr_work.origin, r_seg_work.ref_beg, true,
                         origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(
            l_seg_work.ca, l_elem_cnt, cntr_work.origin, origin_cursor, cnt_a);
    }

    if (0 < cnt_b) {
        circular_array::IdxWrite(l_seg_work.ca, l_elem_cnt + cnt_a, cnt_b,
                                 writer);
    }

    if (0 < cnt_c) {
        seq_cntr::Access(cntr_work.origin, r_seg_work.ref_beg + cnt_a, true,
                         origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(
            l_seg_work.ca, l_elem_cnt + cnt_a + cnt_b, cntr_work.origin,
            origin_cursor, cnt_c);
    }

    if (0 < cnt_c) {
        l_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

        r_seg_work.ref_beg += cnt_a + cnt_c;
        r_seg_work.ca.elem_cnt += ins_cnt - shove_cnt;
        r_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt -
            compare_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                    cntr_work.seg_elem_slot_cnt);

        return;
    }

    void* data{ (AllocateData_)(cntr_work.data_size, cntr_work.data_alctr) };

    char* data_i{ static_cast<char*>(data) };

    size_t rr_cnt{ r_seg_work.ca.elem_cnt - rl_cnt };

    if (0 < rl_cnt - cnt_a) {
        seq_cntr::Access(cntr_work.origin, r_seg_work.ref_beg + cnt_a, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(cntr_work.origin, origin_cursor, rl_cnt - cnt_a,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = cntr_work.elem_size,
                           .elem_stride = cntr_work.elem_stride,
                       },
                       nullptr);

        data_i += cntr_work.elem_stride * (rl_cnt - cnt_a);
    }

    if (0 < ins_cnt - cnt_b) {
        writer(data_i, cntr_work.elem_stride, ins_cnt - cnt_b);
        data_i += cntr_work.elem_stride * (ins_cnt - cnt_b);
    }

    if (0 < rr_cnt - cnt_c) {
        seq_cntr::Access(cntr_work.origin, r_seg_work.ref_beg + rl_cnt, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(cntr_work.origin, origin_cursor, rr_cnt - cnt_c,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = cntr_work.elem_size,
                           .elem_stride = cntr_work.elem_stride,
                       },
                       nullptr);
    }

    l_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

    r_seg_work.color = dat_color;
    r_seg_work.ca.data = data;
    r_seg_work.ca.elem_cnt += ins_cnt - shove_cnt;
    r_seg_work.ca.rot = 0;
    r_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
#endif
}

template <typename CntrWorkType, typename Writer>
void SegInsertShoveR_(
    CntrWorkType& cntr_work, SegWork_& l_seg_work, SegWork_& r_seg_work,
    size_t lr_cnt, size_t ins_cnt, size_t shove_cnt,
    Writer&& writer  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    (CheckCntrSegWorkMatched_)(cntr_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(cntr_work, r_seg_work);

    ZETA_Core_DebugAssert(!l_seg_work.is_null);
    ZETA_Core_DebugAssert(!r_seg_work.is_null);

    size_t l_vac{ cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt };
    size_t r_vac{ cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt };

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(lr_cnt <= l_seg_work.ca.elem_cnt);
    ZETA_Core_DebugAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_seg_work.ca.elem_cnt + ins_cnt);
    ZETA_Core_DebugAssert(l_seg_work.ca.elem_cnt + ins_cnt - shove_cnt <=
                          l_seg_work.ca.slot_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (r_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(cntr_work, r_seg_work);
    }
#endif

#if EnStaging
    if (l_seg_work.color == dat_color)
#endif
    {
        seg_utils::SegInsertShoveR(l_seg_work.ca, r_seg_work.ca, lr_cnt,
                                   ins_cnt, shove_cnt, writer);

        l_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
        r_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

#if EnStaging
    size_t cnt_a{ compare_utils::BasicMin(lr_cnt, shove_cnt) };
    size_t cnt_b{ compare_utils::BasicMin(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

    circular_array::PushL(r_seg_work.ca, shove_cnt, seq_cntr::EmptyWriter{},
                          nullptr);

    if (0 < cnt_c) {
        seq_cntr::Access(
            cntr_work.origin,
            l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - cnt_a - cnt_c, true,
            origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(r_seg_work.ca, 0, cntr_work.origin,
                                          origin_cursor, cnt_c);
    }

    if (0 < cnt_b) {
        circular_array::IdxWrite(r_seg_work.ca, cnt_c, cnt_b, writer);
    }

    if (0 < cnt_a) {
        seq_cntr::Access(cntr_work.origin,
                         l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - cnt_a,
                         true, origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(r_seg_work.ca, cnt_c + cnt_b,
                                          cntr_work.origin, origin_cursor,
                                          cnt_a);
    }

    if (0 < cnt_c) {
        l_seg_work.ca.elem_cnt += ins_cnt - shove_cnt;
        l_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt -
            compare_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                    cntr_work.seg_elem_slot_cnt);

        r_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

    void* data{ (AllocateData_)(cntr_work.data_size, cntr_work.data_alctr) };

    char* data_i{ static_cast<char*>(data) };

    size_t ll_cnt{ l_seg_work.ca.elem_cnt - lr_cnt };

    if (0 < ll_cnt - cnt_c) {
        seq_cntr::Access(cntr_work.origin, l_seg_work.ref_beg, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(cntr_work.origin, origin_cursor, ll_cnt - cnt_c,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = cntr_work.elem_size,
                           .elem_stride = cntr_work.elem_stride,
                       },
                       nullptr);

        data_i += cntr_work.elem_stride * (ll_cnt - cnt_c);
    }

    if (0 < ins_cnt - cnt_b) {
        writer(data_i, cntr_work.elem_stride, ins_cnt - cnt_b);
        data_i += cntr_work.elem_stride * (ins_cnt - cnt_b);
    }

    if (0 < lr_cnt - cnt_a) {
        seq_cntr::Access(cntr_work.origin, l_seg_work.ref_beg + ll_cnt, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(cntr_work.origin, origin_cursor, lr_cnt - cnt_a,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = cntr_work.elem_size,
                           .elem_stride = cntr_work.elem_stride,
                       },
                       nullptr);
    }

    l_seg_work.color = dat_color;
    l_seg_work.ca.data = data;
    l_seg_work.ca.elem_cnt += ins_cnt - shove_cnt;
    l_seg_work.ca.rot = 0;
    l_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

    r_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
#endif
}

template <typename CntrWorkType>
void SegEraseShoveL_(CntrWorkType& cntr_work, SegWork_& l_seg_work,
                     SegWork_& r_seg_work, size_t rl_cnt, size_t ers_cnt,
                     size_t shove_cnt) {
    (CheckCntrSegWorkMatched_)(cntr_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(cntr_work, r_seg_work);

    size_t l_vac{ cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt };

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(rl_cnt <= r_seg_work.ca.elem_cnt);
    ZETA_Core_DebugAssert(ers_cnt <= r_seg_work.ca.elem_cnt - rl_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_seg_work.ca.elem_cnt - ers_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (l_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(cntr_work, l_seg_work);
    }
#endif

#if EnStaging
    if (r_seg_work.color == dat_color)
#endif
    {
        seg_utils::SegEraseShoveL(l_seg_work.ca, r_seg_work.ca, rl_cnt, ers_cnt,
                                  shove_cnt);

        l_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
        r_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

#if EnStaging
    size_t cnt_a{ compare_utils::BasicMin(rl_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    size_t l_elem_cnt{ l_seg_work.ca.elem_cnt };

    circular_array::PushR(l_seg_work.ca, shove_cnt, seq_cntr::EmptyWriter{},
                          nullptr);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

    if (0 < cnt_a) {
        seq_cntr::Access(cntr_work.origin, r_seg_work.ref_beg, true,
                         origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(
            l_seg_work.ca, l_elem_cnt, cntr_work.origin, origin_cursor, cnt_a);
    }

    if (0 < cnt_c) {
        seq_cntr::Access(cntr_work.origin, r_seg_work.ref_beg + cnt_a + cnt_b,
                         true, origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(l_seg_work.ca, l_elem_cnt + cnt_a,
                                          cntr_work.origin, origin_cursor,
                                          cnt_c);
    }

    l_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

    r_seg_work.ref_beg += cnt_a + cnt_b + cnt_c;
    r_seg_work.ca.elem_cnt -= cnt_a + cnt_b + cnt_c;
    r_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt -
                          compare_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                                  cntr_work.seg_elem_slot_cnt);
#endif
}

template <typename CntrWorkType>
void SegEraseShoveR_(CntrWorkType& cntr_work, SegWork_& l_seg_work,
                     SegWork_& r_seg_work, size_t lr_cnt, size_t ers_cnt,
                     size_t shove_cnt) {
    (CheckCntrSegWorkMatched_)(cntr_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(cntr_work, r_seg_work);

    size_t r_vac{ cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt };

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(lr_cnt <= l_seg_work.ca.elem_cnt);
    ZETA_Core_DebugAssert(ers_cnt <= l_seg_work.ca.elem_cnt - lr_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_seg_work.ca.elem_cnt - ers_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (r_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(cntr_work, r_seg_work);
    }
#endif

#if EnStaging
    if (l_seg_work.color == dat_color)
#endif
    {
        seg_utils::SegEraseShoveR(l_seg_work.ca, r_seg_work.ca, lr_cnt, ers_cnt,
                                  shove_cnt);

        l_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
        r_seg_work.elem_vac =
            cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

#if EnStaging
    size_t cnt_a{ compare_utils::BasicMin(lr_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    circular_array::PushL(r_seg_work.ca, shove_cnt, seq_cntr::EmptyWriter{},
                          nullptr);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

    if (0 < cnt_c) {
        seq_cntr::Access(
            cntr_work.origin,
            l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - cnt_a - cnt_b - cnt_c,
            true, origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(r_seg_work.ca, 0, cntr_work.origin,
                                          origin_cursor, cnt_c);
    }

    if (0 < cnt_a) {
        seq_cntr::Access(cntr_work.origin,
                         l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - cnt_a,
                         true, origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(
            r_seg_work.ca, cnt_c, cntr_work.origin, origin_cursor, cnt_a);
    }

    l_seg_work.ca.elem_cnt -= cnt_a + cnt_b + cnt_c;
    l_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt -
                          compare_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                                  cntr_work.seg_elem_slot_cnt);

    r_seg_work.elem_vac = cntr_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
#endif
}

template <typename CntrWorkType>
inline int Merge2_(CntrWorkType& cntr_work, SegWork_& l_seg_work,
                   SegWork_& r_seg_work) {
    ZETA_Core_DebugAssert(!l_seg_work.is_null);
    ZETA_Core_DebugAssert(!r_seg_work.is_null);

    ZETA_Core_DebugAssert(0 < l_seg_work.ca.elem_cnt);
    ZETA_Core_DebugAssert(0 < r_seg_work.ca.elem_cnt);

    unsigned long long random_seed{ utils::GetRandom() };

#if EnStaging
    size_t total_elem_cnt{ l_seg_work.ca.elem_cnt + r_seg_work.ca.elem_cnt };

    if (l_seg_work.color == ref_color && r_seg_work.color == ref_color) {
        void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

        void* data{ (AllocateData_)(cntr_work.data_size,
                                    cntr_work.data_alctr) };

        seq_cntr::Access(cntr_work.origin, l_seg_work.ref_beg, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(cntr_work.origin, origin_cursor, l_seg_work.ca.elem_cnt,
                       seq_cntr::MemReader{
                           .data = data,
                           .elem_size = cntr_work.elem_size,
                           .elem_stride = cntr_work.elem_stride,
                       },
                       nullptr);

        seq_cntr::Access(cntr_work.origin, r_seg_work.ref_beg, true,
                         origin_cursor, nullptr);
        seq_cntr::Read(
            cntr_work.origin, origin_cursor, r_seg_work.ca.elem_cnt,
            seq_cntr::MemReader{
                .data = static_cast<char*>(data) +
                        cntr_work.elem_stride * l_seg_work.ca.elem_cnt,
                .elem_size = cntr_work.elem_size,
                .elem_stride = cntr_work.elem_stride,
            },
            nullptr);

        SegWork_* dst_seg_work;
        SegWork_* other_seg_work;

        int side{ static_cast<int>(utils::SimpleRandomRotate(&random_seed) %
                                   2) };

        switch (side) {
        case 0:
            dst_seg_work = &l_seg_work;
            other_seg_work = &r_seg_work;
            break;
        case 1:
            dst_seg_work = &r_seg_work;
            other_seg_work = &l_seg_work;
            break;
        default: __builtin_unreachable();
        }

        dst_seg_work->color = dat_color;
        dst_seg_work->ca.data = data;
        dst_seg_work->ca.elem_cnt = total_elem_cnt;
        dst_seg_work->elem_vac = cntr_work.seg_elem_slot_cnt - total_elem_cnt;

        other_seg_work->ca.elem_cnt = 0;
        other_seg_work->elem_vac = cntr_work.seg_elem_slot_cnt;

        l_seg_work.is_dirty = true;
        r_seg_work.is_dirty = true;

        return side;
    }
#endif

    size_t l_based_merge_cost{ EnStagingTernary(l_seg_work.color == ref_color,
                                                l_seg_work.ca.elem_cnt, 0) +
                               r_seg_work.ca.elem_cnt };

    size_t r_based_merge_cost{ l_seg_work.ca.elem_cnt +
                               EnStagingTernary(r_seg_work.color == ref_color,
                                                r_seg_work.ca.elem_cnt, 0) };

    int side{ utils::Choose2(l_based_merge_cost <= r_based_merge_cost,
                             r_based_merge_cost <= l_based_merge_cost,
                             &random_seed) };

    switch (side) {
    case 0:
        (SegShoveL_)(cntr_work, l_seg_work, r_seg_work, r_seg_work.ca.elem_cnt);
        break;
    case 1:
        (SegShoveR_)(cntr_work, l_seg_work, r_seg_work, l_seg_work.ca.elem_cnt);
        break;
    default: __builtin_unreachable();
    }

    return side;
}

struct ElemCntBalancer_ {
    size_t total_seg_cnt;
    size_t res_seg_cnt;
    size_t base_seg_elem_cnt;
    size_t res;
    size_t err;

    inline ElemCntBalancer_(size_t elem_cnt, size_t seg_elem_slot_cnt) {
        this->total_seg_cnt = this->res_seg_cnt =
            integral_math::CeilDiv(elem_cnt, seg_elem_slot_cnt);

        this->base_seg_elem_cnt = elem_cnt / this->total_seg_cnt;

        this->res = elem_cnt % this->total_seg_cnt;

        this->err = 0;
    }

    inline size_t Fetch() {
        ZETA_Core_DebugAssert(0 < this->res_seg_cnt);

        --this->res_seg_cnt;
        this->err += this->res;

        if (this->err < this->total_seg_cnt) { return this->base_seg_elem_cnt; }

        this->err -= this->total_seg_cnt;
        return this->base_seg_elem_cnt + 1;
    }
};

template <bool EnRead, CntrTplParamList, typename ReaderWriterCore>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
     ReaderWriterCore& reader_writer_core, Cursor* dst_cursor) {
    (CheckCursor_)(cntr, pos_cursor);

    auto const cntr_work{ (MakeCntrWork_)(cntr) };

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->n = pos_cursor->n;
            dst_cursor->seg_idx = pos_cursor->seg_idx;
            dst_cursor->elem = pos_cursor->elem;
        }

        return;
    }

    ZETA_Core_DebugAssert(
        seq_cntr::IsDereferable(pos_cursor->idx, cnt, cntr_work.elem_cnt));

    Node* rb{ cntr.rb };

    size_t idx{ pos_cursor->idx };
    size_t end_idx{ idx + cnt };

    size_t seg_idx{ pos_cursor->seg_idx };

    Node* n{ static_cast<Node*>(pos_cursor->n) };
    Seg* seg;

#if EnStaging
    Node* l_n{ nullptr };
#endif

    Node* r_n{ nullptr };

#if EnStaging
    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };

    bool any_written{ false };

    unsigned long long random_seed{ utils::GetRandom() };
#endif

    for (;;) {
        seg = (NToSeg_)(n);

#if EnStaging
        if ((GetNColor_)(n) == dat_color)
#endif
        {

            size_t cur_cnt{ compare_utils::BasicMin(
                cnt, seg->dat.elem_cnt - seg_idx) };

            reader_writer_core.ReadWriteDat(seg->dat.data, seg->dat.rot,
                                            seg_idx, cur_cnt);

#if EnStaging
            if (!any_written) {
                any_written = true;
                pos_cursor->n = n;
                pos_cursor->seg_idx = seg_idx;
                pos_cursor->elem = circular_array::ReferElem(
                    seg->dat.data, cntr_work.elem_stride,
                    cntr_work.seg_elem_slot_cnt, seg->dat.rot, seg_idx);
            }
#endif

            seg_idx += cur_cnt;

            if (seg_idx == seg->dat.elem_cnt) {
#if EnStaging
                l_n = n;
#endif

                n = r_n == nullptr ? bin_tree::StepR(n) : r_n;
                r_n = nullptr;

                seg_idx = 0;
            }

            cnt -= cur_cnt;

            if (cnt == 0) { break; }

            continue;
        }

#if EnStaging
        size_t seg_elem_cnt{ seg->ref.elem_cnt };

        size_t cnt_a{ seg_idx };
        size_t cnt_b{ compare_utils::BasicMin(cnt, seg_elem_cnt - seg_idx) };
        size_t cnt_c{ seg_elem_cnt - cnt_a - cnt_b };

        size_t needed_seg_cnt{ integral_math::CeilDiv(
            cnt_b, cntr_work.seg_elem_slot_cnt) };

        if (integral_math::CeilDiv(seg_elem_cnt, cntr_work.seg_elem_slot_cnt) ==
            needed_seg_cnt) {
            goto WRITE;
        }

        {
            bool mat_ab_is_ok{ integral_math::CeilDiv(
                                   cnt_a + cnt_b,
                                   cntr_work.seg_elem_slot_cnt) ==
                               needed_seg_cnt };

            bool mat_bc_is_ok{ integral_math::CeilDiv(
                                   cnt_c + cnt_b,
                                   cntr_work.seg_elem_slot_cnt) ==
                               needed_seg_cnt };

            switch (static_cast<int>(mat_ab_is_ok) * 0b10 +
                    static_cast<int>(mat_bc_is_ok) * 0b01) {
            case 0b00: goto MAT_B;
            case 0b01: goto MAT_BC;
            case 0b10: goto MAT_AB;
            case 0b11:
                switch (utils::Choose2(cnt_a <= cnt_c, cnt_c <= cnt_a,
                                       &random_seed)) {
                case 0: goto MAT_AB;
                case 1: goto MAT_BC;
                default: __builtin_unreachable();
                }
            default: __builtin_unreachable();
            }
        }

    MAT_AB: {
        Seg* new_seg{ (AllocateRefSeg_)(cntr_work.seg_alctr) };

        cntr.root = rbtree::InsertR(n, &new_seg->n);

        new_seg->ref.beg = seg->ref.beg + cnt_a + cnt_b;
        new_seg->ref.elem_cnt = cnt_c;

        seg->ref.elem_cnt = cnt_a + cnt_b;

        bin_tree::AddDiffSize(&new_seg->n, new_seg->ref.elem_cnt);

        r_n = &new_seg->n;

        goto WRITE;
    }

    MAT_BC: {
        Seg* new_seg{ (AllocateRefSeg_)(cntr_work.seg_alctr) };

        cntr.root = rbtree::InsertL(n, &new_seg->n);

        new_seg->ref.beg = seg->ref.beg;
        new_seg->ref.elem_cnt = cnt_a;

        seg->ref.beg += cnt_a;
        seg->ref.elem_cnt = cnt_b + cnt_c;

        bin_tree::AddDiffSize(&new_seg->n, new_seg->ref.elem_cnt);

        l_n = &new_seg->n;

        seg_idx = 0;

        goto WRITE;
    }

    MAT_B: {
        Seg* new_l_seg{ (AllocateRefSeg_)(cntr_work.seg_alctr) };

        Seg* new_r_seg{ (AllocateRefSeg_)(cntr_work.seg_alctr) };

        cntr.root = rbtree::InsertL(n, &new_l_seg->n);
        cntr.root = rbtree::InsertR(n, &new_r_seg->n);

        new_l_seg->ref.beg = seg->ref.beg;
        new_l_seg->ref.elem_cnt = cnt_a;

        new_r_seg->ref.beg = seg->ref.beg + cnt_a + cnt_b;
        new_r_seg->ref.elem_cnt = cnt_c;

        seg->ref.beg += cnt_a;
        seg->ref.elem_cnt = cnt_b;

        bin_tree::AddDiffSize(&new_l_seg->n, new_l_seg->ref.elem_cnt);
        bin_tree::AddDiffSize(&new_r_seg->n, new_r_seg->ref.elem_cnt);

        l_n = &new_l_seg->n;
        r_n = &new_r_seg->n;

        seg_idx = 0;

        goto WRITE;
    }

    WRITE: {
        ElemCntBalancer_ balancer{ seg->ref.elem_cnt,
                                   cntr_work.seg_elem_slot_cnt };

        for (;;) {
            size_t new_seg_elem_cnt{ balancer.Fetch() };

            size_t cur_ref_beg{ seg->ref.beg };

            if (balancer.res_seg_cnt == 0) {
                l_n = n;
                n = r_n == nullptr ? bin_tree::StepR(n) : r_n;
                r_n = nullptr;

                (SetNColor_)(l_n, dat_color);

                Seg* l_seg{ (NToSeg_)(l_n) };

                l_seg->dat.data =
                    (AllocateData_)(cntr_work.data_size, cntr_work.data_alctr);

                l_seg->dat.rot = 0;

                bin_tree::AddDiffSize(l_n, new_seg_elem_cnt - seg_elem_cnt);
            } else {
                Seg* new_seg{ (AllocateDatSeg_)(cntr_work.data_size,
                                                cntr_work.seg_alctr,
                                                cntr_work.data_alctr) };

                cntr.root = l_n == nullptr
                                ? rbtree::InsertL(n, &new_seg->n)
                                : rbtree::Insert(l_n, n, &new_seg->n);

                new_seg->dat.elem_cnt =
                    static_cast<unsigned short>(new_seg_elem_cnt);

                seg->ref.beg += new_seg_elem_cnt;
                seg->ref.elem_cnt -= new_seg_elem_cnt;

                l_n = &new_seg->n;

                bin_tree::AddDiffSize(l_n, new_seg->dat.elem_cnt);
            }

            Seg* l_seg{ (NToSeg_)(l_n) };

            size_t cur_cnt{ compare_utils::BasicMin(
                cnt, new_seg_elem_cnt - seg_idx) };

            char* data{ static_cast<char*>(l_seg->dat.data) };

            if constexpr (EnRead) {
                seq_cntr::Access(cntr_work.origin, cur_ref_beg, true,
                                 origin_cursor, nullptr);

                seq_cntr::Read(cntr_work.origin, origin_cursor,
                               new_seg_elem_cnt,
                               seq_cntr::MemReader{
                                   .data = data,
                                   .elem_size = cntr_work.elem_size,
                                   .elem_stride = cntr_work.elem_stride,
                               },
                               nullptr);
            } else {
                if (0 < seg_idx) {
                    seq_cntr::Access(cntr_work.origin, cur_ref_beg, true,
                                     origin_cursor, nullptr);

                    seq_cntr::Read(cntr_work.origin, origin_cursor, seg_idx,
                                   seq_cntr::MemReader{
                                       .data = data,
                                       .elem_size = cntr_work.elem_size,
                                       .elem_stride = cntr_work.elem_stride,
                                   },
                                   nullptr);
                }

                if (seg_idx + cur_cnt < new_seg_elem_cnt) {
                    seq_cntr::Access(cntr_work.origin,
                                     cur_ref_beg + seg_idx + cur_cnt, true,
                                     origin_cursor, nullptr);

                    seq_cntr::Read(cntr_work.origin, origin_cursor,
                                   new_seg_elem_cnt - seg_idx - cur_cnt,
                                   seq_cntr::MemReader{
                                       .data = data + cntr_work.elem_stride *
                                                          (seg_idx + cur_cnt),
                                       .elem_size = cntr_work.elem_size,
                                       .elem_stride = cntr_work.elem_stride,
                                   },
                                   nullptr);
                }
            }

            reader_writer_core.ReadWriteDat(
                data + cntr_work.elem_stride * seg_idx, 0, 0, cur_cnt);

            if (!any_written) {
                any_written = true;
                pos_cursor->n = l_n;
                pos_cursor->seg_idx = seg_idx;
                pos_cursor->elem = data + cntr_work.elem_stride * seg_idx;
            }

            cnt -= cur_cnt;

            seg_idx += cur_cnt;

            if (seg_idx < new_seg_elem_cnt) {
                n = l_n;
            } else {
                seg_idx = 0;
            }

            if (balancer.res_seg_cnt == 0) { break; }
        }

        if (cnt == 0) { break; }
    }
#endif
    }

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = end_idx;
    dst_cursor->n = n;
    dst_cursor->seg_idx = seg_idx;

    if (rb == n
#if EnStaging
        || (GetNColor_)(n) == ref_color
#endif
    ) {
        dst_cursor->elem = nullptr;
        return;
    }

    seg = (NToSeg_)(n);

    CircularArray ca{
        .data = seg->dat.data,
        .elem_size = cntr_work.elem_size,
        .elem_stride = cntr_work.elem_stride,
        .elem_cnt = seg->dat.elem_cnt,
        .slot_cnt = cntr_work.seg_elem_slot_cnt,
        .rot = seg->dat.rot,
    };

    dst_cursor->elem =
        circular_array::Access(ca, seg_idx, true, nullptr, nullptr);
}

template <CntrTplParamList>
void InitTree_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr) {
    cntr.lb->Init(1);
    cntr.rb->Init(1);

    Node* root{ nullptr };

    root = rbtree::InsertR(root, cntr.lb);
    root = rbtree::InsertR(root, cntr.rb);

    cntr.root = root;
}

#if EnStaging

template <CntrTplParamList>
void RefOrigin_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr) {
    auto& origin{ meta::GetInstRef(cntr.origin) };

    size_t origin_elem_cnt{ seq_cntr::GetElemCnt(origin) };

    if (origin_elem_cnt == 0) { return; }

    Seg* seg{ (AllocateRefSeg_)(meta::GetInstRef(cntr.seg_alctr)) };

    seg->n.SetAccSize(origin_elem_cnt);
    seg->ref.beg = 0;
    seg->ref.elem_cnt = origin_elem_cnt;

    cntr.root = rbtree::Insert(cntr.lb, cntr.rb, &seg->n);
}

#endif

template <typename SegAllocator, typename DataAllocator>
pair::Pair<Namespace::Node*, Namespace::Node*> Erase_(
    Node* root, SegAllocator& seg_alctr, DataAllocator& data_alctr) {
    constexpr size_t buffer_capacity{ rbtree::max_height * 4 };

    Node* buffer[buffer_capacity];
    size_t buffer_i{ 0 };

    pair::Pair<Node*, Node*> ret;

    Node* root_l{ bin_tree::GetL(root) };
    Node* root_r{ bin_tree::GetR(root) };

    if (root_l != nullptr && root_r != nullptr) {
        detail::DeallocateSeg_(detail::NToSeg_(root), seg_alctr, data_alctr);
    }

    if (root_l == nullptr) {
        ret.first = root;
    } else {
        for (Node* n{ root_l };;) {
            Node* nl{ bin_tree::GetL(n) };
            Node* nr{ bin_tree::GetR(n) };

            if (nr != nullptr) {
                ZETA_Core_DebugAssert(buffer_i < buffer_capacity);
                buffer[buffer_i++] = nr;
            }

            if (nl == nullptr) {
                ret.first = n;
                break;
            }

            detail::DeallocateSeg_(detail::NToSeg_(n), seg_alctr, data_alctr);

            n = nl;
        }
    }

    if (root_r == nullptr) {
        ret.second = root;
    } else {
        for (Node* n{ root_r };;) {
            Node* nl{ bin_tree::GetL(n) };
            Node* nr{ bin_tree::GetR(n) };

            if (nl != nullptr) {
                ZETA_Core_DebugAssert(buffer_i < buffer_capacity);
                buffer[buffer_i++] = nl;
            }

            if (nr == nullptr) {
                ret.second = n;
                break;
            }

            detail::DeallocateSeg_(detail::NToSeg_(n), seg_alctr, data_alctr);

            n = nr;
        }
    }

    while (0 < buffer_i) {
        Node* n{ buffer[--buffer_i] };

        Node* nl{ bin_tree::GetL(n) };
        Node* nr{ bin_tree::GetR(n) };

        if (nl != nullptr) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);
            buffer[buffer_i++] = nl;
        }

        if (nr != nullptr) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);
            buffer[buffer_i++] = nr;
        }

        detail::DeallocateSeg_(detail::NToSeg_(n), seg_alctr, data_alctr);
    }

    return ret;
}

template <
#if EnStaging
    typename SrcOriginLike,
#endif
    typename SrcSegAllocatorLike, typename SrcDataAllocatorLike,
    typename SegAllocator, typename DataAllocator>
Node* Copy_(size_t elem_stride, size_t seg_elem_slot_cnt, Node* lb, Node* rb,
            Cntr<
#if EnStaging
                SrcOriginLike,
#endif
                SrcSegAllocatorLike, SrcDataAllocatorLike> const& src_cntr,
            SegAllocator& seg_alctr, DataAllocator& data_alctr) {
    size_t elem_size{ src_cntr.elem_size };

    ZETA_Core_DebugAssert(elem_size <= elem_stride);
    ZETA_Core_DebugAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugAssert(seg_elem_slot_cnt <= max_seg_elem_slot_cnt);
    ZETA_Core_DebugAssert(lb != nullptr);
    ZETA_Core_DebugAssert(rb != nullptr);

    size_t data_size{ (CalcDataSize_)(elem_size, elem_stride,
                                      seg_elem_slot_cnt) };

    lb->Init(1);
    rb->Init(1);

    Node* root{ nullptr };

    root = rbtree::InsertR(root, lb);
    root = rbtree::InsertR(root, rb);

    Node* last_n{ lb };

    Node* src_n{ bin_tree::StepR(src_cntr.lb) };

    SegWork_ src_seg_work;
    src_seg_work.SetBasics((MakeCntrWork_)(src_cntr));

    while (src_n != src_cntr.rb) {
#if EnStaging
        if ((GetNColor_)(src_n) == ref_color) {
            Seg* src_seg{ (NToSeg_)(src_n) };

            Seg* dst_seg{ (AllocateRefSeg_)(seg_alctr) };

            dst_seg->ref.beg = src_seg->ref.beg;
            dst_seg->ref.elem_cnt = src_seg->ref.elem_cnt;

            root = rbtree::Insert(last_n, rb, &dst_seg->n);
            bin_tree::AddDiffSize(&dst_seg->n, dst_seg->ref.elem_cnt);
            last_n = &dst_seg->n;

            src_n = bin_tree::StepR(src_n);

            continue;
        }
#endif

        size_t acc_elem_cnt{ 0 };

        {
            Node* src_scan_n{ src_n };

            do {
                acc_elem_cnt += (NToSeg_)(src_scan_n)->dat.elem_cnt;
                src_scan_n = bin_tree::StepR(src_scan_n);
            } while (src_scan_n != src_cntr.rb
#if EnStaging
                     && (GetNColor_)(src_scan_n) == dat_color
#endif
            );
        }

        void* dst_data;
        size_t dst_idx;

        {
            Seg* dst_seg{ (AllocateDatSeg_)(data_size, seg_alctr, data_alctr) };
            root = rbtree::Insert(last_n, rb, &dst_seg->n);
            last_n = &dst_seg->n;

            dst_data = dst_seg->dat.data;
            dst_idx = 0;
        }

        size_t src_idx{ 0 };
        src_seg_work.Load((NToSeg_)(src_n));

        ElemCntBalancer_ balancer{ acc_elem_cnt, seg_elem_slot_cnt };

        size_t res_cnt{ balancer.Fetch() };

        for (;;) {
            size_t cur_cnt{ compare_utils::BasicMin(
                res_cnt, src_seg_work.ca.elem_cnt - src_idx) };

            circular_array::IdxRead(
                src_seg_work.ca, src_idx, cur_cnt,
                seq_cntr::MemReader{ .data = dst_data,
                                     .elem_size = elem_size,
                                     .elem_stride = elem_stride });

            dst_data = static_cast<char*>(dst_data) + elem_stride * cur_cnt;
            dst_idx += cur_cnt;

            src_idx += cur_cnt;

            res_cnt -= cur_cnt;

            if (res_cnt == 0) {
                (NToSeg_)(last_n)->dat.elem_cnt =
                    static_cast<unsigned short>(dst_idx);

                bin_tree::AddDiffSize(last_n, dst_idx);

                if (balancer.res_seg_cnt == 0) {
                    ZETA_Core_DebugAssert(src_idx == src_seg_work.ca.elem_cnt);

                    src_n = bin_tree::StepR(src_n);

                    break;
                }

                Seg* dst_seg{ (AllocateDatSeg_)(data_size, seg_alctr,
                                                data_alctr) };
                root = rbtree::Insert(last_n, rb, &dst_seg->n);
                last_n = &dst_seg->n;

                dst_data = dst_seg->dat.data;
                dst_idx = 0;

                res_cnt = balancer.Fetch();
            }

            if (src_idx == src_seg_work.ca.elem_cnt) {
                src_n = bin_tree::StepR(src_n);
                src_idx = 0;
                src_seg_work.Load((NToSeg_)(src_n));
            }
        }
    }

    return root;
}

}  // namespace Namespace::detail

template <CntrTplParamList,
#if EnStaging
          typename OriginLikeInitArg,
#endif
          typename SegAllocatorLikeInitArg, typename DataAllocatorLikeInitArg>
void Namespace::Init(Cntr<CntrTplArgList>& cntr,
#if !EnStaging
                     size_t elem_size,
#endif
                     size_t elem_stride, size_t seg_elem_slot_cnt,
#if EnStaging
                     OriginLikeInitArg&& origin_like_init_arg,
#endif
                     SegAllocatorLikeInitArg&& seg_alctr_like_init_arg,
                     DataAllocatorLikeInitArg&& data_alctr_like_init_arg) {
    ZETA_Core_DebugAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugAssert(seg_elem_slot_cnt <= max_seg_elem_slot_cnt);

#if EnStaging
    lifecycle::Init(cntr.origin,
                    meta::Forward<OriginLikeInitArg>(origin_like_init_arg));
    auto& origin{ meta::GetInstRef(cntr.origin) };
    seq_cntr::CheckContract(origin);

    size_t elem_size{ seq_cntr::GetElemSize(origin) };
#endif

    lifecycle::Init(cntr.seg_alctr, meta::Forward<SegAllocatorLikeInitArg>(
                                        seg_alctr_like_init_arg));
    allocator::CheckContract(meta::GetInstRef(cntr.seg_alctr));

    lifecycle::Init(cntr.data_alctr, meta::Forward<DataAllocatorLikeInitArg>(
                                         data_alctr_like_init_arg));
    allocator::CheckContract(meta::GetInstRef(cntr.data_alctr));

    ZETA_Core_DebugAssert(0 < elem_size);
    ZETA_Core_DebugAssert(elem_size <= elem_stride);

    cntr.elem_size = elem_size;

    cntr.elem_stride = elem_stride;

    cntr.seg_elem_slot_cnt = seg_elem_slot_cnt;

    cntr.lb = static_cast<Node*>(allocator::SafeAllocate(
        meta::GetInstRef(cntr.seg_alctr), alignof(Node), sizeof(Node)));

    cntr.rb = static_cast<Node*>(allocator::SafeAllocate(
        meta::GetInstRef(cntr.seg_alctr), alignof(Node), sizeof(Node)));

    detail::InitTree_(cntr);

#if EnStaging
    detail::RefOrigin_(cntr);
#endif
}

template <CntrTplParamList,
#if EnStaging
          typename OriginLikeInitArg,
#endif
          typename SegAllocatorLikeInitArg, typename DataAllocatorLikeInitArg,
          typename SrcOriginLike, typename SrcSegAllocatorLike,
          typename SrcDataAllocatorLike>
void Namespace::Init(
    Cntr<CntrTplArgList>& cntr, size_t elem_stride, size_t seg_elem_slot_cnt,
#if EnStaging
    OriginLikeInitArg&& origin_like_init_arg,
#endif
    SegAllocatorLikeInitArg&& seg_alctr_like_init_arg,
    DataAllocatorLikeInitArg&& data_alctr_like_init_arg,
    Cntr<
#if EnStaging
        SrcOriginLike,
#endif
        SrcSegAllocatorLike, SrcDataAllocatorLike> const& src_cntr) {
    size_t elem_size{ src_cntr.elem_size };

    ZETA_Core_DebugAssert(elem_size <= elem_stride);
    ZETA_Core_DebugAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugAssert(seg_elem_slot_cnt <= max_seg_elem_slot_cnt);

#if EnStaging
    lifecycle::Init(cntr.origin,
                    meta::Forward<OriginLikeInitArg>(origin_like_init_arg));
    auto& origin{ meta::GetInstRef(cntr.origin) };
    seq_cntr::CheckContract(origin);

    ZETA_Core_DebugAssert(seq_cntr::GetReferedInstPtr(cntr.origin) ==
                          seq_cntr::GetReferedInstPtr(src_cntr.origin));
#endif

    lifecycle::Init(cntr.seg_alctr, meta::Forward<SegAllocatorLikeInitArg>(
                                        seg_alctr_like_init_arg));
    auto& seg_alctr{ meta::GetInstRef(cntr.seg_alctr) };
    allocator::CheckContract(seg_alctr);

    lifecycle::Init(cntr.data_alctr, meta::Forward<DataAllocatorLikeInitArg>(
                                         data_alctr_like_init_arg));
    auto& data_alctr{ meta::GetInstRef(cntr.data_alctr) };
    allocator::CheckContract(data_alctr);

    cntr.elem_size = elem_size;
    cntr.elem_stride = elem_stride;
    cntr.seg_elem_slot_cnt = seg_elem_slot_cnt;

    cntr.lb = static_cast<Node*>(allocator::SafeAllocate(
        meta::GetInstRef(cntr.seg_alctr), alignof(Node), sizeof(Node)));

    cntr.rb = static_cast<Node*>(allocator::SafeAllocate(
        meta::GetInstRef(cntr.seg_alctr), alignof(Node), sizeof(Node)));

    cntr.root = detail::Copy_(elem_stride, seg_elem_slot_cnt, cntr.lb, cntr.rb,
                              src_cntr, seg_alctr, data_alctr);
}

template <CntrTplParamList>
void Namespace::Deinit(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    detail::Erase_(cntr.root, cntr.seg_alctr, cntr.data_alctr);

    allocator::Deallocate(cntr.seg_alctr, cntr.lb);
    allocator::Deallocate(cntr.seg_alctr, cntr.rb);
}

template <CntrTplParamList
#if EnStaging
          ,
          typename OriginLikeInitArg
#endif
          >
void Namespace::Copy(Cntr<CntrTplArgList>& cntr,
#if EnStaging
                     OriginLikeInitArg&& origin_like_init_arg,
#endif
                     Cntr<CntrTplArgList> const& src_cntr) {
    auto& seg_alctr{ meta::GetInstRef(cntr.seg_alctr) };
    auto& data_alctr{ meta::GetInstRef(cntr.data_alctr) };

    pool_allocator::Allocator<pool_allocator::ReuseStrategy::Oldest,
                              pool_allocator::ReleaseStrategy::Never,
                              decltype(seg_alctr)>
        seg_pool_alctr;

    pool_allocator::Init(seg_pool_alctr, seg_alctr);

    pool_allocator::Allocator<pool_allocator::ReuseStrategy::Oldest,
                              pool_allocator::ReleaseStrategy::Never,
                              decltype(data_alctr)>
        data_pool_alctr;

    detail::Erase_(cntr.root, seg_pool_alctr, data_pool_alctr);

#if EnStaging
    lifecycle::Deinit(cntr.origin);

    lifecycle::Init(cntr.origin,
                    meta::Forward<OriginLikeInitArg>(origin_like_init_arg));
    auto& origin{ meta::GetInstRef(cntr.origin) };
    seq_cntr::CheckContract(origin);

    ZETA_Core_DebugAssert(seq_cntr::GetReferedInstPtr(cntr.origin) ==
                          seq_cntr::GetReferedInstPtr(src_cntr.origin));
#endif

    cntr.root =
        detail::Copy_(cntr.elem_stride, cntr.seg_elem_slot_cnt, cntr.lb,
                      cntr.rb, src_cntr, seg_pool_alctr, data_pool_alctr);

    pool_allocator::Deinit(seg_pool_alctr);
    pool_allocator::Deinit(data_pool_alctr);
}

template <CntrTplParamList>
constexpr size_t Namespace::GetCursorSize(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

template <CntrTplParamList>
size_t Namespace::GetElemSize(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.elem_size;
}

template <CntrTplParamList>
size_t Namespace::GetElemCnt(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return cntr.root->GetAccSize() - 2;
}

template <CntrTplParamList>
size_t Namespace::GetMaxElemCnt(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void Namespace::GetLBCursor(Cntr<CntrTplArgList> const& cntr,
                            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->n = cntr.lb;
    dst_cursor->seg_idx = 0;
    dst_cursor->elem = nullptr;
}

template <CntrTplParamList>
void Namespace::GetRBCursor(Cntr<CntrTplArgList> const& cntr,
                            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = &cntr;
    dst_cursor->idx = GetElemCnt(cntr);
    dst_cursor->n = cntr.rb;
    dst_cursor->seg_idx = 0;
    dst_cursor->elem = nullptr;
}

template <CntrTplParamList>
void* Namespace::PeekL(Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
                       Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto const cntr_work{ detail::MakeCntrWork_(cntr) };

    Node* n{ bin_tree::StepR(cntr.lb) };

    if (cntr.rb == n) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = 0;
            dst_cursor->n = n;
            dst_cursor->seg_idx = 0;
            dst_cursor->elem = nullptr;
        }

        return nullptr;
    }

    Seg* seg{ detail::NToSeg_(n) };

    void* elem;

#if EnStaging
    if (detail::GetNColor_(n) == ref_color) {
        seq_cntr::Access(cntr_work.origin, seg->ref.beg, false, nullptr,
                         dst_elem);
        elem = nullptr;
    } else
#endif
    {
        CircularArray ca{
            .data = seg->dat.data,
            .elem_size = cntr_work.elem_size,
            .elem_stride = cntr_work.elem_stride,
            .elem_cnt = seg->dat.elem_cnt,
            .slot_cnt = cntr_work.seg_elem_slot_cnt,
            .rot = seg->dat.rot,
        };

        elem = circular_array::PeekL(ca, lazy_copy_elem, nullptr, dst_elem);
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = 0;
        dst_cursor->n = n;
        dst_cursor->seg_idx = 0;
        dst_cursor->elem = elem;
    }

    return elem;
}

template <CntrTplParamList>
void* Namespace::PeekR(Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
                       Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto const cntr_work{ detail::MakeCntrWork_(cntr) };

    Node* n{ bin_tree::StepL(cntr.rb) };

    if (cntr.lb == n) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = static_cast<size_t>(-1);
            dst_cursor->n = n;
            dst_cursor->seg_idx = 0;
            dst_cursor->elem = nullptr;
        }

        return nullptr;
    }

    Seg* seg{ detail::NToSeg_(n) };

    void* elem;

    size_t seg_elem_cnt;

#if EnStaging
    if (detail::GetNColor_(n) == ref_color) {
        seg_elem_cnt = seg->ref.elem_cnt;

        seq_cntr::Access(cntr_work.origin, seg->ref.beg + seg_elem_cnt - 1,
                         false, nullptr, dst_elem);

        elem = nullptr;
    } else
#endif
    {
        seg_elem_cnt = seg->dat.elem_cnt;

        CircularArray ca{
            .data = seg->dat.data,
            .elem_size = cntr_work.elem_size,
            .elem_stride = cntr_work.elem_stride,
            .elem_cnt = seg_elem_cnt,
            .slot_cnt = cntr_work.seg_elem_slot_cnt,
            .rot = seg->dat.rot,
        };

        elem = circular_array::PeekR(ca, lazy_copy_elem, nullptr, dst_elem);
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = cntr_work.elem_cnt - 1;
        dst_cursor->n = n;
        dst_cursor->seg_idx = seg_elem_cnt - 1;
        dst_cursor->elem = elem;
    }

    return elem;
}

template <CntrTplParamList>
void* Namespace::Access(Cntr<CntrTplArgList> const& cntr, size_t idx,
                        bool lazy_copy_elem, Cursor* dst_cursor,
                        void* dst_elem) {
    detail::CheckCntr_(cntr);

    auto const cntr_work{ detail::MakeCntrWork_(cntr) };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, cntr_work.elem_cnt));

    auto [n, seg_idx]{ bin_tree::AccessL(cntr.root, idx + 1) };

    void* elem{ nullptr };

    if (idx < cntr_work.elem_cnt) {
        Seg* seg{ detail::NToSeg_(n) };

#if EnStaging
        if (detail::GetNColor_(n) == ref_color) {
            if (dst_elem != nullptr) {
                seq_cntr::Access(cntr_work.origin, seg->ref.beg + seg_idx,
                                 false, nullptr, dst_elem);
            }
        } else
#endif
        {
            CircularArray ca{
                .data = seg->dat.data,
                .elem_size = cntr_work.elem_size,
                .elem_stride = cntr_work.elem_stride,
                .elem_cnt = seg->dat.elem_cnt,
                .slot_cnt = cntr_work.seg_elem_slot_cnt,
                .rot = seg->dat.rot,
            };

            elem = circular_array::Access(ca, seg_idx, lazy_copy_elem, nullptr,
                                          dst_elem);
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = idx;
        dst_cursor->n = n;
        dst_cursor->seg_idx = seg_idx;
        dst_cursor->elem = elem;
    }

    return elem;
}

template <CntrTplParamList>
void* Namespace::Derefer(Cntr<CntrTplArgList> const& cntr,
                         Cursor const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto const cntr_work{ detail::MakeCntrWork_(cntr) };

    Node* n{ static_cast<Node*>(pos_cursor->n) };
    size_t seg_idx{ pos_cursor->seg_idx };
    void* elem{ pos_cursor->elem };

    if (elem == nullptr) {
#if EnStaging
        if (cntr.lb != n && cntr.rb != n && dst_elem != nullptr) {
            Seg* seg{ detail::NToSeg_(static_cast<Node*>(pos_cursor->n)) };

            seq_cntr::Access(cntr_work.origin, seg->ref.beg + seg_idx, false,
                             nullptr, dst_elem);
        }
#endif

        return nullptr;
    }

    if (dst_elem == nullptr || lazy_copy_elem) { return elem; }

    Seg* seg{ detail::NToSeg_(n) };

    CircularArray ca{
        .data = seg->dat.data,
        .elem_size = cntr_work.elem_size,
        .elem_stride = cntr_work.elem_stride,
        .elem_cnt = cntr_work.seg_elem_slot_cnt,
        .slot_cnt = cntr_work.seg_elem_slot_cnt,
        .rot = seg->dat.rot,
    };

    circular_array::Access(ca, seg_idx, false, nullptr, dst_elem);

    return elem;
}

template <CntrTplParamList, typename Reader>
void Namespace::Read(
    Cntr<CntrTplArgList> const& cntr, Cursor const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    ZETA_Core_DebugAssert(
        seq_cntr::IsDereferable(pos_cursor->idx, cnt, GetElemCnt(cntr)));

    auto const cntr_work{ detail::MakeCntrWork_(cntr) };

    Node* n{ static_cast<Node*>(pos_cursor->n) };
    Seg* seg;

    size_t seg_idx{ pos_cursor->seg_idx };

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = &cntr;
        dst_cursor->idx = pos_cursor->idx + cnt;
    }

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->n = n;
            dst_cursor->seg_idx = seg_idx;
            dst_cursor->elem = pos_cursor->elem;
        }

        return;
    }

#if EnStaging
    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(cntr_work.origin) };
#endif

    CircularArray ca{
        .data = {},
        .elem_size = cntr_work.elem_size,
        .elem_stride = cntr_work.elem_stride,
        .elem_cnt = {},
        .slot_cnt = cntr_work.seg_elem_slot_cnt,
        .rot = {},
    };

    while (0 < cnt) {
        seg = detail::NToSeg_(n);

        size_t seg_elem_cnt;
        size_t cur_cnt;

#if EnStaging
        if (detail::GetNColor_(n) == ref_color) {
            seg_elem_cnt = seg->ref.elem_cnt;
            cur_cnt = compare_utils::BasicMin(cnt, seg_elem_cnt - seg_idx);

            seq_cntr::Access(cntr_work.origin, seg->ref.beg + seg_idx, true,
                             origin_cursor, nullptr);

            seq_cntr::Read(cntr_work.origin, origin_cursor, cur_cnt, reader,
                           nullptr);
        } else
#endif
        {
            seg_elem_cnt = seg->dat.elem_cnt;
            cur_cnt = compare_utils::BasicMin(cnt, seg_elem_cnt - seg_idx);

            ca.data = seg->dat.data;
            ca.elem_cnt = seg_elem_cnt;
            ca.rot = seg->dat.rot;

            circular_array::IdxRead(ca, seg_idx, cur_cnt, reader);
        }

        seg_idx += cur_cnt;

        if (seg_idx == seg_elem_cnt) {
            n = bin_tree::StepR(n);
            seg_idx = 0;
        }

        cnt -= cur_cnt;
    }

    if (dst_cursor == nullptr) { return; }

    dst_cursor->n = n;
    dst_cursor->seg_idx = seg_idx;

    if (cntr.rb == n
#if EnStaging
        || detail::GetNColor_(n) == ref_color
#endif
    ) {
        dst_cursor->elem = nullptr;
        return;
    }

    seg = detail::NToSeg_(n);

    ca.data = seg->dat.data;
    ca.elem_cnt = seg->dat.elem_cnt;
    ca.rot = seg->dat.rot;

    dst_cursor->elem =
        circular_array::Access(ca, seg_idx, true, nullptr, nullptr);
}

template <CntrTplParamList, typename Writer>
void Namespace::Write(
    Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };

    struct {
        Writer&&
            writer;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        CircularArray ca;

        void ReadWriteDat(void* data, size_t offset, size_t idx, size_t cnt) {
            this->ca.data = data;
            this->ca.rot = offset;

            circular_array::IdxWrite(this->ca, idx, cnt, this->writer);
        }
    } reader_core{
        .writer =writer,

        .ca = {
            .data = {},
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = seg_elem_slot_cnt,
            .slot_cnt = seg_elem_slot_cnt,
            .rot = {},
        },
    };

    detail::ReadWrite_<false>(cntr, pos_cursor, cnt, reader_core, dst_cursor);
}

template <CntrTplParamList, typename ReaderWriter>
void Namespace::ReadWrite(
    Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };

    struct {
        ReaderWriter&&
            reader_writer;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        CircularArray ca;

        void ReadWriteDat(void* data, size_t offset, size_t idx, size_t cnt) {
            this->ca.data = data;
            this->ca.rot = offset;

            circular_array::IdxWrite(this->ca, idx, cnt, this->reader_writer);
        }
    } reader_writer_core{
        .reader_writer = reader_writer,

        .ca = {
            .data = {},
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = seg_elem_slot_cnt,
            .slot_cnt = seg_elem_slot_cnt,
            .rot = {},
        },
    };

    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer_core,
                             dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* Namespace::PushL(
    Cntr<CntrTplArgList>& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    Cursor pos_cursor;
    (PeekL)(cntr, true, &pos_cursor, nullptr);

    return (Insert)(cntr, &pos_cursor, cnt, writer, dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* Namespace::PushR(
    Cntr<CntrTplArgList>& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    Cursor pos_cursor;
    (GetRBCursor)(cntr, &pos_cursor);

    return (Insert)(cntr, &pos_cursor, cnt, writer, dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* Namespace::Insert(
    Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    auto const cntr_work{ detail::MakeCntrWork_(cntr) };

    ZETA_Core_DebugAssert(seq_cntr::IsInsertable(
        pos_cursor->idx, cnt, cntr_work.elem_cnt, (GetMaxElemCnt)(cntr)));

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = &cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->n = pos_cursor->n;
            dst_cursor->seg_idx = pos_cursor->seg_idx;
            dst_cursor->elem = pos_cursor->elem;
        }

        return pos_cursor->elem;
    }

    Node* m_n{ static_cast<Node*>(pos_cursor->n) };
    size_t seg_idx{ pos_cursor->seg_idx };
    size_t end_idx{ pos_cursor->idx + cnt };

    ZETA_Core_DebugAssert(cntr.lb != m_n);

    unsigned long long random_seed{ utils::GetRandom() };

    Node* l_n;
    Node* r_n;

    detail::SegWork_ l_seg_work;
    detail::SegWork_ m_seg_work;
    detail::SegWork_ r_seg_work;

    l_seg_work.SetBasics(cntr_work);
    m_seg_work.SetBasics(cntr_work);
    r_seg_work.SetBasics(cntr_work);

    if (seg_idx == 0) {
        /*

        if the insertion point is between segments, the state has already be a
        nice form to insert (optionally) new segments to store more data.

        */

        l_n = bin_tree::StepL(m_n);

        r_n = m_n;

        m_n = nullptr;
        m_seg_work.SetNull();

        if (cntr.lb == l_n) {
            l_seg_work.SetNull();
        } else {
            l_seg_work.Load(detail::NToSeg_(l_n));
        }

        if (cntr.rb == r_n) {
            r_seg_work.SetNull();
        } else {
            r_seg_work.Load(detail::NToSeg_(r_n));
        }
    } else {
        m_seg_work.Load(detail::NToSeg_(m_n));

        size_t ml_elem_cnt{ seg_idx };
        size_t mr_elem_cnt{ m_seg_work.ca.elem_cnt - seg_idx };

        if (cnt <= m_seg_work.elem_vac) {
            void* elem;

#if EnStaging
            if (m_seg_work.color == ref_color) {
                detail::AugMaterializeRefSeg_(cntr_work, m_seg_work,
                                              ml_elem_cnt, cnt, mr_elem_cnt,
                                              writer);

                elem = circular_array::Access(m_seg_work.ca, ml_elem_cnt, true,
                                              nullptr, nullptr);
            } else
#endif
            {

                elem = circular_array::IdxInsert(m_seg_work.ca, ml_elem_cnt,
                                                 cnt, writer);
            }

            m_seg_work.Store();

            pos_cursor->elem = elem;

            if (dst_cursor != nullptr) {
                dst_cursor->cntr = &cntr;
                dst_cursor->idx = end_idx;
                dst_cursor->n = m_n;
                dst_cursor->seg_idx = ml_elem_cnt + cnt;
                dst_cursor->elem = circular_array::Access(
                    m_seg_work.ca, ml_elem_cnt + cnt, true, nullptr, nullptr);
            }

            return elem;
        }

        l_n = bin_tree::StepL(m_n);
        r_n = bin_tree::StepR(m_n);

        if (cntr.lb == l_n) {
            l_seg_work.SetNull();
        } else {
            l_seg_work.Load(detail::NToSeg_(l_n));
        }

        if (cntr.rb == r_n) {
            r_seg_work.SetNull();
        } else {
            r_seg_work.Load(detail::NToSeg_(r_n));
        }

        bool l_m_is_ok{ l_seg_work.ca.elem_cnt + m_seg_work.ca.elem_cnt + cnt <=
                        cntr_work.seg_elem_slot_cnt * 2 };
        bool m_r_is_ok{ m_seg_work.ca.elem_cnt + r_seg_work.ca.elem_cnt + cnt <=
                        cntr_work.seg_elem_slot_cnt * 2 };

        if (!l_m_is_ok && !m_r_is_ok) {
            /*

            if neither l + m and m + r can hold all old + new data, it means it
            is too crowded near this section, we do not try to use l + m + r to
            hold all old + new data, instead we jump to split logic to transform
            the insertion point to be between segments.

            */

            goto SPLIT_M;
        }

        // choose the better side to insert

        switch (utils::Choose2(
            l_m_is_ok && l_seg_work.ca.elem_cnt <= r_seg_work.ca.elem_cnt,
            m_r_is_ok && r_seg_work.ca.elem_cnt <= l_seg_work.ca.elem_cnt,
            &random_seed)) {
        case 0: goto INSERT_INTO_L_M;
        case 1: goto INSERT_INTO_M_R;
        default: __builtin_unreachable();
        }

    INSERT_INTO_L_M: {
        // use l + m to hold all old + new data

        detail::SegInsertShoveL_(
            cntr_work, l_seg_work, m_seg_work, ml_elem_cnt, cnt,
            (m_seg_work.ca.elem_cnt + cnt - l_seg_work.ca.elem_cnt) / 2,
            writer);

        l_seg_work.Store();
        m_seg_work.Store();

        void* ret_elem;

        if (m_seg_work.ca.elem_cnt < cnt + mr_elem_cnt) {
            pos_cursor->n = l_n;
            pos_cursor->seg_idx = l_seg_work.ca.elem_cnt +
                                  m_seg_work.ca.elem_cnt - cnt - mr_elem_cnt;
            pos_cursor->elem = ret_elem = circular_array::Access(
                l_seg_work.ca, pos_cursor->seg_idx, true, nullptr, nullptr);
        } else {
            pos_cursor->n = m_n;
            pos_cursor->seg_idx = m_seg_work.ca.elem_cnt - cnt - mr_elem_cnt;
            pos_cursor->elem = ret_elem = EnStagingTernary(
                m_seg_work.color == ref_color, nullptr,
                circular_array::Access(m_seg_work.ca, pos_cursor->seg_idx, true,
                                       nullptr, nullptr));
        }

        if (dst_cursor == nullptr) { return ret_elem; }

        dst_cursor->cntr = &cntr;
        dst_cursor->idx = end_idx;

        if (m_seg_work.ca.elem_cnt < mr_elem_cnt) {
            dst_cursor->n = l_n;
            dst_cursor->seg_idx =
                l_seg_work.ca.elem_cnt + m_seg_work.ca.elem_cnt - mr_elem_cnt;
            dst_cursor->elem = circular_array::Access(
                l_seg_work.ca, dst_cursor->seg_idx, true, nullptr, nullptr);
        } else {
            dst_cursor->n = m_n;
            dst_cursor->seg_idx = m_seg_work.ca.elem_cnt - mr_elem_cnt;
            dst_cursor->elem = EnStagingTernary(
                m_seg_work.color == ref_color, nullptr,
                circular_array::Access(m_seg_work.ca, dst_cursor->seg_idx, true,
                                       nullptr, nullptr));
        }

        return ret_elem;
    }

    INSERT_INTO_M_R: {
        // use m + r to hold all old + new data

        ZETA_Core_PrintVar(m_seg_work.ca.elem_cnt);
        ZETA_Core_PrintVar(cnt);
        ZETA_Core_PrintVar(r_seg_work.ca.elem_cnt);
        ZETA_Core_PrintVar(cntr_work.seg_elem_slot_cnt);

        detail::SegInsertShoveR_(
            cntr_work, m_seg_work, r_seg_work, mr_elem_cnt, cnt,
            (m_seg_work.ca.elem_cnt + cnt - r_seg_work.ca.elem_cnt) / 2,
            writer);

        m_seg_work.Store();
        r_seg_work.Store();

        void* ret_elem;

        if (ml_elem_cnt < m_seg_work.ca.elem_cnt) {
            pos_cursor->n = m_n;
            pos_cursor->seg_idx = ml_elem_cnt;
            pos_cursor->elem = circular_array::Access(
                m_seg_work.ca, pos_cursor->seg_idx, true, nullptr, nullptr);
        } else {
            pos_cursor->n = r_n;
            pos_cursor->seg_idx = ml_elem_cnt - m_seg_work.ca.elem_cnt;
            pos_cursor->elem = ret_elem = circular_array::Access(
                r_seg_work.ca, pos_cursor->seg_idx, true, nullptr, nullptr);
        }

        if (dst_cursor == nullptr) { return ret_elem; }

        dst_cursor->cntr = &cntr;
        dst_cursor->idx = end_idx;

        if (ml_elem_cnt + cnt < m_seg_work.ca.elem_cnt) {
            dst_cursor->n = m_n;
            dst_cursor->seg_idx = ml_elem_cnt + cnt;
            dst_cursor->elem = circular_array::Access(
                m_seg_work.ca, dst_cursor->seg_idx, true, nullptr, nullptr);
        } else {
            dst_cursor->n = r_n;
            dst_cursor->seg_idx = ml_elem_cnt + cnt - m_seg_work.ca.elem_cnt;
            dst_cursor->elem = circular_array::Access(
                r_seg_work.ca, dst_cursor->seg_idx, true, nullptr, nullptr);
        }

        return ret_elem;
    }

    SPLIT_M: {
        /*

        the aim is to transform the insertion point to be between segments.

        case 0: push ml to l
        case 1: push mr to r
        case 2: insert a seg between l and m, push ml to new seg
        case 3: insert a seg between m and r, push mr to new seg

        */

        switch (static_cast<int>(ml_elem_cnt <= l_seg_work.elem_vac) * 0b10 +
                static_cast<int>(mr_elem_cnt <= r_seg_work.elem_vac) * 0b01) {
        case 0b01: goto SPLIT_M__SHOVE_MR_TO_R;
        case 0b10: goto SPLIT_M__SHOVE_ML_TO_L;
        case 0b11:
            switch (({
                size_t l_ml_elem_cnt{ l_seg_work.ca.elem_cnt + ml_elem_cnt };
                size_t mr_r_elem_cnt{ mr_elem_cnt + r_seg_work.ca.elem_cnt };

                utils::Choose2(l_ml_elem_cnt <= mr_r_elem_cnt,
                               mr_r_elem_cnt <= l_ml_elem_cnt, &random_seed) %
                    2;
            })) {
            case 0: goto SPLIT_M__SHOVE_ML_TO_L;
            case 1: goto SPLIT_M__SHOVE_MR_TO_R;
            default: __builtin_unreachable();
            }
        case 0b00:
            switch (utils::Choose2(ml_elem_cnt <= mr_elem_cnt,
                                   mr_elem_cnt <= ml_elem_cnt, &random_seed) %
                    2) {
            case 0: goto SPLIT_M__SPLIT_ML;
            case 1: goto SPLIT_M__SPLIT_MR;
            default: __builtin_unreachable();
            }
        default: __builtin_unreachable();
        }

    SPLIT_M__SHOVE_ML_TO_L: {
        detail::SegShoveL_(cntr_work, l_seg_work, m_seg_work, ml_elem_cnt);

        r_n = m_n;
        r_seg_work.CopyFromSameBasics(m_seg_work);

        m_n = nullptr;
        m_seg_work.SetNull();

        goto INSERT_BETWEEN_L_R;
    }

    SPLIT_M__SHOVE_MR_TO_R: {
        detail::SegShoveR_(cntr_work, m_seg_work, r_seg_work, mr_elem_cnt);

        l_n = m_n;
        l_seg_work.CopyFromSameBasics(m_seg_work);

        m_n = nullptr;
        m_seg_work.SetNull();

        goto INSERT_BETWEEN_L_R;
    }

    SPLIT_M__SPLIT_ML: {
#if EnStaging
        if (m_seg_work.color == ref_color) {
            Seg* new_l_seg{ detail::AllocateRefSeg_(cntr_work.seg_alctr) };

            cntr.root = rbtree::Insert(l_n, m_n, &new_l_seg->n);

            l_n = &new_l_seg->n;
            l_seg_work.is_null = false;
            l_seg_work.is_dirty = true;
            l_seg_work.seg = new_l_seg;
            l_seg_work.color = ref_color;
            l_seg_work.ref_beg = m_seg_work.ref_beg;
            l_seg_work.ca.elem_cnt = ml_elem_cnt;
            l_seg_work.elem_vac =
                cntr_work.seg_elem_slot_cnt -
                compare_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                        cntr_work.seg_elem_slot_cnt);

            r_n = m_n;
            r_seg_work.is_null = false;
            r_seg_work.is_dirty = true;
            r_seg_work.seg = m_seg_work.seg;
            r_seg_work.color = ref_color;
            r_seg_work.ref_beg = m_seg_work.ref_beg + ml_elem_cnt;
            r_seg_work.ca.elem_cnt = mr_elem_cnt;
            r_seg_work.elem_vac =
                cntr_work.seg_elem_slot_cnt -
                compare_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                        cntr_work.seg_elem_slot_cnt);
        } else
#endif
        {

            Seg* new_l_seg{ detail::AllocateDatSeg_(cntr_work.data_size,
                                                    cntr_work.seg_alctr,
                                                    cntr_work.data_alctr) };

            cntr.root = rbtree::Insert(l_n, m_n, &new_l_seg->n);

            l_n = &new_l_seg->n;
            l_seg_work.Load(new_l_seg);

            r_n = m_n;
            r_seg_work.CopyFromSameBasics(m_seg_work);

            detail::SegShoveL_(cntr_work, l_seg_work, r_seg_work, ml_elem_cnt);
        }

        goto INSERT_BETWEEN_L_R;
    }

    SPLIT_M__SPLIT_MR: {
#if EnStaging
        if (m_seg_work.color == ref_color) {
            Seg* new_r_seg{ detail::AllocateRefSeg_(cntr_work.seg_alctr) };

            cntr.root = rbtree::Insert(m_n, r_n, &new_r_seg->n);

            l_n = m_n;
            l_seg_work.is_null = false;
            l_seg_work.is_dirty = true;
            l_seg_work.seg = m_seg_work.seg;
            l_seg_work.color = ref_color;
            l_seg_work.ref_beg = m_seg_work.ref_beg;
            l_seg_work.ca.elem_cnt = ml_elem_cnt;
            l_seg_work.elem_vac =
                cntr_work.seg_elem_slot_cnt -
                compare_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                        cntr_work.seg_elem_slot_cnt);

            r_n = &new_r_seg->n;
            r_seg_work.is_null = false;
            r_seg_work.is_dirty = true;
            r_seg_work.seg = new_r_seg;
            r_seg_work.color = ref_color;
            r_seg_work.ref_beg = m_seg_work.ref_beg + ml_elem_cnt;
            r_seg_work.ca.elem_cnt = mr_elem_cnt;
            r_seg_work.elem_vac =
                cntr_work.seg_elem_slot_cnt -
                compare_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                        cntr_work.seg_elem_slot_cnt);
        } else
#endif
        {

            Seg* new_r_seg{ detail::AllocateDatSeg_(cntr_work.data_size,
                                                    cntr_work.seg_alctr,
                                                    cntr_work.data_alctr) };

            cntr.root = rbtree::Insert(m_n, r_n, &new_r_seg->n);

            l_n = m_n;
            l_seg_work.CopyFromSameBasics(m_seg_work);

            r_n = &new_r_seg->n;
            r_seg_work.Load(new_r_seg);

            detail::SegShoveR_(cntr_work, l_seg_work, r_seg_work, mr_elem_cnt);
        }

        goto INSERT_BETWEEN_L_R;
    }
    }
    }

INSERT_BETWEEN_L_R:;

    /*

    the insertion point is between segments now. try to insert new
    segments to hold more data if necessary. consider use l and r to
    hold partial data if segments usage can be reduced.

    */

    size_t extra_res_cnt{ cnt % cntr_work.seg_elem_slot_cnt };
    size_t extra_vac_cnt{ extra_res_cnt == 0
                              ? 0
                              : cntr_work.seg_elem_slot_cnt - extra_res_cnt };

    bool l_shove;
    bool r_shove;

    bool l_shove_is_ok{ !l_seg_work.is_null &&
                        l_seg_work.ca.elem_cnt <= extra_vac_cnt };

    bool r_shove_is_ok{ !r_seg_work.is_null &&
                        r_seg_work.ca.elem_cnt <= extra_vac_cnt };

    if (!l_shove_is_ok && !r_shove_is_ok) {
        l_shove = false;
        r_shove = false;
    } else if (l_shove_is_ok != r_shove_is_ok) {
        l_shove = l_shove_is_ok;
        r_shove = r_shove_is_ok;
    } else if (l_seg_work.ca.elem_cnt + r_seg_work.ca.elem_cnt <=
               extra_vac_cnt) {
        l_shove = true;
        r_shove = true;
    } else {
        switch (utils::Choose2(l_seg_work.ca.elem_cnt <= r_seg_work.ca.elem_cnt,
                               r_seg_work.ca.elem_cnt <= l_seg_work.ca.elem_cnt,
                               &random_seed) %
                2) {
        case 0:
            l_shove = true;
            r_shove = false;
            break;

        case 1:
            l_shove = false;
            r_shove = true;
            break;
        }
    }

    detail::ElemCntBalancer_ balancer{
        (l_shove ? l_seg_work.ca.elem_cnt : 0) + cnt +
            (r_shove ? r_seg_work.ca.elem_cnt : 0),
        cntr_work.seg_elem_slot_cnt
    };

    bool pos_cursor_is_set{ false };
    void* ret_elem;

    if (l_shove) {
        size_t new_l_elem_cnt{ balancer.Fetch() };

#if EnStaging
        if (l_seg_work.color == ref_color) {
            detail::MaterializeRefSeg_(cntr_work, l_seg_work);
        }
#endif

        l_seg_work.is_dirty = true;

        pos_cursor->n = l_n;
        pos_cursor->seg_idx = l_seg_work.ca.elem_cnt;
        pos_cursor->elem = ret_elem = circular_array::PushR(
            l_seg_work.ca, new_l_elem_cnt - l_seg_work.ca.elem_cnt, writer,
            nullptr);

        pos_cursor_is_set = true;
    }

    if (!l_seg_work.is_null && l_seg_work.is_dirty) { l_seg_work.Store(); }

    {
        Node* prv_n{ l_n };

        while ((r_shove ? 1 : 0) < balancer.res_seg_cnt) {
            size_t cur_cnt{ balancer.Fetch() };

            Seg* new_seg{ detail::AllocateDatSeg_(cntr_work.data_size,
                                                  cntr_work.seg_alctr,
                                                  cntr_work.data_alctr) };

            cntr.root = rbtree::Insert(prv_n, r_n, &new_seg->n);

            writer(new_seg->dat.data, cntr_work.elem_stride, cur_cnt);

            new_seg->dat.elem_cnt = static_cast<unsigned short>(cur_cnt);

            bin_tree::AddDiffSize(&new_seg->n, cur_cnt);

            prv_n = &new_seg->n;

            if (!pos_cursor_is_set) {
                pos_cursor->n = &new_seg->n;
                pos_cursor->seg_idx = 0;
                pos_cursor->elem = ret_elem = new_seg->dat.data;

                pos_cursor_is_set = true;
            }
        }
    }

    size_t old_r_elem_cnt{ r_seg_work.ca.elem_cnt };

    if (r_shove) {
        size_t new_r_elem_cnt{ balancer.Fetch() };

#if EnStaging
        if (r_seg_work.color == ref_color) {
            detail::MaterializeRefSeg_(cntr_work, r_seg_work);
        }
#endif

        r_seg_work.is_dirty = true;

        void* tmp_elem{ circular_array::PushL(
            r_seg_work.ca, new_r_elem_cnt - r_seg_work.ca.elem_cnt, writer,
            nullptr) };

        if (!pos_cursor_is_set) {
            pos_cursor->n = r_n;
            pos_cursor->seg_idx = 0;
            pos_cursor->elem = ret_elem = tmp_elem;
            pos_cursor_is_set = true;
        }
    }

    if (!r_seg_work.is_null && r_seg_work.is_dirty) { r_seg_work.Store(); }

    if (dst_cursor == nullptr) { return ret_elem; }

    dst_cursor->cntr = &cntr;
    dst_cursor->n = r_n;
    dst_cursor->idx = end_idx;
    dst_cursor->seg_idx = r_seg_work.ca.elem_cnt - old_r_elem_cnt;
    dst_cursor->elem =
        r_seg_work.is_null
#if EnStaging
                || r_seg_work.color == ref_color
#endif
            ? nullptr
            : circular_array::Access(r_seg_work.ca, dst_cursor->seg_idx, true,
                                     nullptr, nullptr);

    return ret_elem;
}

template <CntrTplParamList>
void Namespace::PopL(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    Cursor pos_cursor;
    (PeekL)(cntr, true, &pos_cursor, nullptr);

    (Erase)(cntr, &pos_cursor, cnt);
}

template <CntrTplParamList>
void Namespace::PopR(Cntr<CntrTplArgList>& cntr, size_t cnt) {
    size_t elem_cnt{ GetElemCnt(cntr) };

    ZETA_Core_DebugAssert(cnt <= elem_cnt);

    Cursor pos_cursor;
    (Access)(cntr, elem_cnt - cnt, true, &pos_cursor, nullptr);

    (Erase)(cntr, &pos_cursor, cnt);
}

template <CntrTplParamList>
void Namespace::Erase(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor,
                      size_t cnt) {
    detail::CheckCursor_(cntr, pos_cursor);

    if (cnt == 0) { return; }

    auto const cntr_work{ detail::MakeCntrWork_(cntr) };

    unsigned long long random_seed{ utils::GetRandom() };

    ZETA_Core_DebugAssert(
        seq_cntr::IsErasable(pos_cursor->idx, cnt, cntr_work.elem_cnt));

    Node* m_n{ static_cast<Node*>(pos_cursor->n) };
    size_t seg_idx{ pos_cursor->seg_idx };

    detail::SegWork_ seg_works[5];
    // The last nullptr is used for termination.
    size_t seg_work_cnt{ 0 };

    for (int i{ 0 }; i < 5; ++i) { seg_works[i].SetBasics(cntr_work); }

    Node* l_n{ bin_tree::StepL(m_n) };
    Node* r_n;

    if (cntr.lb != l_n) {
        seg_works[seg_work_cnt++].Load(detail::NToSeg_(l_n));
    }

    seg_works[seg_work_cnt++].Load(detail::NToSeg_(m_n));

    if (seg_idx + cnt < seg_works[seg_work_cnt - 1].ca.elem_cnt) {
        detail::SegWork_ l_seg_work;

        detail::SegWork_ m_seg_work{ seg_works[seg_work_cnt - 1] };

        detail::SegWork_ r_seg_work;

        if (seg_work_cnt < 2) {
            l_seg_work.SetBasics(cntr_work);
            l_seg_work.SetNull();
        } else {
            l_seg_work = seg_works[seg_work_cnt - 2];
        }

        r_n = bin_tree::StepR(m_n);

        r_seg_work.SetBasics(cntr_work);

        if (cntr.rb == r_n) {
            r_seg_work.SetNull();
        } else {
            r_seg_work.Load(detail::NToSeg_(r_n));
        }

        size_t ml_elem_cnt{ seg_idx };
        size_t mr_elem_cnt{ m_seg_work.ca.elem_cnt - seg_idx - cnt };

        switch (({
            bool l_m_is_ok{ !l_seg_work.is_null &&
                            l_seg_work.ca.elem_cnt + ml_elem_cnt +
                                    mr_elem_cnt <=
                                cntr_work.seg_elem_slot_cnt };

            bool m_r_is_ok{ !r_seg_work.is_null &&
                            ml_elem_cnt + mr_elem_cnt +
                                    r_seg_work.ca.elem_cnt <=
                                cntr_work.seg_elem_slot_cnt };

            static_cast<int>(l_m_is_ok) * 0b10 +
                static_cast<int>(m_r_is_ok) * 0b01;
        })) {
        case 0b00: goto INTER_M_ERASE__M_ERASE;
        case 0b01: goto INTER_M_ERASE__M_R_ERASE_MERGE;
        case 0b10: goto INTER_M_ERASE__L_M_ERASE_MERGE;
        }

        switch (({
            size_t l_ml_elem_cnt{ l_seg_work.ca.elem_cnt + ml_elem_cnt };
            size_t mr_r_elem_cnt{ mr_elem_cnt + r_seg_work.ca.elem_cnt };

            utils::Choose2(l_ml_elem_cnt <= mr_r_elem_cnt,
                           mr_r_elem_cnt <= l_ml_elem_cnt, &random_seed) %
                2;
        })) {
        case 0: goto INTER_M_ERASE__L_M_ERASE_MERGE;
        case 1: goto INTER_M_ERASE__M_R_ERASE_MERGE;
        default: __builtin_unreachable();
        }

    INTER_M_ERASE__L_M_ERASE_MERGE: {
        switch (({
            size_t l_based_merge_cost{ EnStagingTernary(
                                           l_seg_work.color == ref_color,
                                           l_seg_work.ca.elem_cnt, 0) +
                                       ml_elem_cnt + mr_elem_cnt };

            size_t m_based_merge_cost{
                l_seg_work.ca.elem_cnt +
                EnStagingTernary(
                    m_seg_work.color == ref_color, ml_elem_cnt + mr_elem_cnt,
                    compare_utils::BasicMin(ml_elem_cnt, mr_elem_cnt))
            };

            utils::Choose2(l_based_merge_cost <= m_based_merge_cost,
                           m_based_merge_cost <= l_based_merge_cost,
                           &random_seed);
        })) {
        case 0: goto INTER_M_ERASE__L_M_ERASE_MERGE__L_BASED;
        case 1: goto INTER_M_ERASE__L_M_ERASE_MERGE__M_BASED;
        default: __builtin_unreachable();
        }

    INTER_M_ERASE__L_M_ERASE_MERGE__L_BASED: {
        detail::SegEraseShoveL_(cntr_work, l_seg_work, m_seg_work, ml_elem_cnt,
                                cnt, ml_elem_cnt + mr_elem_cnt);

        cntr.root = rbtree::Extract(m_n);
        allocator::Deallocate(cntr_work.seg_alctr, m_seg_work.seg);
        m_seg_work.TryDeallocateData(cntr_work.data_alctr);

        l_seg_work.Store();

        pos_cursor->n = l_n;
        pos_cursor->seg_idx = l_seg_work.ca.elem_cnt - mr_elem_cnt;

        pos_cursor->elem = circular_array::Access(
            l_seg_work.ca, pos_cursor->seg_idx, true, nullptr, nullptr);

        return;
    }

    INTER_M_ERASE__L_M_ERASE_MERGE__M_BASED: {
#if EnStaging
        if (m_seg_work.color == ref_color) {
            detail::AugMaterializeRefSeg_(cntr_work, m_seg_work, ml_elem_cnt, 0,
                                          mr_elem_cnt, seq_cntr::EmptyWriter{});
        } else
#endif
        {
            circular_array::IdxErase(m_seg_work.ca, ml_elem_cnt, cnt);
        }

        detail::SegShoveR_(cntr_work, l_seg_work, m_seg_work,
                           l_seg_work.ca.elem_cnt);

        cntr.root = rbtree::Extract(l_n);
        l_seg_work.TryDeallocateData(cntr_work.data_alctr);
        allocator::Deallocate(cntr_work.seg_alctr, l_seg_work.seg);

        m_seg_work.Store();

        pos_cursor->n = m_n;

        pos_cursor->seg_idx = m_seg_work.ca.elem_cnt - mr_elem_cnt;

        pos_cursor->elem = circular_array::Access(
            m_seg_work.ca, pos_cursor->seg_idx, true, nullptr, nullptr);

        return;
    }
    }

    INTER_M_ERASE__M_R_ERASE_MERGE: {
        switch (({
            size_t m_based_merge_cost{
                EnStagingTernary(
                    m_seg_work.color == ref_color, ml_elem_cnt + mr_elem_cnt,
                    compare_utils::BasicMin(ml_elem_cnt, mr_elem_cnt)) +
                r_seg_work.ca.elem_cnt
            };

            size_t r_based_merge_cost{ EnStagingTernary(
                                           r_seg_work.color == ref_color,
                                           r_seg_work.ca.elem_cnt, 0) +
                                       ml_elem_cnt + mr_elem_cnt };

            utils::Choose2(m_based_merge_cost <= r_based_merge_cost,
                           r_based_merge_cost <= m_based_merge_cost,
                           &random_seed);
        })) {
        case 0: goto INTER_M_ERASE__M_R_ERASE_MERGE__M_BASED;
        case 1: goto INTER_M_ERASE__M_R_ERASE_MERGE__R_BASED;
        default: __builtin_unreachable();
        }

    INTER_M_ERASE__M_R_ERASE_MERGE__M_BASED: {
#if EnStaging
        if (m_seg_work.color == ref_color) {
            detail::AugMaterializeRefSeg_(cntr_work, m_seg_work, ml_elem_cnt, 0,
                                          mr_elem_cnt, seq_cntr::EmptyWriter{});
        } else
#endif
        {
            circular_array::IdxErase(m_seg_work.ca, seg_idx, cnt);
        }

        detail::SegShoveL_(cntr_work, m_seg_work, r_seg_work,
                           r_seg_work.ca.elem_cnt);

        cntr.root = rbtree::Extract(r_n);
        allocator::Deallocate(cntr_work.seg_alctr, r_seg_work.seg);
        r_seg_work.TryDeallocateData(cntr_work.data_alctr);

        m_seg_work.Store();

        pos_cursor->n = m_n;

        pos_cursor->seg_idx = ml_elem_cnt;

        pos_cursor->elem = circular_array::Access(
            m_seg_work.ca, pos_cursor->seg_idx, true, nullptr, nullptr);

        return;
    }

    INTER_M_ERASE__M_R_ERASE_MERGE__R_BASED: {
        detail::SegEraseShoveR_(cntr_work, m_seg_work, r_seg_work, mr_elem_cnt,
                                cnt, ml_elem_cnt + mr_elem_cnt);

        cntr.root = rbtree::Extract(m_n);
        allocator::Deallocate(cntr_work.seg_alctr, m_seg_work.seg);
        m_seg_work.TryDeallocateData(cntr_work.data_alctr);

        r_seg_work.Store();

        pos_cursor->n = r_n;

        pos_cursor->seg_idx = ml_elem_cnt;

        pos_cursor->elem = circular_array::Access(
            r_seg_work.ca, pos_cursor->seg_idx, true, nullptr, nullptr);

        return;
    }
    }

    INTER_M_ERASE__M_ERASE: {
#if EnStaging
        if (m_seg_work.color != ref_color)
#endif
        {
            circular_array::IdxErase(m_seg_work.ca, ml_elem_cnt, cnt);

            m_seg_work.Store();

            pos_cursor->elem = circular_array::Access(
                m_seg_work.ca, ml_elem_cnt, true, nullptr, nullptr);

            return;
        }

#if EnStaging
        switch (static_cast<int>(ml_elem_cnt == 0) * 0b10 +
                static_cast<int>(mr_elem_cnt == 0) * 0b01) {
        case 0b11:
            cntr.root = rbtree::Extract(m_n);
            detail::DeallocateRefSeg_(m_seg_work.seg, cntr_work.seg_alctr);

            pos_cursor->n = r_n;
            pos_cursor->seg_idx = 0;
            pos_cursor->elem = nullptr;

            return;

        case 0b10:
            m_seg_work.ref_beg += cnt;
            m_seg_work.ca.elem_cnt = mr_elem_cnt;
            m_seg_work.Store();
            return;

        case 0b01:
            m_seg_work.ca.elem_cnt = ml_elem_cnt;
            m_seg_work.Store();
            return;

        case 0b00: break;

        default: __builtin_unreachable();
        }

        Seg* new_seg{ detail::AllocateRefSeg_(cntr_work.seg_alctr) };

        if (utils::SimpleRandomRotate(&random_seed) % 2 == 0) {
            new_seg->ref.beg = m_seg_work.ref_beg;
            new_seg->ref.elem_cnt = ml_elem_cnt;

            m_seg_work.ref_beg += ml_elem_cnt + cnt;
            m_seg_work.ca.elem_cnt = mr_elem_cnt;

            cntr.root = rbtree::Insert(l_n, m_n, &new_seg->n);

            pos_cursor->seg_idx = 0;
        } else {
            new_seg->ref.beg = m_seg_work.ref_beg + ml_elem_cnt + cnt;
            new_seg->ref.elem_cnt = mr_elem_cnt;

            m_seg_work.ca.elem_cnt = ml_elem_cnt;

            cntr.root = rbtree::Insert(m_n, r_n, &new_seg->n);

            pos_cursor->n = &new_seg->n;
            pos_cursor->seg_idx = 0;
        }

        bin_tree::AddDiffSize(&new_seg->n, new_seg->ref.elem_cnt);
        m_seg_work.Store();

        return;
#endif
    }
    }

    if (seg_idx == 0) {
        --seg_work_cnt;
    } else {
        detail::SegWork_& m_seg_work{ seg_works[seg_work_cnt - 1] };

        cnt -= m_seg_work.ca.elem_cnt - seg_idx;

        m_seg_work.is_dirty = true;
        m_seg_work.ca.elem_cnt = seg_idx;

        m_n = bin_tree::StepR(m_n);
        seg_idx = 0;
    }

    while (0 < cnt) {
        Seg* m_seg{ detail::NToSeg_(m_n) };

#if EnStaging
        if (detail::GetNColor_(m_n) == ref_color) {
            if (m_seg->ref.elem_cnt <= cnt) {
                cnt -= m_seg->ref.elem_cnt;

                Node* nxt_m_n{ bin_tree::StepR(m_n) };

                cntr.root = rbtree::Extract(m_n);
                detail::DeallocateRefSeg_(m_seg, cntr_work.seg_alctr);

                m_n = nxt_m_n;

                continue;
            }

            detail::SegWork_& seg_work{ seg_works[seg_work_cnt++] };
            seg_work.Load(m_seg);

            seg_work.is_dirty = true;
            seg_work.ref_beg += cnt;
            seg_work.ca.elem_cnt -= cnt;

            break;
        }
#endif

        if (m_seg->dat.elem_cnt <= cnt) {
            cnt -= m_seg->dat.elem_cnt;

            Node* nxt_m_n{ bin_tree::StepR(m_n) };

            cntr.root = rbtree::Extract(m_n);
            detail::DeallocateDatSeg_(m_seg, cntr_work.seg_alctr,
                                      cntr_work.data_alctr);

            m_n = nxt_m_n;

            continue;
        }

        detail::SegWork_& seg_work{ seg_works[seg_work_cnt++] };
        seg_work.Load(m_seg);

        seg_work.is_dirty = true;
        circular_array::IdxErase(seg_work.ca, 0, cnt);

        break;
    }

    if (0 < seg_work_cnt && &seg_works[seg_work_cnt - 1].seg->n == m_n) {
        Node* nxt_m_n{ bin_tree::StepR(m_n) };

        if (cntr.rb != nxt_m_n) {
            detail::SegWork_& seg_work{ seg_works[seg_work_cnt++] };
            seg_work.Load(detail::NToSeg_(nxt_m_n));
        }
    } else if (cntr.rb != m_n) {
        Seg* m_seg{ detail::NToSeg_(m_n) };

#if EnStaging
        if (0 < seg_work_cnt && detail::GetNColor_(m_n) == ref_color &&
            seg_works[seg_work_cnt - 1].ref_beg +
                    seg_works[seg_work_cnt - 1].ca.elem_cnt ==
                m_seg->ref.beg) {
            size_t old_l_elem_cnt{ seg_works[0].ca.elem_cnt };

            Seg* dst_seg;
            Seg* other_seg;

            switch (utils::SimpleRandomRotate(&random_seed) % 2) {
            case 0:
                dst_seg = seg_works[0].seg;
                other_seg = m_seg;
                break;
            case 1:
                dst_seg = m_seg;
                other_seg = seg_works[0].seg;
                break;
            default: __builtin_unreachable();
            }

            seg_works[0].is_dirty = true;
            seg_works[0].seg = dst_seg;
            seg_works[0].ca.elem_cnt += m_seg->ref.beg;

            cntr.root = rbtree::Extract(&other_seg->n);
            detail::DeallocateRefSeg_(other_seg, cntr_work.seg_alctr);

            pos_cursor->n = &seg_works[0].seg->n;
            pos_cursor->seg_idx = old_l_elem_cnt;
            pos_cursor->elem = nullptr;

            return;
        }
#endif

        if (cntr.rb != m_n) {
            detail::SegWork_& seg_work{ seg_works[seg_work_cnt++] };
            seg_work.Load(m_seg);
        }
    }

    seg_works[seg_work_cnt].seg = nullptr;  // for termination

    Node* ret_n{ m_n };
    size_t ret_seg_idx{ 0 };

    auto merge_then_update_ret{ [&](int i, int j) {
        size_t old_seg_elem_cnt{ seg_works[i].ca.elem_cnt };

        size_t old_total_elem_cnt{ seg_works[i].ca.elem_cnt +
                                   seg_works[j].ca.elem_cnt };

        switch (detail::Merge2_(cntr_work, seg_works[i], seg_works[j])) {
        case 0:
            if (ret_n == &seg_works[i].seg->n) { ret_n = &seg_works[j].seg->n; }
            break;
        case 1:
            if (ret_n == &seg_works[j].seg->n) {
                ret_n = &seg_works[i].seg->n;
                ret_seg_idx += old_seg_elem_cnt;
            }
            break;
        default: __builtin_unreachable();
        }

        size_t new_total_elem_cnt{ seg_works[i].ca.elem_cnt +
                                   seg_works[j].ca.elem_cnt };

        ZETA_Core_DebugAssert(new_total_elem_cnt == old_total_elem_cnt);
        ZETA_Core_DebugAssert(seg_works[i].ca.elem_cnt == 0 ||
                              seg_works[j].ca.elem_cnt == 0);
    } };

    if (seg_work_cnt <= 1) {
    } else if (seg_work_cnt == 2) {
        size_t elem_vac_01{ seg_works[0].elem_vac + seg_works[1].elem_vac };

        if (cntr_work.seg_elem_slot_cnt <= elem_vac_01) {
            merge_then_update_ret(0, 1);
        }
    } else if (seg_work_cnt == 3) {
        size_t elem_vac_01{ seg_works[0].elem_vac + seg_works[1].elem_vac };
        size_t elem_vac_12{ seg_works[1].elem_vac + seg_works[2].elem_vac };

        size_t max_elem_vac{ compare_utils::BasicMax(elem_vac_01,
                                                     elem_vac_12) };

        if (cntr_work.seg_elem_slot_cnt <= max_elem_vac) {
            int k{ utils::Choose2(elem_vac_01 == max_elem_vac,
                                  elem_vac_12 == max_elem_vac, &random_seed) };

            merge_then_update_ret(k, k + 1);
        }
    } else if (seg_work_cnt == 4) {
        size_t elem_vac_01{ seg_works[0].elem_vac + seg_works[1].elem_vac };
        size_t elem_vac_12{ seg_works[1].elem_vac + seg_works[2].elem_vac };
        size_t elem_vac_23{ seg_works[2].elem_vac + seg_works[3].elem_vac };

        if (cntr_work.seg_elem_slot_cnt <= elem_vac_01 &&
            cntr_work.seg_elem_slot_cnt <= elem_vac_23) {
            merge_then_update_ret(0, 1);
            merge_then_update_ret(2, 3);
        } else {
            size_t max_elem_vac{ compare_utils::BasicMax(
                elem_vac_01, elem_vac_12, elem_vac_23) };

            if (cntr_work.seg_elem_slot_cnt <= max_elem_vac) {
                int k{ utils::Choose3(
                    elem_vac_01 == max_elem_vac, elem_vac_12 == max_elem_vac,
                    elem_vac_23 == max_elem_vac, &random_seed) };

                merge_then_update_ret(k, k + 1);
            }
        }
    }

    for (int i{ 0 }; seg_works[i].seg != nullptr; ++i) {
        if (seg_works[i].ca.elem_cnt == 0) {
            cntr.root = rbtree::Extract(&seg_works[i].seg->n);
            allocator::Deallocate(cntr_work.seg_alctr, seg_works[i].seg);
            seg_works[i].TryDeallocateData(cntr_work.data_alctr);
        }
    }

    for (int i{ 0 }; seg_works[i].seg != nullptr; ++i) {
        if (0 < seg_works[i].ca.elem_cnt && seg_works[i].is_dirty) {
            seg_works[i].Store();
        }
    }

    pos_cursor->n = ret_n;
    pos_cursor->seg_idx = ret_seg_idx;

    if (cntr.rb == ret_n) {
        pos_cursor->elem = nullptr;
    } else {
        for (int i{ 0 }; seg_works[i].seg != nullptr; ++i) {
            if (&seg_works[i].seg->n == pos_cursor->n) {
                pos_cursor->elem = EnStagingTernary(
                    seg_works[i].color == ref_color, nullptr,
                    circular_array::Access(seg_works[i].ca, ret_seg_idx, true,
                                           nullptr, nullptr));

                break;
            }
        }
    }
}

template <CntrTplParamList>
void Namespace::EraseAll(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    detail::Erase_(cntr.root, cntr.seg_alctr, cntr.data_alctr);

    detail::InitTree_(cntr);
}

template <CntrTplParamList>
void Namespace::Reset(Cntr<CntrTplArgList>& cntr) {
    detail::CheckCntr_(cntr);

    detail::Erase_(cntr.root, cntr.seg_alctr, cntr.data_alctr);

    detail::InitTree_(cntr);

#if EnStaging
    detail::RefOrigin_(cntr);
#endif
}

#if EnStaging

template <CntrTplParamList, typename OriginOriginLike,
          typename OriginSegAllocator, typename OriginDataAllocator,
          typename NewOriginLikeInitArg>
void Namespace::Collapse(Cntr<CntrTplArgList>& cntr,
                         Cntr<OriginOriginLike, OriginSegAllocator,
                              OriginDataAllocator> const& origin_cntr,
                         NewOriginLikeInitArg&& new_origin_like_init_arg) {
    detail::CheckCntr_(cntr);

    auto& origin{ meta::GetInstRef(cntr.origin) };

    ZETA_Core_DebugAssert(seq_cntr::GetReferedInstPtr(origin) ==
                          seq_cntr::GetReferedInstPtr(origin_cntr));

    ZETA_Core_PrintVar(seq_cntr::GetReferedInstPtr(origin));
    ZETA_Core_PrintVar(seq_cntr::GetReferedInstPtr(origin_cntr));

    lifecycle::Deinit(origin);

    lifecycle::Init(
        origin, meta::Forward<NewOriginLikeInitArg>(new_origin_like_init_arg));

    ZETA_Core_PrintVar(seq_cntr::GetReferedInstPtr(origin));
    ZETA_Core_PrintVar(
        seq_cntr::GetReferedInstPtr(meta::GetInstRef(origin_cntr.origin)));

    ZETA_Core_DebugAssert(
        seq_cntr::GetReferedInstPtr(origin) ==
        seq_cntr::GetReferedInstPtr(meta::GetInstRef(origin_cntr.origin)));

    auto const cntr_work{ detail::MakeCntrWork_(cntr) };

    Node* n{ bin_tree::StepR(cntr.lb) };

    Cursor origin_cursor;
    (PeekL)(origin_cntr, true, &origin_cursor, nullptr);

    CircularArray origin_ca{
        .data = {},
        .elem_size = origin_cntr.elem_size,
        .elem_stride = origin_cntr.elem_stride,
        .elem_cnt = {},
        .slot_cnt = origin_cntr.seg_elem_slot_cnt,
        .rot = {},
    };

    while (n != cntr.rb) {
        Seg* seg{ detail::NToSeg_(n) };

        if (detail::GetNColor_(n) == dat_color) {
            n = bin_tree::StepR(n);
            continue;
        }

        (CursorAdvanceR)(origin_cntr, &origin_cursor,
                         seg->ref.beg - origin_cursor.idx);

        Node* origin_n{ static_cast<Node*>(origin_cursor.n) };

        size_t origin_seg_idx{ origin_cursor.seg_idx };

        Seg* origin_seg{ detail::NToSeg_(origin_n) };

        unsigned origin_seg_color{ detail::GetNColor_(&origin_seg->n) };

        size_t origin_seg_size{ origin_seg_color == ref_color
                                    ? origin_seg->ref.elem_cnt
                                    : origin_seg->dat.elem_cnt };

        size_t origin_res_size{ origin_seg_size - origin_seg_idx };

        if (origin_seg_color == ref_color) {
            if (seg->ref.elem_cnt <= origin_res_size) {
                seg->ref.beg = origin_seg->ref.beg + origin_seg_idx;
                n = bin_tree::StepR(n);
                continue;
            }

            Seg* new_ref_seg{ detail::AllocateRefSeg_(cntr_work.seg_alctr) };

            new_ref_seg->ref.beg = origin_seg->ref.beg + origin_seg_idx;
            new_ref_seg->ref.elem_cnt = origin_res_size;

            bin_tree::AddDiffSize(&new_ref_seg->n, new_ref_seg->ref.elem_cnt);

            seg->ref.beg += origin_res_size;
            seg->ref.elem_cnt -= origin_res_size;

            bin_tree::AddDiffSize(n, -origin_res_size);

            cntr.root = rbtree::InsertL(n, &new_ref_seg->n);

            continue;
        }

        origin_ca.data = origin_seg->dat.data;
        origin_ca.elem_cnt = origin_seg->dat.elem_cnt;
        origin_ca.rot = origin_seg->dat.rot;

        bool cur_seg_will_exhausted{ seg->ref.elem_cnt <= origin_res_size };

        detail::ElemCntBalancer_ balancer{
            compare_utils::BasicMin(seg->ref.elem_cnt, origin_res_size),
            cntr_work.seg_elem_slot_cnt
        };

        size_t src_seg_idx{ origin_seg_idx };

        size_t old_seg_ref_elem_cnt{ seg->ref.elem_cnt };

        while (0 < balancer.res_seg_cnt) {
            size_t cur_cnt{ balancer.Fetch() };

            void* dst_data;

            if (cur_cnt == seg->ref.elem_cnt) {
                detail::SetNColor_(n, dat_color);

                seg->dat.data = detail::AllocateData_(cntr_work.data_size,
                                                      cntr_work.data_alctr);
                seg->dat.elem_cnt = static_cast<unsigned short>(cur_cnt);
                seg->dat.rot = 0;

                bin_tree::AddDiffSize(n, cur_cnt - old_seg_ref_elem_cnt);

                dst_data = seg->dat.data;
            } else {
                Seg* new_dat_seg{ detail::AllocateDatSeg_(
                    cntr_work.data_size, cntr_work.seg_alctr,
                    cntr_work.data_alctr) };

                new_dat_seg->dat.elem_cnt =
                    static_cast<unsigned short>(cur_cnt);

                bin_tree::AddDiffSize(&new_dat_seg->n, cur_cnt);

                dst_data = new_dat_seg->dat.data;

                seg->ref.beg += cur_cnt;
                seg->ref.elem_cnt -= cur_cnt;

                if (balancer.res_seg_cnt == 0) {
                    bin_tree::AddDiffSize(
                        n, seg->ref.elem_cnt - old_seg_ref_elem_cnt);
                }

                cntr.root = rbtree::InsertL(n, &new_dat_seg->n);
            }

            circular_array::IdxRead(origin_ca, src_seg_idx, cur_cnt,
                                    seq_cntr::MemReader{
                                        .data = dst_data,
                                        .elem_size = cntr_work.elem_size,
                                        .elem_stride = cntr_work.elem_stride,
                                    });

            src_seg_idx += cur_cnt;
        }

        if (cur_seg_will_exhausted) { n = bin_tree::StepR(n); }
    }
}

/*

namespace detail {

struct WBSeg_ {
    size_t beg;
    size_t size;

    size_t dst_idx;
    size_t acc_ref;

    char* data;
    unsigned short offset;
};

struct OffsetCntNode_ {
    generic_hash_table::Node ghtn;

    size_t offset;
    size_t cnt;
};

struct OffsetCntNodeHash_ {
    size_t operator()(generic_hash_table::Node const* ghtn,
                      unsigned long long salt) const {
        OffsetCntNode_* node{ ZETA_Core_MemberToStruct(OffsetCntNode_, ghtn,
                                                       ghtn) };

        return utils::ULLHash(node->offset, salt);
    }
};

struct OffsetCntNodeCompare_ {
    int operator()(generic_hash_table::Node const* a_ghtn,
                   generic_hash_table::Node const* b_ghtn) const {
        return compare::BasicCompare(
            ZETA_Core_MemberToStruct(OffsetCntNode_, ghtn, a_ghtn)->offset,
            ZETA_Core_MemberToStruct(OffsetCntNode_, ghtn, b_ghtn)->offset);
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
    return compare::BasicCompare(
        offset, ZETA_Core_MemberToStruct(OffsetCntNode_, ghtn, ghtn)->offset);
}

using OffsetCntGenericHashTable_ =
    generic_hash_table::Cntr<OffsetCntNodeHash_, OffsetCntNodeCompare_,
                             allocator::Ref>;

template <CntrTplParamList>
size_t RecordOffset_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, Node* n, size_t dst_idx,
     OffsetCntGenericHashTable_* ght) {
    while (n != nullptr) {
        Node* nl{ bin_tree::GetL(n) };
        Node* nr{ bin_tree::GetR(n) };

        if (nl != nullptr) { dst_idx = RecordOffset_(cntr, nl, dst_idx, ght); }

        if (lb == n || rb == n) {
            n = nr;
            continue;
        }

        Seg* seg{ (NToSeg_)(n) };

        if (GetNColor_(n) == dat_color) {
            dst_idx += seg->dat.size;
            n = nr;
            continue;
        }

        size_t offset{ seg->ref.beg - dst_idx };

        generic_hash_table::Node* ghtn{ generic_hash_table::Find(
            ght, &offset, OffsetHash_, OffsetOffsetCntNodeCompare_) };

        OffsetCntNode_* offset_cnt_node;

        if (ghtn == nullptr) {
            offset_cnt_node =
                static_cast<OffsetCntNode_*>(allocator::SafeAllocate(
                    allocator::weak_lifo_allocator, alignof(OffsetCntNode_),
                    sizeof(OffsetCntNode_)));

            ghtn = &offset_cnt_node->ghtn;

            offset_cnt_node->offset = offset;
            offset_cnt_node->cnt = 0;

            ghtn->Init();

            generic_hash_table::Insert(ght, ghtn);
        } else {
            offset_cnt_node =
                ZETA_Core_MemberToStruct(OffsetCntNode_, ghtn, ghtn);
        }

        offset_cnt_node->cnt += seg->ref.elem_cnt;

        dst_idx += seg->ref.elem_cnt;

        n = nr;
    }

    return dst_idx;
}

template <CntrTplParamList>
pair::Pair<size_t, size_t> ToWBSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, Node* n, WBSeg_* dst) {
    Pair<size_t, size_t> ret{ 0, 0 };

    while (n != nullptr) {
        Node* nl{ bin_tree::GetL(n) };
        Node* nr{ bin_tree::GetR(n) };

        auto l_ret{ ToWBSeg_(cntr, nl, dst) };

        dst += l_ret.first + l_ret.second;

        ret.first += l_ret.first;
        ret.second += l_ret.second;

        if (lb == n || rb == n) {
            n = nr;
            continue;
        }

        Seg* seg{ (NToSeg_)(n) };

        if (GetNColor_(n) == ref_color) {
            ++ret.first;

            dst->beg = seg->ref.beg;
            dst->size = seg->ref.elem_cnt;
        } else {
            ++ret.second;

            dst->beg = ZETA_Core_size_max;
            dst->size = seg->dat.size;
            dst->data = seg->dat.data;
            dst->offset = seg->dat.rot;
        }

        allocator::Deallocate(cntr.seg_alctr, seg);

        WBSeg_* prv{ dst - 1 };

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
    (Cntr<CntrTplArgList>& cntr, CircularArray* ca, WBSeg_* wb_segs,
     size_t wb_seg_cnt, size_t dst_offset, size_t ref_offset) {
    auto& origin{ meta::GetInstRef(cntr.origin) };
    DataAllocatorLike* data_alctr{ meta::GetInstPtr(cntr.data_alctr) };

    if (wb_seg_cnt == 0) { return; }

    constexpr size_t buffer_capacity{
        rbtree::recommended_buffer_capacity
    };

    struct {
        size_t wb_seg_lb;
        size_t wb_seg_rb;
    } buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = { 0, wb_seg_cnt };

    while (0 < buffer_i) {
        auto [wb_seg_lb, wb_seg_rb]{ buffer[--buffer_i] };

        if (wb_seg_lb + 1 < wb_seg_rb) {
            size_t wb_seg_mb{ (wb_seg_lb + wb_seg_rb) / 2 };

            WBSeg_* mid_wb_seg{ wb_segs + wb_seg_mb };

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

            seq_cntr::RangeAssign(
                origin, ca, dst_offset + wb_segs->dst_idx, 0, wb_segs->size);

            allocator::Deallocate(data_alctr, ca->data);
        } else {
            size_t cur_dst_idx{ dst_offset + wb_segs->dst_idx };
            size_t cur_ref_beg{ ref_offset + wb_segs->beg };

            if (cur_dst_idx != cur_ref_beg) {
                seq_cntr::RangeAssign(origin, origin, cur_dst_idx,
                                           cur_ref_beg, wb_segs->size);
            }
        }
    }
}

template <CntrTplParamList>
void WriteBack_LR_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, int write_back_strategy,
     unsigned long long cost_coeff_read, unsigned long long cost_coeff_write,
     unsigned long long cost_coeff_insert,
     unsigned long long cost_coeff_erase) {
    auto* origin{ GetInstrPtr(cntr.origin) };

    size_t stride{ cntr.elem_stride };

    size_t size{ GetSize(cntr) };

    size_t origin_size{ seq_cntr::GetSize(origin) };

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

        generic_hash_table::Init(&ght);

        RecordOffset_(cntr, cntr.root, 0, &ght);

        unsigned long long best_cost{ ZETA_Core_ullong_max };

        del_l_cnt = 0;
        del_r_cnt = size - origin_size;

        for (;;) {
            generic_hash_table::Node* ghtn{
generic_hash_table::ExtractAny( &ght) };

            if (ghtn == nullptr) { break; }

            auto* offset_cnt_node{ ZETA_Core_MemberToStruct(OffsetCntNode_,
                                                            ghtn, ghtn) };

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

            if (cur_cost < best_cost ||
                (cur_cost == best_cost && cur_del_l_cnt < del_l_cnt)) {
                best_cost = cur_cost;
                del_l_cnt = cur_del_l_cnt;
                del_r_cnt = cur_del_r_cnt;
            }

            allocator::Deallocate(allocator::weak_lifo_allocator,
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

    size_t seg_cnt{ bin_tree::Count(cntr.root) - 2 };

    auto* wb_segs{ static_cast<WBSeg_*>(allocator::SafeAllocate(
                       allocator::weak_lifo_allocator, alignof(WBSeg_),
                       sizeof(WBSeg_) * (seg_cnt + 1))) +
                   1 };

    WBSeg_* lb_wb_seg{ wb_segs - 1 };

    lb_wb_seg->beg = 0;
    lb_wb_seg->size = 0;

    lb_wb_seg->dst_idx = pop_l_cnt;
    lb_wb_seg->acc_ref = pop_l_cnt;

    ToWBSeg_(cntr, cntr.root, wb_segs);

    CircularArray ca{
        .data = {},
        .elem_size = origin->elem_size,
        .elem_stride = stride,
        .elem_cnt = 0,
        .slot_cnt = cntr.seg_elem_slot_cnt,
        .rot = 0,
    };

    WriteWBSeg_(cntr, ca, wb_segs, seg_cnt, 0, push_l_cnt);

    allocator::Deallocate(allocator::weak_lifo_allocator, lb_wb_seg);

    if (0 < pop_l_cnt) { SeqCntrPopL(origin, pop_l_cnt); }

    if (0 < pop_r_cnt) { SeqCntrPopR(origin, pop_r_cnt); }

    InitTree_(cntr);
    RefOrigin_(cntr);
}

template <CntrTplParamList>
void WriteBack_Random_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, unsigned long long cost_coeff_read,
     unsigned long long cost_coeff_write, unsigned long long cost_coeff_insert,
     unsigned long long cost_coeff_erase) {
    auto* origin{ GetInstrPtr(cntr.origin) };
    size_t origin_size{ SeqCntrGetSize(origin) };

    size_t stride{ cntr.elem_stride };
    size_t size{ GetSize(cntr) };

    size_t seg_cnt{ bin_tree::Count(cntr.root) - 2 };

    auto* wb_segs{ static_cast<WBSeg_*>(allocator::SafeAllocate(
                       allocator::weak_lifo_allocator, alignof(WBSeg_),
                       sizeof(WBSeg_) * (seg_cnt + 2))) +
                   1 };

    WBSeg_* lb_wb_seg{ wb_segs - 1 };

    lb_wb_seg->beg = 0;
    lb_wb_seg->size = 0;

    lb_wb_seg->dst_idx = 0;
    lb_wb_seg->acc_ref = 0;

    auto to_wb_seg_ret{ ToWBSeg_(cntr, cntr.root, wb_segs) };

    {
        size_t check_ref_seg_cnt{ 0 };
        size_t check_dat_seg_cnt{ 0 };

        for (size_t i{ 0 }; i < seg_cnt; ++i) {
            if (wb_segs[i].beg == ZETA_Core_size_max) {
                ++check_dat_seg_cnt;
            } else {
                ++check_ref_seg_cnt;
            }
        }

        ZETA_Core_SanitizeAssert(to_wb_seg_ret.first == check_ref_seg_cnt);
        ZETA_Core_SanitizeAssert(to_wb_seg_ret.second == check_dat_seg_cnt);
    }

    WBSeg_* rb_wb_seg{ wb_segs + seg_cnt };

    rb_wb_seg->beg = origin_size;
    rb_wb_seg->size = 0;

    rb_wb_seg->dst_idx = size;
    rb_wb_seg->acc_ref = (rb_wb_seg - 1)->acc_ref;

    size_t ref_seg_cnt = to_wb_seg_ret.ref_seg_cnt;

    CircularArray ca{
        .data = {},
        .elem_size = origin->elem_size,
        .elem_stride = stride,
        .elem_cnt = 0,
        .slot_cnt = cntr.seg_elem_slot_cnt,
        .rot = 0,
    };

    if (ref_seg_cnt == 0) {
        if (origin_size < size) {
            SeqCntrPopR(origin, size - origin_size);
        } else if (size < origin_size) {
            SeqCntrPushR(origin, origin_size - size, nullptr);
        }

        WriteWBSeg_(cntr, ca, wb_segs, seg_cnt, 0, 0);

        allocator::Deallocate(allocator::weak_lifo_allocator, wb_segs - 1);

        InitTree_(cntr);
        RefOrigin_(cntr);

        return;
    }

    unsigned long long cost_coeff_read_write{ cost_coeff_read +
                                              cost_coeff_write };

    auto* ref_wb_segs{ static_cast<WBSeg_**>(allocator::SafeAllocate(
                           allocator::weak_lifo_allocator, alignof(WBSeg_*),
                           sizeof(WBSeg_*) * (ref_seg_cnt + 2))) +
                       1 };

    ref_wb_segs[-1] = lb_wb_seg;
    ref_wb_segs[ref_seg_cnt] = rb_wb_seg;

    for (size_t i{ 0 }, j{ 0 }; j < ref_seg_cnt; ++i) {
        ZETA_Core_SanitizeAssert(j < ref_seg_cnt);

        WBSeg_* wb_seg{ wb_segs + i };

        if (wb_seg->beg != ZETA_Core_size_max) { ref_wb_segs[j++] = wb_seg; }
    }

    auto* acc_arr{ static_cast<size_t*>(allocator::SafeAllocate(
        allocator::weak_lifo_allocator, alignof(size_t),
        sizeof(size_t) * (ref_seg_cnt + 2))) };

    auto* acc_brr{ static_cast<unsigned long long*>(
        allocator::SafeAllocate(
            allocator::weak_lifo_allocator, alignof(unsigned long long),
            sizeof(unsigned long long) * (ref_seg_cnt + 1))) };

    acc_arr[0] = 0;
    acc_brr[0] = 0;

    size_t prv_ref_end{ 0 };
    size_t prv_dst_end{ 0 };

    for (size_t i{ 1 }; i <= ref_seg_cnt; ++i) {
        WBSeg_* cur_ref_seg{ ref_wb_segs[i - 1] };

        size_t cur_beg{ cur_ref_seg->beg };
        size_t cur_size{ cur_ref_seg->size };
        size_t cur_dst_idx{ cur_ref_seg->dst_idx };

        acc_arr[i] = acc_arr[i - 1] + (cur_dst_idx - prv_dst_end) -
                     (cur_beg - prv_ref_end);
        acc_brr[i] = acc_brr[i - 1] + cost_coeff_read_write * cur_size;

        prv_ref_end = cur_beg + cur_size;
        prv_dst_end = cur_dst_idx + cur_size;
    }

    acc_arr[ref_seg_cnt + 1] = acc_arr[ref_seg_cnt] + (size - prv_dst_end) -
                                (origin_size - prv_ref_end);

#if ZETA_Core_EnableDebug
    auto* dp_best_cost{ static_cast<unsigned long long*>(
        allocator::SafeAllocate(
            allocator::weak_lifo_allocator, alignof(unsigned long long),
            sizeof(unsigned long long) * (ref_seg_cnt + 2))) };

    dp_best_cost[0] = 0;
#endif

    auto* dp_cost{ static_cast<unsigned long long*>(
        allocator::SafeAllocate(
            allocator::weak_lifo_allocator, alignof(unsigned long long),
            sizeof(unsigned long long) * (ref_seg_cnt + 2))) };

    auto* dp_prv{ static_cast<size_t*>(allocator::SafeAllocate(
        allocator::weak_lifo_allocator, alignof(size_t),
        sizeof(size_t) * (ref_seg_cnt + 2))) };

    dp_cost[0] = 0;
    dp_prv[0] = ZETA_Core_size_max;

#if ZETA_Core_EnableDebug
    for (size_t i{ 1 }; i <= ref_seg_cnt + 1; ++i) {
        unsigned long long ans_cost{ ZETA_Core_ullong_max };

        for (size_t j{ i }, j_end{ 0 }; j_end < j--;) {
            size_t sum_arr{ acc_arr[i] - acc_arr[j] };

            unsigned long long cur_cost{ dp_best_cost[j] +
                                         (sum_arr <= ZETA_Core_size_max / 2
                                              ? cost_coeff_erase * sum_arr
                                              : cost_coeff_insert * -sum_arr) +
                                         acc_brr[i - 1] - acc_brr[j] };

            ans_cost = compare_utils::BasicMin(ans_cost, cur_cost);
        }

        dp_best_cost[i] = ans_cost;
    }
#endif

    for (size_t i{ 1 }; i <= ref_seg_cnt + 1; ++i) {
        unsigned long long ans_cost{ ZETA_Core_ullong_max };
        unsigned long long ans_prv{ 0 };

        for (size_t j{ i }, j_end{ compare_utils::BasicMax(8ULL, i) - 8 }; j_end
< j--;) { size_t sum_arr{ acc_arr[i] - acc_arr[j] };

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
        unsigned long long best_cost{ dp_best_cost[ref_seg_cnt + 1] };
        unsigned long long better_cost{ dp_cost[ref_seg_cnt + 1] };

        ZETA_Core_DebugAssert(best_cost <= 512 || better_cost <= best_cost * 2);
    }
#endif

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    unsigned long long check_cost{ 0 };

    for (size_t cur_end{ ref_seg_cnt + 1 }; 0 < cur_end;) {
        size_t cur_beg{ dp_prv[cur_end] };

        WBSeg_* beg_wb_seg{ (ref_wb_segs - 1)[cur_beg] };
        WBSeg_* end_wb_seg{ (ref_wb_segs - 1)[cur_end] };

        size_t src_beg{ beg_wb_seg->beg + beg_wb_seg->size };
        size_t src_end{ end_wb_seg->beg };

        size_t dst_beg{ beg_wb_seg->dst_idx + beg_wb_seg->size };
        size_t dst_end{ end_wb_seg->dst_idx };

        size_t dst_size{ dst_end - dst_beg };
        size_t src_size{ src_end - src_beg };

        {
            WBSeg_* segs_{ beg_wb_seg + 1 };
            size_t seg_cnt_{ static_cast<size_t>(end_wb_seg - beg_wb_seg) -
                              1 };

            size_t check_dst_size{ 0 };

            for (size_t i{ 0 }; i < seg_cnt_; ++i) {
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

        WriteWBSeg_(cntr, ca, beg_wb_seg + 1, end_wb_seg - beg_wb_seg - 1,
                    src_beg - dst_beg, 0);

        if (dst_size < src_size) {
            size_t diff_size{ src_size - dst_size };

            SeqCntrAccess(origin, src_end - diff_size, origin_cursor, nullptr);

            SeqCntrErase(origin, origin_cursor, diff_size);
        }

        cur_end = cur_beg;
    }

    ZETA_Core_SanitizeAssert(check_cost == dp_cost[ref_seg_cnt + 1]);

    allocator::Deallocate(allocator::weak_lifo_allocator, wb_segs - 1);
    allocator::ops ::Deallocate(allocator::weak_lifo_allocator,
                                static_cast<void*>(ref_wb_segs - 1));

    allocator::Deallocate(allocator::weak_lifo_allocator, acc_arr);
    allocator::Deallocate(allocator::weak_lifo_allocator, acc_brr);

#if ZETA_Core_EnableDebug
    allocator::Deallocate(allocator::weak_lifo_allocator, dp_best_cost);
#endif

    allocator::Deallocate(allocator::weak_lifo_allocator, dp_cost);
    allocator::Deallocate(allocator::weak_lifo_allocator, dp_prv);

    InitTree_(cntr);
    RefOrigin_(cntr);
}

}  // namespace detail

template <CntrTplParamList>
void WriteBack(Cntr<CntrTplArgList>& cntr, int write_back_strategy,
               unsigned long long cost_coeff_read,
               unsigned long long cost_coeff_write,
               unsigned long long cost_coeff_insert,
               unsigned long long cost_coeff_erase) {
    detail::CheckCntr_(cntr);

    auto* origin{ GetInstrPtr(cntr.origin) };

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

*/

#endif

template <CntrTplParamList>
void Namespace::CopyCursor(Cntr<CntrTplArgList> const& cntr,
                           Cursor const* src_cursor, Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);
    detail::CheckCursor_(cntr, dst_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool Namespace::AreEqualCursor(Cntr<CntrTplArgList> const& cntr,
                               Cursor const* cursor_a, Cursor const* cursor_b) {
    return (GetCursorIdx)(cntr, cursor_a) == (GetCursorIdx)(cntr, cursor_b);
}

template <CntrTplParamList>
int Namespace::CompareCursor(Cntr<CntrTplArgList> const& cntr,
                             Cursor const* cursor_a, Cursor const* cursor_b) {
    return compare::BasicCompare(compare::compare_type::ThreeWay{},
                                 (GetCursorIdx)(cntr, cursor_a) + 1,
                                 (GetCursorIdx)(cntr, cursor_b) + 1);
}

template <CntrTplParamList>
size_t Namespace::GetCursorDist(Cntr<CntrTplArgList> const& cntr,
                                Cursor const* cursor_a,
                                Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_b) - GetCursorIdx(cntr, cursor_a);
}

template <CntrTplParamList>
size_t Namespace::GetCursorIdx(Cntr<CntrTplArgList> const& cntr,
                               Cursor const* cursor) {
    detail::CheckCursor_(cntr, cursor);

    return cursor->idx;
}

template <CntrTplParamList>
void Namespace::CursorStepL(Cntr<CntrTplArgList> const& cntr, Cursor* cursor) {
    (CursorAdvanceL)(cntr, cursor, 1);
}

template <CntrTplParamList>
void Namespace::CursorStepR(Cntr<CntrTplArgList> const& cntr, Cursor* cursor) {
    (CursorAdvanceR)(cntr, cursor, 1);
}

template <CntrTplParamList>
void Namespace::CursorAdvanceL(Cntr<CntrTplArgList> const& cntr, Cursor* cursor,
                               size_t step) {
    detail::CheckCursor_(cntr, cursor);

    if (step == 0) { return; }

    size_t elem_size{ cntr.elem_size };

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    Node* n{ static_cast<Node*>(cursor->n) };

    size_t n_size;

    n_size = cntr.lb == n || cntr.rb == n ? 1 : ({
        Seg* seg{ detail::NToSeg_(n) };
        EnStagingTernary(detail::GetNColor_(n) == ref_color, seg->ref.elem_cnt,
                         seg->dat.elem_cnt);
    });

    auto [dst_n, dst_seg_idx]{ bin_tree::AdvanceL(
        n, n_size - 1 - cursor->seg_idx + step) };

    ZETA_Core_DebugAssert(dst_n != nullptr);

    cursor->idx -= step;
    cursor->n = dst_n;

    if (cntr.lb == dst_n) {
        cursor->seg_idx = 0;
        cursor->elem = nullptr;
        return;
    }

    Seg* dst_seg{ detail::NToSeg_(dst_n) };

    cursor->seg_idx =
        EnStagingTernary(detail::GetNColor_(dst_n) == ref_color,
                         dst_seg->ref.elem_cnt, dst_seg->dat.elem_cnt) -
        1 - dst_seg_idx;

#if EnStaging
    if (detail::GetNColor_(dst_n) == ref_color) {
        cursor->elem = nullptr;
        return;
    }
#endif

    CircularArray ca{
        .data = dst_seg->dat.data,
        .elem_size = elem_size,
        .elem_stride = cntr.elem_stride,
        .elem_cnt = dst_seg->dat.elem_cnt,
        .slot_cnt = cntr.seg_elem_slot_cnt,
        .rot = dst_seg->dat.rot,
    };

    cursor->elem =
        circular_array::Access(ca, cursor->seg_idx, true, nullptr, nullptr);
}

template <CntrTplParamList>
void Namespace::CursorAdvanceR(Cntr<CntrTplArgList> const& cntr, Cursor* cursor,
                               size_t step) {
    detail::CheckCursor_(cntr, cursor);

    if (step == 0) { return; }

    size_t elem_size{ cntr.elem_size };
    size_t elem_cnt{ (GetElemCnt)(cntr) };

    Node* rb{ cntr.rb };

    ZETA_Core_DebugAssert(step <= elem_cnt - cursor->idx);

    auto [dst_n, dst_seg_idx]{ bin_tree::AdvanceR(static_cast<Node*>(cursor->n),
                                                  cursor->seg_idx + step) };

    ZETA_Core_DebugAssert(dst_n != nullptr);

    cursor->idx += step;
    cursor->n = dst_n;
    cursor->seg_idx = dst_seg_idx;

    if (rb == dst_n) {
        cursor->elem = nullptr;
        return;
    }

#if EnStaging
    if (detail::GetNColor_(dst_n) == ref_color) {
        cursor->elem = nullptr;
        return;
    }
#endif

    Seg* dst_seg{ detail::NToSeg_(dst_n) };

    CircularArray ca{
        .data = dst_seg->dat.data,
        .elem_size = elem_size,
        .elem_stride = cntr.elem_stride,
        .elem_cnt = dst_seg->dat.elem_cnt,
        .slot_cnt = cntr.seg_elem_slot_cnt,
        .rot = dst_seg->dat.rot,
    };

    cursor->elem =
        circular_array::Access(ca, cursor->seg_idx, true, nullptr, nullptr);
}

namespace Namespace::detail {

template <CntrTplParamList>
void PrintState_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& cntr, Node* n) {
    Node* l_n{ bin_tree::GetL(n) };
    Node* r_n{ bin_tree::GetR(n) };

    if (l_n != nullptr) { (PrintState_)(cntr, l_n); }

    if (cntr.lb != n && cntr.rb != n) {
        Seg* seg{ (NToSeg_)(n) };

#if EnStaging
        if ((GetNColor_)(&seg->n) == ref_color) {
            std::cout << "ref " << static_cast<size_t>(seg->ref.elem_cnt)
                      << " [" << static_cast<size_t>(seg->ref.beg) << ", "
                      << static_cast<size_t>(seg->ref.beg + seg->ref.elem_cnt)
                      << "]\n";
        } else
#endif
        {
            std::cout << "dat " << static_cast<size_t>(seg->dat.elem_cnt)
                      << "\n";
        }
    }

    if (r_n != nullptr) { (PrintState_)(cntr, r_n); }
}

}  // namespace Namespace::detail

template <CntrTplParamList>
void Namespace::PrintState(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    detail::PrintState_(cntr, cntr.root);
}

namespace Namespace::detail {

template <CntrTplParamList>
Stats GetStats_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& cntr, Node* n) {
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
        if (cntr.lb != n && cntr.rb != n) {
            Seg* seg{ (NToSeg_)(n) };

#if EnStaging
            if ((GetNColor_)(n) == ref_color) {
                ++ret.ref_seg_cnt;
                ret.ref_size += seg->ref.elem_cnt;
            } else
#endif
            {
                ++ret.dat_seg_cnt;
                ret.dat_size += seg->dat.elem_cnt;
            }
        }

        Node* l_n{ bin_tree::GetL(n) };
        Node* r_n{ bin_tree::GetR(n) };

        if (l_n == nullptr) { utils::Swap(l_n, r_n); }

        if (l_n == nullptr) { break; }

        if (r_n == nullptr) {
            n = l_n;
            continue;
        }

        Stats l_stats{ (GetStats_)(cntr, l_n) };

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

}  // namespace Namespace::detail

template <CntrTplParamList>
Namespace::Stats Namespace::GetStats(Cntr<CntrTplArgList> const& cntr) {
    detail::CheckCntr_(cntr);

    Node* root{ cntr.root };

    if (root == nullptr) {
        return {
#if EnStaging
            .ref_seg_cnt = 0,
#endif

            .dat_seg_cnt = 0,

#if EnStaging
            .ref_size = 0,
#endif

            .dat_size = 0,
        };
    }

    return detail::GetStats_(cntr, cntr.root);
}

namespace Namespace::detail {

struct SanitizeRet_ {
    bool b;
    size_t ref_beg;
    size_t ref_end;
};

template <CntrTplParamList>
SanitizeRet_ Sanitize_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& cntr, mem_recorder::MemRecorder* dst_seg,
     mem_recorder::MemRecorder* dst_data, Node* n) {
#if EnStaging
    auto& origin{ meta::GetInstRef(cntr.origin) };
    size_t origin_elem_cnt{ seq_cntr::GetElemCnt(origin) };
#endif

    size_t elem_size{ cntr.elem_size };
    size_t elem_stride{ cntr.elem_stride };
    size_t seg_elem_slot_cnt{ cntr.seg_elem_slot_cnt };
    size_t data_size{ detail::CalcDataSize_(elem_size, elem_stride,
                                            seg_elem_slot_cnt) };

    Node* l_n{ bin_tree::StepL(n) };
    Node* r_n{ bin_tree::StepR(n) };

    if (cntr.lb == n) {
        ZETA_Core_DebugAssert(l_n == nullptr);
        ZETA_Core_DebugAssert(r_n != nullptr);

        ZETA_Core_DebugAssert(bin_tree::GetSize(n) == 1);

        mem_recorder::Record(*dst_seg, cntr.lb, sizeof(Node));
    } else if (cntr.rb == n) {
        ZETA_Core_DebugAssert(l_n != nullptr);
        ZETA_Core_DebugAssert(r_n == nullptr);

        ZETA_Core_DebugAssert(bin_tree::GetSize(n) == 1);

        mem_recorder::Record(*dst_seg, cntr.rb, sizeof(Node));
    }
#if EnStaging
    else if (GetNColor_(n) == ref_color) {
        ZETA_Core_DebugAssert(l_n != nullptr);
        ZETA_Core_DebugAssert(r_n != nullptr);

        Seg* seg{ (NToSeg_)(n) };

        mem_recorder::Record(*dst_seg, seg, sizeof(Seg));

        ZETA_Core_DebugAssert(0 < seg->ref.elem_cnt);

        ZETA_Core_DebugAssert(seg->ref.beg < origin_elem_cnt);
        ZETA_Core_DebugAssert(seg->ref.beg + seg->ref.elem_cnt <=
                              origin_elem_cnt);

        size_t size{ seg->ref.elem_cnt };

        ZETA_Core_DebugAssert(size == bin_tree::GetSize(n));

        if (cntr.lb != l_n) {
            Seg* l_seg{ (NToSeg_)(l_n) };

            if (GetNColor_(&l_seg->n) == ref_color) {
                ZETA_Core_DebugAssert(l_seg->ref.beg + l_seg->ref.elem_cnt <
                                      seg->ref.beg);
            }
        }

        if (cntr.rb != r_n) {
            Seg* r_seg{ (NToSeg_)(r_n) };

            if ((GetNColor_)(&r_seg->n) == ref_color) {
                ZETA_Core_DebugAssert(seg->ref.beg + seg->ref.elem_cnt <
                                      r_seg->ref.beg);
            }
        }
    }
#endif
    else {
#if EnStaging
        ZETA_Core_DebugAssert((GetNColor_)(n) == dat_color);
#endif

        ZETA_Core_DebugAssert(l_n != nullptr);
        ZETA_Core_DebugAssert(r_n != nullptr);

        Seg* seg{ (NToSeg_)(n) };

        mem_recorder::Record(*dst_seg, seg, sizeof(Seg));

        size_t elem_cnt{ seg->dat.elem_cnt };

        ZETA_Core_DebugAssert(elem_cnt == bin_tree::GetSize(n));

        ZETA_Core_DebugAssert(0 < elem_cnt);
        ZETA_Core_DebugAssert(elem_cnt <= seg_elem_slot_cnt);
        ZETA_Core_DebugAssert(seg->dat.rot < seg_elem_slot_cnt);

        mem_recorder::Record(*dst_data, seg->dat.data, data_size);

        size_t l_vac;
        size_t r_vac;

        if (cntr.lb == l_n) {
            l_vac = 0;
        } else {
            Seg* l_seg{ (NToSeg_)(l_n) };

            l_vac =
                seg_elem_slot_cnt -
                EnStagingTernary(GetNColor_(l_n) == ref_color,
                                 compare_utils::BasicMin(l_seg->ref.elem_cnt,
                                                         seg_elem_slot_cnt),
                                 l_seg->dat.elem_cnt);
        }

        if (cntr.rb == r_n) {
            r_vac = 0;
        } else {
            Seg* r_seg{ (NToSeg_)(r_n) };

            r_vac =
                seg_elem_slot_cnt -
                EnStagingTernary(GetNColor_(r_n) == ref_color,
                                 compare_utils::BasicMin(r_seg->ref.elem_cnt,
                                                         seg_elem_slot_cnt),
                                 r_seg->dat.elem_cnt);
        }

        ZETA_Core_DebugAssert(0 <= l_vac);
        ZETA_Core_DebugAssert(0 <= r_vac);
    }

    Node* sub_l_n{ bin_tree::GetL(n) };
    Node* sub_r_n{ bin_tree::GetR(n) };

    SanitizeRet_ l_check_ret{ false, 0, 0 };
    SanitizeRet_ r_check_ret{ false, 0, 0 };

    if (sub_l_n != nullptr) {
        l_check_ret = Sanitize_(cntr, dst_seg, dst_data, sub_l_n);
    }

    if (sub_r_n != nullptr) {
        r_check_ret = Sanitize_(cntr, dst_seg, dst_data, sub_r_n);
    }

    SanitizeRet_ ret{ false, 0, 0 };

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

    if (cntr.lb == n || cntr.rb == n) { return ret; }

#if EnStaging
    if (GetNColor_(n) == ref_color) {
        Seg* seg{ (NToSeg_)(n) };

        size_t cur_ref_beg{ seg->ref.beg };
        size_t cur_ref_end{ seg->ref.beg + seg->ref.elem_cnt };

        if (l_check_ret.b) {
            ZETA_Core_DebugAssert(l_check_ret.ref_end <= cur_ref_beg);
        }
        if (r_check_ret.b) {
            ZETA_Core_DebugAssert(cur_ref_end <= r_check_ret.ref_beg);
        }

        if (ret.b) {
            ret.ref_beg = compare_utils::BasicMin(ret.ref_beg, cur_ref_beg);
            ret.ref_end = compare_utils::BasicMax(ret.ref_end, cur_ref_end);
        } else {
            ret.b = true;
            ret.ref_beg = cur_ref_beg;
            ret.ref_end = cur_ref_end;
        }
    }
#endif

    return ret;
}

}  // namespace Namespace::detail

template <CntrTplParamList>
void Namespace::Sanitize(Cntr<CntrTplArgList> const& cntr,
                         mem_recorder::MemRecorder* dst_seg,
                         mem_recorder::MemRecorder* dst_data) {
#if !ZETA_Core_EnableDebug
    ZETA_Core_Unused(cntr);
    ZETA_Core_Unused(dst_seg);
    ZETA_Core_Unused(dst_data);
#else
    detail::CheckCntr_(cntr);

    bin_tree::Sanitize(cntr.root);
    rbtree::Sanitize(nullptr, cntr.root);

    mem_recorder::MemRecorder* origin_dst_seg{ dst_seg };
    mem_recorder::MemRecorder* origin_dst_data{ dst_data };

    if (dst_seg == nullptr) { dst_seg = mem_recorder::Create(); }
    if (dst_data == nullptr) { dst_data = mem_recorder::Create(); }

    detail::Sanitize_(cntr, dst_seg, dst_data, cntr.root);

    if (origin_dst_seg != dst_seg) {
        mem_recorder::Destroy(dst_seg);
        dst_seg = origin_dst_seg;
    }

    if (origin_dst_data != dst_data) {
        mem_recorder::Destroy(dst_data);
        dst_data = origin_dst_data;
    }
#endif
}

template <CntrTplParamList>
template <typename... Args>
void lifecycle::Traits<Namespace::Cntr<CntrTplArgList>>::Init(
    Namespace::Cntr<CntrTplArgList>& cntr, Args&&... args) {
    Namespace::Init(cntr, meta::Forward<Args>(args)...);
}

template <CntrTplParamList>
void lifecycle::Traits<Namespace::Cntr<CntrTplArgList>>::Deinit(
    Namespace::Cntr<CntrTplArgList>& cntr) {
    Namespace::Deinit(cntr);
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::
    GetReferedInstPtr(Namespace::Cntr<CntrTplArgList> const& cntr) {
    return const_cast<Namespace::Cntr<CntrTplArgList>*>(&cntr);
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    Namespace::Cntr<CntrTplArgList>>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Access = true,
        .Derefer = true,

        .Read = true,
        .Write = true,
        .ReadWrite = true,

        .PushL = true,
        .PushR = true,
        .Insert = true,

        .PopL = true,
        .PopR = true,
        .Erase = true,
        .EraseAll = true,

        .CopyCursor = true,

        .AreEqualCursor = true,
        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    Namespace::Cntr<CntrTplArgList> const>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::CntrTraits<
               Namespace::Cntr<CntrTplArgList>>::GetStaticEnabledAbilityFlag() &
           seq_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    Namespace::Cntr<CntrTplArgList>>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    Namespace::Cntr<CntrTplArgList> const>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::non_const_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::
    GetDynamicEnabledAbilityFlag(Namespace::Cntr<CntrTplArgList> const&) {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::
    GetDynamicDisabledAbilityFlag(Namespace::Cntr<CntrTplArgList> const&) {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::GetCursorSize(
    Namespace::Cntr<CntrTplArgList> const& cntr) {
    return Namespace::GetCursorSize(cntr);
}

template <CntrTplParamList>
size_t seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::GetElemSize(
    Namespace::Cntr<CntrTplArgList> const& cntr) {
    return Namespace::GetElemSize(cntr);
}

template <CntrTplParamList>
size_t seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::GetElemCnt(
    Namespace::Cntr<CntrTplArgList> const& cntr) {
    return Namespace::GetElemCnt(cntr);
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::GetMaxElemCnt(
    Namespace::Cntr<CntrTplArgList> const& cntr) {
    return Namespace::GetMaxElemCnt(cntr);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::GetLBCursor(
    Namespace::Cntr<CntrTplArgList> const& cntr, void* dst_cursor) {
    Namespace::GetLBCursor(cntr, static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::GetRBCursor(
    Namespace::Cntr<CntrTplArgList> const& cntr, void* dst_cursor) {
    Namespace::GetRBCursor(cntr, static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::PeekL(
    Namespace::Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return Namespace::PeekL(cntr, lazy_copy_elem,
                            static_cast<Namespace::Cursor*>(dst_cursor),
                            dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::PeekR(
    Namespace::Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return Namespace::PeekR(cntr, lazy_copy_elem,
                            static_cast<Namespace::Cursor*>(dst_cursor),
                            dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::Access(
    Namespace::Cntr<CntrTplArgList> const& cntr, size_t idx,
    bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return Namespace::Access(cntr, idx, lazy_copy_elem,
                             static_cast<Namespace::Cursor*>(dst_cursor),
                             dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::Derefer(
    Namespace::Cntr<CntrTplArgList> const& cntr, void const* pos_cursor,
    bool lazy_copy_elem, void* dst_elem) {
    return Namespace::Derefer(cntr,
                              static_cast<Namespace::Cursor const*>(pos_cursor),
                              lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename Reader>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::Read(
    Namespace::Cntr<CntrTplArgList> const& cntr, void const* pos_cursor,
    size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    Namespace::Read(cntr, static_cast<Namespace::Cursor const*>(pos_cursor),
                    cnt, reader, static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::Write(
    Namespace::Cntr<CntrTplArgList>& cntr, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    Namespace::Write(cntr, static_cast<Namespace::Cursor*>(pos_cursor), cnt,
                     writer, static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename ReaderWriter>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::ReadWrite(
    Namespace::Cntr<CntrTplArgList>& cntr, void* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    Namespace::ReadWrite(cntr, static_cast<Namespace::Cursor*>(pos_cursor), cnt,
                         reader_writer,
                         static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::PushL(
    Namespace::Cntr<CntrTplArgList>& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return Namespace::PushL(cntr, cnt, writer,
                            static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::PushR(
    Namespace::Cntr<CntrTplArgList>& cntr, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return Namespace::PushR(cntr, cnt, writer,
                            static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::Insert(
    Namespace::Cntr<CntrTplArgList>& cntr, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    return Namespace::Insert(cntr, static_cast<Namespace::Cursor*>(pos_cursor),
                             cnt, writer,
                             static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::PopL(
    Namespace::Cntr<CntrTplArgList>& cntr, size_t cnt) {
    Namespace::PopL(cntr, cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::PopR(
    Namespace::Cntr<CntrTplArgList>& cntr, size_t cnt) {
    Namespace::PopR(cntr, cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::Erase(
    Namespace::Cntr<CntrTplArgList>& cntr, void* pos_cursor, size_t cnt) {
    Namespace::Erase(cntr, static_cast<Namespace::Cursor*>(pos_cursor), cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>::EraseAll(
    Namespace::Cntr<CntrTplArgList>& cntr) {
    Namespace::EraseAll(cntr);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::CopyCursor(
    Namespace::Cntr<CntrTplArgList> const& cntr, void const* src_cursor,
    void* dst_cursor) {
    Namespace::CopyCursor(cntr,
                          static_cast<Namespace::Cursor const*>(src_cursor),
                          static_cast<Namespace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
bool seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::
    AreEqualCursor(Namespace::Cntr<CntrTplArgList> const& cntr,
                   void const* cursor_a, void const* cursor_b) {
    return Namespace::AreEqualCursor(
        cntr, static_cast<Namespace::Cursor const*>(cursor_a),
        static_cast<Namespace::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
int seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::CompareCursor(
    Namespace::Cntr<CntrTplArgList> const& cntr, void const* cursor_a,
    void const* cursor_b) {
    return Namespace::CompareCursor(
        cntr, static_cast<Namespace::Cursor const*>(cursor_a),
        static_cast<Namespace::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::GetCursorDist(
    Namespace::Cntr<CntrTplArgList> const& cntr, void const* cursor_a,
    void const* cursor_b) {
    return Namespace::GetCursorDist(
        cntr, static_cast<Namespace::Cursor const*>(cursor_a),
        static_cast<Namespace::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::GetCursorIdx(
    Namespace::Cntr<CntrTplArgList> const& cntr, void const* cursor) {
    return Namespace::GetCursorIdx(
        cntr, static_cast<Namespace::Cursor const*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::CursorStepL(
    Namespace::Cntr<CntrTplArgList> const& cntr, void* cursor) {
    Namespace::CursorStepL(cntr, static_cast<Namespace::Cursor*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::CursorStepR(
    Namespace::Cntr<CntrTplArgList> const& cntr, void* cursor) {
    Namespace::CursorStepR(cntr, static_cast<Namespace::Cursor*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::
    CursorAdvanceL(Namespace::Cntr<CntrTplArgList> const& cntr, void* cursor,
                   size_t step) {
    Namespace::CursorAdvanceL(cntr, static_cast<Namespace::Cursor*>(cursor),
                              step);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>::
    CursorAdvanceR(Namespace::Cntr<CntrTplArgList> const& cntr, void* cursor,
                   size_t step) {
    Namespace::CursorAdvanceR(cntr, static_cast<Namespace::Cursor*>(cursor),
                              step);
}

}  // namespace zeta::core

#pragma pop_macro("Namespace")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("EnStagingTernary")

// NOLINTEND(cppcoreguidelines-pro-type-union-access)
