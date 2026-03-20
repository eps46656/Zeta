#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

#define EnStaging 1

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

#pragma push_macro("NameSpace")
#define NameSpace staging_seg_vector

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList \
    typename OriginLike, typename SegAllocatorLike, typename DataAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList OriginLike, SegAllocatorLike, DataAllocatorLike

#pragma push_macro("EnStagingTernary")
#define EnStagingTernary(cond, x, y) ((cond) ? (x) : (y))

#else

#pragma push_macro("NameSpace")
#define NameSpace seg_vector

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename SegAllocatorLike, typename DataAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList SegAllocatorLike, DataAllocatorLike

#pragma push_macro("EnStagingTernary")
#define EnStagingTernary(cond, x, y) (y)

#endif

namespace zeta::core {

constexpr bool bin_tree::NodeTraits<NameSpace::TreeNode>::IsConst() {
    return false;
}

constexpr bool bin_tree::NodeTraits<NameSpace::TreeNode const>::IsConst() {
    return true;
}

constexpr bool bin_tree::NodeTraits<NameSpace::TreeNode const>::HasAccSize() {
    return true;
}

inline NameSpace::TreeNode* bin_tree::NodeTraits<NameSpace::TreeNode>::GetP(
    NameSpace::TreeNode* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<NameSpace::TreeNode*>(n->GetPPtr());
}

inline NameSpace::TreeNode* bin_tree::NodeTraits<NameSpace::TreeNode>::GetL(
    NameSpace::TreeNode* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<NameSpace::TreeNode*>(n->GetLPtr());
}

inline NameSpace::TreeNode* bin_tree::NodeTraits<NameSpace::TreeNode>::GetR(
    NameSpace::TreeNode* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<NameSpace::TreeNode*>(n->GetRPtr());
}

inline NameSpace::TreeNode const* bin_tree::NodeTraits<
    NameSpace::TreeNode const>::GetP(NameSpace::TreeNode const* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<NameSpace::TreeNode const*>(n->GetPPtr());
}

inline NameSpace::TreeNode const* bin_tree::NodeTraits<
    NameSpace::TreeNode const>::GetL(NameSpace::TreeNode const* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<NameSpace::TreeNode const*>(n->GetLPtr());
}

inline NameSpace::TreeNode const* bin_tree::NodeTraits<
    NameSpace::TreeNode const>::GetR(NameSpace::TreeNode const* n) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
    return static_cast<NameSpace::TreeNode const*>(n->GetRPtr());
}

inline void bin_tree::NodeTraits<NameSpace::TreeNode>::SetP(
    NameSpace::TreeNode* n, NameSpace::TreeNode* m) {
    n->SetPPtr(m);
}

inline void bin_tree::NodeTraits<NameSpace::TreeNode>::SetL(
    NameSpace::TreeNode* n, NameSpace::TreeNode* m) {
    n->SetLPtr(m);
}

inline void bin_tree::NodeTraits<NameSpace::TreeNode>::SetR(
    NameSpace::TreeNode* n, NameSpace::TreeNode* m) {
    n->SetRPtr(m);
}

constexpr size_t
bin_tree::NodeTraits<NameSpace::TreeNode const>::GetNullAccSize() {
    return 0;
}

inline size_t bin_tree::NodeTraits<NameSpace::TreeNode const>::GetAccSize(
    NameSpace::TreeNode const* n) {
    return n->GetAccSize();
}

inline void bin_tree::NodeTraits<NameSpace::TreeNode>::SetAccSize(
    NameSpace::TreeNode* n, size_t acc_size) {
    n->SetAccSize(acc_size);
}

inline unsigned rbtree::NodeTraits<NameSpace::TreeNode const>::GetColor(
    NameSpace::TreeNode const* n) {
    return n->GetPColor();
}

inline void rbtree::NodeTraits<NameSpace::TreeNode>::SetColor(
    NameSpace::TreeNode* n, unsigned color) {
    n->SetPColor(color);
}

namespace NameSpace::detail {

struct SegWork {
    bool is_null;

#if EnStaging
    unsigned color;
#endif

    size_t ref_beg;

    CircularArray ca;

    size_t elem_vac;

    void SetBasics(size_t elem_size, size_t elem_stride,
                   size_t seg_elem_capacity) {
        this->ca.elem_size = elem_size;
        this->ca.elem_stride = elem_stride;
        this->ca.elem_cnt = 0;
        this->ca.elem_capacity = seg_elem_capacity;
    }

    void SetNull() {
        this->is_null = true;
        this->ca.elem_cnt = 0;
        this->elem_vac = 0;
    }
};

template <CntrTplParamList>
void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };
    seq_cntr::CheckContract(origin);
#endif

    size_t elem_size{ cntr->elem_size };

    ZETA_Core_DebugAssert(0 < elem_size);

#if EnStaging
    ZETA_Core_DebugAssert(elem_size == seq_cntr::GetElemSize(origin));
#endif

    size_t elem_stride{ cntr->elem_stride };
    ZETA_Core_DebugAssert(elem_size <= elem_stride);

    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    ZETA_Core_DebugAssert(0 < seg_elem_capacity);
    ZETA_Core_DebugAssert(seg_elem_capacity <= max_seg_elem_capacity);
}

template <CntrTplParamList>
void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* cntr, Cursor const* cursor) {
    (CheckCntr_)(cntr);

    ZETA_Core_DebugAssert(cursor != nullptr);

    Cursor re_cursor;
    (Access)(cntr, cursor->idx, true, &re_cursor, nullptr);

    ZETA_Core_DebugAssert(*cursor == re_cursor);
}

struct ElemCntBalancer_ {
    size_t total_seg_cnt;
    size_t res_seg_cnt;
    size_t base_seg_elem_cnt;
    size_t res;
    size_t err;

    inline ElemCntBalancer_(size_t elem_cnt, size_t seg_elem_capacity) {
        this->total_seg_cnt = this->res_seg_cnt =
            integral_math::CeilDiv(elem_cnt, seg_elem_capacity);

        this->res = elem_cnt / this->total_seg_cnt;
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

/*

base +
((i + 1) * hi_cnt / seg_cnt
 - i * hi_cnt / seg_cnt);

seg_cnt = CeilDiv(elem_cnt, seg_elem_capacity)

(elem_cnt * (i + 1) // seg_cnt) - (elem_cnt * i // seg_cnt)

base = elem_cnt // seg_cnt

err == elem_cnt % seg_cnt

*/

inline size_t GetAvgCnt_  // NOLINT(misc-use-internal-linkage)
    (size_t cnt, size_t seg_elem_capacity, unsigned long long& random_seed) {
    size_t seg_cnt{ integral_math::CeilDiv(cnt, seg_elem_capacity) };

    return (cnt + utils::SimpleRandomRotate(&random_seed) % seg_cnt) / seg_cnt;
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

#if EnStaging

inline void GetRefSegState_  // NOLINT(misc-use-internal-linkage)
    (size_t seg_elem_capacity, Seg* seg, CircularArray* dst_ca,
     size_t* dst_elem_vac) {
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == ref_color);

    size_t elem_cnt{ seg->ref.elem_cnt };

    dst_ca->elem_cnt = elem_cnt;

    *dst_elem_vac = seg_elem_capacity - utils::Min(elem_cnt, seg_elem_capacity);
}

#endif

inline void GetDatSegState_  // NOLINT(misc-use-internal-linkage)
    (size_t seg_elem_capacity, Seg* seg, CircularArray* dst_ca,
     size_t* dst_vac) {
#if EnStaging
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == dat_color);
#endif

    size_t elem_cnt{ seg->dat.elem_cnt };

    dst_ca->data = seg->dat.data;
    dst_ca->elem_cnt = elem_cnt;
    dst_ca->idx_offset = seg->dat.idx_offset;

    *dst_vac = seg_elem_capacity - elem_cnt;
}

inline void GetSegState_  // NOLINT(misc-use-internal-linkage)
    (size_t seg_elem_capacity, Seg* seg,
#if EnStaging
     unsigned* dst_color,
#endif
     CircularArray* dst_ca, size_t* dst_vac) {
#if EnStaging
    unsigned color{ GetNColor_(&seg->n) };
    *dst_color = color;

    if (color == ref_color) {
        GetRefSegState_(seg_elem_capacity, seg, dst_ca, dst_vac);
    } else
#endif
    {
        GetDatSegState_(seg_elem_capacity, seg, dst_ca, dst_vac);
    }
}

inline size_t(GetDataSize_)(  // NOLINT(misc-use-internal-linkage)
    size_t elem_size, size_t elem_stride, size_t seg_elem_capacity) {
    return elem_stride * (seg_elem_capacity - 1) + elem_size;
}

template <typename SegAllocator>
Seg* AllocateSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator* seg_alctr) {
    Seg* seg{ static_cast<Seg*>(
        allocator::SafeAllocate(seg_alctr, alignof(Seg), sizeof(Seg))) };

    seg->n.Init();

    return seg;
}

template <typename DataAllocator>
void* AllocateData_  // NOLINT(misc-use-internal-linkage)
    (DataAllocator* data_alctr, size_t data_size) {
    return allocator::SafeAllocate(data_alctr, 1, data_size);
}

#if EnStaging

template <typename SegAllocator>
Seg* AllocateRefSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator* seg_alctr) {
    Seg* seg{ AllocateSeg_(seg_alctr) };

    DirectlySetNColor_(&seg->n, ref_color);

    seg->ref.beg = 0;
    seg->ref.elem_cnt = 0;

    return seg;
}

#endif

template <typename SegAllocator, typename DataAllocator>
Seg* AllocateDatSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator* seg_alctr, DataAllocator* data_alctr, size_t data_size) {
    Seg* seg{ AllocateSeg_(seg_alctr) };

#if EnStaging
    DirectlySetNColor_(&seg->n, dat_color);
#endif

    seg->dat.data = AllocateData_(data_alctr, data_size);
    seg->dat.elem_cnt = 0;
    seg->dat.idx_offset = 0;

    return seg;
}

#if EnStaging

template <typename SegAllocator>
void DeallocateRefSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator* seg_alctr, Seg* seg) {
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == ref_color);

    allocator::Deallocate(seg_alctr, seg);
}

#endif

template <typename SegAllocator, typename DataAllocator>
void DeallocateDatSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator* seg_alctr, DataAllocator* data_alctr, Seg* seg) {
#if EnStaging
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == dat_color);
#endif

    allocator::Deallocate(data_alctr, seg->dat.data);
    allocator::Deallocate(seg_alctr, seg);
}

template <typename SegAllocator, typename DataAllocator>
void DeallocateSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator* seg_alctr, DataAllocator* data_alctr, Seg* seg) {
#if EnStaging
    unsigned color{ GetNColor_(&seg->n) };
    ZETA_Core_DebugAssert(color == ref_color || color == dat_color);
#endif

#if EnStaging
    if (color == ref_color) {
        DeallocateRefSeg_(seg_alctr, seg);
    } else
#endif
    {
        DeallocateDatSeg_(seg_alctr, data_alctr, seg);
    }
}

template <typename SegAllocator, typename DataAllocator>
TreeNode* CopyTreeToLinkedSegList_(size_t elem_size,
                                   size_t src_seg_elem_capacity,
                                   size_t dst_seg_elem_capacity,
                                   TreeNode* src_root, TreeNode* src_lb,
                                   TreeNode* src_rb, SegAllocator* seg_alctr,
                                   DataAllocator* data_alctr) {
    //

    //
}

inline TreeNode* BuildTreeFromLinkedSegList_(TreeNode* lb, TreeNode* rb,
                                             TreeNode*& cur, size_t cnt,
                                             bool long_side) {
    size_t l_cnt{ cnt / 2 };
    size_t r_cnt{ cnt - 1 - l_cnt };

    TreeNode* l_n{ l_cnt == 0
                       ? nullptr
                       : BuildTreeFromLinkedSegList_(
                             lb, rb, cur, l_cnt, long_side && r_cnt <= l_cnt) };

    TreeNode* root{ cur };

    cur = bin_tree::GetR(cur);

    TreeNode* r_n{ r_cnt == 0
                       ? nullptr
                       : BuildTreeFromLinkedSegList_(
                             lb, rb, cur, r_cnt, long_side && l_cnt <= r_cnt) };

    bin_tree::SetL(root, l_n);
    bin_tree::SetR(root, r_n);

    if (l_n != nullptr) { bin_tree::SetP(l_n, root); }
    if (r_n != nullptr) { bin_tree::SetP(r_n, root); }

    size_t root_size{ root == lb || root == rb ? 1U : ({
        Seg* root_seg{ NToSeg_(root) };
        EnStagingTernary(GetNColor_(root) == ref_color, root_seg->ref.elem_cnt,
                         root_seg->dat.elem_cnt);
    }) };

    size_t l_acc_size{ l_n == nullptr
                           ? 0U
                           : bin_tree::NodeTraits<TreeNode>::GetAccSize(l_n) };

    size_t r_acc_size{ r_n == nullptr
                           ? 0U
                           : bin_tree::NodeTraits<TreeNode>::GetAccSize(r_n) };

    bin_tree::NodeTraits<TreeNode>::SetAccSize(
        root, l_acc_size + root_size + r_acc_size);

    rbtree::SetColor(root, long_side && l_n == nullptr && r_n == nullptr
                               ? rbtree::red
                               : rbtree::black);

    return root;
}

#if EnStaging

template <CntrTplParamList>
void TransferRefSegToDatSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* seg) {
    ZETA_Core_DebugAssert(GetNColor_(&seg->n) == ref_color);

    auto* origin{ utils::GetInstPtr(cntr->origin) };

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t data_size{ (GetDataSize_)(elem_size, elem_stride,
                                     seg_elem_capacity) };

    auto* data_alctr{ utils::GetInstPtr(cntr->data_alctr) };

    void* data{ AllocateData_(data_alctr, data_size) };

    size_t elem_cnt{ seg->ref.elem_cnt };

    ZETA_Core_DebugAssert(elem_cnt <= seg_elem_capacity);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    seq_cntr::Access(origin, seg->ref.beg, true, origin_cursor, nullptr);
    seq_cntr::Read(origin, origin_cursor, elem_cnt,
                   seq_cntr::MemReader{
                       .data = data,
                       .elem_size = elem_size,
                       .elem_stride = elem_stride,
                   },
                   nullptr);

    SetNColor_(&seg->n, dat_color);

    seg->dat.data = data;
    seg->dat.elem_cnt = static_cast<unsigned short>(elem_cnt);
    seg->dat.idx_offset = 0;
}

template <CntrTplParamList>
void RefSegShoveL_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, CircularArray* ca, size_t beg, size_t cnt) {
    auto* origin{ utils::GetInstPtr(cntr->origin) };

    circular_array::PushL(ca, cnt, [](void*, size_t, size_t) {}, nullptr);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    seq_cntr::Access(origin, beg, true, origin_cursor, nullptr);

    circular_array::AssignFromSeqCntr(ca, 0, origin, origin_cursor, cnt);
}

template <CntrTplParamList>
void RefSegShoveR_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, CircularArray* ca, size_t beg, size_t cnt) {
    auto* origin{ utils::GetInstPtr(cntr->origin) };

    circular_array::PushR(ca, cnt, [](void*, size_t, size_t) {}, nullptr);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    seq_cntr::Access(origin, beg, true, origin_cursor, nullptr);

    circular_array::AssignFromSeqCntr(ca, ca->elem_cnt - cnt, origin,
                                      origin_cursor, cnt);
}

template <CntrTplParamList, typename Writer>
void RefShoveL_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, CircularArray* l_ca, Seg* r_seg,
     size_t rl_elem_cnt, size_t ins_cnt, size_t shove_cnt, Writer& writer) {
    auto* origin{ utils::GetInstPtr(cntr->origin) };

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_size };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t data_size{ (GetDataSize_)(elem_size, elem_stride,
                                     seg_elem_capacity) };

    auto* data_alctr{ utils::GetInstPtr(cntr->data_alctr) };

    size_t cnt_a{ utils::Min(rl_elem_cnt, shove_cnt) };
    size_t cnt_b{ utils::Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_i{ l_ca->elem_cnt };

    void* r_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::PushR(l_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    seq_cntr::Access(origin, r_seg->ref.beg, true, r_cursor, nullptr);

    size_t l_ca_elem_cnt{ l_ca->elem_cnt };
    size_t l_ca_elem_capacity{ l_ca->elem_capacity };
    size_t l_ca_idx_offset{ l_ca->idx_offset };

    for (size_t i{ cnt_a }; 0 < i;) {
        size_t cur_i{ utils::Min(
            i, circular_array::GetLongestContSucr(
                   l_ca_elem_cnt, l_ca_elem_capacity, l_ca_idx_offset, l_i)) };

        i -= cur_i;

        seq_cntr::Read(origin, r_cursor, cur_i,
                       seq_cntr::MemReader{
                           .data = circular_array::Access(l_ca, l_i, true,
                                                          nullptr, nullptr),
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       r_cursor);

        l_i += cur_i;
    }

    circular_array::IdxWrite(l_ca, l_i, cnt_b, writer);

    l_i += cnt_b;

    for (size_t i{ cnt_c }; 0 < i;) {
        size_t cur_i{ utils::Min(
            i, circular_array::GetLongestContSucr(
                   l_ca_elem_cnt, l_ca_elem_capacity, l_ca_idx_offset, l_i)) };

        i -= cur_i;

        seq_cntr::Read(origin, r_cursor, cur_i,
                       seq_cntr::MemReader{
                           .data = circular_array::Access(l_ca, l_i, true,
                                                          nullptr, nullptr),
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       r_cursor);

        l_i += cur_i;
    }

    size_t new_r_elem_cnt{ r_seg->ref.elem_cnt + ins_cnt - shove_cnt };

    if (ins_cnt <= cnt_b) {
        r_seg->ref.beg += cnt_a + cnt_c;
        r_seg->ref.elem_cnt = new_r_elem_cnt;
        return;
    }

    void* data{ AllocateData_(data_alctr, data_size) };
    auto* data_i{ static_cast<char*>(data) };

    if (cnt_a < rl_elem_cnt) {
        seq_cntr::Read(origin, r_cursor, rl_elem_cnt - cnt_a,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       r_cursor);

        data_i += elem_stride * (rl_elem_cnt - cnt_a);
    }

    writer(data_i, elem_stride, ins_cnt - cnt_b);

    data_i += elem_stride * (ins_cnt - cnt_b);

    if (rl_elem_cnt < r_seg->ref.elem_cnt) {
        seq_cntr::Read(origin, r_cursor, r_seg->ref.elem_cnt - rl_elem_cnt,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       r_cursor);
    }

    SetNColor_(&r_seg->n, dat_color);

    r_seg->dat.data = data;
    r_seg->dat.elem_cnt = static_cast<unsigned short>(new_r_elem_cnt);
    r_seg->dat.idx_offset = 0;
}

template <typename Origin, typename Writer>
void RefShoveL_  // NOLINT(misc-use-internal-linkage)
    (Origin* origin, CircularArray* l_ca, Seg* r_seg, size_t shove_cnt) {
    ZETA_Core_DebugAssert(GetNColor_(&r_seg->n) == ref_color);

    ZETA_Core_DebugAssert(shove_cnt <= r_seg->ref.elem_cnt);

    if (shove_cnt == 0) { return; }

    void* r_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::PushR(l_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    seq_cntr::Access(origin, r_seg->ref.beg, true, r_cursor, nullptr);

    circular_array::AssignFromSeqCntr(l_ca, l_ca->elem_cnt - shove_cnt, origin,
                                      r_cursor, shove_cnt);

    r_seg->ref.beg += shove_cnt;
    r_seg->ref.elem_cnt -= shove_cnt;
}

template <typename Origin, typename Writer>
void RefShoveR_  // NOLINT(misc-use-internal-linkage)
    (Origin* origin, Seg* l_seg, CircularArray* r_ca, size_t shove_cnt) {
    ZETA_Core_DebugAssert(GetNColor_(&l_seg->n) == ref_color);

    ZETA_Core_DebugAssert(shove_cnt <= l_seg->ref.elem_cnt);

    if (shove_cnt == 0) { return; }

    void* l_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::PushL(r_ca, shove_cnt, seq_cntr::EmptyWriter{}, nullptr);

    seq_cntr::Access(origin, l_seg->ref.beg + l_seg->ref.elem_cnt - shove_cnt,
                     true, l_cursor, nullptr);

    circular_array::AssignFromSeqCntr(r_ca, 0, origin, l_cursor, shove_cnt);

    l_seg->ref.elem_cnt -= shove_cnt;
}

template <CntrTplParamList, typename Writer>
void RefShoveR_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* l_seg, CircularArray* r_ca,
     size_t lr_size, size_t ins_cnt, size_t shove_cnt, Writer& writer) {
    auto* origin{ utils::GetInstPtr(cntr->origin) };

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t data_size{ (GetDataSize_)(elem_size, elem_stride,
                                     seg_elem_capacity) };

    auto* data_alctr{ utils::GetInstPtr(cntr->data_alctr) };

    size_t cnt_a{ utils::Min(lr_size, shove_cnt) };
    size_t cnt_b{ utils::Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t r_i{ 0 };

    void* l_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::PushL(
        r_ca, shove_cnt, [](void*, size_t, size_t) {}, nullptr);

    seq_cntr::Access(origin,
                     l_seg->ref.beg + l_seg->ref.elem_cnt - cnt_a - cnt_c, true,
                     l_cursor, nullptr);

    size_t r_ca_elem_cnt{ r_ca->elem_cnt };
    size_t r_ca_elem_capacity{ r_ca->elem_capacity };
    size_t r_ca_idx_offset{ r_ca->idx_offset };

    for (size_t i{ cnt_c }; 0 < i;) {
        size_t cur_i{ utils::Min(
            i, circular_array::GetLongestContSucr(
                   r_ca_idx_offset, r_i, r_ca_elem_cnt, r_ca_elem_capacity)) };

        i -= cur_i;

        seq_cntr::Read(origin, l_cursor, cur_i,
                       seq_cntr::MemReader{
                           .data = circular_array::Access(r_ca, r_i, true,
                                                          nullptr, nullptr),
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       l_cursor);

        r_i += cur_i;
    }

    circular_array::IdxWrite(r_ca, r_i, cnt_b, writer);

    r_i += cnt_b;

    for (size_t i{ cnt_a }; 0 < i;) {
        size_t cur_i{ utils::Min(
            i, circular_array::GetLongestContSucr(
                   r_ca_idx_offset, r_i, r_ca_elem_cnt, r_ca_elem_capacity)) };

        i -= cur_i;

        seq_cntr::Read(origin, l_cursor, cur_i,
                       seq_cntr::MemReader{
                           .data = circular_array::Access(r_ca, r_i, true,
                                                          nullptr, nullptr),
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       l_cursor);

        r_i += cur_i;
    }

    size_t new_l_elem_cnt{ l_seg->ref.elem_cnt + ins_cnt - shove_cnt };

    if (ins_cnt <= cnt_b) {
        l_seg->ref.elem_cnt = new_l_elem_cnt;
        return;
    }

    void* data{ AllocateData_(data_alctr, data_size) };
    auto* data_i{ static_cast<char*>(data) };

    seq_cntr::Access(origin, l_seg->ref.beg, true, l_cursor, nullptr);

    if (lr_size < l_seg->ref.elem_cnt) {
        seq_cntr::Read(origin, l_cursor, l_seg->ref.elem_cnt - lr_size,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       l_cursor);

        data_i += elem_stride * (l_seg->ref.elem_cnt - lr_size);
    }

    writer(data_i, elem_stride, ins_cnt - cnt_b);

    data_i += elem_stride * (ins_cnt - cnt_b);

    if (cnt_a < lr_size) {
        seq_cntr::Read(origin, l_cursor, lr_size - cnt_a,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       l_cursor);
    }

    SetNColor_(&l_seg->n, dat_color);

    l_seg->dat.data = data;
    l_seg->dat.elem_cnt = static_cast<unsigned short>(new_l_elem_cnt);
    l_seg->dat.idx_offset = 0;
}

#endif

#if EnStaging

template <typename Origin, typename SegAllocator, typename DataAllocator>
void MaterializeSeg_(Origin* origin, size_t elem_size, size_t elem_stride,
                     size_t data_size, DataAllocator* data_alctr,
                     SegWork* seg_work, bool need_data) {
    ZETA_Core_DebugAssert(!seg_work->is_null);
    ZETA_Core_DebugAssert(seg_work->color == ref_color);

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    void* data{ AllocateData_(data_alctr, data_size) };

    if (need_data) {
        seq_cntr::Access(origin, seg_work->ref_beg, true, origin_cursor,
                         nullptr);
        seq_cntr::Read(origin, origin_cursor, seg_work->ca.elem_cnt,
                       seq_cntr::MemReader{
                           .data = data,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       nullptr);
    }

    seg_work->color = dat_color;
    seg_work->ca.data = data;
    seg_work->ca.idx_offset = 0;
}

#endif

template <typename Origin, typename SegAllocator, typename DataAllocator>
void SegShoveL_(Origin* origin, size_t elem_size, size_t elem_stride,
                size_t seg_elem_capacity, size_t data_size,
                DataAllocator* data_alctr, SegWork* l_seg_work,
                SegWork* r_seg_work, size_t shove_cnt
#if EnStaging
                ,
                bool l_need_data, bool r_need_data
#endif
) {
    ZETA_Core_DebugAssert(!l_seg_work->is_null);
    ZETA_Core_DebugAssert(!r_seg_work->is_null);

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= r_seg_work->ca.elem_cnt);
    ZETA_Core_DebugAssert(l_seg_work->ca.elem_cnt + shove_cnt <=
                          seg_elem_capacity);

#if EnStaging
    if (l_seg_work->color == ref_color) {
        MaterializeSeg_(origin, elem_size, elem_stride, data_size, data_alctr,
                        l_seg_work, l_need_data);
    }
#endif

#if EnStaging
    if (r_seg_work->color == ref_color) {
        void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

        seq_cntr::Access(origin, r_seg_work->ref_beg, true, origin_cursor,
                         nullptr);

        circular_array::PushR(&l_seg_work->ca, shove_cnt,
                              seq_cntr::EmptyWriter{}, nullptr);

        if (r_need_data) {
            circular_array::AssignFromSeqCntr(
                &l_seg_work->ca, l_seg_work->ca.elem_cnt - shove_cnt, origin,
                origin_cursor, shove_cnt);
        }

        r_seg_work->ref_beg += shove_cnt;
        r_seg_work->ca.elem_cnt -= shove_cnt;
    } else
#endif
    {
        seg_utils::SegShoveL(&l_seg_work->ca, &r_seg_work->ca, shove_cnt);
    }
}

template <typename Origin, typename SegAllocator, typename DataAllocator>
void SegShoveR_(Origin* origin, size_t elem_size, size_t elem_stride,
                size_t seg_elem_capacity, size_t data_size,
                DataAllocator* data_alctr, SegWork* l_seg_work,
                SegWork* r_seg_work, size_t shove_cnt
#if EnStaging
                ,
                bool l_need_data, bool r_need_data
#endif
) {
    ZETA_Core_DebugAssert(!l_seg_work->is_null);
    ZETA_Core_DebugAssert(!r_seg_work->is_null);

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(shove_cnt <= l_seg_work->ca.elem_cnt);
    ZETA_Core_DebugAssert(r_seg_work->ca.elem_cnt + shove_cnt <=
                          seg_elem_capacity);

#if EnStaging
    if (r_seg_work->color == ref_color) {
        MaterializeSeg_(origin, elem_size, elem_stride, data_size, data_alctr,
                        r_seg_work, r_need_data);
    }
#endif

#if EnStaging
    if (l_seg_work->color == ref_color) {
        void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

        seq_cntr::Access(
            origin, l_seg_work->ref_beg + l_seg_work->ca.elem_cnt - shove_cnt,
            true, origin_cursor, nullptr);

        circular_array::PushL(&r_seg_work->ca, shove_cnt,
                              seq_cntr::EmptyWriter{}, nullptr);

        if (l_need_data) {
            circular_array::AssignFromSeqCntr(&r_seg_work->ca, 0, origin,
                                              origin_cursor, shove_cnt);
        }

        l_seg_work->ca.elem_cnt -= shove_cnt;
    } else
#endif
    {
        seg_utils::SegShoveR(&l_seg_work->ca, &r_seg_work->ca, shove_cnt);
    }
}

template <typename Origin, typename SegAllocator, typename DataAllocator,
          typename Writer>
void SegInsertShoveL_(Origin* origin, size_t elem_size, size_t elem_stride,
                      size_t seg_elem_capacity, size_t data_size,
                      DataAllocator* data_alctr, SegWork* l_seg_work,
                      SegWork* r_seg_work, size_t rl_cnt, size_t ins_cnt,
                      size_t shove_cnt, Writer&& writer
#if EnStaging
                      ,
                      bool l_need_data, bool r_need_data
#endif
) {
    ZETA_Core_DebugAssert(!l_seg_work->is_null);
    ZETA_Core_DebugAssert(!r_seg_work->is_null);

    size_t l_vac{ seg_elem_capacity - l_seg_work->ca.elem_cnt };
    size_t r_vac{ seg_elem_capacity - r_seg_work->ca.elem_cnt };

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(rl_cnt <= r_seg_work->ca.elem_cnt);
    ZETA_Core_DebugAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_seg_work->ca.elem_cnt + ins_cnt);
    ZETA_Core_DebugAssert(r_seg_work->ca.elem_cnt + ins_cnt - shove_cnt <=
                          r_seg_work->ca.elem_capacity);

#if EnStaging
    if (l_seg_work->color == ref_color) {
        MaterializeSeg_(origin, elem_size, elem_stride, data_size, data_alctr,
                        l_seg_work, l_need_data);
    }
#endif

#if EnStaging
    if (r_seg_work->color == dat_color)
#endif
    {
        seg_utils::SegInsertShoveL(&l_seg_work->ca, &r_seg_work->ca, rl_cnt,
                                   ins_cnt, shove_cnt,
                                   meta::Forward<Writer>(writer));
        return;
    }

#if EnStaging
    size_t cnt_a{ utils::Min(rl_cnt, shove_cnt) };
    size_t cnt_b{ utils::Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_elem_cnt{ l_seg_work->ca.elem_cnt };

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::PushR(&l_seg_work->ca, shove_cnt, seq_cntr::EmptyWriter{},
                          nullptr);

    if (r_need_data && 0 < cnt_a) {
        seq_cntr::Access(origin, r_seg_work->ref_beg, true, origin_cursor,
                         nullptr);

        circular_array::AssignFromSeqCntr(&l_seg_work->ca, l_elem_cnt, origin,
                                          origin_cursor, cnt_a);
    }

    if (0 < cnt_b) {
        circular_array::IdxWrite(&l_seg_work->ca, l_elem_cnt + cnt_a, cnt_b,
                                 meta::Forward<Writer>(writer));
    }

    if (r_need_data && 0 < cnt_c) {
        seq_cntr::Access(origin, r_seg_work->ref_beg + cnt_a, true,
                         origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(&l_seg_work->ca,
                                          l_elem_cnt + cnt_a + cnt_b, origin,
                                          origin_cursor, cnt_c);
    }

    void* data{ AllocateData_(data_alctr, data_size) };

    auto* data_i{ static_cast<char*>(data) };

    size_t rr_cnt{ r_seg_work->ca.elem_cnt - rl_cnt };

    if (r_need_data && 0 < rl_cnt - cnt_a) {
        seq_cntr::Access(origin, r_seg_work->ref_beg + cnt_a, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(origin, origin_cursor, rl_cnt - cnt_a,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       nullptr);

        data_i += elem_stride * (rl_cnt - cnt_a);
    }

    if (0 < ins_cnt - cnt_b) {
        meta::Forward<Writer>()(data_i, elem_stride, ins_cnt - cnt_b);
        data_i += elem_stride * (ins_cnt - cnt_b);
    }

    if (r_need_data && 0 < rr_cnt - cnt_c) {
        seq_cntr::Access(origin, r_seg_work->ref_beg + rl_cnt, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(origin, origin_cursor, rr_cnt - cnt_c,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       nullptr);
    }

    r_seg_work->color = dat_color;
    r_seg_work->ca.data = data;
    r_seg_work->ca.elem_cnt = r_seg_work->ca.elem_cnt + ins_cnt - shove_cnt;
    r_seg_work->ca.idx_offset = 0;
#endif
}

template <typename Origin, typename SegAllocator, typename DataAllocator,
          typename Writer>
void SegInsertShoveR_(Origin* origin, size_t elem_size, size_t elem_stride,
                      size_t seg_elem_capacity, size_t data_size,
                      DataAllocator* data_alctr, SegWork* l_seg_work,
                      SegWork* r_seg_work, size_t lr_cnt, size_t ins_cnt,
                      size_t shove_cnt, Writer&& writer
#if EnStaging
                      ,
                      bool l_need_data, bool r_need_data
#endif
) {
    ZETA_Core_DebugAssert(!l_seg_work->is_null);
    ZETA_Core_DebugAssert(!r_seg_work->is_null);

    size_t l_vac{ seg_elem_capacity - l_seg_work->ca.elem_cnt };
    size_t r_vac{ seg_elem_capacity - r_seg_work->ca.elem_cnt };

    ZETA_Core_DebugAssert(0 < shove_cnt);
    ZETA_Core_DebugAssert(lr_cnt <= l_seg_work->ca.elem_cnt);
    ZETA_Core_DebugAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugAssert(shove_cnt <= l_seg_work->ca.elem_cnt + ins_cnt);
    ZETA_Core_DebugAssert(l_seg_work->ca.elem_cnt + ins_cnt - shove_cnt <=
                          l_seg_work->ca.elem_capacity);

#if EnStaging
    if (r_seg_work->color == ref_color) {
        MaterializeSeg_(origin, elem_size, elem_stride, data_size, data_alctr,
                        r_seg_work, r_need_data);
    }
#endif

#if EnStaging
    if (l_seg_work->color == dat_color)
#endif
    {
        seg_utils::SegInsertShoveL(&l_seg_work->ca, &r_seg_work->ca, lr_cnt,
                                   ins_cnt, shove_cnt,
                                   meta::Forward<Writer>(writer));
        return;
    }

#if EnStaging
    size_t cnt_a{ utils::Min(lr_cnt, shove_cnt) };
    size_t cnt_b{ utils::Min(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    size_t l_elem_cnt{ l_seg_work->ca.elem_cnt };

    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::PushL(&r_seg_work->ca, shove_cnt, seq_cntr::EmptyWriter{},
                          nullptr);

    if (l_need_data && 0 < cnt_c) {
        seq_cntr::Access(origin,
                         l_seg_work->ref_beg + l_elem_cnt - cnt_a - cnt_c, true,
                         origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(&r_seg_work->ca, 0, origin,
                                          origin_cursor, cnt_c);
    }

    if (0 < cnt_b) {
        circular_array::IdxWrite(&r_seg_work->ca, cnt_c, cnt_b,
                                 meta::Forward<Writer>(writer));
    }

    if (l_need_data && 0 < cnt_a) {
        seq_cntr::Access(origin, l_seg_work->ref_beg + l_elem_cnt - cnt_a, true,
                         origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(&r_seg_work->ca, cnt_c + cnt_b,
                                          origin, origin_cursor, cnt_a);
    }

    void* data{ AllocateData_(data_alctr, data_size) };

    auto* data_i{ static_cast<char*>(data) };

    size_t ll_cnt{ l_seg_work->ca.elem_cnt - lr_cnt };

    if (l_need_data && 0 < ll_cnt - cnt_c) {
        seq_cntr::Access(origin, l_seg_work->ref_beg, true, origin_cursor,
                         nullptr);

        seq_cntr::Read(origin, origin_cursor, ll_cnt - cnt_c,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       nullptr);

        data_i += elem_stride * (ll_cnt - cnt_c);
    }

    if (0 < ins_cnt - cnt_b) {
        meta::Forward<Writer>(writer)(data_i, elem_stride, ins_cnt - cnt_b);
        data_i += elem_stride * (ins_cnt - cnt_b);
    }

    if (l_need_data && 0 < lr_cnt - cnt_a) {
        seq_cntr::Access(origin, l_seg_work->ref_beg + ll_cnt, true,
                         origin_cursor, nullptr);

        seq_cntr::Read(origin, origin_cursor, lr_cnt - cnt_a,
                       seq_cntr::MemReader{
                           .data = data_i,
                           .elem_size = elem_size,
                           .elem_stride = elem_stride,
                       },
                       nullptr);
    }

    l_seg_work->color = dat_color;
    l_seg_work->ca.data = data;
    l_seg_work->ca.elem_cnt = l_seg_work->ca.elem_cnt + ins_cnt - shove_cnt;
    l_seg_work->ca.idx_offset = 0;
#endif
}

template <CntrTplParamList>
int Merge2_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Seg* a_seg, Seg* b_seg, bool a_read_data,
     bool b_read_data) {
#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };

#endif

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };

#if EnStaging
    size_t data_size{ (GetDataSize_)(elem_size, elem_stride,
                                     seg_elem_capacity) };

    auto* data_alctr{ utils::GetInstPtr(cntr->data_alctr) };

    unsigned a_color{ GetNColor_(&a_seg->n) };
    unsigned b_color{ GetNColor_(&b_seg->n) };
#endif

    size_t a_elem_cnt{ EnStagingTernary(
        a_color == ref_color, a_seg->ref.elem_cnt, a_seg->dat.elem_cnt) };
    size_t b_elem_cnt{ EnStagingTernary(
        b_color == ref_color, b_seg->ref.elem_cnt, b_seg->dat.elem_cnt) };

    unsigned long long random_seed{ utils::GetRandom() };

#if EnStaging
    if (a_color == ref_color && b_color == ref_color) {
        size_t total_elem_cnt{ a_elem_cnt + b_elem_cnt };

        void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

        char* data{ static_cast<char*>(AllocateData_(data_alctr, data_size)) };

        if (a_read_data) {
            seq_cntr::Access(origin, a_seg->ref.beg, true, origin_cursor,
                             nullptr);
            seq_cntr::Read(origin, origin_cursor, a_elem_cnt,
                           seq_cntr::MemReader{
                               .data = data,
                               .elem_size = elem_size,
                               .elem_stride = elem_stride,
                           },
                           nullptr);
        }

        if (b_read_data) {
            seq_cntr::Access(origin, b_seg->ref.beg, true, origin_cursor,
                             nullptr);
            seq_cntr::Read(origin, origin_cursor, b_seg->ref.elem_cnt,
                           seq_cntr::MemReader{
                               .data = data + elem_stride * a_elem_cnt,
                               .elem_size = elem_size,
                               .elem_stride = elem_stride,
                           },
                           nullptr);
        }

        a_seg->ref.elem_cnt = 0;
        b_seg->ref.elem_cnt = 0;

        Seg* dst_seg;

        int side{ static_cast<int>(utils::SimpleRandomRotate(&random_seed) %
                                   2) };

        switch (side) {
        case 0: dst_seg = a_seg; break;
        case 1: dst_seg = b_seg; break;
        }

        SetNColor_(&dst_seg->n, dat_color);
        dst_seg->dat.data = data;
        dst_seg->dat.elem_cnt = static_cast<unsigned short>(total_elem_cnt);

        return side;
    }
#endif

    CircularArray a_ca{
        .data = a_seg->dat.data,
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = a_elem_cnt,
        .elem_capacity = seg_elem_capacity,
        .idx_offset = a_seg->dat.idx_offset,
    };

    CircularArray b_ca{
        .data = b_seg->dat.data,
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = b_elem_cnt,
        .elem_capacity = seg_elem_capacity,
        .idx_offset = b_seg->dat.idx_offset,
    };

    size_t merge_a_cost{ EnStagingTernary(
        a_color == ref_color, a_elem_cnt + b_elem_cnt, b_elem_cnt) };
    size_t merge_b_cost{ EnStagingTernary(
        b_color == ref_color, a_elem_cnt + b_elem_cnt, a_elem_cnt) };

    int side{ utils::Choose2(merge_a_cost <= merge_b_cost,
                             merge_b_cost <= merge_a_cost, &random_seed) };

    switch (side) {
    case 0:
        if (!b_read_data) {
            circular_array::PushR(
                &a_ca, b_elem_cnt, [](void*, size_t, size_t) {}, nullptr);
        }
#if EnStaging
        else if (b_color == ref_color) {
            RefSegShoveR_(cntr, &a_ca, b_seg->ref.beg, b_elem_cnt);
        }
#endif
        else {
            seg_utils::SegShoveL(&a_ca, &b_ca, b_elem_cnt);
        }

        break;

    case 1:
        if (!a_read_data) {
            circular_array::PushL(
                &b_ca, a_elem_cnt, [](void*, size_t, size_t) {}, nullptr);
        }
#if EnStaging
        else if (a_color == ref_color) {
            RefSegShoveL_(cntr, &b_ca, a_seg->ref.beg, a_elem_cnt);
        }
#endif
        else {
            seg_utils::SegShoveR(&a_ca, &b_ca, a_elem_cnt);
        }

        break;
    }

#if EnStaging
    if (a_color == ref_color) {
        a_seg->ref.elem_cnt = 0;
    } else
#endif
    {
        a_seg->dat.elem_cnt = static_cast<unsigned short>(a_ca.elem_cnt);
        a_seg->dat.idx_offset = static_cast<unsigned short>(a_ca.idx_offset);
    }

#if EnStaging
    if (b_color == ref_color) {
        b_seg->ref.elem_cnt = 0;
    } else
#endif
    {
        b_seg->dat.elem_cnt = static_cast<unsigned short>(b_ca.elem_cnt);
        b_seg->dat.idx_offset = static_cast<unsigned short>(b_ca.idx_offset);
    }

    return side;
}

template <typename Origin, typename SegAllocator, typename DataAllocator>
inline int NewMerge2_(Origin* origin, size_t elem_size, size_t elem_stride,
                      size_t seg_elem_capacity, size_t data_size,
                      SegWork* l_seg_work, SegWork* r_seg_work,
#if EnStaging
                      bool l_need_data, bool r_need_data,
#endif
                      SegAllocator* seg_alctr, DataAllocator* data_alctr) {
    ZETA_Core_DebugAssert(!l_seg_work->is_null);
    ZETA_Core_DebugAssert(!r_seg_work->is_null);

    ZETA_Core_DebugAssert(0 < l_seg_work->ca.elem_cnt);
    ZETA_Core_DebugAssert(0 < r_seg_work->ca.elem_cnt);

    unsigned long long random_seed{ utils::GetRandom() };

    size_t total_elem_cnt{ l_seg_work->ca.elem_cnt + r_seg_work->ca.elem_cnt };

#if EnStaging
    if (l_seg_work->color == ref_color && r_seg_work->color == ref_color) {
        void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

        char* data{ static_cast<char*>(AllocateData_(data_alctr, data_size)) };

        if (l_need_data) {
            seq_cntr::Access(origin, l_seg_work->ref_beg, true, origin_cursor,
                             nullptr);
            seq_cntr::Read(origin, origin_cursor, l_seg_work->ca.elem_cnt,
                           seq_cntr::MemReader{
                               .data = data,
                               .elem_size = elem_size,
                               .elem_stride = elem_stride,
                           },
                           nullptr);
        }

        if (r_need_data) {
            seq_cntr::Access(origin, r_seg_work->ref_beg, true, origin_cursor,
                             nullptr);
            seq_cntr::Read(
                origin, origin_cursor, r_seg_work->ca.elem_cnt,
                seq_cntr::MemReader{
                    .data = data + elem_stride * l_seg_work->ca.elem_cnt,
                    .elem_size = elem_size,
                    .elem_stride = elem_stride,
                },
                nullptr);
        }

        SegWork* dst_seg_work;
        SegWork* other_seg_work;

        int side{ static_cast<int>(utils::SimpleRandomRotate(&random_seed) %
                                   2) };

        switch (side) {
        case 0:
            dst_seg_work = l_seg_work;
            other_seg_work = r_seg_work;
            break;
        case 1:
            dst_seg_work = r_seg_work;
            other_seg_work = l_seg_work;
            break;
        }

        dst_seg_work->color = dat_color;
        dst_seg_work->ca.data = data;
        dst_seg_work->ca.elem_cnt = total_elem_cnt;
        dst_seg_work->elem_vac = seg_elem_capacity - total_elem_cnt;

        other_seg_work->is_null = true;
        other_seg_work->elem_vac = 0;

        return side;
    }
#endif

    size_t move_l_cost{ EnStagingTernary(
        !(l_seg_work->color == ref_color) || l_need_data,
        l_seg_work->ca.elem_cnt, 0) };

    size_t move_r_cost{ EnStagingTernary(
        !(r_seg_work->color == ref_color) || r_need_data,
        r_seg_work->ca.elem_cnt, 0) };

    size_t l_based_merge_cost{ EnStagingTernary(
        l_seg_work->color == ref_color && l_need_data, total_elem_cnt,
        move_r_cost) };

    size_t r_based_merge_cost{ EnStagingTernary(
        r_seg_work->color == ref_color && r_need_data, total_elem_cnt,
        move_l_cost) };

    /*

    when l_seg_work->color == ref_color && l_need_data:
        l_based_merge_cost = total_elem_cnt

        we already eliminate l_seg_work->color == ref_color && r_seg_work->color
    == ref_color, so r_seg_work->color == ref_color is false, thus
        r_based_merge_cost = move_l_cost < total_elem_cnt

        side 1

    when l_seg_work->color == ref_color && !l_need_data:
        l_based_merge_cost = move_r_cost

        we already eliminate l_seg_work->color == ref_color && r_seg_work->color
    == ref_color, so r_seg_work->color == ref_color is false, thus move_r_cost =
    r_seg_work->ca.elem_cnt > 0

        move_l_cost = 0

        side 1

        vice versa


    */

    int side{ utils::Choose2(l_based_merge_cost <= r_based_merge_cost,
                             r_based_merge_cost <= l_based_merge_cost,
                             &random_seed) };

    switch (side) {
    case 0:
        SegShoveL_(origin, elem_size, elem_stride, seg_elem_capacity, data_size,
                   data_alctr, l_seg_work, r_seg_work, r_seg_work->ca.elem_cnt,
#if EnStaging
                   l_need_data, r_need_data,
#endif
                   seg_alctr, data_alctr);

        break;

    case 1:
        SegShoveR_(origin, elem_size, elem_stride, seg_elem_capacity, data_size,
                   data_alctr, l_seg_work, r_seg_work, l_seg_work->ca.elem_cnt,
#if EnStaging
                   l_need_data, r_need_data,
#endif
                   seg_alctr, data_alctr);

        break;
    }

    return side;
}

template <typename Origin, typename SegAllocator, typename DataAllocator>
void MergeL_(Origin* origin, SegAllocator* seg_alctr, DataAllocator* data_alctr,
             CircularArray* l_ca, Seg* r_seg, CircularArray* r_ca,
             size_t rl_elem_cnt, size_t rr_elem_cnt) {
#if EnStaging
    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };

    circular_array::PushR(l_ca, rl_elem_cnt + rr_elem_cnt,
                          seq_cntr::EmptyWriter{}, nullptr);

    if (GetNColor_(&r_seg->n) == ref_color) {
        seq_cntr::Access(origin, r_seg->ref.beg, true, origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(
            &l_ca, l_ca->elem_cnt - rl_elem_cnt - rr_elem_cnt, rl_elem_cnt,
            origin, origin_cursor, rl_elem_cnt);

        seq_cntr::Access(origin, r_seg->ref.beg + r_ca->elem_size - rr_elem_cnt,
                         true, origin_cursor, nullptr);

        circular_array::AssignFromSeqCntr(&l_ca, l_ca->elem_cnt - rr_elem_cnt,
                                          rr_elem_cnt, origin, origin_cursor,
                                          rl_elem_cnt);

        DeallocateRefSeg_(seg_alctr, r_seg);
    } else
#endif
    {
        circular_array::AssignFromCircularArray(
            l_ca, l_ca->elem_cnt - rl_elem_cnt - rr_elem_cnt, l_ca, 0,
            rl_elem_cnt);

        circular_array::AssignFromCircularArray(
            l_ca, l_ca->elem_cnt - rr_elem_cnt, r_ca, rl_elem_cnt + cnt,
            rr_elem_cnt);

        DeallocateDatSeg_(seg_alctr, data_alctr, r_seg);
    }
}

template <bool EnRead, CntrTplParamList, typename ReaderWriterCore>
void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
     ReaderWriterCore& reader_writer_core, Cursor* dst_cursor) {
    CheckCursor_(cntr, pos_cursor);

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->n = pos_cursor->n;
            dst_cursor->seg_idx = pos_cursor->seg_idx;
            dst_cursor->elem = pos_cursor->elem;
        }

        return;
    }

    ZETA_Core_DebugAssert(
        seq_cntr::IsDereferable(pos_cursor->idx, cnt, GetElemCnt(cntr)));

#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };
#endif

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t elem_cnt{ GetElemCnt(cntr) };

    TreeNode* rb{ cntr->rb };

#if EnStaging
    size_t data_size{ (GetDataSize_)(elem_size, elem_stride,
                                     seg_elem_capacity) };

    auto* seg_alctr{ utils::GetInstPtr(cntr->seg_alctr) };
    auto* data_alctr{ utils::GetInstPtr(cntr->data_alctr) };
#endif

    size_t idx{ pos_cursor->idx };

    ZETA_Core_DebugAssert(idx <= elem_cnt);
    ZETA_Core_DebugAssert(cnt <= elem_cnt - idx);

    size_t end{ idx + cnt };

    size_t seg_idx{ pos_cursor->seg_idx };

    TreeNode* n{ pos_cursor->n };
    Seg* seg;

    TreeNode* l_n{ nullptr };
    TreeNode* r_n{ nullptr };

#if EnStaging
    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };
#endif

    bool any_written{ false };

    unsigned long long random_seed{ utils::GetRandom() };

    for (;;) {
        seg = NToSeg_(n);

#if EnStaging
        if (GetNColor_(n) == dat_color)
#endif
        {
            size_t cur_cnt{ utils::Min(cnt, seg->dat.elem_cnt - seg_idx) };

            reader_writer_core.ReadWriteDat(seg->dat.data, seg->dat.idx_offset,
                                            seg_idx, cur_cnt);

            seg_idx += cur_cnt;

            if (seg_idx == seg->dat.elem_cnt) {
                l_n = n;
                n = r_n == nullptr ? bin_tree::StepR(n) : r_n;
                r_n = nullptr;

                seg_idx = 0;
            }

            cnt -= cur_cnt;

            any_written = true;

            if (cnt == 0) { break; }

            continue;
        }

#if EnStaging
        size_t seg_elem_cnt{ seg->ref.elem_cnt };

        size_t cnt_a{ seg_idx };
        size_t cnt_b{ utils::Min(cnt, seg_elem_cnt - seg_idx) };
        size_t cnt_c{ seg_elem_cnt - cnt_a - cnt_b };

        size_t needed_seg_cnt{ integral_math::CeilDiv(cnt_b,
                                                      seg_elem_capacity) };

        if (integral_math::CeilDiv(seg_elem_cnt, seg_elem_capacity) ==
            needed_seg_cnt) {
            goto WRITE;
        }

        {
            bool mat_ab_is_ok{ integral_math::CeilDiv(cnt_a + cnt_b,
                                                      seg_elem_capacity) ==
                               needed_seg_cnt };

            bool mat_bc_is_ok{ integral_math::CeilDiv(cnt_c + cnt_b,
                                                      seg_elem_capacity) ==
                               needed_seg_cnt };

            if (mat_ab_is_ok && mat_bc_is_ok) {
                if (utils::Choose2(cnt_a <= cnt_c, cnt_c <= cnt_a,
                                   &random_seed) == 0) {
                    goto MAT_AB;
                } else {
                    goto MAT_BC;
                }
            } else if (mat_ab_is_ok) {
                goto MAT_AB;
            } else if (mat_bc_is_ok) {
                goto MAT_BC;
            } else {
                goto MAT_B;
            }
        }

    MAT_AB: {
        Seg* new_seg{ AllocateRefSeg_(seg_alctr) };

        cntr->root = rbtree::InsertR(n, &new_seg->n);

        new_seg->ref.beg = seg->ref.beg + cnt_a + cnt_b;
        new_seg->ref.elem_cnt = cnt_c;

        seg->ref.elem_cnt = cnt_a + cnt_b;

        bin_tree::SetSize(&new_seg->n, new_seg->ref.elem_cnt);

        r_n = &new_seg->n;

        goto WRITE;
    }

    MAT_BC: {
        Seg* new_seg{ AllocateRefSeg_(seg_alctr) };

        cntr->root = rbtree::InsertL(n, &new_seg->n);

        new_seg->ref.beg = seg->ref.beg;
        new_seg->ref.elem_cnt = cnt_a;

        seg->ref.beg += cnt_a;
        seg->ref.elem_cnt = cnt_b + cnt_c;

        bin_tree::SetSize(&new_seg->n, new_seg->ref.elem_cnt);

        l_n = &new_seg->n;

        seg_idx = 0;

        if (!any_written) { pos_cursor->seg_idx = 0; }

        goto WRITE;
    }

    MAT_B: {
        Seg* new_l_seg{ AllocateRefSeg_(seg_alctr) };
        Seg* new_r_seg{ AllocateRefSeg_(seg_alctr) };

        cntr->root = rbtree::InsertL(n, &new_l_seg->n);
        cntr->root = rbtree::InsertR(n, &new_r_seg->n);

        new_l_seg->ref.beg = seg->ref.beg;
        new_l_seg->ref.elem_cnt = cnt_a;

        new_r_seg->ref.beg = seg->ref.beg + cnt_a + cnt_b;
        new_r_seg->ref.elem_cnt = cnt_c;

        seg->ref.beg += cnt_a;
        seg->ref.elem_cnt = cnt_b;

        bin_tree::SetSize(&new_l_seg->n, cnt_a);
        bin_tree::SetSize(&new_r_seg->n, cnt_c);

        l_n = &new_l_seg->n;
        r_n = &new_r_seg->n;

        seg_idx = 0;

        if (!any_written) { pos_cursor->seg_idx = 0; }

        goto WRITE;
    }

    WRITE: {
        ElemCntBalancer_ balancer{ seg->ref.elem_cnt, seg_elem_capacity };

        for (;;) {
            size_t new_seg_elem_cnt{ balancer.Fetch() };

            size_t old_ref_beg{ seg->ref.beg };

            if (balancer.res_seg_cnt == 0) {
                l_n = n;
                n = r_n == nullptr ? bin_tree::StepR(n) : r_n;
                r_n = nullptr;

                SetNColor_(l_n, dat_color);

                Seg* l_seg{ NToSeg_(l_n) };

                l_seg->dat.data = AllocateData_(data_alctr, data_size);
                l_seg->dat.elem_cnt =
                    static_cast<unsigned short>(new_seg_elem_cnt);
                l_seg->dat.idx_offset = 0;
            } else {
                Seg* new_seg{ AllocateDatSeg_(seg_alctr) };

                cntr->root = l_n == nullptr
                                 ? rbtree::InsertL(n, &new_seg->n)
                                 : rbtree::Insert(l_n, n, &new_seg->n);

                new_seg->dat.elem_cnt =
                    static_cast<unsigned short>(new_seg_elem_cnt);

                seg->ref.beg += new_seg_elem_cnt;
                seg->ref.elem_cnt -= new_seg_elem_cnt;

                l_n = &new_seg->n;
            }

            Seg* l_seg{ NToSeg_(l_n) };

            bin_tree::SetSize(&l_n, new_seg_elem_cnt);

            size_t cur_cnt{ utils::Min(cnt, new_seg_elem_cnt - seg_idx) };

            char* data{ static_cast<char*>(l_seg->dat.data) };

            if constexpr (EnRead) {
                seq_cntr::Access(origin, old_ref_beg, true, origin_cursor,
                                 nullptr);

                seq_cntr::Read(origin, origin_cursor, new_seg_elem_cnt,
                               seq_cntr::MemReader{
                                   .data = data,
                                   .elem_size = elem_size,
                                   .elem_stride = elem_stride,
                               },
                               nullptr);
            } else {
                if (0 < seg_idx) {
                    seq_cntr::Access(origin, old_ref_beg, true, origin_cursor,
                                     nullptr);

                    seq_cntr::Read(origin, origin_cursor, seg_idx,
                                   seq_cntr::MemReader{
                                       .data = data,
                                       .elem_size = elem_size,
                                       .elem_stride = elem_stride,
                                   },
                                   nullptr);
                }

                if (seg_idx + cur_cnt < new_seg_elem_cnt) {
                    seq_cntr::Access(origin, old_ref_beg + seg_idx + cur_cnt,
                                     true, origin_cursor, nullptr);

                    seq_cntr::Read(
                        origin, origin_cursor,
                        new_seg_elem_cnt - seg_idx - cur_cnt,
                        seq_cntr::MemReader{
                            .data = data + elem_stride * (seg_idx + cur_cnt),
                            .elem_size = elem_size,
                            .elem_stride = elem_stride,
                        },
                        nullptr);
                }
            }

            reader_writer_core.ReadWriteDat(data + elem_stride * seg_idx, 0, 0,
                                            cur_cnt);

            cnt -= cur_cnt;

            seg_idx += cur_cnt;

            if (seg_idx == new_seg_elem_cnt) { seg_idx = 0; }

            if (!any_written) {
                any_written = true;
                pos_cursor->n = l_n;
            }

            if (balancer.res_seg_cnt == 0) { break; }
        }
    }
#endif
    }

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = end;
    dst_cursor->n = n;
    dst_cursor->seg_idx = seg_idx;

    if (rb == n
#if EnStaging
        || GetNColor_(n) == ref_color
#endif
    ) {
        dst_cursor->elem = nullptr;
        return;
    }

    seg = NToSeg_(n);

    CircularArray ca{
        .data = seg->dat.data,
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = seg->dat.elem_cnt,
        .elem_capacity = seg_elem_capacity,
        .idx_offset = seg->dat.idx_offset,
    };

    dst_cursor->elem =
        circular_array::Access(&ca, seg_idx, true, nullptr, nullptr);
}

template <CntrTplParamList>
void InitTree_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr) {
    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    lb->Init();
    rb->Init();

    lb->SetAccSize(1);
    rb->SetAccSize(1);

    TreeNode* root{ nullptr };

    root = rbtree::InsertR(root, lb);
    root = rbtree::InsertR(root, rb);

    cntr->root = root;
}

#if EnStaging

template <CntrTplParamList>
void RefOrigin_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr) {
    auto* origin{ utils::GetInstPtr(cntr->origin) };

    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    size_t origin_elem_cnt{ seq_cntr::GetElemCnt(origin) };

    if (origin_elem_cnt == 0) { return; }

    Seg* seg{ (AllocateRefSeg_)(utils::GetInstPtr(cntr->seg_alctr)) };

    seg->n.SetAccSize(origin_elem_cnt);

    cntr->root = rbtree::Insert(lb, rb, &seg->n);

    seg->ref.beg = 0;
    seg->ref.elem_cnt = origin_elem_cnt;
}

#endif

}  // namespace NameSpace::detail

template <typename SegAllocatorLike, typename DataAllocatorLike>
void* NameSpace::CopyTree(
    size_t elem_size, size_t elem_stride, size_t seg_elem_capacity,
    TreeNode* src_root, TreeNode* src_lb, TreeNode* src_rb, TreeNode* dst_lb,
    TreeNode* dst_rb,
    SegAllocatorLike&&
        seg_alctr  // NOLINT(cppcoreguidelines-missing-std-forward)
    ,
    DataAllocatorLike&&
        data_alctr  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    size_t data_size{ detail::GetDataSize_(elem_size, elem_stride,
                                           seg_elem_capacity) };

    constexpr size_t buffer_capacity{ rbtree::recommended_buffer_capacity };

    struct {
        TreeNode* p_n;
        TreeNode* src_n;
        int dir;
    } buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[++buffer_i] = { src_root, nullptr };

    CircularArray ca{
        .data = {},
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = seg_elem_capacity,
        .elem_capacity = seg_elem_capacity,
        .idx_offset = {},
    };

    TreeNode* dst_root{ nullptr };

    while (0 < buffer_i) {
        auto [p_n, src_n, dir]{ buffer[--buffer_i] };

        TreeNode* src_nl{ bin_tree::GetL(src_n) };
        TreeNode* src_nr{ bin_tree::GetR(src_n) };

        TreeNode* dst_n{ nullptr };

        if (src_lb == src_n) {
            dst_n = dst_lb;
            dst_n->Init();
            bin_tree::SetSize(dst_n, 1);
        } else if (src_rb == src_n) {
            dst_n = dst_rb;
            dst_n->Init();
            bin_tree::SetSize(dst_n, 1);
        } else {
            Seg* seg{ AllocateSeg_(seg_alctr) };
            dst_n = &seg->n;
            dst_n->Init();

            Seg* src_seg{ detail::NToSeg_(src_n) };

#if EnStaging
            unsigned color{ GetNColor_(src_n) };
            DirectlySetNColor_(dst_n, color);
#endif

#if EnStaging
            if (color == ref_color) {
                seg->ref.beg = src_seg->ref.beg;
                seg->ref.elem_cnt = src_seg->ref.elem_cnt;

                bin_tree::SetSize(dst_n, seg->ref.elem_cnt);
            } else
#endif
            {
                seg->dat.data = AllocateData_(seg_alctr, data_alctr, data_size);
                seg->dat.elem_cnt = src_seg->dat.elem_cnt;
                seg->dat.idx_offset = 0;

                ca.data = src_seg->dat.data;
                ca.idx_offset = src_seg->dat.idx_offset;

                circular_array::IdxRead(&ca, 0, ca.elem_cnt, seg->dat.data,
                                        elem_stride, nullptr);

                bin_tree::SetSize(dst_n, ca.elem_cnt);
            }
        }

        dst_n->SetPColor(src_n->GetPColor());

        if (p_n == nullptr) {
            dst_root = dst_n;
        } else {
            switch (dir) {
            case 0: bin_tree::AttatchL(p_n, dst_n); break;
            case 1: bin_tree::AttatchR(p_n, dst_n); break;
            }
        }

        if (src_nl != nullptr) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);
            buffer[buffer_i++] = { src_n, src_nl, 0 };
        }

        if (src_nr != nullptr) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);
            buffer[buffer_i++] = { src_n, src_nr, 1 };
        }
    }

    return dst_root;
}

template <typename SegAllocatorLike, typename DataAllocatorLike>
utils::Pair<NameSpace::TreeNode*, NameSpace::TreeNode*> NameSpace::EraseTree(
    TreeNode* root,
    SegAllocatorLike&&
        seg_alctr_  // NOLINT(cppcoreguidelines-missing-std-forward)
    ,
    DataAllocatorLike&&
        data_alctr_  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    auto* seg_alctr{ utils::GetInstPtr(seg_alctr_) };
    auto* data_alctr{ utils::GetInstPtr(data_alctr_) };

    constexpr size_t buffer_capacity{ rbtree::recommended_buffer_capacity };

    struct {
        TreeNode* n;
        bool l_walked;
        bool r_walked;
    } buffer[buffer_capacity];

    size_t buffer_i{ 0 };

    buffer[buffer_i++] = { root, false, false };

    utils::Pair<TreeNode*, TreeNode*> ret;

    while (0 < buffer_i) {
        auto [n, l_walked, r_walked]{ buffer[--buffer_i] };

        TreeNode* nl{ bin_tree::GetL(n) };
        TreeNode* nr{ bin_tree::GetR(n) };

        bool is_b{ false };

        if (nl != nullptr) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);
            buffer[buffer_i++] = { nl, true, r_walked };
        } else if (!r_walked) {
            is_b = true;
            ret.first = n;
        }

        if (nr != nullptr) {
            ZETA_Core_DebugAssert(buffer_i < buffer_capacity);
            buffer[buffer_i++] = { nr, l_walked, true };
        } else if (!l_walked) {
            is_b = true;
            ret.second = n;
        }

        if (!is_b) {
            detail::DeallocateSeg_(seg_alctr, data_alctr, detail::NToSeg_(n));
        }
    }

    return ret;
}

template <CntrTplParamList,
#if EnStaging
          typename OriginLikeInitArg,
#endif
          typename SegAllocatorLikeInitArg, typename DataAllocatorLikeInitArg>
void NameSpace::Init(Cntr<CntrTplArgList>* cntr,
#if EnStaging
                     OriginLikeInitArg&& origin_like_init_arg,
#else
                     size_t elem_size,
#endif
                     size_t elem_stride, size_t seg_elem_capacity,
                     SegAllocatorLikeInitArg&& seg_alctr_like_init_arg,
                     DataAllocatorLikeInitArg&& data_alctr_like_init_arg) {
    ZETA_Core_DebugAssert(cntr != nullptr);

#if EnStaging
    lifecycle::Init(cntr->origin,
                    meta::Forward<OriginLikeInitArg>(origin_like_init_arg));
    auto* origin{ utils::GetInstPtr(cntr->origin) };
    seq_cntr::CheckContract(origin);
#endif

    ZETA_Core_DebugAssert(elem_size <= elem_stride);
    ZETA_Core_DebugAssert(0 < seg_elem_capacity);
    ZETA_Core_DebugAssert(cntr->seg_elem_capacity <= max_seg_elem_capacity);

    lifecycle::Init(cntr->seg_alctr, meta::Forward<SegAllocatorLikeInitArg>(
                                         seg_alctr_like_init_arg));
    lifecycle::Init(cntr->data_alctr, meta::Forward<DataAllocatorLikeInitArg>(
                                          data_alctr_like_init_arg));

#if EnStaging
    cntr->elem_size = seq_cntr::GetElemSize(origin);
#else
    cntr->elem_size = elem_size;
#endif

    cntr->elem_stride = elem_stride;

    cntr->seg_elem_capacity = seg_elem_capacity;

    cntr->lb = static_cast<TreeNode*>(allocator::SafeAllocate(
        cntr->seg_alctr, alignof(TreeNode), sizeof(TreeNode)));

    cntr->rb = static_cast<TreeNode*>(allocator::SafeAllocate(
        cntr->seg_alctr, alignof(TreeNode), sizeof(TreeNode)));

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
          typename SrcCntrTplArgList, typename SrcSegAllocatorLike,
          typename SrcDataAllocatorLike>
void NameSpace::Init(Cntr<CntrTplArgList>* cntr,
#if EnStaging
                     OriginLikeInitArg&& origin_like_init_arg,
#else
                     size_t elem_size,
#endif
                     size_t elem_stride, size_t seg_elem_capacity,
                     SegAllocatorLikeInitArg&& seg_alctr_like_init_arg,
                     DataAllocatorLikeInitArg&& data_alctr_like_init_arg,
                     Cntr<
#if EnStaging
                         SrcOriginLike,
#endif
                         SrcSegAllocatorLike, SrcDataAllocatorLike>* src_cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);

#if EnStaging
    lifecycle::Init(cntr->origin,
                    meta::Forward<OriginLikeInitArg>(origin_like_init_arg));
    auto* origin{ utils::GetInstPtr(cntr->origin) };
    seq_cntr::CheckContract(origin);

    ZETA_Core_DebugAssert(seq_cntr::GetReferedInstPtr(cntr->origin) ==
                          seq_cntr::GetReferedInstPtr(src_cntr->origin));
#endif

    ZETA_Core_DebugAssert(elem_size <= elem_stride);
    ZETA_Core_DebugAssert(0 < seg_elem_capacity);
    ZETA_Core_DebugAssert(cntr->seg_elem_capacity <= max_seg_elem_capacity);

    lifecycle::Init(cntr->seg_alctr, meta::Forward<SegAllocatorLikeInitArg>(
                                         seg_alctr_like_init_arg));
    lifecycle::Init(cntr->data_alctr, meta::Forward<DataAllocatorLikeInitArg>(
                                          data_alctr_like_init_arg));

#if EnStaging
    cntr->elem_size = seq_cntr::GetElemSize(origin);
#else
    cntr->elem_size = elem_size;
#endif

    cntr->elem_stride = elem_stride;

    cntr->seg_elem_capacity = seg_elem_capacity;

    TreeNode* lb{ static_cast<TreeNode*>(allocator::SafeAllocate(
        cntr->seg_alctr, alignof(TreeNode), sizeof(TreeNode))) };

    TreeNode* rb{ static_cast<TreeNode*>(allocator::SafeAllocate(
        cntr->seg_alctr, alignof(TreeNode), sizeof(TreeNode))) };

    cntr->lb = lb;
    cntr->rb = rb;

    size_t n_cnt{ 1 };

    {
        TreeNode* last_n{ lb };

        TreeNode* src_lb{ src_cntr->lb };
        TreeNode* src_rb{ src_cntr->rb };

        TreeNode* src_n{ bin_tree::StepR(src_lb) };

        CircularArray ca{
            .data = {},
            .elem_size = elem_size,
            .elem_stride = src_cntr->elem_stride,
            .elem_cnt = src_cntr->seg_elem_capacity,
            .elem_capacity = src_cntr->seg_elem_capacity,
            .idx_offset = {},
        };

        while (src_n != src_rb) {
            Seg* src_seg{ detail::NToSeg_(src_n) };

#if EnStaging
            if (GetNColor_(src_n) == ref_color) {
                Seg* dst_seg{ detail::AllocateRefSeg_(
                    utils::GetInstPtr(cntr->seg_alctr)) };

                dst_seg->ref.beg = src_seg->ref.beg;
                dst_seg->ref.elem_cnt = src_seg->ref.elem_cnt;

                bin_tree::SetR(last_n, &dst_seg->n);

                last_n = &dst_seg->n;
                ++n_cnt;

                src_n = bin_tree::StepR(src_n);

                continue;
            }
#endif

            size_t acc_elem_cnt{ 0 };
            TreeNode* nxt_src_n{ src_n };

            do {
                acc_elem_cnt += NToSeg_(nxt_src_n)->dat.elem_cnt;
                nxt_src_n = bin_tree::StepR(nxt_src_n);
            } while (nxt_src_n != src_rb
#if EnStaging
                     && GetNColor_(nxt_src_n) == dat_color
#endif
            );

            unsigned long long random_seed{ utils::GetRandom() };

            size_t src_idx{ 0 };
            size_t dst_cnt{ 0 };

            Seg* dst_seg;

            ca.data = src_seg->dat.data;
            ca.idx_offset = src_seg->dat.idx_offset;

            for (;;) {
                size_t seg_elem_cnt{ src_seg->dat.elem_cnt };

                if (dst_cnt == 0) {
                    if (acc_elem_cnt == 0) { break; }

                    dst_cnt = detail::GetAvgCnt_(
                        acc_elem_cnt, seg_elem_capacity, random_seed);
                    acc_elem_cnt -= dst_cnt;

                    dst_seg = detail::AllocateDatSeg_(
                        utils::GetInstPtr(cntr->seg_alctr));

                    bin_tree::SetR(last_n, &dst_seg->n);

                    last_n = &dst_seg->n;
                    ++n_cnt;
                }

                if (src_idx == src_seg->dat.elem_cnt) {
                    src_n = bin_tree::StepR(src_n);
                    src_seg = detail::NToSeg_(src_n);

                    src_idx = 0;

                    ca.data = src_seg->dat.data;
                    ca.idx_offset = src_seg->dat.idx_offset;
                }

                size_t cur_cnt{ utils::Min(dst_cnt, seg_elem_cnt - src_idx) };

                circular_array::IdxRead(
                    &ca, src_idx, cur_cnt,
                    seq_cntr::MemReader{ .data = dst_seg->dat.data,
                                         .elem_size = elem_size,
                                         .elem_stride = elem_stride });

                src_idx += cur_cnt;
                dst_cnt -= cur_cnt;
            }
        }
    }
}

}

/*
template <CntrTplParamList>
void NameSpace::CopyInit(Cntr<CntrTplArgList>* cntr,
                              Cntr<CntrTplArgList> const* src_cntr) {
    ZETA_Core_DebugAssert(cntr != nullptr);
    detail::CheckCntr_(cntr);

#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };
    seq_cntr::CheckContract(origin);

    // if constexpr ()

    cntr->elem_size = seq_cntr::GetElemSize(origin);
#endif

    size_t seg_elem_capacity{ cntr->seg_elem_capacity };

    ZETA_Core_DebugAssert(0 < seg_elem_capacity);
    ZETA_Core_DebugAssert(cntr->seg_elem_capacity <= max_seg_elem_capacity);

    cntr->lb = static_cast<TreeNode*>(allocator::SafeAllocate(
        cntr->seg_alctr, alignof(TreeNode), sizeof(TreeNode)));

    cntr->rb = static_cast<TreeNode*>(allocator::SafeAllocate(
        cntr->seg_alctr, alignof(TreeNode), sizeof(TreeNode)));

    detail::InitTree_(cntr);

#if EnStaging
    detail::RefOrigin_(cntr);
#endif
}
*/

template <CntrTplParamList>
void NameSpace::Deinit(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr_(cntr);

    EraseTree(cntr->root, cntr->seg_alctr, cntr->data_alctr);

    allocator::Deallocate(cntr->seg_alctr, cntr->lb);
    allocator::Deallocate(cntr->seg_alctr, cntr->rb);
}

template <CntrTplParamList>
constexpr size_t NameSpace::GetCursorSize(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return sizeof(Cursor);
}

template <CntrTplParamList>
size_t NameSpace::GetElemSize(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->elem_size;
}

template <CntrTplParamList>
size_t NameSpace::GetElemCnt(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return cntr->root->GetAccSize() - 2;
}

template <CntrTplParamList>
size_t NameSpace::GetMaxElemCnt(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
void NameSpace::GetLBCursor(Cntr<CntrTplArgList> const* cntr,
                            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = static_cast<size_t>(-1);
    dst_cursor->n = cntr->lb;
    dst_cursor->seg_idx = 0;
    dst_cursor->elem = nullptr;
}

template <CntrTplParamList>
void NameSpace::GetRBCursor(Cntr<CntrTplArgList> const* cntr,
                            Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->cntr = cntr;
    dst_cursor->idx = GetElemCnt(cntr);
    dst_cursor->n = cntr->rb;
    dst_cursor->seg_idx = 0;
    dst_cursor->elem = nullptr;
}

template <CntrTplParamList>
void* NameSpace::PeekL(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
                       Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(cntr);

#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };
#endif

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };

    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    TreeNode* n{ bin_tree::StepR(lb) };

    if (rb == n) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
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
        seq_cntr::Access(origin, seg->ref.beg, false, nullptr, dst_elem);
        elem = nullptr;
    } else
#endif
    {
        CircularArray ca{
            .data = seg->dat.data,
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = seg->dat.elem_cnt,
            .elem_capacity = seg_elem_capacity,
            .idx_offset = seg->dat.idx_offset,
        };

        elem = circular_array::PeekL(&ca, lazy_copy_elem, nullptr, dst_elem);
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = 0;
        dst_cursor->n = n;
        dst_cursor->seg_idx = 0;
        dst_cursor->elem = elem;
    }

    return elem;
}

template <CntrTplParamList>
void* NameSpace::PeekR(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
                       Cursor* dst_cursor, void* dst_elem) {
    detail::CheckCntr_(cntr);

#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };

#endif

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t elem_cnt{ GetElemCnt(cntr) };

    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    TreeNode* n{ bin_tree::StepL(rb) };

    if (lb == n) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
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

        seq_cntr::Access(origin, seg->ref.beg + seg_elem_cnt - 1, false,
                         nullptr, dst_elem);

        elem = nullptr;
    } else
#endif
    {
        seg_elem_cnt = seg->dat.elem_cnt;

        CircularArray ca{
            .data = seg->dat.data,
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = seg_elem_cnt,
            .elem_capacity = seg_elem_capacity,
            .idx_offset = seg->dat.idx_offset,
        };

        elem = circular_array::PeekR(&ca, lazy_copy_elem, nullptr, dst_elem);
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = elem_cnt - 1;
        dst_cursor->n = n;
        dst_cursor->seg_idx = seg_elem_cnt - 1;
        dst_cursor->elem = elem;
    }

    return elem;
}

template <CntrTplParamList>
void* NameSpace::Access(Cntr<CntrTplArgList> const* cntr, size_t idx,
                        bool lazy_copy_elem, Cursor* dst_cursor,
                        void* dst_elem) {
    detail::CheckCntr_(cntr);

#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };
#endif

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t elem_cnt{ GetElemCnt(cntr) };

    ZETA_Core_DebugAssert(seq_cntr::IsReferable(idx, 1, elem_cnt));

    auto [n, seg_idx]{ bin_tree::AccessL(cntr->root, idx + 1) };

    void* elem{ nullptr };

    if (idx < elem_cnt) {
        Seg* seg{ detail::NToSeg_(n) };

#if EnStaging
        if (detail::GetNColor_(n) == ref_color) {
            if (dst_elem != nullptr) {
                seq_cntr::Access(origin, seg->ref.beg + seg_idx, false, nullptr,
                                 dst_elem);
            }
        } else
#endif
        {
            CircularArray ca{
                .data = seg->dat.data,
                .elem_size = elem_size,
                .elem_stride = elem_stride,
                .elem_cnt = seg->dat.elem_cnt,
                .elem_capacity = seg_elem_capacity,
                .idx_offset = seg->dat.idx_offset,
            };

            elem = circular_array::Access(&ca, seg_idx, lazy_copy_elem, nullptr,
                                          dst_elem);
        }
    }

    if (dst_cursor != nullptr) {
        dst_cursor->cntr = cntr;
        dst_cursor->idx = idx;
        dst_cursor->n = n;
        dst_cursor->seg_idx = seg_idx;
        dst_cursor->elem = elem;
    }

    return elem;
}

template <CntrTplParamList>
void* NameSpace::Derefer(Cntr<CntrTplArgList> const* cntr,
                         Cursor const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem) {
    detail::CheckCursor_(cntr, pos_cursor);

#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };
#endif

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };

#if EnStaging
    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };
#endif

    void* elem{ pos_cursor->elem };

    if (elem == nullptr) {
#if EnStaging
        if (pos_cursor->n != lb && pos_cursor->n != rb && dst_elem != nullptr) {
            Seg* seg{ detail::NToSeg_(pos_cursor->n) };

            seq_cntr::Access(origin, seg->ref.beg + pos_cursor->seg_idx, false,
                             nullptr, dst_elem);
        }
#endif

        return nullptr;
    }

    if (dst_elem == nullptr || lazy_copy_elem) { return elem; }

    Seg* seg{ detail::NToSeg_(pos_cursor->n) };

    CircularArray ca{
        .data = seg->dat.data,
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = seg_elem_capacity,
        .elem_capacity = seg_elem_capacity,
        .idx_offset = seg->dat.idx_offset,
    };

    circular_array::Access(&ca, pos_cursor->seg_idx, false, nullptr, dst_elem);

    return elem;
}

template <CntrTplParamList, typename Reader>
void NameSpace::Read(
    Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    ZETA_Core_DebugAssert(
        seq_cntr::IsDereferable(pos_cursor->idx, cnt, GetElemCnt(cntr)));

#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };

#endif

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };

    TreeNode* rb{ cntr->rb };

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
            dst_cursor->elem = pos_cursor->elem;
        }

        return;
    }

#if EnStaging
    void* origin_cursor{ ZETA_Core_SeqCntr_AllocaCursor(origin) };
#endif

    CircularArray ca{
        .data = {},
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = seg_elem_capacity,
        .elem_capacity = seg_elem_capacity,
        .idx_offset = {},
    };

    while (0 < cnt) {
        seg = detail::NToSeg_(n);

        size_t cur_cnt;
        size_t seg_elem_cnt;

#if EnStaging
        if (detail::GetNColor_(n) == ref_color) {
            seg_elem_cnt = seg->ref.elem_cnt;
            cur_cnt = utils::Min(cnt, seg_elem_cnt - seg_idx);

            seq_cntr::Access(origin, seg->ref.beg + seg_idx, true,
                             origin_cursor, nullptr);

            seq_cntr::Read(origin, origin_cursor, cur_cnt, reader, nullptr);
        } else
#endif
        {
            seg_elem_cnt = seg->dat.elem_cnt;
            cur_cnt = utils::Min(cnt, seg_elem_cnt - seg_idx);

            ca.data = seg->dat.data;
            ca.idx_offset = seg->dat.idx_offset;

            circular_array::IdxRead(&ca, seg_idx, cur_cnt, reader);
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

    if (rb == n
#if EnStaging
        || detail::GetNColor_(n) == ref_color
#endif
    ) {
        dst_cursor->elem = nullptr;
        return;
    }

    seg = detail::NToSeg_(n);

    ca.data = seg->dat.data;
    ca.idx_offset = seg->dat.idx_offset;

    dst_cursor->elem =
        circular_array::Access(&ca, seg_idx, true, nullptr, nullptr);
}

template <CntrTplParamList, typename Writer>
void NameSpace::Write(
    Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };

    struct {
        Writer&&
            writer;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        CircularArray ca;

        void ReadWriteDat(void* data, size_t offset, size_t idx, size_t cnt) {
            this->ca.data = data;
            this->ca.idx_offset = offset;

            circular_array::IdxWrite(&this->ca, idx, cnt, this->writer);
        }
    } reader_core{
        .writer = meta::Forward<Writer>(writer),

        .ca = {
            .data = {},
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = seg_elem_capacity,
            .elem_capacity = seg_elem_capacity,
            .idx_offset = {},
        },
    };

    detail::ReadWrite_<false>(cntr, pos_cursor, cnt, reader_core, dst_cursor);
}

template <CntrTplParamList, typename ReaderWriter>
void NameSpace::ReadWrite(
    Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };

    struct {
        ReaderWriter&&
            reader_writer;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        CircularArray ca;

        void ReadWriteDat(void* data, size_t offset, size_t idx, size_t cnt) {
            this->ca.data = data;
            this->ca.idx_offset = offset;

            circular_array::IdxWrite(&this->ca, idx, cnt, this->reader_writer);
        }
    } reader_writer_core{
        .reader_writer = meta::Forward<ReaderWriter>(reader_writer),

        .ca = {
            .data = {},
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = seg_elem_capacity,
            .elem_capacity = seg_elem_capacity,
            .idx_offset = {},
        },
    };

    detail::ReadWrite_<true>(cntr, pos_cursor, cnt, reader_writer_core,
                             dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* NameSpace::PushL(Cntr<CntrTplArgList>* cntr, size_t cnt, Writer&& writer,
                       Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    Cursor pos_cursor;
    (PeekL)(cntr, true, &pos_cursor, nullptr);

    return (Insert)(cntr, &pos_cursor, cnt, meta::Forward<Writer>(writer),
                    dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* NameSpace::PushR(Cntr<CntrTplArgList>* cntr, size_t cnt, Writer&& writer,
                       Cursor* dst_cursor) {
    detail::CheckCntr_(cntr);

    Cursor pos_cursor;
    (GetRBCursor)(cntr, &pos_cursor);

    return (Insert)(cntr, &pos_cursor, cnt, meta::Forward<Writer>(writer),
                    dst_cursor);
}

template <CntrTplParamList, typename Writer>
void* NameSpace::Insert(
    Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, pos_cursor);

    ZETA_Core_DebugAssert(seq_cntr::IsInsertable(
        pos_cursor->idx, cnt, GetElemCnt(cntr), GetMaxElemCnt(cntr)));

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->cntr = cntr;
            dst_cursor->idx = pos_cursor->idx;
            dst_cursor->n = pos_cursor->n;
            dst_cursor->seg_idx = pos_cursor->seg_idx;
            dst_cursor->elem = pos_cursor->elem;
        }

        return pos_cursor->elem;
    }

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t data_size{ detail::GetDataSize_(elem_size, elem_stride,
                                           seg_elem_capacity) };

    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    auto* seg_alctr{ utils::GetInstPtr(cntr->seg_alctr) };
    auto* data_alctr{ utils::GetInstPtr(cntr->data_alctr) };

    size_t end_idx{ pos_cursor->idx + cnt };

    TreeNode* m_n{ pos_cursor->n };
    size_t seg_idx{ pos_cursor->seg_idx };

    ZETA_Core_DebugAssert(lb != m_n);

    unsigned long long random_seed{ utils::GetRandom() };

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

    size_t l_elem_vac;
    size_t m_elem_vac;
    size_t r_elem_vac;

    CircularArray l_ca{
        .data = {},
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = {},
        .elem_capacity = seg_elem_capacity,
        .idx_offset = {},
    };

    CircularArray m_ca{
        .data = {},
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = {},
        .elem_capacity = seg_elem_capacity,
        .idx_offset = {},
    };

    CircularArray r_ca{
        .data = {},
        .elem_size = elem_size,
        .elem_stride = elem_stride,
        .elem_cnt = {},
        .elem_capacity = seg_elem_capacity,
        .idx_offset = {},
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

        if (lb == l_n) {
            l_seg = nullptr;
            l_ca.elem_cnt = 0;
            l_elem_vac = 0;
        } else {
            l_seg = detail::NToSeg_(l_n);

            detail::GetSegState_(seg_elem_capacity, l_seg,
#if EnStaging
                                 &l_color,
#endif
                                 &l_ca, &l_elem_vac);
        }

        if (rb == r_n) {
            r_seg = nullptr;
            r_ca.elem_cnt = 0;
            r_elem_vac = 0;
        } else {
            r_seg = detail::NToSeg_(r_n);

            detail::GetSegState_(seg_elem_capacity, r_seg,
#if EnStaging
                                 &r_color,
#endif
                                 &r_ca, &r_elem_vac);
        }
    } else {
        m_seg = detail::NToSeg_(m_n);

        detail::GetSegState_(seg_elem_capacity, m_seg,
#if EnStaging
                             &m_color,
#endif
                             &m_ca, &m_elem_vac);

        if (cnt <= m_elem_vac) {
            /*

            if the insertion segment can hold all old + new data, just insert
            into it.

            */

            void* elem;

#if EnStaging
            if (m_color == ref_color) {
                m_ca.data = detail::AllocateData_(data_alctr, data_size);
                m_ca.idx_offset = 0;
                m_ca.elem_cnt = 0;

                detail::RefShoveL_(cntr, &m_ca, m_seg, seg_idx, cnt,
                                   m_seg->ref.elem_cnt + cnt, writer);

                detail::SetNColor_(&m_seg->n, dat_color);

                m_seg->dat.data = m_ca.data;

                elem = circular_array::Access(&m_ca, seg_idx, true, nullptr,
                                              nullptr);
            } else
#endif
            {
                elem = circular_array::IdxInsert(&m_ca, seg_idx, cnt, writer);
            }

            m_seg->dat.elem_cnt = static_cast<unsigned short>(m_ca.elem_cnt);
            m_seg->dat.idx_offset =
                static_cast<unsigned short>(m_ca.idx_offset);

            bin_tree::SetSize(m_n, m_ca.elem_cnt);

            pos_cursor->n = m_n;
            pos_cursor->seg_idx = seg_idx;
            pos_cursor->elem = elem;

            if (dst_cursor != nullptr) {
                dst_cursor->cntr = cntr;
                dst_cursor->idx = end_idx;
                dst_cursor->n = m_n;
                dst_cursor->seg_idx = seg_idx + cnt;
                dst_cursor->elem = circular_array::Access(
                    &m_ca, seg_idx + cnt, true, nullptr, nullptr);
            }

            return elem;
        }

        l_n = bin_tree::StepL(m_n);
        r_n = bin_tree::StepR(m_n);

        if (lb == l_n) {
            l_seg = nullptr;
            l_ca.elem_cnt = 0;
            l_elem_vac = 0;
        } else {
            l_seg = detail::NToSeg_(l_n);

            detail::GetSegState_(seg_elem_capacity, l_seg,
#if EnStaging
                                 &l_color,
#endif
                                 &l_ca, &l_elem_vac);
        }

        if (rb == r_n) {
            r_seg = nullptr;
            r_ca.elem_cnt = 0;
            r_elem_vac = 0;
        } else {
            r_seg = detail::NToSeg_(r_n);

            detail::GetSegState_(seg_elem_capacity, r_seg,
#if EnStaging
                                 &r_color,
#endif
                                 &r_ca, &r_elem_vac);
        }

        size_t ml_elem_cnt{ seg_idx };
        size_t mr_elem_cnt{ m_ca.elem_cnt - seg_idx };

        size_t new_l_m_elem_cnt{ l_ca.elem_cnt + m_ca.elem_cnt + cnt };
        size_t new_m_r_elem_cnt{ m_ca.elem_cnt + r_ca.elem_cnt + cnt };

        size_t l_m_elem_vac{ l_elem_vac + m_elem_vac };
        size_t m_r_elem_vac{ m_elem_vac + r_elem_vac };

        bool l_m_ok{ cnt <= l_m_elem_vac };
        bool m_r_ok{ cnt <= m_r_elem_vac };

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

        switch (utils::Choose2(l_m_ok && m_r_elem_vac <= l_m_elem_vac,
                               m_r_ok && l_m_elem_vac <= m_r_elem_vac,
                               &random_seed)) {
        case 0: {
            // use l + m to hold all old + new data

#if EnStaging
            if (l_color == ref_color) {
                detail::TransferRefSegToDatSeg_(cntr, l_seg);
                l_color = dat_color;
                l_ca.data = l_seg->dat.data;
                l_ca.idx_offset = l_seg->dat.idx_offset;
            }
#endif

            size_t ret_seg_idx{ l_ca.elem_cnt + ml_elem_cnt };

            size_t new_l_size{
                (new_l_m_elem_cnt + (m_ca.elem_cnt <= l_ca.elem_cnt)) / 2
            };

#if EnStaging
            if (m_color == ref_color) {
                detail::RefShoveL_(cntr, &l_ca, m_seg, ml_elem_cnt, cnt,
                                   new_l_size - l_ca.elem_cnt, writer);

                if (m_color == ref_color) {
                    m_ca.elem_cnt = m_seg->ref.elem_cnt;
                } else {
                    m_ca.data = m_seg->dat.data;
                    m_ca.elem_cnt = m_seg->dat.elem_cnt;
                    m_ca.idx_offset = m_seg->dat.idx_offset;
                }
            } else
#endif
            {
                seg_utils::SegShoveL(&l_ca, &m_ca, ml_elem_cnt, cnt,
                                     new_l_size - l_ca.elem_cnt, writer);

                m_seg->dat.elem_cnt =
                    static_cast<unsigned short>(m_ca.elem_cnt);
                m_seg->dat.idx_offset =
                    static_cast<unsigned short>(m_ca.idx_offset);
            }

            bin_tree::SetSize(m_n, m_ca.elem_cnt);

            l_seg->dat.elem_cnt = static_cast<unsigned short>(l_ca.elem_cnt);
            l_seg->dat.idx_offset =
                static_cast<unsigned short>(l_ca.idx_offset);

            bin_tree::SetSize(l_n, l_ca.elem_cnt);

            if (ret_seg_idx < l_ca.elem_cnt) {
                pos_cursor->n = l_n;
                pos_cursor->seg_idx = ret_seg_idx;
                pos_cursor->elem = circular_array::Access(
                    &l_ca, ret_seg_idx, true, nullptr, nullptr);
            } else {
                ret_seg_idx -= l_ca.elem_cnt;

                pos_cursor->n = m_n;
                pos_cursor->seg_idx = ret_seg_idx;

#if EnStaging
                if (m_color == ref_color) {
                    pos_cursor->elem = nullptr;
                } else
#endif
                {
                    pos_cursor->elem = circular_array::Access(
                        &m_ca, ret_seg_idx, true, nullptr, nullptr);
                }
            }

            if (dst_cursor != nullptr) {
                dst_cursor->cntr = cntr;
                dst_cursor->idx = end_idx;
                dst_cursor->n = m_n;
                dst_cursor->seg_idx = m_ca.elem_cnt - mr_elem_cnt;
                dst_cursor->elem = EnStagingTernary(
                    m_color == ref_color, nullptr,
                    circular_array::Access(&m_ca, m_ca.elem_cnt - mr_elem_cnt,
                                           true, nullptr, nullptr));
            }

            break;
        }

        case 1: {
            // use m + r to hold all old + new data

#if EnStaging
            if (r_color == ref_color) {
                detail::TransferRefSegToDatSeg_(cntr, r_seg);
                r_color = dat_color;
                r_ca.data = r_seg->dat.data;
                r_ca.idx_offset = r_seg->dat.idx_offset;
            }
#endif

            size_t ret_seg_idx{ seg_idx };

            size_t old_r_size{ r_ca.elem_cnt };
            size_t new_r_size{
                (new_m_r_elem_cnt + (m_ca.elem_cnt <= r_ca.elem_cnt)) / 2
            };

#if EnStaging
            if (m_color == ref_color) {
                detail::RefShoveR_(cntr, m_seg, &r_ca, mr_elem_cnt, cnt,
                                   new_r_size - r_ca.elem_cnt, writer);

                m_ca.elem_cnt = m_seg->ref.elem_cnt;
            } else
#endif
            {
                seg_utils::SegShoveR(&m_ca, &r_ca, mr_elem_cnt, cnt,
                                     new_r_size - r_ca.elem_cnt, writer);

                m_seg->dat.idx_offset =
                    static_cast<unsigned short>(m_ca.idx_offset);
                m_seg->dat.elem_cnt =
                    static_cast<unsigned short>(m_ca.elem_cnt);
            }

            bin_tree::SetSize(m_n, m_ca.elem_cnt);

            r_seg->dat.idx_offset =
                static_cast<unsigned short>(r_ca.idx_offset);
            r_seg->dat.elem_cnt = static_cast<unsigned short>(r_ca.elem_cnt);

            bin_tree::SetSize(r_n, r_ca.elem_cnt);

            if (ret_seg_idx < m_ca.elem_cnt) {
                pos_cursor->n = m_n;
                pos_cursor->seg_idx = ret_seg_idx;

#if EnStaging
                if (m_color == ref_color) {
                    pos_cursor->elem = nullptr;
                } else
#endif
                {
                    pos_cursor->elem = circular_array::Access(
                        &m_ca, ret_seg_idx, true, nullptr, nullptr);
                }
            } else {
                ret_seg_idx -= m_ca.elem_cnt;

                pos_cursor->n = r_n;
                pos_cursor->seg_idx = ret_seg_idx;
                pos_cursor->elem = circular_array::Access(
                    &r_ca, ret_seg_idx, true, nullptr, nullptr);
            }

            if (dst_cursor != nullptr) {
                dst_cursor->cntr = cntr;
                dst_cursor->idx = end_idx;
                dst_cursor->n = r_n;
                dst_cursor->seg_idx = r_ca.elem_cnt - old_r_size;
                dst_cursor->elem = circular_array::Access(
                    &r_ca, r_ca.elem_cnt - old_r_size, true, nullptr, nullptr);
            }

            break;
        }
        }

        return pos_cursor->elem;

    SPLIT: {
        /*

        the aim is to transform the insertion point to be between segments.

        case 0: push ml to l
        case 1: push mr to r
        case 2: insert a seg between l and m, push ml to new seg
        case 3: insert a seg between m and r, push mr to new seg

        */

        bool l_m_ok{ ml_elem_cnt <= l_elem_vac };
        bool m_r_ok{ mr_elem_cnt <= r_elem_vac };

        size_t new_l_elem_cnt{ ml_elem_cnt + l_ca.elem_cnt };
        size_t new_r_elem_cnt{ r_ca.elem_cnt + mr_elem_cnt };

        switch (
            l_m_ok || m_r_ok
                ? utils::Choose2(
                      l_m_ok && (!m_r_ok || new_l_elem_cnt <= new_r_elem_cnt),
                      m_r_ok && (!l_m_ok || new_r_elem_cnt <= new_l_elem_cnt),
                      &random_seed)
                : utils::Choose2(ml_elem_cnt <= mr_elem_cnt,
                                 mr_elem_cnt <= ml_elem_cnt, &random_seed) +
                      2) {
        case 0: {
            // push ml to l

#if EnStaging
            if (l_color == ref_color) {
                detail::TransferRefSegToDatSeg_(cntr, l_seg);
                l_color = dat_color;
                l_ca.data = l_seg->dat.data;
                l_ca.idx_offset = l_seg->dat.idx_offset;
            }
#endif

#if EnStaging
            if (m_color == ref_color) {
                detail::RefSegShoveR_(cntr, &l_ca, m_seg->ref.beg, ml_elem_cnt);

                m_seg->ref.beg += ml_elem_cnt;
                m_ca.elem_cnt -= ml_elem_cnt;
            } else
#endif
            {
                seg_utils::SegShoveL(&l_ca, &m_ca, ml_elem_cnt);
            }

            ml_elem_cnt = 0;
            l_elem_vac = seg_elem_capacity - l_ca.elem_cnt;

            r_n = m_n;
            r_seg = m_seg;
#if EnStaging
            r_color = m_color;
#endif
            r_ca.data = m_ca.data;
            r_ca.elem_cnt = m_ca.elem_cnt;
            r_ca.idx_offset = m_ca.idx_offset;
            r_elem_vac = seg_elem_capacity -
                         utils::Min(r_ca.elem_cnt, seg_elem_capacity);

            m_n = nullptr;
            m_seg = nullptr;

            break;
        }

        case 1: {
            // push mr to r

#if EnStaging
            if (r_color == ref_color) {
                detail::TransferRefSegToDatSeg_(cntr, r_seg);
                r_color = dat_color;
                r_ca.data = r_seg->dat.data;
                r_ca.idx_offset = r_seg->dat.idx_offset;
            }
#endif

#if EnStaging
            if (m_color == ref_color) {
                detail::RefSegShoveL_(cntr, &r_ca, m_seg->ref.beg + ml_elem_cnt,
                                      mr_elem_cnt);

                m_ca.elem_cnt -= mr_elem_cnt;
            } else
#endif
            {
                seg_utils::SegShoveR(&m_ca, &r_ca, mr_elem_cnt);
            }

            mr_elem_cnt = 0;
            r_elem_vac = seg_elem_capacity - r_ca.elem_cnt;

            l_n = m_n;
            l_seg = m_seg;
#if EnStaging
            l_color = m_color;
#endif
            l_ca.data = m_ca.data;
            l_ca.elem_cnt = m_ca.elem_cnt;
            l_ca.idx_offset = m_ca.idx_offset;
            l_elem_vac = seg_elem_capacity -
                         utils::Min(l_ca.elem_cnt, seg_elem_capacity);

            m_n = nullptr;
            m_seg = nullptr;

            break;
        }

        case 2: {
            // insert a seg between l and m, push ml to new seg

#if EnStaging
            if (m_color == ref_color) {
                Seg* new_l_seg{ detail::AllocateRefSeg_(seg_alctr) };

                cntr->root = rbtree::Insert(l_n, m_n, &new_l_seg->n);

                l_n = &new_l_seg->n;
                l_seg = new_l_seg;
                l_color = ref_color;
                l_seg->ref.beg = m_seg->ref.beg;
                l_ca.elem_cnt = ml_elem_cnt;
                l_elem_vac = seg_elem_capacity -
                             utils::Min(l_ca.elem_cnt, seg_elem_capacity);

                r_n = m_n;
                r_seg = m_seg;
                r_color = ref_color;
                r_seg->ref.beg += ml_elem_cnt;
                r_ca.elem_cnt = mr_elem_cnt;
                r_elem_vac = seg_elem_capacity -
                             utils::Min(r_ca.elem_cnt, seg_elem_capacity);
            } else
#endif
            {
                Seg* new_l_seg{ detail::AllocateDatSeg_(seg_alctr, data_alctr,
                                                        data_size) };

                cntr->root = rbtree::Insert(l_n, m_n, &new_l_seg->n);

                l_n = &new_l_seg->n;
                l_seg = new_l_seg;
#if EnStaging
                l_color = dat_color;
#endif
                l_ca.data = new_l_seg->dat.data;
                l_ca.elem_cnt = 0;
                l_ca.idx_offset = 0;

                r_n = m_n;
                r_seg = m_seg;
#if EnStaging
                r_color = dat_color;
#endif
                r_ca.data = m_ca.data;
                r_ca.elem_cnt = m_ca.elem_cnt;
                r_ca.idx_offset = m_ca.idx_offset;

                seg_utils::SegShoveL(&l_ca, &r_ca, ml_elem_cnt);

                l_elem_vac = seg_elem_capacity - l_ca.elem_cnt;
                r_elem_vac = seg_elem_capacity - r_ca.elem_cnt;
            }

            break;
        }

        case 3: {
            // insert a seg between m and r, push mr to new seg

#if EnStaging
            if (m_color == ref_color) {
                Seg* new_r_seg{ detail::AllocateRefSeg_(seg_alctr) };

                cntr->root = rbtree::Insert(m_n, r_n, &new_r_seg->n);

                l_n = m_n;
                l_seg = m_seg;
                l_color = m_color;
                l_ca.elem_cnt = ml_elem_cnt;
                l_elem_vac = seg_elem_capacity -
                             utils::Min(l_ca.elem_cnt, seg_elem_capacity);

                r_n = &new_r_seg->n;
                r_seg = new_r_seg;
                r_color = ref_color;
                r_seg->ref.beg = m_seg->ref.beg + ml_elem_cnt;
                r_ca.elem_cnt = mr_elem_cnt;
                r_elem_vac = seg_elem_capacity -
                             utils::Min(r_ca.elem_cnt, seg_elem_capacity);
            } else
#endif
            {
                Seg* new_r_seg{ detail::AllocateDatSeg_(seg_alctr, data_alctr,
                                                        data_size) };

                cntr->root = rbtree::Insert(m_n, r_n, &new_r_seg->n);

                l_n = m_n;
                l_seg = m_seg;
#if EnStaging
                l_color = m_color;
#endif
                l_ca.data = m_ca.data;
                l_ca.elem_cnt = m_ca.elem_cnt;
                l_ca.idx_offset = m_ca.idx_offset;

                r_n = &new_r_seg->n;
                r_seg = new_r_seg;
#if EnStaging
                r_color = dat_color;
#endif
                r_ca.data = new_r_seg->dat.data;
                r_ca.elem_cnt = 0;
                r_ca.idx_offset = 0;

                seg_utils::SegShoveR(&l_ca, &r_ca, mr_elem_cnt);

                l_elem_vac = seg_elem_capacity - l_ca.elem_cnt;
                r_elem_vac = seg_elem_capacity - r_ca.elem_cnt;
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

    size_t extra_seg_cnt_00{ integral_math::CeilDiv(cnt, seg_elem_capacity) };
    size_t extra_seg_cnt_01{ integral_math::CeilDiv(
        cnt - utils::Min(r_elem_vac, cnt), seg_elem_capacity) };
    size_t extra_seg_cnt_10{ integral_math::CeilDiv(
        cnt - utils::Min(l_elem_vac, cnt), seg_elem_capacity) };
    size_t extra_seg_cnt_11{ integral_math::CeilDiv(
        cnt - utils::Min(l_elem_vac + r_elem_vac, cnt), seg_elem_capacity) };

    size_t extra_seg_cnt{ utils::Min(extra_seg_cnt_00, extra_seg_cnt_01,
                                     extra_seg_cnt_10, extra_seg_cnt_11) };

    bool l_shove;
    bool r_shove;

    if (extra_seg_cnt == extra_seg_cnt_00) {
        l_shove = false;
        r_shove = false;
    } else if (extra_seg_cnt == extra_seg_cnt_01 &&
               extra_seg_cnt == extra_seg_cnt_10) {
        switch (utils::Choose2(l_elem_vac <= r_elem_vac,
                               r_elem_vac <= l_elem_vac, &random_seed)) {
        case 0:
            l_shove = false;
            r_shove = true;
            break;

        case 1:
            l_shove = true;
            r_shove = false;
            break;
        }
    } else if (extra_seg_cnt == extra_seg_cnt_01) {
        l_shove = false;
        r_shove = true;
    } else if (extra_seg_cnt == extra_seg_cnt_10) {
        l_shove = true;
        r_shove = false;
    } else {
        l_shove = true;
        r_shove = true;
    }

    size_t total_size{ (l_shove ? l_ca.elem_cnt : 0) + cnt +
                       (r_shove ? r_ca.elem_cnt : 0) };

    size_t total_seg_cnt{ integral_math::CeilDiv(total_size,
                                                 seg_elem_capacity) };

    size_t total_base_seg_elem_cnt{ total_size / total_seg_cnt };

    size_t total_extra_elem_cnt{ total_size / total_seg_cnt };

    bool ref_is_set{ false };

    if (l_shove) {
#if EnStaging
        if (l_color == ref_color) {
            detail::TransferRefSegToDatSeg_(cntr, l_seg);

            l_color = dat_color;
            l_ca.data = l_seg->dat.data;
            l_ca.idx_offset = 0;
        }
#endif

        size_t new_l_size{ ({
            --total_seg_cnt;
            bool has_extra_elem{ 0 < total_extra_elem_cnt };
            total_extra_elem_cnt -= b;
            total_base_seg_elem_cnt + b;
        }) };

        pos_cursor->n = l_n;
        pos_cursor->seg_idx = l_ca.elem_cnt;
        pos_cursor->elem = circular_array::PushR(
            &l_ca, new_l_size - l_ca.elem_cnt, writer, nullptr);

        ref_is_set = true;
    }

    if (l_seg != nullptr) {
#if EnStaging
        if (l_color == ref_color) {
            l_seg->ref.elem_cnt = l_ca.elem_cnt;
        } else
#endif
        {
            l_seg->dat.idx_offset =
                static_cast<unsigned short>(l_ca.idx_offset);
            l_seg->dat.elem_cnt = static_cast<unsigned short>(l_ca.elem_cnt);
        }

        bin_tree::SetSize(l_n, l_ca.elem_cnt);
    }

    for (size_t extra_seg_i{ 0 }; extra_seg_i < extra_seg_cnt; ++extra_seg_i) {
        size_t cur_size{ ({
            --total_seg_cnt;
            bool has_extra_elem{ 0 < total_extra_elem_cnt };
            total_extra_elem_cnt -= b;
            total_base_seg_elem_cnt + b;
        }) };

        Seg* new_l_seg{ detail::AllocateDatSeg_(seg_alctr, data_alctr,
                                                data_size) };

        cntr->root = rbtree::Insert(l_n, r_n, &new_l_seg->n);

        writer(new_l_seg->dat.data, elem_stride, cur_size);

        new_l_seg->dat.elem_cnt = static_cast<unsigned short>(cur_size);

        bin_tree::SetSize(&new_l_seg->n, cur_size);

        l_n = &new_l_seg->n;
        l_seg = new_l_seg;

        if (!ref_is_set) {
            pos_cursor->n = l_n;
            pos_cursor->seg_idx = 0;

            pos_cursor->elem = l_seg->dat.data;

            ref_is_set = true;
        }
    }

    size_t r_ins_cnt{ 0 };

    if (r_shove) {
#if EnStaging
        if (r_color == ref_color) {
            detail::TransferRefSegToDatSeg_(cntr, r_seg);

            r_color = dat_color;
            r_ca.data = r_seg->dat.data;
            r_ca.idx_offset = 0;
        }
#endif

        size_t new_r_size{ detail::GetAvgCnt_(total_size, seg_elem_capacity,
                                              random_seed) };

        total_size -= new_r_size;

        void* ref{ circular_array::PushL(
            &r_ca, r_ins_cnt = new_r_size - r_ca.elem_cnt, writer, nullptr) };

        if (!ref_is_set) {
            pos_cursor->n = r_n;
            pos_cursor->seg_idx = 0;
            pos_cursor->elem = ref;

            ref_is_set = true;
        }
    }

    if (r_seg != nullptr) {
#if EnStaging
        if (r_color == ref_color) {
            r_seg->ref.elem_cnt = r_ca.elem_cnt;
        } else
#endif
        {
            r_seg->dat.idx_offset =
                static_cast<unsigned short>(r_ca.idx_offset);
            r_seg->dat.elem_cnt = static_cast<unsigned short>(r_ca.elem_cnt);
        }

        bin_tree::SetSize(r_n, r_ca.elem_cnt);
    }

    if (dst_cursor == nullptr) { return pos_cursor->elem; }

    dst_cursor->cntr = cntr;
    dst_cursor->n = r_n;
    dst_cursor->idx = end_idx;

    if (r_shove) {
        dst_cursor->seg_idx = r_ins_cnt;
        dst_cursor->elem =
            circular_array::Access(&r_ca, r_ins_cnt, true, nullptr, nullptr);
    } else {
        dst_cursor->seg_idx = 0;

        dst_cursor->elem =
            r_seg == nullptr
#if EnStaging
                    || r_color == ref_color
#endif
                ? nullptr
                : circular_array::PeekL(&r_ca, true, nullptr, nullptr);
    }

    return pos_cursor->elem;
}

template <CntrTplParamList>
void NameSpace::PopL(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    Cursor pos_cursor;
    (PeekL)(cntr, true, &pos_cursor, nullptr);

    (Erase)(cntr, &pos_cursor, cnt);
}

template <CntrTplParamList>
void NameSpace::PopR(Cntr<CntrTplArgList>* cntr, size_t cnt) {
    size_t elem_cnt{ GetElemCnt(cntr) };

    ZETA_Core_DebugAssert(cnt <= elem_cnt);

    Cursor pos_cursor;
    (Access)(cntr, elem_cnt - cnt, true, &pos_cursor, nullptr);

    (Erase)(cntr, &pos_cursor, cnt);
}

template <CntrTplParamList>
void NameSpace::Erase(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor,
                      size_t cnt) {
    detail::CheckCursor_(cntr, pos_cursor);

    if (cnt == 0) { return; }

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t elem_cnt{ GetElemCnt(cntr) };

    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    auto* seg_alctr{ utils::GetInstPtr(cntr->seg_alctr) };
    auto* data_alctr{ utils::GetInstPtr(cntr->data_alctr) };

    ZETA_Core_DebugAssert(seq_cntr::IsErasable(pos_cursor->idx, cnt, elem_cnt));

    TreeNode* m_n{ pos_cursor->n };
    size_t seg_idx{ pos_cursor->seg_idx };

    Seg* m_seg{ detail::NToSeg_(m_n) };

    if (seg_idx + cnt < m_ca.elem_cnt) {
        TreeNode* l_n{ bin_tree::StepL(m_n) };
        TreeNode* r_n{ bin_tree::StepR(m_n) };

        Seg* l_seg;
        Seg* r_seg;

#if EnStaging
        unsigned l_color;
        unsigned m_color{ detail::GetNColor_(m_n) };
        unsigned r_color;
#endif

        CircularArray l_ca{
            .data = {},
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = {},
            .elem_capacity = seg_elem_capacity,
            .idx_offset = {},
        };

        CircularArray m_ca{
            .data = {},
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = EnStagingTernary(
                m_color == ref_color, m_seg->ref.elem_cnt, m_seg->dat.elem_cnt),
            .elem_capacity = seg_elem_capacity,
            .idx_offset = {},
        };

        CircularArray r_ca{
            .data = {},
            .elem_size = elem_size,
            .elem_stride = elem_stride,
            .elem_cnt = {},
            .elem_capacity = seg_elem_capacity,
            .idx_offset = {},
        };

        size_t ml_elem_cnt{ seg_idx };
        size_t mr_elem_cnt{ m_ca.elem_cnt - seg_idx - cnt };

        size_t l_vac;
        size_t r_vac;

        if (lb == l_n) {
            l_seg = nullptr;
            l_ca.elem_size = 0;
            l_vac = 0;
        } else {
            l_seg = detail::NToSeg_(l_n);

            detail::GetSegState_(seg_elem_capacity, l_seg,
#if EnStaging
                                 &l_color,
#endif
                                 &l_ca, &l_vac);
        }

        if (rb == r_n) {
            r_seg = nullptr;
            r_ca.elem_size = 0;
            r_vac = 0;
        } else {
            r_seg = detail::NToSeg_(r_n);

            detail::GetSegState_(seg_elem_capacity, r_seg,
#if EnStaging
                                 &r_color,
#endif
                                 &r_vac);
        }

        size_t total_elem_cnt{ l_ca.elem_cnt + m_ca.elem_cnt - cnt +
                               r_ca.elem_cnt };

        bool l_m_is_ok{ l_seg != nullptr &&
                        l_ca.elem_cnt + ml_elem_cnt + mr_elem_cnt <=
                            seg_elem_capacity };

        bool m_r_is_ok{ r_seg != nullptr &&
                        ml_elem_cnt + mr_elem_cnt + r_ca.elem_cnt <=
                            seg_elem_capacity };

        if (l_m_is_ok && !m_r_is_ok) {
            goto DO_CASE_L_M;
        } else if (!l_m_is_ok && m_r_is_ok) {
            goto DO_CASE_M_R;
        } else if (l_m_is_ok && m_r_is_ok) {
            // compare cost to choose
        } else {
            goto DO_CASE_M;
        }

    DO_CASE_L_M: {
        size_t use_l_cost{ EnStagingTernary(l_color == ref_color, l_elem_cnt,
                                            0) +
                           m_ca.elem_cnt - cnt };

        size_t use_m_cost{ l_elem_cnt +
                           EnStagingTernary(
                               m_color == ref_color, ml_elem_cnt + mr_elem_cnt,
                               utils::Min(ml_elem_cnt, mr_elem_cnt)) };

        if (utils::Choose2(use_l_cost <= use_m_cost, use_m_cost <= use_l_cost,
                           &utils::GetRandom()) == 0) {
            size_t old_l_elem_cnt{ l_ca.elem_cnt };

#if EnStaging
            if (l_color == ref_color) {
                size_t l_ref_beg{ l_seg->ref.beg };

                SetNColor_(l_n, dat_color);

                l_seg->dat.data = l_ca.data =
                    AllocateData_(data_alctr, data_size);

                l_ca.elem_cnt += ml_elem_cnt + mr_elem_cnt;

                l_ca.idx_offset = 0;

                seq_cntr::Access(origin, l_ref_beg, true, origin_cursor,
                                 nullptr);

                seq_cntr::Read(origin, origin_cursor, l_ca.elem_cnt,
                               seq_cntr::MemReader{
                                   .data = l_ca.data,
                                   .elem_size = elem_size,
                                   .elem_stride = elem_stride,
                               },
                               nullptr);
            } else
#endif
            {
                circular_array::PushR(&l_ca, ml_elem_cnt + mr_elem_cnt,
                                      seq_cntr::EmptyWriter{}, nullptr);
            }

            cntr->root = rbtree::Extract(m_n);

#if EnStaging
            if (m_color == ref_color) {
                seq_cntr::Access(origin, m_seg->ref.beg, true, origin_cursor,
                                 nullptr);

                circular_array::AssignFromSeqCntr(&l_ca, old_l_elem_cnt,
                                                  ml_elem_cnt, origin,
                                                  origin_cursor, ml_elem_cnt);

                seq_cntr::Access(origin, m_seg->ref.beg + ml_elem_cnt + cnt,
                                 true, origin_cursor, nullptr);

                circular_array::AssignFromSeqCntr(
                    &l_ca, old_l_elem_cnt + ml_elem_cnt, mr_elem_cnt, origin,
                    origin_cursor, ml_elem_cnt);

                detail::DeallocateRefSeg_(seg_alctr, m_seg);
            } else
#endif
            {
                circular_array::AssignFromCircularArray(&l_ca, old_l_elem_cnt,
                                                        &m_ca, 0, ml_elem_cnt);

                circular_array::AssignFromCircularArray(
                    &l_ca, old_l_elem_cnt + ml_elem_cnt, &m_ca,
                    ml_elem_cnt + cnt, mr_elem_cnt);

                detail::DeallocateDatSeg_(seg_alctr, m_seg);
            }

            l_seg->dat.elem_cnt = static_cast<unsigned short>(l_ca.elem_cnt);

            l_seg->dat.idx_offset =
                static_cast<unsigned short>(l_ca.idx_offset);

            bin_tree::SetSize(l_n, l_ca.elem_cnt);

            pos_cursor->n = l_n;
            pos_cursor->seg_idx = l_ca.elem_cnt - mr_elem_cnt;

            pos_cursor->elem = circular_array::Access(
                &l_ca, pos_cursor->seg_idx, true, nullptr, nullptr);

            return;
        } else {
#if EnStaging
            if (m_color == ref_color) {
                size_t m_ref_beg{ m_seg->ref.beg };

                SetNColor_(m_n, dat_color);

                m_seg->dat.data = m_ca.data =
                    AllocateData_(data_alctr, data_size);

                m_ca.elem_cnt = ml_elem_cnt + mr_elem_cnt;

                m_ca.idx_offset = 0;

                if (0 < ml_elem_cnt) {
                    seq_cntr::Access(origin, m_ref_beg, true, origin_cursor,
                                     nullptr);

                    seq_cntr::Read(origin, origin_cursor, ml_elem_cnt,
                                   seq_cntr::MemReader{
                                       .data = m_seg->dat.data,
                                       .elem_size = elem_size,
                                       .elem_stride = elem_stride,
                                   },
                                   nullptr);
                }

                if (0 < mr_elem_cnt) {
                    seq_cntr::Access(origin, m_ref_beg + ml_elem_cnt + cnt,
                                     true, origin_cursor, nullptr);

                    seq_cntr::Read(
                        origin, origin_cursor, mr_elem_cnt,
                        seq_cntr::MemReader{
                            .data = static_cast<char*>(m_seg->dat.data) +
                                    elem_stride * ml_elem_cnt,
                            .elem_size = elem_size,
                            .elem_stride = elem_stride,
                        },
                        nullptr);
                }
            } else
#endif
            {
                circular_array::Erase(&m_ca, seg_idx, cnt);
            }

            cntr->root = rbtree::Extract(l_n);

#if EnStaging
            if (l_color == ref_color) {
                detail::RefShoveR_(&m_ca, l_seg, l_ca.elem_cnt);

                detail::DeallocateRefSeg_(seg_alctr, l_seg);
            } else
#endif
            {
                seg_utils::SegShoveR(&l_ca, &m_ca, l_ca.elem_cnt);

                detail::DeallocateDatSeg_(seg_alctr, data_alctr, l_seg);
            }

            m_seg->dat.elem_cnt = static_cast<unsigned short>(m_ca.elem_cnt);

            m_seg->dat.idx_offset =
                static_cast<unsigned short>(m_ca.idx_offset);

            bin_tree::SetSize(m_n, m_ca.elem_cnt);

            pos_cursor->n = m_n;

            pos_cursor->seg_idx = m_ca.elem_cnt - mr_elem_cnt;

            pos_cursor->elem = circular_array::Access(
                &m_ca, pos_cursor->seg_idx, true, nullptr, nullptr);

            return;
        }
    }

    DO_CASE_M_R: {
        size_t use_m_cost{ EnStagingTernary(
                               m_color == ref_color, ml_elem_cnt + mr_elem_cnt,
                               utils::Min(ml_elem_cnt, mr_elem_cnt)) +
                           r_elem_cnt };

        size_t use_r_cost{ EnStagingTernary(r_color == ref_color, r_elem_cnt,
                                            0) +
                           m_elem_cnt - cnt };

        if (utils::Choose2(use_m_cost <= use_r_cost, use_r_cost <= use_m_cost,
                           &utils::GetRandom()) == 0) {
#if EnStaging
            if (m_color == ref_color) {
                size_t m_ref_beg{ m_seg->ref.beg };

                SetNColor_(m_n, dat_color);

                m_seg->dat.data = m_ca.data =
                    detail::AllocateData_(data_alctr, data_size);

                m_ca.elem_cnt = ml_elem_cnt + mr_elem_cnt;

                m_ca.idx_offset = 0;

                if (0 < ml_elem_cnt) {
                    seq_cntr::Access(origin, m_ref_beg, true, origin_cursor,
                                     nullptr);

                    seq_cntr::Read(origin, origin_cursor, ml_elem_cnt,
                                   seq_cntr::MemReader{
                                       .data = m_seg->dat.data,
                                       .elem_size = elem_size,
                                       .elem_stride = elem_stride,
                                   },
                                   nullptr);
                }

                if (0 < mr_elem_cnt) {
                    seq_cntr::Access(origin, m_ref_beg + ml_elem_cnt + cnt,
                                     true, origin_cursor, nullptr);

                    seq_cntr::Read(
                        origin, origin_cursor, mr_elem_cnt,
                        seq_cntr::MemReader{
                            .data = static_cast<char*>(m_seg->dat.data) +
                                    elem_stride * ml_elem_cnt,
                            .elem_size = elem_size,
                            .elem_stride = elem_stride,
                        },
                        nullptr);
                }
            } else
#endif
            {
                circular_array::Erase(&m_ca, seg_idx, cnt);
            }

            cntr->root = rbtree::Extract(r_n);

#if EnStaging
            if (r_color == ref_color) {
                detail::RefShoveL_(&m_ca, r_seg, r_ca.elem_cnt);

                detail::DeallocateRefSeg_(seg_alctr, r_seg);
            } else
#endif
            {
                seg_utils::SegShoveL(&r_ca, &m_ca, r_ca.elem_cnt);

                detail::DeallocateDatSeg_(seg_alctr, data_alctr, r_seg);
            }

            m_seg->dat.elem_cnt = static_cast<unsigned short>(m_ca.elem_cnt);
            m_seg->dat.idx_offset =
                static_cast<unsigned short>(m_ca.idx_offset);

            bin_tree::SetSize(m_n, m_ca.elem_cnt);

            pos_cursor->n = m_n;

            pos_cursor->seg_idx = ml_elem_cnt;

            pos_cursor->elem = circular_array::Access(
                &m_ca, pos_cursor->seg_idx, true, nullptr, nullptr);

            return;
        } else {
#if EnStaging
            if (r_color == ref_color) {
                size_t r_ref_beg{ r_seg->ref.beg };

                SetNColor_(r_n, dat_color);

                r_seg->dat.data = r_ca.data =
                    AllocateData_(data_alctr, data_size);

                r_ca.elem_cnt += mr_elem_cnt + ml_elem_cnt;

                r_ca.idx_offset = 0;

                seq_cntr::Access(origin, r_ref_beg, true, origin_cursor,
                                 nullptr);

                seq_cntr::Read(
                    origin, origin_cursor, l_ca.elem_cnt,
                    seq_cntr::MemReader{
                        .data = static_cast<char*>(r_ca.data) +
                                elem_stride * (ml_elem_cnt + mr_elem_cnt),
                        .elem_size = elem_size,
                        .elem_stride = elem_stride,
                    },
                    nullptr);
            } else
#endif
            {
                circular_array::PushL(&r_ca, ml_elem_cnt + mr_elem_cnt,
                                      seq_cntr::EmptyWriter{}, nullptr);
            }

            cntr->root = rbtree::Extract(m_n);

#if EnStaging
            if (m_color == ref_color) {
                seq_cntr::Access(origin, m_seg->ref.beg, true, origin_cursor,
                                 nullptr);

                circular_array::AssignFromSeqCntr(&r_ca, 0, ml_elem_cnt, origin,
                                                  origin_cursor, ml_elem_cnt);

                seq_cntr::Access(origin, m_seg->ref.beg + ml_elem_cnt + cnt,
                                 true, origin_cursor, nullptr);

                circular_array::AssignFromSeqCntr(&r_ca, ml_elem_cnt,
                                                  mr_elem_cnt, origin,
                                                  origin_cursor, ml_elem_cnt);

                detail::DeallocateRefSeg_(seg_alctr, m_seg);
            } else
#endif
            {
                circular_array::AssignFromCircularArray(&r_ca, 0, &m_ca, 0,
                                                        ml_elem_cnt);

                circular_array::AssignFromCircularArray(
                    &r_ca, ml_elem_cnt, &m_ca, ml_elem_cnt + cnt, mr_elem_cnt);

                detail::DeallocateDatSeg_(seg_alctr, data_alctr, m_seg);
            }

            r_seg->dat.elem_cnt = static_cast<unsigned short>(r_ca.elem_cnt);

            r_seg->dat.idx_offset =
                static_cast<unsigned short>(r_ca.idx_offset);

            bin_tree::SetSize(r_n, r_ca.elem_cnt);

            pos_cursor->n = r_n;

            pos_cursor->seg_idx = ml_elem_cnt;

            pos_cursor->elem = circular_array::Access(
                &r_ca, pos_cursor->seg_idx, true, nullptr, nullptr);

            return;
        }
    }

    DO_CASE_M: {
#if EnStaging
        if (m_color == ref_color) {
            Seg* new_seg{ detail::AllocateRefSeg_(seg_alctr) };

            if (utils::SimpleRandomRotate(&random_seed) % 2 == 0) {
                new_seg->ref.beg = m_seg->ref.beg;
                new_seg->ref.elem_cnt = ml_elem_cnt;

                m_seg->ref.beg += ml_elem_cnt + cnt;
                m_seg->ref.elem_cnt = mr_elem_cnt;

                cntr->root = rbtree::Insert(l_n, m_n, &new_seg->n);

                pos_cursor->seg_idx = 0;
            } else {
                new_seg->ref.beg = m_seg->ref.beg + ml_elem_cnt + cnt;
                new_seg->ref.elem_cnt = mr_elem_cnt;

                m_seg->ref.elem_cnt = ml_elem_cnt;

                cntr->root = rbtree::Insert(m_n, r_n, &new_seg->n);

                pos_cursor->n = &new_seg->n;
                pos_cursor->seg_idx = 0;
            }

            bin_tree::SetSize(&new_seg->n, new_seg->ref.elem_cnt);
            bin_tree::SetSize(m_n, m_seg->ref.elem_cnt);
        } else
#endif
        {
            circular_array::Erase(&m_ca, ml_elem_cnt, cnt);

            m_seg->dat.elem_cnt = static_cast<unsigned short>(m_ca.elem_cnt);
            m_seg->dat.idx_offset =
                static_cast<unsigned short>(m_ca.idx_offset);

            bin_tree::SetSize(m_n, m_ca.elem_cnt);

            pos_cursor->elem = circular_array::Access(&m_ca, ml_elem_cnt, true,
                                                      nullptr, nullptr);
        }

        return;
    }
    }

    Seg* first_partial_erased_seg{ nullptr };
    Seg* last_partial_erased_seg{ nullptr };

    if (0 < seg_idx) {
        m_seg = detail::NToSeg_(m_n);

        first_partial_erased_seg = m_seg;
        last_partial_erased_seg = m_seg;

#if EnStaging
        if (detail::GetNColor_(m_n) == ref_color) {
            cnt -= m_seg->ref.elem_cnt - seg_idx;

            m_seg->ref.elem_cnt = static_cast<unsigned short>(seg_idx);
        } else
#endif
        {
            cnt -= m_seg->dat.elem_cnt - seg_idx;

            m_seg->dat.elem_cnt = static_cast<unsigned short>(seg_idx);
        }

        m_n = bin_tree::StepR(m_n);
        seg_idx = 0;
    }

    while (0 < cnt) {
        m_seg = detail::NToSeg_(m_n);

#if EnStaging
        if (detail::GetNColor_(m_n) == ref_color) {
            if (m_seg->ref.elem_cnt <= cnt) {
                cnt -= m_seg->ref.elem_cnt;
                TreeNode* nxt_m_n{ bin_tree::StepR(m_n) };

                cntr->root = rbtree::Extract(m_n);

                detail::DeallocateRefSeg_(seg_alctr, m_seg);

                m_n = nxt_m_n;

                continue;
            }

            m_seg->ref.beg += cnt;
            m_seg->ref.elem_cnt -= cnt;

            last_partial_erased_seg = m_seg;

            break;
        }
#endif

        if (m_seg->dat.elem_cnt <= cnt) {
            cnt -= m_seg->dat.elem_cnt;

            TreeNode* nxt_m_n{ bin_tree::StepR(m_n) };

            cntr->root = rbtree::Extract(m_n);

            detail::DeallocateDatSeg_(seg_alctr, data_alctr, m_seg);

            m_n = nxt_m_n;

            continue;
        }

        m_ca.data = m_seg->dat.data;
        m_ca.elem_cnt = m_seg->dat.elem_cnt;
        m_ca.idx_offset = m_seg->dat.idx_offset;

        circular_array::IdxErase(&m_ca, 0, cnt);

        m_seg->dat.elem_cnt = static_cast<unsigned short>(m_ca.elem_cnt);
        m_seg->dat.idx_offset = static_cast<unsigned short>(m_ca.idx_offset);

        last_partial_erased_seg = m_seg;

        break;
    }

    TreeNode* ret_n{ m_n };
    size_t ret_seg_idx{ 0 };

    unsigned long long random_seed{ utils::GetRandom() };

    bool last_exist{ &last_seg->n == m_n };

    Seg* a_seg;
    Seg* b_seg;
    Seg* c_seg;
    Seg* d_seg;

    size_t a_size;
    size_t b_size;
    size_t c_size;
    size_t d_size;

    size_t a_vac;
    size_t b_vac;
    size_t c_vac;
    size_t d_vac;

    {
        /*

        first_partial_erased_seg last_partial_erased_seg
        O                        O (m_n)
        O                        X                        m_n
        X                        O (m_n)
        X                        X                        m_n

        */

        Seg* segs_buffer[4]{ nullptr, nullptr, nullptr, nullptr };
        size_t segs_buffer_i{ 0 };

        {
            TreeNode* l_n{
                bin_tree::StepL(first_partial_erased_seg == nullptr
                                    ? m_n
                                    : first_partial_erased_seg) :
            };

            if (lb != l_n) {
                segs_buffer[segs_buffer_i++] = detail::NToSeg_(l_n);
            }
        }

        if (first_partial_erased_seg != nullptr) {
            segs_buffer[segs_buffer_i++] = first_partial_erased_seg;
        }

        if (last_partial_erased_seg == nullptr) {
            segs_buffer[segs_buffer_i++] = detail::NToSeg_(m_n);
        } else {
            segs_buffer[segs_buffer_i++] = last_partial_erased_seg;

            TreeNode* r_n{ bin_tree::StepR(m_n) };

            if (rb != r_n) {
                segs_buffer[segs_buffer_i++] = detail::NToSeg_(r_n);
            }
        }

        a_seg = segs_buffer[0];
        b_seg = segs_buffer[1];
        c_seg = segs_buffer[2];
        d_seg = segs_buffer[3];
    }

    if (a_seg == nullptr) {
    }
#if EnStaging
    else if (detail::GetNColor_(&a_seg->n) == ref_color) {
        a_size = a_seg->ref.elem_cnt;
        a_vac = seg_elem_capacity - utils::Min(a_size, seg_elem_capacity);
    }
#endif
    else {
        a_size = a_seg->dat.elem_cnt;
        a_vac = seg_elem_capacity - a_size;
    }

    if (b_seg == nullptr) {
    }
#if EnStaging
    else if (detail::GetNColor_(&b_seg->n) == ref_color) {
        b_size = b_seg->ref.elem_cnt;
        b_vac = seg_elem_capacity - utils::Min(b_size, seg_elem_capacity);
    }
#endif
    else {
        b_size = b_seg->dat.elem_cnt;
        b_vac = seg_elem_capacity - b_size;
    }

    if (c_seg == nullptr) {
    }
#if EnStaging
    else if (detail::GetNColor_(&c_seg->n) == ref_color) {

        c_size = c_seg->ref.elem_cnt;
        c_vac = seg_elem_capacity - utils::Min(c_size, seg_elem_capacity);
    }
#endif
    else {
        c_size = c_seg->dat.elem_cnt;
        c_vac = seg_elem_capacity - c_size;
    }

    if (d_seg == nullptr) {
    }
#if EnStaging
    else if (detail::GetNColor_(&d_seg->n) == ref_color) {
        d_size = d_seg->ref.elem_cnt;
        d_vac = seg_elem_capacity - utils::Min(d_size, seg_elem_capacity);
    }
#endif
    else {
        d_size = d_seg->dat.elem_cnt;
        d_vac = seg_elem_capacity - d_size;
    }

    if (a_seg == nullptr) { goto UPDATE; }

    if (ret_n == &a_seg->n) { ret_n = nullptr; }

    if (b_seg == nullptr) { goto UPDATE; }

    if (ret_n == &b_seg->n) {
        ret_n = nullptr;
        ret_seg_idx += a_size;
    }

#if EnStaging
    if (detail::GetNColor_(&a_seg->n) == ref_color &&
        detail::GetNColor_(&b_seg->n) == ref_color &&
        a_seg->ref.beg + a_seg->ref.elem_cnt == b_seg->ref.beg) {
        a_seg->ref.elem_cnt += b_seg->ref.elem_cnt;

        cntr->root = rbtree::Extract(&b_seg->n);

        detail::DeallocateRefSeg_(seg_alctr, b_seg);

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
    if (detail::GetNColor_(&b_seg->n) == ref_color &&
        detail::GetNColor_(&c_seg->n) == ref_color &&
        b_seg->ref.beg + b_seg->ref.elem_cnt == c_seg->ref.beg) {
        b_seg->ref.elem_cnt += c_seg->ref.elem_cnt;

        cntr->root = rbtree::Extract(&c_seg->n);

        detail::DeallocateRefSeg_(seg_alctr, c_seg);

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
    if (detail::GetNColor_(&c_seg->n) == ref_color &&
        detail::GetNColor_(&d_seg->n) == ref_color &&
        c_seg->ref.beg + c_seg->ref.elem_cnt == d_seg->ref.beg) {
        c_seg->ref.elem_cnt += d_seg->ref.elem_cnt;

        cntr->root = rbtree::Extract(&d_seg->n);

        detail::DeallocateRefSeg_(seg_alctr, d_seg);

        d_seg = nullptr;

        goto MERGE_3;
    }
#endif

    goto MERGE_4;

MERGE_2: {
    size_t ab_vac{ a_vac + b_vac };

    if (seg_elem_capacity <= ab_vac) {
        detail::Merge2_(cntr, a_seg, b_seg, true, true);
    }

    goto UPDATE;
}

MERGE_3: {
    size_t ab_vac{ a_vac + b_vac };
    size_t bc_vac{ b_vac + c_vac };

    size_t max_vac = utils::Max(ab_vac, bc_vac);

    if (seg_elem_capacity < max_vac) {
        switch (
            utils::Choose2(bc_vac <= ab_vac, ab_vac <= bc_vac, &random_seed)) {
        case 0: detail::Merge2_(cntr, a_seg, b_seg, true, true); break;
        case 1: detail::Merge2_(cntr, b_seg, c_seg, true, true); break;
        }
    }

    goto UPDATE;
}

MERGE_4: {
    size_t ab_vac{ a_vac + b_vac };
    size_t bc_vac{ b_vac + c_vac };
    size_t cd_vac{ c_vac + d_vac };

    if (seg_elem_capacity <= ab_vac && seg_elem_capacity <= cd_vac) {
        detail::Merge2_(cntr, a_seg, b_seg, true, true);
        detail::Merge2_(cntr, c_seg, d_seg, true, true);
    } else {
        size_t max_vac{ utils::Max(ab_vac, bc_vac, cd_vac) };

        if (seg_elem_capacity <= max_vac) {
            switch (utils::Choose3(ab_vac == max_vac, bc_vac == max_vac,
                                   cd_vac == max_vac, &random_seed)) {
            case 0: detail::Merge2_(cntr, a_seg, b_seg, true, true); break;
            case 1: detail::Merge2_(cntr, b_seg, c_seg, true, true); break;
            case 2: detail::Merge2_(cntr, c_seg, d_seg, true, true); break;
            }
        }
    }

    goto UPDATE;
}

UPDATE: {
    if (a_seg == nullptr) { goto UPDATE_END; }

    a_size = EnStagingTernary(detail::GetNColor_(&a_seg->n) == ref_color,
                              a_seg->ref.elem_cnt, a_seg->dat.elem_cnt);

    if (a_size == 0) {
        cntr->root = rbtree::Extract(&a_seg->n);

        detail::DeallocateSeg_(seg_alctr, data_alctr, a_seg);
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

    b_size = EnStagingTernary(detail::GetNColor_(&b_seg->n) == ref_color,
                              b_seg->ref.elem_cnt, b_seg->dat.elem_cnt);

    if (b_size == 0) {
        cntr->root = rbtree::Extract(&b_seg->n);

        detail::DeallocateSeg_(seg_alctr, data_alctr, b_seg);
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

    c_size = EnStagingTernary(detail::GetNColor_(&c_seg->n) == ref_color,
                              c_seg->ref.elem_cnt, c_seg->dat.elem_cnt);

    if (c_size == 0) {
        cntr->root = rbtree::Extract(&c_seg->n);

        detail::DeallocateSeg_(seg_alctr, data_alctr, c_seg);
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

    d_size = EnStagingTernary(detail::GetNColor_(&d_seg->n) == ref_color,
                              d_seg->ref.elem_cnt, d_seg->dat.elem_cnt);

    if (d_size == 0) {
        cntr->root = rbtree::Extract(&d_seg->n);

        detail::DeallocateSeg_(seg_alctr, data_alctr, d_seg);
    } else {
        bin_tree::SetSize(&d_seg->n, d_size);
    }

    if (ret_n == nullptr) { ret_n = &d_seg->n; }
}

UPDATE_END:

    pos_cursor->n = ret_n;
    pos_cursor->seg_idx = ret_seg_idx;

    if (rb == pos_cursor->n
#if EnStaging
        || detail::GetNColor_(pos_cursor->n) == ref_color
#endif
    ) {
        pos_cursor->elem = nullptr;
        return;
    }

    Seg* seg{ detail::NToSeg_(pos_cursor->n) };

    m_ca.data = seg->dat.data;
    m_ca.elem_cnt = seg->dat.elem_cnt;
    m_ca.idx_offset = seg->dat.idx_offset;

    pos_cursor->elem = circular_array::Access(&m_ca, pos_cursor->seg_idx, true,
                                              nullptr, nullptr);
}

template <CntrTplParamList>
void NameSpace::EraseAll(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr_(cntr);

    EraseTree(cntr->root, cntr->seg_alctr, cntr->data_alctr);

    detail::InitTree_(cntr);
}

template <CntrTplParamList>
void NameSpace::Reset(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr_(cntr);

    EraseTree(cntr->root, cntr->seg_alctr, cntr->data_alctr);

    detail::InitTree_(cntr);

#if EnStaging
    detail::RefOrigin_(cntr);
#endif
}

#if EnStaging

/*

namespace detail {

template <typename OriginLike>
constexpr bool IsStagingSegVectorPtr_F_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const volatile*) {
    return true;
}

constexpr bool IsStagingSegVectorPtr_F_  // NOLINT(misc-use-internal-linkage)
    (void*) {
    return false;
}

template <typename T>
constexpr bool IsStagingSegVectorPtr_{ []() {
    ZETA_Core_StaticAssert(IsPointer<T>);
    return IsStagingSegVectorPtr_F_(static_cast<T>(nullptr));
} };

template <typename T>
struct GetOriginLikeCore_;

template <CntrTplParamList>
struct GetOriginLikeCore_<Cntr<CntrTplArgList>> {
    using type = OriginLike;
};

template <typename T>
using GetOriginLike_ = GetOriginLikeCore_<T>::type;

}  // namespace detail

template <CntrTplParamList>
void Collapse(Cntr<CntrTplArgList>* cntr) {
    detail::CheckCntr_(cntr);

    auto* origin{ utils::GetInstPtr(cntr->origin) };
    using Origin = RemovePointer<decltype(origin)>;

    constexpr bool origin_like_is_pointer{ IsPointer<Origin> };

    constexpr bool origin_like_is_ref{ IsAnyOf<OriginLike, seq_cntr::Ref> };

    constexpr bool origin_like_is_ref_ptr{
        origin_like_is_pointer &&
        IsAnyOf<RemoveCVRef<RemovePointer<OriginLike>>, seq_cntr::Ref*>
    };

    constexpr bool origin_like_is_cntr_ptr{
        detail::IsStagingSegVectorPtr_<OriginLike>()
    };

    if constexpr (origin_like_is_ref || origin_like_is_ref_ptr) {
        // TODO
        // Check ref's vtable if sv's vtable
    } else if constexpr (origin_like_is_cntr_ptr) {
    } else {
        ZETA_Core_StaticAssert(false);
    }

    cntr->origin = origin_cntr->origin;

    size_t stride{ cntr->elem_stride };

    TreeNode* n{ bin_tree::StepR(reinterpret_cast<TreeNodeView*>(cntr->lb))
};

    Cursor origin_cursor;
    Access(origin_cntr, 0, &origin_cursor, nullptr);

    CircularArray origin_ca{
        .elem_size = cntr->elem_size,
        .elem_stride = stride,
        .elem_capacity = origin_cntr->seg_elem_capacity,
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
                                    ? origin_seg->ref.elem_cnt
                                    : origin_seg->dat.elem_cnt };

        size_t origin_res_size{ origin_seg_size - origin_seg_idx };

        if (origin_seg_color == ref_color) {
            if (seg->ref.elem_cnt <= origin_res_size) {
                seg->ref.beg = origin_seg->ref.beg + origin_seg_idx;
                n = bin_tree::StepR(n);
                continue;
            }

            Seg* new_ref_seg{ AllocateRefSeg_(cntr) };

            new_ref_seg->ref.beg = origin_seg->ref.beg + origin_seg_idx;
            new_ref_seg->ref.elem_cnt = origin_res_size;

            bin_tree::SetSize(reinterpret_cast<TreeNodeView*>(&new_ref_seg->n),
new_ref_seg->ref.elem_cnt);

            seg->ref.beg += origin_res_size;
            seg->ref.elem_cnt -= origin_res_size;

            bin_tree::SetSize(reinterpret_cast<TreeNodeView*>(seg),
seg->ref.elem_cnt);

            cntr->root = rbtree::InsertL(n, &new_ref_seg->n);

            continue;
        }

        size_t cur_cnt{ detail::GetAvgCnt_(Min(seg->ref.elem_cnt,
origin_res_size))
};

        if (cur_cnt == seg->ref.elem_cnt) {
            SetNColor_(n, dat_color);
            seg->dat.data = AllocateData_(data_alctr, data_size);
            seg->dat.size = cur_cnt;
            seg->dat.idx_offset = 0;
        } else {
            Seg* new_dat_seg{ AllocateDatSeg_(cntr) };

            new_dat_seg->dat.size = cur_cnt;

            bin_tree::SetSize(reinterpret_cast<TreeNodeView*>(&new_dat_seg->n),
cur_cnt);

            seg->ref.beg += origin_res_size;
            seg->ref.elem_cnt -= origin_res_size;

            bin_tree::SetSize(n,
seg->ref.elem_cnt);

            cntr->root = rbtree::InsertL(n, &new_dat_seg->n);

            n = &new_dat_seg->n;
            seg = new_dat_seg;
        }

        origin_ca.data = origin_seg->dat.data;
        origin_ca.size = origin_seg_size;
        origin_ca.idx_offset = origin_seg->dat.idx_offset;

        circular_array::IdxRead(&origin_ca, origin_seg_idx, cur_cnt,
                                     seg->dat.data, stride, nullptr);

        n = bin_tree::StepR(n);
    }
}


namespace detail {

struct WBSeg_ {
    size_t beg;
    size_t size;

    size_t dst_idx;
    size_t acc_ref;

    void* data;
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
    (Cntr<CntrTplArgList>* cntr, TreeNode* n, size_t dst_idx,
     OffsetCntGenericHashTable_* ght) {
    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    while (n != nullptr) {
        TreeNode* nl{ bin_tree::GetL(n) };
        TreeNode* nr{ bin_tree::GetR(n) };

        if (nl != nullptr) { dst_idx = RecordOffset_(cntr, nl, dst_idx, ght); }

        if (lb == n || rb == n) {
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
utils::Pair<size_t, size_t> ToWBSeg_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>* cntr, TreeNode* n, WBSeg_* dst) {
    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    Pair<size_t, size_t> ret{ 0, 0 };

    while (n != nullptr) {
        TreeNode* nl{ bin_tree::GetL(n) };
        TreeNode* nr{ bin_tree::GetR(n) };

        auto l_ret{ ToWBSeg_(cntr, nl, dst) };

        dst += l_ret.first + l_ret.second;

        ret.first += l_ret.first;
        ret.second += l_ret.second;

        if (lb == n || rb == n) {
            n = nr;
            continue;
        }

        Seg* seg{ NToSeg_(n) };

        if (GetNColor_(n) == ref_color) {
            ++ret.first;

            dst->beg = seg->ref.beg;
            dst->size = seg->ref.elem_cnt;
        } else {
            ++ret.second;

            dst->beg = ZETA_Core_size_max;
            dst->size = seg->dat.size;
            dst->data = seg->dat.data;
            dst->offset = seg->dat.idx_offset;
        }

        allocator::Deallocate(cntr->seg_alctr, seg);

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
    (Cntr<CntrTplArgList>* cntr, CircularArray* ca, WBSeg_* wb_segs,
     size_t wb_seg_cnt, size_t dst_offset, size_t ref_offset) {
    auto* origin{ utils::GetInstPtr(cntr->origin) };
    DataAllocatorLike* data_alctr{ utils::GetInstPtr(cntr->data_alctr) };

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
                origin, &ca, dst_offset + wb_segs->dst_idx, 0, wb_segs->size);

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
    (Cntr<CntrTplArgList>* cntr, int write_back_strategy,
     unsigned long long cost_coeff_read, unsigned long long cost_coeff_write,
     unsigned long long cost_coeff_insert,
     unsigned long long cost_coeff_erase) {
    auto* origin{ GetInstrPtr(cntr->origin) };

    size_t stride{ cntr->elem_stride };

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

        RecordOffset_(cntr, cntr->root, 0, &ght);

        unsigned long long best_cost{ ZETA_Core_ullong_max };

        del_l_cnt = 0;
        del_r_cnt = size - origin_size;

        for (;;) {
            generic_hash_table::Node* ghtn{ generic_hash_table::ExtractAny(
                &ght) };

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

    size_t seg_cnt{ bin_tree::Count(cntr->root) - 2 };

    auto* wb_segs{ static_cast<WBSeg_*>(allocator::SafeAllocate(
                       allocator::weak_lifo_allocator, alignof(WBSeg_),
                       sizeof(WBSeg_) * (seg_cnt + 1))) +
                   1 };

    WBSeg_* lb_wb_seg{ wb_segs - 1 };

    lb_wb_seg->beg = 0;
    lb_wb_seg->size = 0;

    lb_wb_seg->dst_idx = pop_l_cnt;
    lb_wb_seg->acc_ref = pop_l_cnt;

    ToWBSeg_(cntr, cntr->root, wb_segs);

    CircularArray ca{
        .data = {},
        .elem_size = origin->elem_size,
        .elem_stride = stride,
        .elem_cnt = 0,
        .elem_capacity = cntr->seg_elem_capacity,
        .idx_offset = 0,
    };

    WriteWBSeg_(cntr, &ca, wb_segs, seg_cnt, 0, push_l_cnt);

    allocator::Deallocate(allocator::weak_lifo_allocator, lb_wb_seg);

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
    auto* origin{ GetInstrPtr(cntr->origin) };
    size_t origin_size{ SeqCntrGetSize(origin) };

    size_t stride{ cntr->elem_stride };
    size_t size{ GetSize(cntr) };

    size_t seg_cnt{ bin_tree::Count(cntr->root) - 2 };

    auto* wb_segs{ static_cast<WBSeg_*>(allocator::SafeAllocate(
                       allocator::weak_lifo_allocator, alignof(WBSeg_),
                       sizeof(WBSeg_) * (seg_cnt + 2))) +
                   1 };

    WBSeg_* lb_wb_seg{ wb_segs - 1 };

    lb_wb_seg->beg = 0;
    lb_wb_seg->size = 0;

    lb_wb_seg->dst_idx = 0;
    lb_wb_seg->acc_ref = 0;

    auto to_wb_seg_ret{ ToWBSeg_(cntr, cntr->root, wb_segs) };

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
        .elem_capacity = cntr->seg_elem_capacity,
        .idx_offset = 0,
    };

    if (ref_seg_cnt == 0) {
        if (origin_size < size) {
            SeqCntrPopR(origin, size - origin_size);
        } else if (size < origin_size) {
            SeqCntrPushR(origin, origin_size - size, nullptr);
        }

        WriteWBSeg_(cntr, &ca, wb_segs, seg_cnt, 0, 0);

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

            ans_cost = utils::Min(ans_cost, cur_cost);
        }

        dp_best_cost[i] = ans_cost;
    }
#endif

    for (size_t i{ 1 }; i <= ref_seg_cnt + 1; ++i) {
        unsigned long long ans_cost{ ZETA_Core_ullong_max };
        unsigned long long ans_prv{ 0 };

        for (size_t j{ i }, j_end{ utils::Max(8ULL, i) - 8 }; j_end < j--;) {
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

        WriteWBSeg_(cntr, &ca, beg_wb_seg + 1, end_wb_seg - beg_wb_seg - 1,
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
void WriteBack(Cntr<CntrTplArgList>* cntr, int write_back_strategy,
               unsigned long long cost_coeff_read,
               unsigned long long cost_coeff_write,
               unsigned long long cost_coeff_insert,
               unsigned long long cost_coeff_erase) {
    detail::CheckCntr_(cntr);

    auto* origin{ GetInstrPtr(cntr->origin) };

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
void NameSpace::CopyCursor(Cntr<CntrTplArgList> const* cntr,
                           Cursor const* src_cursor, Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);
    detail::CheckCursor_(cntr, dst_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
bool NameSpace::AreEqualCursor(Cntr<CntrTplArgList> const* cntr,
                               Cursor const* cursor_a, Cursor const* cursor_b) {
    return (GetCursorIdx)(cntr, cursor_a) == (GetCursorIdx)(cntr, cursor_b);
}

template <CntrTplParamList>
int NameSpace::CompareCursor(Cntr<CntrTplArgList> const* cntr,
                             Cursor const* cursor_a, Cursor const* cursor_b) {
    return compare::BasicCompare((GetCursorIdx)(cntr, cursor_a) + 1,
                                 (GetCursorIdx)(cntr, cursor_b) + 1);
}

template <CntrTplParamList>
size_t NameSpace::GetCursorDist(Cntr<CntrTplArgList> const* cntr,
                                Cursor const* cursor_a,
                                Cursor const* cursor_b) {
    return GetCursorIdx(cntr, cursor_b) - GetCursorIdx(cntr, cursor_a);
}

template <CntrTplParamList>
size_t NameSpace::GetCursorIdx(Cntr<CntrTplArgList> const* cntr,
                               Cursor const* cursor) {
    detail::CheckCursor_(cntr, cursor);

    return cursor->idx;
}

template <CntrTplParamList>
void NameSpace::CursorStepL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor) {
    (CursorAdvanceL)(cntr, cursor, 1);
}

template <CntrTplParamList>
void NameSpace::CursorStepR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor) {
    (CursorAdvanceR)(cntr, cursor, 1);
}

template <CntrTplParamList>
void NameSpace::CursorAdvanceL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor,
                               size_t step) {
    detail::CheckCursor_(cntr, cursor);

    if (step == 0) { return; }

    size_t elem_size{ cntr->elem_size };

    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    ZETA_Core_DebugAssert(step <= cursor->idx + 1);

    TreeNode* n{ cursor->n };

    size_t n_size;

    n_size = lb == n || rb == n ? 1 : ({
        Seg* seg{ detail::NToSeg_(n) };
        EnStagingTernary(detail::GetNColor_(n) == ref_color, seg->ref.elem_cnt,
                         seg->dat.elem_cnt);
    });

    auto [dst_n, dst_seg_idx]{ bin_tree::AdvanceL(
        n, n_size - 1 - cursor->seg_idx + step) };

    ZETA_Core_DebugAssert(dst_n != nullptr);

    cursor->idx -= step;
    cursor->n = dst_n;

    if (lb == dst_n) {
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
        .elem_stride = cntr->elem_stride,
        .elem_cnt = dst_seg->dat.elem_cnt,
        .elem_capacity = cntr->seg_elem_capacity,
        .idx_offset = dst_seg->dat.idx_offset,
    };

    cursor->elem =
        circular_array::Access(&ca, cursor->seg_idx, true, nullptr, nullptr);
}

template <CntrTplParamList>
void NameSpace::CursorAdvanceR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor,
                               size_t step) {
    detail::CheckCursor_(cntr, cursor);

    if (step == 0) { return; }

    size_t elem_size{ cntr->elem_size };
    size_t elem_cnt{ (GetElemCnt)(cntr) };

    TreeNode* rb{ cntr->rb };

    ZETA_Core_DebugAssert(step <= elem_cnt - cursor->idx);

    auto [dst_n,
          dst_seg_idx]{ bin_tree::AdvanceR(cursor->n, cursor->seg_idx + step) };

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
        .elem_stride = cntr->elem_stride,
        .elem_cnt = dst_seg->dat.elem_cnt,
        .elem_capacity = cntr->seg_elem_capacity,
        .idx_offset = dst_seg->dat.idx_offset,
    };

    cursor->elem =
        circular_array::Access(&ca, cursor->seg_idx, true, nullptr, nullptr);
}

namespace NameSpace::detail {

template <CntrTplParamList>
void PrintState_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* cntr, TreeNode* n) {
    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    TreeNode* l_n{ bin_tree::GetL(n) };
    TreeNode* r_n{ bin_tree::GetR(n) };

    if (l_n != nullptr) { (PrintState_)(cntr, l_n); }

    if (lb != n && rb != n) {
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

    if (r_n != nullptr) { PrintState_(cntr, r_n); }
}

}  // namespace NameSpace::detail

template <CntrTplParamList>
void NameSpace::PrintState(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    detail::PrintState_(cntr, cntr->root);
}

namespace NameSpace::detail {

template <CntrTplParamList>
Stats GetStats_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* cntr, TreeNode* n) {
    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

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
        if (lb != n && rb != n) {
            Seg* seg{ NToSeg_(n) };

#if EnStaging
            if (GetNColor_(n) == ref_color) {
                ++ret.ref_seg_cnt;
                ret.ref_size += seg->ref.elem_cnt;
            } else
#endif
            {
                ++ret.dat_seg_cnt;
                ret.dat_size += seg->dat.elem_cnt;
            }
        }

        TreeNode* l_n{ bin_tree::GetL(n) };
        TreeNode* r_n{ bin_tree::GetR(n) };

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

}  // namespace NameSpace::detail

template <CntrTplParamList>
NameSpace::Stats NameSpace::GetStats(Cntr<CntrTplArgList> const* cntr) {
    detail::CheckCntr_(cntr);

    TreeNode* root{ cntr->root };

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

    return detail::GetStats_(cntr, cntr->root);
}

namespace NameSpace::detail {

struct SanitizeRet_ {
    bool b;
    size_t ref_beg;
    size_t ref_end;
};

template <CntrTplParamList>
SanitizeRet_ Sanitize_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const* cntr, mem_recorder::MemRecorder* dst_seg,
     mem_recorder::MemRecorder* dst_data, TreeNode* n) {
#if EnStaging
    auto* origin{ utils::GetInstPtr(cntr->origin) };
    size_t origin_elem_cnt{ seq_cntr::GetElemCnt(origin) };
#endif

    size_t elem_size{ cntr->elem_size };
    size_t elem_stride{ cntr->elem_stride };
    size_t seg_elem_capacity{ cntr->seg_elem_capacity };
    size_t data_size{ detail::GetDataSize_(elem_size, elem_stride,
                                           seg_elem_capacity) };

    TreeNode* lb{ cntr->lb };
    TreeNode* rb{ cntr->rb };

    TreeNode* l_n{ bin_tree::StepL(n) };
    TreeNode* r_n{ bin_tree::StepR(n) };

    if (lb == n) {
        ZETA_Core_DebugAssert(l_n == nullptr);
        ZETA_Core_DebugAssert(r_n != nullptr);

        ZETA_Core_DebugAssert(bin_tree::GetSize(n) == 1);

        if (dst_seg != nullptr) {
            mem_recorder::Record(dst_seg, lb, sizeof(TreeNode));
        }
    } else if (rb == n) {
        ZETA_Core_DebugAssert(l_n != nullptr);
        ZETA_Core_DebugAssert(r_n == nullptr);

        ZETA_Core_DebugAssert(bin_tree::GetSize(n) == 1);

        if (dst_seg != nullptr) {
            mem_recorder::Record(dst_seg, rb, sizeof(TreeNode));
        }
    }
#if EnStaging
    else if (GetNColor_(n) == ref_color) {
        ZETA_Core_DebugAssert(l_n != nullptr);
        ZETA_Core_DebugAssert(r_n != nullptr);

        Seg* seg{ NToSeg_(n) };

        if (dst_seg != nullptr) {
            mem_recorder::Record(dst_seg, seg, sizeof(Seg));
        }

        ZETA_Core_DebugAssert(0 < seg->ref.elem_cnt);

        ZETA_Core_DebugAssert(seg->ref.beg < origin_elem_cnt);
        ZETA_Core_DebugAssert(seg->ref.beg + seg->ref.elem_cnt <=
                              origin_elem_cnt);

        size_t size{ seg->ref.elem_cnt };

        ZETA_Core_DebugAssert(size == bin_tree::GetSize(n));

        if (lb != l_n) {
            Seg* l_seg{ NToSeg_(l_n) };

            if (GetNColor_(&l_seg->n) == ref_color) {
                ZETA_Core_DebugAssert(l_seg->ref.beg + l_seg->ref.elem_cnt <
                                      seg->ref.beg);
            }
        }

        if (rb != r_n) {
            Seg* r_seg{ NToSeg_(r_n) };

            if (GetNColor_(&r_seg->n) == ref_color) {
                ZETA_Core_DebugAssert(seg->ref.beg + seg->ref.elem_cnt <
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
            mem_recorder::Record(dst_seg, seg, sizeof(Seg));
        }

        size_t elem_cnt{ seg->dat.elem_cnt };

        ZETA_Core_DebugAssert(elem_cnt == bin_tree::GetSize(n));

        ZETA_Core_DebugAssert(0 < elem_cnt);
        ZETA_Core_DebugAssert(elem_cnt <= seg_elem_capacity);
        ZETA_Core_DebugAssert(seg->dat.idx_offset < seg_elem_capacity);

        if (dst_data != nullptr) {
            mem_recorder::Record(dst_data, seg->dat.data, data_size);
        }

        size_t l_vac;
        size_t r_vac;

        if (lb == l_n) {
            l_vac = 0;
        } else {
            Seg* l_seg{ NToSeg_(l_n) };

            l_vac = seg_elem_capacity -
                    EnStagingTernary(
                        GetNColor_(l_n) == ref_color,
                        utils::Min(l_seg->ref.elem_cnt, seg_elem_capacity),
                        l_seg->dat.elem_cnt);
        }

        if (rb == r_n) {
            r_vac = 0;
        } else {
            Seg* r_seg{ NToSeg_(r_n) };

            r_vac = seg_elem_capacity -
                    EnStagingTernary(
                        GetNColor_(r_n) == ref_color,
                        utils::Min(r_seg->ref.elem_cnt, seg_elem_capacity),
                        r_seg->dat.elem_cnt);
        }

        ZETA_Core_DebugAssert(0 <= l_vac);
        ZETA_Core_DebugAssert(0 <= r_vac);
    }

    TreeNode* sub_l_n{ bin_tree::GetL(n) };
    TreeNode* sub_r_n{ bin_tree::GetR(n) };

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

    if (lb == n || rb == n) { return ret; }

#if EnStaging
    if (GetNColor_(n) == ref_color) {
        Seg* seg{ NToSeg_(n) };

        size_t cur_ref_beg{ seg->ref.beg };
        size_t cur_ref_end{ seg->ref.beg + seg->ref.elem_cnt };

        if (l_check_ret.b) {
            ZETA_Core_DebugAssert(l_check_ret.ref_end <= cur_ref_beg);
        }
        if (r_check_ret.b) {
            ZETA_Core_DebugAssert(cur_ref_end <= r_check_ret.ref_beg);
        }

        if (ret.b) {
            ret.ref_beg = utils::Min(ret.ref_beg, cur_ref_beg);
            ret.ref_end = utils::Max(ret.ref_end, cur_ref_end);
        } else {
            ret.b = true;
            ret.ref_beg = cur_ref_beg;
            ret.ref_end = cur_ref_end;
        }
    }
#endif

    return ret;
}

}  // namespace NameSpace::detail

template <CntrTplParamList>
void NameSpace::Sanitize(Cntr<CntrTplArgList> const* cntr,
                         mem_recorder::MemRecorder* dst_seg,
                         mem_recorder::MemRecorder* dst_data) {
    detail::CheckCntr_(cntr);

    bin_tree::Sanitize(cntr->root);
    rbtree::Sanitize(nullptr, cntr->root);

#if ZETA_Core_EnableDebug
    detail::Sanitize_(cntr, dst_seg, dst_data, cntr->root);
#else
    ZETA_Core_Unused(dst_seg);
    ZETA_Core_Unused(dst_data);
#endif
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    GetReferedInst(NameSpace::Cntr<CntrTplArgList> const* cntr) {
    return const_cast<NameSpace::Cntr<CntrTplArgList>*>(cntr);
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    NameSpace::Cntr<CntrTplArgList>, void>::GetStaticEnabledAbilityFlag() {
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
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const,
                     void>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>,
                                void>::GetStaticEnabledAbilityFlag() &
           seq_cntr::const_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    NameSpace::Cntr<CntrTplArgList>, void>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const,
                     void>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::non_const_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    GetDynamicEnabledAbilityFlag(NameSpace::Cntr<CntrTplArgList> const*) {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    GetDynamicDisabledAbilityFlag(NameSpace::Cntr<CntrTplArgList> const*) {
    return seq_cntr::empty_ability_flag;
}

template <CntrTplParamList>
size_t seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    GetCursorSize(NameSpace::Cntr<CntrTplArgList> const* cntr) {
    return NameSpace::GetCursorSize(cntr);
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::GetElemSize(
    NameSpace::Cntr<CntrTplArgList> const* cntr) {
    return NameSpace::GetElemSize(cntr);
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::GetElemCnt(
    NameSpace::Cntr<CntrTplArgList> const* cntr) {
    return NameSpace::GetElemCnt(cntr);
}

template <CntrTplParamList>
size_t seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    GetMaxElemCnt(NameSpace::Cntr<CntrTplArgList> const* cntr) {
    return NameSpace::GetMaxElemCnt(cntr);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    GetLBCursor(NameSpace::Cntr<CntrTplArgList> const* cntr, void* dst_cursor) {
    NameSpace::GetLBCursor(cntr, static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    GetRBCursor(NameSpace::Cntr<CntrTplArgList> const* cntr, void* dst_cursor) {
    NameSpace::GetRBCursor(cntr, static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::PeekL(
    NameSpace::Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return NameSpace::PeekL(cntr, lazy_copy_elem,
                            static_cast<NameSpace::Cursor*>(dst_cursor),
                            dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::PeekR(
    NameSpace::Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return NameSpace::PeekR(cntr, lazy_copy_elem,
                            static_cast<NameSpace::Cursor*>(dst_cursor),
                            dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::Access(
    NameSpace::Cntr<CntrTplArgList> const* cntr, size_t idx,
    bool lazy_copy_elem, void* dst_cursor, void* dst_elem) {
    return NameSpace::Access(cntr, idx, lazy_copy_elem,
                             static_cast<NameSpace::Cursor*>(dst_cursor),
                             dst_elem);
}

template <CntrTplParamList>
void* seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    Derefer(NameSpace::Cntr<CntrTplArgList> const* cntr, void const* pos_cursor,
            bool lazy_copy_elem, void* dst_elem) {
    return NameSpace::Derefer(cntr,
                              static_cast<NameSpace::Cursor const*>(pos_cursor),
                              lazy_copy_elem, dst_elem);
}

template <CntrTplParamList>
template <typename Reader>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::Read(
    NameSpace::Cntr<CntrTplArgList> const* cntr, void const* pos_cursor,
    size_t cnt, Reader&& reader, void* dst_cursor) {
    NameSpace::Read(cntr, static_cast<NameSpace::Cursor const*>(pos_cursor),
                    cnt, meta::Forward<Reader>(reader),
                    static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::Write(
    NameSpace::Cntr<CntrTplArgList>* cntr, void* pos_cursor, size_t cnt,
    Writer&& writer, void* dst_cursor) {
    NameSpace::Write(cntr, static_cast<NameSpace::Cursor*>(pos_cursor), cnt,
                     meta::Forward<Writer>(writer),
                     static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename ReaderWriter>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::ReadWrite(
    NameSpace::Cntr<CntrTplArgList>* cntr, void* pos_cursor, size_t cnt,
    ReaderWriter&& reader_writer, void* dst_cursor) {
    NameSpace::ReadWrite(cntr, static_cast<NameSpace::Cursor*>(pos_cursor), cnt,
                         meta::Forward<ReaderWriter>(reader_writer),
                         static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::PushL(
    NameSpace::Cntr<CntrTplArgList>* cntr, size_t cnt, Writer&& writer,
    void* dst_cursor) {
    return NameSpace::PushL(cntr, cnt, meta::Forward<Writer>(writer),
                            static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::PushR(
    NameSpace::Cntr<CntrTplArgList>* cntr, size_t cnt, Writer&& writer,
    void* dst_cursor) {
    return NameSpace::PushR(cntr, cnt, meta::Forward<Writer>(writer),
                            static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
template <typename Writer>
void* seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::Insert(
    NameSpace::Cntr<CntrTplArgList>* cntr, void* pos_cursor, size_t cnt,
    Writer&& writer, void* dst_cursor) {
    return NameSpace::Insert(cntr, static_cast<NameSpace::Cursor*>(pos_cursor),
                             cnt, meta::Forward<Writer>(writer),
                             static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::PopL(
    NameSpace::Cntr<CntrTplArgList>* cntr, size_t cnt) {
    NameSpace::PopL(cntr, cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::PopR(
    NameSpace::Cntr<CntrTplArgList>* cntr, size_t cnt) {
    NameSpace::PopR(cntr, cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::Erase(
    NameSpace::Cntr<CntrTplArgList>* cntr, void* pos_cursor, size_t cnt) {
    NameSpace::Erase(cntr, static_cast<NameSpace::Cursor*>(pos_cursor), cnt);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList>, void>::EraseAll(
    NameSpace::Cntr<CntrTplArgList>* cntr) {
    NameSpace::EraseAll(cntr);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    CopyCursor(NameSpace::Cntr<CntrTplArgList> const* cntr,
               void const* src_cursor, void* dst_cursor) {
    NameSpace::CopyCursor(cntr,
                          static_cast<NameSpace::Cursor const*>(src_cursor),
                          static_cast<NameSpace::Cursor*>(dst_cursor));
}

template <CntrTplParamList>
bool seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    AreEqualCursor(NameSpace::Cntr<CntrTplArgList> const* cntr,
                   void const* cursor_a, void const* cursor_b) {
    return NameSpace::AreEqualCursor(
        cntr, static_cast<NameSpace::Cursor const*>(cursor_a),
        static_cast<NameSpace::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
int seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    CompareCursor(NameSpace::Cntr<CntrTplArgList> const* cntr,
                  void const* cursor_a, void const* cursor_b) {
    return NameSpace::CompareCursor(
        cntr, static_cast<NameSpace::Cursor const*>(cursor_a),
        static_cast<NameSpace::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
size_t seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    GetCursorDist(NameSpace::Cntr<CntrTplArgList> const* cntr,
                  void const* cursor_a, void const* cursor_b) {
    return NameSpace::GetCursorDist(
        cntr, static_cast<NameSpace::Cursor const*>(cursor_a),
        static_cast<NameSpace::Cursor const*>(cursor_b));
}

template <CntrTplParamList>
size_t
seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::GetCursorIdx(
    NameSpace::Cntr<CntrTplArgList> const* cntr, void const* cursor) {
    return NameSpace::GetCursorIdx(
        cntr, static_cast<NameSpace::Cursor const*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    CursorStepL(NameSpace::Cntr<CntrTplArgList> const* cntr, void* cursor) {
    NameSpace::CursorStepL(cntr, static_cast<NameSpace::Cursor*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    CursorStepR(NameSpace::Cntr<CntrTplArgList> const* cntr, void* cursor) {
    NameSpace::CursorStepR(cntr, static_cast<NameSpace::Cursor*>(cursor));
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    CursorAdvanceL(NameSpace::Cntr<CntrTplArgList> const* cntr, void* cursor,
                   size_t step) {
    NameSpace::CursorAdvanceL(cntr, static_cast<NameSpace::Cursor*>(cursor),
                              step);
}

template <CntrTplParamList>
void seq_cntr::CntrTraits<NameSpace::Cntr<CntrTplArgList> const, void>::
    CursorAdvanceR(NameSpace::Cntr<CntrTplArgList> const* cntr, void* cursor,
                   size_t step) {
    NameSpace::CursorAdvanceR(cntr, static_cast<NameSpace::Cursor*>(cursor),
                              step);
}

}  // namespace zeta::core

#pragma pop_macro("NameSpace")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("EnStagingTernary")

// NOLINTEND(cppcoreguidelines-pro-type-union-access)
