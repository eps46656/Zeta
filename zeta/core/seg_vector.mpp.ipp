#if ZETA_Core_Editor
#define EnStaging 1
#endif

#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

#include <zeta/core/allocator.hpp>
#include <zeta/core/basic_bin_tree_node.ipp>
#include <zeta/core/bin_tree.ipp>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/comparison.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/debug_utils/mem_recording.ipp>
#include <zeta/core/debug_utils/recording_allocator.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/function_ref.ipp>
// #include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/lifecycle.ipp>
#include <zeta/core/pool_allocator.hpp>
#include <zeta/core/rbtree.ipp>
#include <zeta/core/seg_utils.ipp>
#include <zeta/core/seg_vector.mpp.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr_utils.ipp>
#include <zeta/core/utils.hpp>
#include <zeta/core/utils.ipp>

ZETA_Core_ClangdPreambleBarrier;

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

#if EnStaging

#pragma push_macro("Namespace")
#define Namespace staging_seg_vector

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                             \
    seq_cntr::IsSeqCntrLike OriginLike,              \
        allocator::IsAllocatorLike SegAllocatorLike, \
        allocator::IsAllocatorLike DataAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList OriginLike, SegAllocatorLike, DataAllocatorLike

#pragma push_macro("EnStagingTernary")
#define EnStagingTernary(cond, x, y) ((cond) ? (x) : (y))

#pragma push_macro("CntrWorkTplParamList")
#define CntrWorkTplParamList                                     \
    meta::IsNonConstObjectType Elem, seq_cntr::IsSeqCntr Origin, \
        allocator::IsAllocator SegAllocator,                     \
        allocator::IsAllocator DataAllocator

#pragma push_macro("CntrWorkTplArgList")
#define CntrWorkTplArgList Elem, Origin, SegAllocator, DataAllocator

#else

#pragma push_macro("Namespace")
#define Namespace seg_vector

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList                             \
    meta::IsNonConstObjectType Elem,                 \
        allocator::IsAllocatorLike SegAllocatorLike, \
        allocator::IsAllocatorLike DataAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList Elem, SegAllocatorLike, DataAllocatorLike

#pragma push_macro("EnStagingTernary")
#define EnStagingTernary(cond, x, y) (y)

#pragma push_macro("CntrWorkTplParamList")
#define CntrWorkTplParamList                                              \
    meta::IsNonConstObjectType Elem, allocator::IsAllocator SegAllocator, \
        allocator::IsAllocator DataAllocator

#pragma push_macro("CntrWorkTplArgList")
#define CntrWorkTplArgList Elem, SegAllocator, DataAllocator

#endif

namespace zeta::core {

constexpr Namespace::Seg::Seg() : n{ 0 } {}

namespace Namespace::detail {

#if EnStaging

constexpr unsigned GetNColor_  // NOLINT(misc-use-internal-linkage)
    (Node const* n) {
    unsigned color{ n->GetLColor() };
    ZETA_Core_DebugUtils_Diag_PromiseAssert(color == ref_color ||
                                            color == dat_color);
    return color;
}

constexpr void DirectlySetNColor_  // NOLINT(misc-use-internal-linkage)
    (Node* n, unsigned color) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(color == ref_color ||
                                            color == dat_color);

    n->SetLColor(color);
}

constexpr void SetNColor_  // NOLINT(misc-use-internal-linkage)
    (Node* n, unsigned color) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(color == ref_color ||
                                            color == dat_color);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 <= (GetNColor_)(n));

    (DirectlySetNColor_)(n, color);
}

#endif

constexpr Seg* NToSeg_  // NOLINT(misc-use-internal-linkage)
    (Node* n) {
    return ZETA_Core_MemberToStruct(Seg, n, n);
}

template <meta::IsNonConstObjectType Elem_,
#if EnStaging
          seq_cntr::IsSeqCntr Origin_,
#endif
          allocator::IsAllocator SegAllocator_,
          allocator::IsAllocator DataAllocator_>
struct CntrWork_ {
    using Elem = Elem_;

#if EnStaging
    using Origin = Origin_;

    using OriginCursor =
        meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
            meta::TypeWrapper<Origin_&>{}))>;
#endif

    using SegAllocator  = SegAllocator_;
    using DataAllocator = DataAllocator_;

    size_t seg_elem_slot_cnt;
    size_t data_size;

    size_t elem_cnt;

#if EnStaging
    Origin const& origin;
#endif

    SegAllocator seg_alctr;
    DataAllocator data_alctr;
};

template <CntrTplParamList>
constexpr auto MakeCntrWork_(Cntr<CntrTplArgList> const& self) {
#if EnStaging
    using Elem = typename Cntr<CntrTplArgList>::Elem;

    auto& origin{ meta::GetInstRefFromLike(self.origin_like) };
#endif

    auto& seg_alctr{ meta::GetInstRefFromLike(self.seg_alctr_like) };
    auto& data_alctr{ meta::GetInstRefFromLike(self.data_alctr_like) };

    size_t seg_elem_slot_cnt{ self.seg_elem_slot_cnt };

    size_t elem_cnt{ bin_tree::GetAccSize(self.root) - 2 };

    return CntrWork_<Elem,
#if EnStaging
                     decltype(origin),
#endif
                     meta::RemoveConstReference<decltype(seg_alctr)>&,
                     meta::RemoveConstReference<decltype(data_alctr)>&>{
        .seg_elem_slot_cnt{ seg_elem_slot_cnt },
        .data_size{ sizeof(Elem) * seg_elem_slot_cnt },

        .elem_cnt{ elem_cnt },

#if EnStaging
        .origin{ origin },
#endif

        .seg_alctr{
            const_cast<meta::RemoveConstReference<decltype(seg_alctr)>&>(
                seg_alctr) },

        .data_alctr{
            const_cast<meta::RemoveConstReference<decltype(data_alctr)>&>(
                data_alctr) },
    };
}

template <meta::IsNonConstObjectType Elem>
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

    CircularArray<Elem> ca;

    size_t elem_vac;

    constexpr void SetBasics(size_t seg_elem_slot_cnt) {
        this->ca.elem_stride = sizeof(Elem);
        this->ca.slot_cnt    = seg_elem_slot_cnt;
    }

    template <
#if EnStaging
        seq_cntr::IsSeqCntr Origin,
#endif
        allocator::IsAllocator SegAllocator,
        allocator::IsAllocator DataAllocator>
    constexpr void SetBasics(CntrWork_<CntrWorkTplArgList> const& self_work) {
        this->SetBasics(self_work.seg_elem_slot_cnt);
    }

    constexpr void SetNull() {
        this->is_null     = true;
        this->is_dirty    = false;
        this->ca.elem_cnt = 0;
        this->elem_vac    = 0;
    }

    constexpr void CopyFromSameBasics(SegWork_ const& other) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(this->ca.slot_cnt ==
                                                other.ca.slot_cnt);

        this->is_null  = other.is_null;
        this->is_dirty = other.is_dirty;
        this->seg      = other.seg;

#if EnStaging
        this->color = other.color;

        if (this->color == ref_color) {
            this->ref_beg = other.ref_beg;
        } else
#endif

        {
            this->ca.data = other.ca.data;
            this->ca.rot  = other.ca.rot;
        }

        this->ca.elem_cnt = other.ca.elem_cnt;
        this->elem_vac    = other.elem_vac;
    }

    constexpr void Load(Seg* seg) {
        this->is_null = false;

        this->is_dirty = false;

        this->seg = seg;

#if EnStaging
        this->color = GetNColor_(&seg->n);

        if (this->color == ref_color) {
            this->ref_beg     = seg->ref.beg;
            this->ca.elem_cnt = seg->ref.elem_cnt;
            this->elem_vac    = ca.slot_cnt - comparison_utils::BasicMin(
                                                  seg->ref.elem_cnt, ca.slot_cnt);
        } else
#endif
        {
            this->ca.data     = static_cast<Elem*>(seg->dat.data);
            this->ca.elem_cnt = seg->dat.elem_cnt;
            this->ca.rot      = seg->dat.rot;
            this->elem_vac    = ca.slot_cnt - this->ca.elem_cnt;
        }
    }

    constexpr void Store(Seg* seg) const {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(seg != nullptr);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(!this->is_null);

        bin_tree::AddDiffSize(
            &seg->n,
            this->ca.elem_cnt -
                EnStagingTernary((GetNColor_)(&seg->n) == ref_color,
                                 seg->ref.elem_cnt, seg->dat.elem_cnt));

#if EnStaging
        (SetNColor_)(&seg->n, this->color);

        if (this->color == ref_color) {
            seg->ref.beg      = this->ref_beg;
            seg->ref.elem_cnt = this->ca.elem_cnt;
        } else
#endif
        {
            seg->dat.data     = this->ca.data;
            seg->dat.elem_cnt = static_cast<unsigned short>(this->ca.elem_cnt);
            seg->dat.rot      = static_cast<unsigned short>(this->ca.rot);
        }
    }

    constexpr void Store() const { this->Store(this->seg); }
};

template <CntrWorkTplParamList>
constexpr void CheckCntrSegWorkMatched_(
    CntrWork_<CntrWorkTplArgList> const& self_work,
    SegWork_<Elem> const& seg_work) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_work.ca.elem_stride ==
                                            sizeof(Elem));
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_work.ca.slot_cnt ==
                                            self_work.seg_elem_slot_cnt);
}

template <allocator::IsAllocator DataAllocator>
constexpr void* AllocateData_  // NOLINT(misc-use-internal-linkage)
    (size_t data_size, DataAllocator& data_alctr) {
    return allocator::SafeAllocate(data_alctr, 1, data_size);
}

#if EnStaging

template <allocator::IsAllocator SegAllocator>
constexpr Seg* CreateRefSeg_  // NOLINT(misc-use-internal-linkage)
    (SegAllocator& seg_alctr) {
    Seg* seg{ new (
        allocator::SafeAllocate(seg_alctr, alignof(Seg), sizeof(Seg))) Seg{} };

    (DirectlySetNColor_)(&seg->n, ref_color);

    seg->ref.beg      = 0;
    seg->ref.elem_cnt = 0;

    return seg;
}

#endif

template <allocator::IsAllocator SegAllocator,
          allocator::IsAllocator DataAllocator>
constexpr Seg* CreateDatSeg_  // NOLINT(misc-use-internal-linkage)
    (size_t data_size, SegAllocator& seg_alctr, DataAllocator& data_alctr) {
    Seg* seg{ new (
        allocator::SafeAllocate(seg_alctr, alignof(Seg), sizeof(Seg))) Seg{} };

#if EnStaging
    (DirectlySetNColor_)(&seg->n, dat_color);
#endif

    seg->dat.data     = (AllocateData_)(data_size, data_alctr);
    seg->dat.elem_cnt = 0;
    seg->dat.rot      = 0;

    return seg;
}

#if EnStaging

template <allocator::IsAllocator SegAllocator>
constexpr void DestroyRefSeg_  // NOLINT(misc-use-internal-linkage)
    (Seg* seg, SegAllocator& seg_alctr) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert((GetNColor_)(&seg->n) == ref_color);

    seg->~Seg();

    allocator::Deallocate(seg_alctr, seg);
}

#endif

template <allocator::IsAllocator SegAllocator,
          allocator::IsAllocator DataAllocator>
constexpr void DestroyDatSeg_  // NOLINT(misc-use-internal-linkage)
    (Seg* seg, SegAllocator& seg_alctr, DataAllocator& data_alctr) {
#if EnStaging
    ZETA_Core_DebugUtils_Diag_PromiseAssert((GetNColor_)(&seg->n) == dat_color);
#endif

    allocator::Deallocate(data_alctr, seg->dat.data);

    seg->~Seg();

    allocator::Deallocate(seg_alctr, seg);
}

template <allocator::IsAllocator SegAllocator,
          allocator::IsAllocator DataAllocator>
constexpr void DestroySeg_  // NOLINT(misc-use-internal-linkage)
    (Seg* seg, SegAllocator& seg_alctr, DataAllocator& data_alctr) {
#if EnStaging
    unsigned color{ (GetNColor_)(&seg->n) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(color == ref_color ||
                                            color == dat_color);
#endif

#if EnStaging
    if (color == ref_color) {
        (DestroyRefSeg_)(seg, seg_alctr);
    } else
#endif
    {
        (DestroyDatSeg_)(seg, seg_alctr, data_alctr);
    }
}

#if EnStaging

template <CntrWorkTplParamList>
constexpr void MaterializeRefSeg_(
    CntrWork_<CntrWorkTplArgList> const& self_work, SegWork_<Elem>& seg_work) {
#if EnStaging
    using OriginCursor = typename CntrWork_<CntrWorkTplArgList>::OriginCursor;
#endif

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!seg_work.is_null);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_work.color == ref_color);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_work.ca.elem_cnt <=
                                            self_work.seg_elem_slot_cnt);

    OriginCursor origin_cursor;

    Elem* data{ static_cast<Elem*>(
        (AllocateData_)(self_work.data_size, self_work.data_alctr)) };

    seq_cntr::Refer(self_work.origin, seg_work.ref_beg, &origin_cursor,
                    nullptr);

    seq_cntr::Read(self_work.origin, &origin_cursor, seg_work.ca.elem_cnt,
                   lin_seq_endpoint::acceptor::Acceptor<
                       lifecycle::ConstructingMemToObjView<Elem>>{
                       .data{ data },
                       .elem_stride{ sizeof(Elem) },
                       .elem_cnt{ seq_cntr::max_max_elem_cnt },
                   },
                   nullptr);

    seg_work.is_dirty = true;
    seg_work.color    = dat_color;
    seg_work.ca.data  = data;
    seg_work.ca.rot   = 0;
}

template <CntrWorkTplParamList,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor,
          seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void AugMaterializeRefSeg_(
    CntrWork_<CntrWorkTplArgList> const& self_work, SegWork_<Elem>& seg_work,
    size_t l_cnt, size_t ins_cnt, size_t r_cnt, Acceptor&& acceptor,
    Provider&& provider) {
#if EnStaging
    using OriginCursor = typename CntrWork_<CntrWorkTplArgList>::OriginCursor;
#endif

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!seg_work.is_null);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_work.color == ref_color);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_cnt + ins_cnt + r_cnt <=
                                            seg_work.ca.slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(l_cnt + r_cnt <=
                                            seg_work.ca.elem_cnt);

    OriginCursor origin_cursor;
    bool origin_cursor_is_refered{ false };
    size_t origin_cursor_refer_idx{ 0 };

    Elem* data{ static_cast<Elem*>(
        (AllocateData_)(self_work.data_size, self_work.data_alctr)) };

    if (0 < l_cnt) {
        size_t target_idx{ seg_work.ref_beg };

        if (!origin_cursor_is_refered) {
            seq_cntr::Refer(self_work.origin, target_idx, &origin_cursor,
                            nullptr);

            origin_cursor_is_refered = true;
            origin_cursor_refer_idx  = target_idx;
        }

        ZETA_Core_DebugUtils_Diag_ProbeAssert(origin_cursor_is_refered);
        ZETA_Core_DebugUtils_Diag_ProbeAssert(origin_cursor_refer_idx ==
                                              target_idx);

        seq_cntr::Read(self_work.origin, &origin_cursor, l_cnt,
                       lin_seq_endpoint::acceptor::Acceptor<
                           lifecycle::ConstructingMemToObjView<Elem>>{
                           .data{ data },
                           .elem_stride{ sizeof(Elem) },
                           .elem_cnt{ seq_cntr::max_max_elem_cnt },
                       },
                       &origin_cursor);

        origin_cursor_refer_idx += l_cnt;
    }

    {
        size_t m_cnt{ seg_work.ca.elem_cnt - l_cnt - r_cnt };

        if (0 < m_cnt) {
            if (!origin_cursor_is_refered) {
                size_t target_idx{ seg_work.ref_beg + l_cnt };

                seq_cntr::Refer(self_work.origin, target_idx, &origin_cursor,
                                nullptr);

                origin_cursor_is_refered = true;
                origin_cursor_refer_idx  = target_idx;
            }

            ZETA_Core_DebugUtils_Diag_ProbeAssert(origin_cursor_refer_idx ==
                                                  seg_work.ref_beg + l_cnt);

            seq_cntr::Read(self_work.origin, &origin_cursor, m_cnt, acceptor,
                           &origin_cursor);

            origin_cursor_refer_idx += m_cnt;
        }
    }

    if (0 < ins_cnt) {
        seq_endpoint::provider::Transfer(
            provider, lifecycle::ConstructingMemToObjView{ data + l_cnt },
            sizeof(Elem), ins_cnt);
    }

    if (0 < r_cnt) {
        size_t target_idx{ seg_work.ref_beg + seg_work.ca.elem_cnt - r_cnt };

        if (!origin_cursor_is_refered) {
            seq_cntr::Refer(self_work.origin, target_idx, &origin_cursor,
                            nullptr);

            origin_cursor_is_refered = true;
        } else if (origin_cursor_refer_idx != target_idx) {
            ZETA_Core_DebugUtils_Diag_ProbeAssert(origin_cursor_refer_idx <
                                                  target_idx);

            seq_cntr::CursorAdvanceR(self_work.origin, &origin_cursor,
                                     target_idx - origin_cursor_refer_idx);
        }

        origin_cursor_refer_idx = target_idx;

        seq_cntr::Read(self_work.origin, &origin_cursor, r_cnt,
                       lin_seq_endpoint::acceptor::Acceptor<
                           lifecycle::ConstructingMemToObjView<Elem>>{
                           .data{ data + (l_cnt + ins_cnt) },
                           .elem_stride{ sizeof(Elem) },
                           .elem_cnt{ seq_cntr::max_max_elem_cnt },
                       },
                       nullptr);
    }

    seg_work.is_dirty    = true;
    seg_work.color       = dat_color;
    seg_work.ca.data     = data;
    seg_work.ca.elem_cnt = l_cnt + ins_cnt + r_cnt;
    seg_work.ca.rot      = 0;
    seg_work.elem_vac    = seg_work.ca.slot_cnt - seg_work.ca.elem_cnt;
}

#endif

template <CntrWorkTplParamList>
constexpr void SegShoveL_(CntrWork_<CntrWorkTplArgList> const& self_work,
                          SegWork_<Elem>& l_seg_work,
                          SegWork_<Elem>& r_seg_work, size_t shove_cnt) {
    (CheckCntrSegWorkMatched_)(self_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(self_work, r_seg_work);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!l_seg_work.is_null);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(!r_seg_work.is_null);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < shove_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            r_seg_work.ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        l_seg_work.ca.elem_cnt + shove_cnt <= self_work.seg_elem_slot_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (l_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(self_work, l_seg_work);
    }
#endif

#if EnStaging
    if (r_seg_work.color == ref_color) {
        seq_cntr_utils::provider::Provider<Origin const&> origin_provider{
            .cntr{ self_work.origin },
            .cursor{},
            .cnt{ seq_cntr::max_max_elem_cnt },
        };

        seq_cntr::Refer(self_work.origin, r_seg_work.ref_beg,
                        &origin_provider.cursor, nullptr);

        seq_cntr::PushR(l_seg_work.ca, shove_cnt, origin_provider, nullptr,
                        nullptr);

        r_seg_work.ref_beg += shove_cnt;
        r_seg_work.ca.elem_cnt -= shove_cnt;
        r_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt -
            comparison_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                       self_work.seg_elem_slot_cnt);
    } else
#endif
    {
        seg_utils::SegShoveL(l_seg_work.ca, r_seg_work.ca, shove_cnt);

        r_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
    }

    l_seg_work.elem_vac = self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
}

template <CntrWorkTplParamList>
constexpr void SegShoveR_(CntrWork_<CntrWorkTplArgList> const& self_work,
                          SegWork_<Elem>& l_seg_work,
                          SegWork_<Elem>& r_seg_work, size_t shove_cnt) {
    (CheckCntrSegWorkMatched_)(self_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(self_work, r_seg_work);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!l_seg_work.is_null);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(!r_seg_work.is_null);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < shove_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            l_seg_work.ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        r_seg_work.ca.elem_cnt + shove_cnt <= self_work.seg_elem_slot_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (r_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(self_work, r_seg_work);
    }
#endif

#if EnStaging
    if (l_seg_work.color == ref_color) {
        seq_cntr_utils::provider::Provider<Origin const&> origin_provider{
            .cntr{ self_work.origin },
            .cursor{},
            .cnt{ seq_cntr::max_max_elem_cnt },
        };

        seq_cntr::Refer(self_work.origin,
                        l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - shove_cnt,
                        &origin_provider.cursor, nullptr);

        seq_cntr::PushL(r_seg_work.ca, shove_cnt, origin_provider, nullptr,
                        nullptr);

        l_seg_work.ca.elem_cnt -= shove_cnt;
        l_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt -
            comparison_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                       self_work.seg_elem_slot_cnt);
    } else
#endif
    {
        seg_utils::SegShoveR(l_seg_work.ca, r_seg_work.ca, shove_cnt);

        l_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
    }

    r_seg_work.elem_vac = self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
}

template <CntrWorkTplParamList,
          seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void SegInsertShoveL_(CntrWork_<CntrWorkTplArgList> const& self_work,
                                SegWork_<Elem>& l_seg_work,
                                SegWork_<Elem>& r_seg_work, size_t rl_cnt,
                                size_t ins_cnt, size_t shove_cnt,
                                Provider&& provider) {
    (CheckCntrSegWorkMatched_)(self_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(self_work, r_seg_work);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!l_seg_work.is_null);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(!r_seg_work.is_null);

    size_t l_vac{ l_seg_work.elem_vac };
    size_t r_vac{ r_seg_work.elem_vac };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < shove_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rl_cnt <= r_seg_work.ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            r_seg_work.ca.elem_cnt + ins_cnt);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        rl_cnt + ins_cnt <= shove_cnt ||
        r_seg_work.ca.elem_cnt + ins_cnt - shove_cnt <= r_seg_work.ca.slot_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (l_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(self_work, l_seg_work);
    }
#endif

#if EnStaging
    if (r_seg_work.color == dat_color)
#endif
    {
        seg_utils::SegInsertShoveL(l_seg_work.ca, r_seg_work.ca, rl_cnt,
                                   ins_cnt, shove_cnt, provider);

        l_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

        r_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

#if EnStaging
    size_t cnt_a{ comparison_utils::BasicMin(rl_cnt, shove_cnt) };
    size_t cnt_b{ comparison_utils::BasicMin(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    seq_cntr_utils::provider::Provider<Origin const&> origin_provider{
        .cntr{ self_work.origin },
        .cursor{},
        .cnt{ seq_cntr::max_max_elem_cnt },
    };

    seq_cntr::Refer(self_work.origin, r_seg_work.ref_beg,
                    &origin_provider.cursor, nullptr);

    if (0 < cnt_a) {
        seq_cntr::PushR(l_seg_work.ca, cnt_a, origin_provider, nullptr,
                        nullptr);
    }

    if (0 < cnt_b) {
        seq_cntr::PushR(l_seg_work.ca, cnt_b, provider, nullptr, nullptr);
    }

    if (0 < cnt_c) {
        seq_cntr::PushR(l_seg_work.ca, cnt_c, origin_provider, nullptr,
                        nullptr);

        l_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

        r_seg_work.ref_beg += cnt_a + cnt_c;
        r_seg_work.ca.elem_cnt += ins_cnt - shove_cnt;
        r_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt -
            comparison_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                       self_work.seg_elem_slot_cnt);

        return;
    }

    Elem* data{ static_cast<Elem*>(
        (AllocateData_)(self_work.data_size, self_work.data_alctr)) };

    Elem* data_i{ data };

    size_t rr_cnt{ r_seg_work.ca.elem_cnt - rl_cnt };

    if (0 < rl_cnt - cnt_a) {
        seq_cntr::Read(self_work.origin, &origin_provider.cursor,
                       rl_cnt - cnt_a,
                       lin_seq_endpoint::acceptor::Acceptor<
                           lifecycle::ConstructingMemToObjView<Elem>>{
                           .data{ data_i },
                           .elem_stride{ sizeof(Elem) },
                           .elem_cnt{ seq_cntr::max_max_elem_cnt },
                       },
                       nullptr);

        data_i += rl_cnt - cnt_a;
    }

    if (0 < ins_cnt - cnt_b) {
        seq_endpoint::provider::Transfer(
            provider, lifecycle::ConstructingMemToObjView<Elem>{ data_i },
            sizeof(Elem), ins_cnt - cnt_b);

        data_i += ins_cnt - cnt_b;
    }

    if (0 < rr_cnt - cnt_c) {
        seq_cntr::Read(self_work.origin, &origin_provider.cursor,
                       rr_cnt - cnt_c,
                       lin_seq_endpoint::acceptor::Acceptor<
                           lifecycle::ConstructingMemToObjView<Elem>>{
                           .data{ data_i },
                           .elem_stride{ sizeof(Elem) },
                           .elem_cnt{ seq_cntr::max_max_elem_cnt },
                       },
                       nullptr);
    }

    l_seg_work.elem_vac = self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

    r_seg_work.color   = dat_color;
    r_seg_work.ca.data = data;
    r_seg_work.ca.elem_cnt += ins_cnt - shove_cnt;
    r_seg_work.ca.rot   = 0;
    r_seg_work.elem_vac = self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
#endif
}

template <CntrWorkTplParamList,
          seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void SegInsertShoveR_(CntrWork_<CntrWorkTplArgList> const& self_work,
                                SegWork_<Elem>& l_seg_work,
                                SegWork_<Elem>& r_seg_work, size_t lr_cnt,
                                size_t ins_cnt, size_t shove_cnt,
                                Provider&& provider) {
    (CheckCntrSegWorkMatched_)(self_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(self_work, r_seg_work);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!l_seg_work.is_null);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(!r_seg_work.is_null);

    size_t l_vac{ l_seg_work.elem_vac };
    size_t r_vac{ r_seg_work.elem_vac };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < shove_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(lr_cnt <= l_seg_work.ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ins_cnt <= l_vac + r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            l_seg_work.ca.elem_cnt + ins_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        l_seg_work.ca.elem_cnt + ins_cnt - shove_cnt <= l_seg_work.ca.slot_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (r_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(self_work, r_seg_work);
    }
#endif

#if EnStaging
    if (l_seg_work.color == dat_color)
#endif
    {
        seg_utils::SegInsertShoveR(l_seg_work.ca, r_seg_work.ca, lr_cnt,
                                   ins_cnt, shove_cnt, provider);

        l_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
        r_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

#if EnStaging
    size_t cnt_a{ comparison_utils::BasicMin(lr_cnt, shove_cnt) };
    size_t cnt_b{ comparison_utils::BasicMin(ins_cnt, shove_cnt - cnt_a) };
    size_t cnt_c{ shove_cnt - cnt_a - cnt_b };

    seq_cntr_utils::provider::Provider<Origin const&> origin_provider{
        .cntr{ self_work.origin },
        .cursor{},
        .cnt{ seq_cntr::max_max_elem_cnt },
    };

    if (0 < cnt_c) {
        seq_cntr::Refer(
            self_work.origin,
            l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - cnt_a - cnt_c,
            &origin_provider.cursor, nullptr);

        seq_cntr::PushL(r_seg_work.ca, cnt_c, origin_provider, nullptr,
                        nullptr);
    }

    if (0 < cnt_b) {
        seq_cntr::PushL(r_seg_work.ca, cnt_b, provider, nullptr, nullptr);
    }

    if (0 < cnt_a) {
        seq_cntr::Refer(self_work.origin,
                        l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - cnt_a,
                        &origin_provider.cursor, nullptr);

        seq_cntr::PushL(r_seg_work.ca, cnt_a, origin_provider, nullptr,
                        nullptr);
    }

    if (0 < cnt_c) {
        l_seg_work.ca.elem_cnt += ins_cnt - shove_cnt;
        l_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt -
            comparison_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                       self_work.seg_elem_slot_cnt);

        r_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

    Elem* data{ static_cast<Elem*>(
        (AllocateData_)(self_work.data_size, self_work.data_alctr)) };

    Elem* data_i{ data };

    size_t ll_cnt{ l_seg_work.ca.elem_cnt - lr_cnt };

    if (0 < ll_cnt - cnt_c) {
        seq_cntr::Refer(self_work.origin, l_seg_work.ref_beg,
                        &origin_provider.cursor, nullptr);

        seq_cntr::Read(self_work.origin, &origin_provider.cursor,
                       ll_cnt - cnt_c,
                       lin_seq_endpoint::acceptor::Acceptor<
                           lifecycle::ConstructingMemToObjView<Elem>>{
                           .data{ data_i },
                           .elem_stride{ sizeof(Elem) },
                           .elem_cnt{ seq_cntr::max_max_elem_cnt },
                       },
                       nullptr);

        data_i += ll_cnt - cnt_c;
    }

    if (0 < ins_cnt - cnt_b) {
        seq_endpoint::provider::Transfer(
            provider, lifecycle::ConstructingMemToObjView<Elem>{ data_i },
            sizeof(Elem), ins_cnt - cnt_b);

        data_i += ins_cnt - cnt_b;
    }

    if (0 < lr_cnt - cnt_a) {
        seq_cntr::Refer(self_work.origin, l_seg_work.ref_beg + ll_cnt,
                        &origin_provider.cursor, nullptr);

        seq_cntr::Read(self_work.origin, &origin_provider.cursor,
                       lr_cnt - cnt_a,
                       lin_seq_endpoint::acceptor::Acceptor<
                           lifecycle::ConstructingMemToObjView<Elem>>{
                           .data{ data_i },
                           .elem_stride{ sizeof(Elem) },
                           .elem_cnt{ seq_cntr::max_max_elem_cnt },
                       },
                       nullptr);
    }

    l_seg_work.color   = dat_color;
    l_seg_work.ca.data = data;
    l_seg_work.ca.elem_cnt += ins_cnt - shove_cnt;
    l_seg_work.ca.rot   = 0;
    l_seg_work.elem_vac = self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

    r_seg_work.elem_vac = self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
#endif
}

template <CntrWorkTplParamList,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void SegEraseShoveL_(CntrWork_<CntrWorkTplArgList> const& self_work,
                               SegWork_<Elem>& l_seg_work,
                               SegWork_<Elem>& r_seg_work, size_t rl_cnt,
                               size_t ers_cnt, size_t shove_cnt,
                               Acceptor&& acceptor) {
    (CheckCntrSegWorkMatched_)(self_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(self_work, r_seg_work);

    size_t l_vac{ self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < shove_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rl_cnt <= r_seg_work.ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ers_cnt <=
                                            r_seg_work.ca.elem_cnt - rl_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= l_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            r_seg_work.ca.elem_cnt - ers_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (l_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(self_work, l_seg_work);
    }
#endif

#if EnStaging
    if (r_seg_work.color == dat_color)
#endif
    {
        seg_utils::SegEraseShoveL(l_seg_work.ca, r_seg_work.ca, rl_cnt, ers_cnt,
                                  shove_cnt, acceptor);

        l_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
        r_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

#if EnStaging
    size_t cnt_a{ comparison_utils::BasicMin(rl_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    seq_cntr_utils::provider::Provider<Origin const&> origin_provider{
        .cntr{ self_work.origin },
        .cursor{},
        .cnt{ seq_cntr::max_max_elem_cnt },
    };

    if (0 < cnt_a) {
        seq_cntr::Refer(self_work.origin, r_seg_work.ref_beg,
                        &origin_provider.cursor, nullptr);

        seq_cntr::PushR(l_seg_work.ca, cnt_a, origin_provider, nullptr,
                        nullptr);
    }

    if (0 < cnt_c) {
        seq_cntr::Refer(self_work.origin, r_seg_work.ref_beg + cnt_a + cnt_b,
                        &origin_provider.cursor, nullptr);

        seq_cntr::PushR(l_seg_work.ca, cnt_c, origin_provider, nullptr,
                        nullptr);
    }

    l_seg_work.elem_vac = self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;

    r_seg_work.ref_beg += cnt_a + cnt_b + cnt_c;
    r_seg_work.ca.elem_cnt -= cnt_a + cnt_b + cnt_c;
    r_seg_work.elem_vac =
        self_work.seg_elem_slot_cnt -
        comparison_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                   self_work.seg_elem_slot_cnt);
#endif
}

template <CntrWorkTplParamList,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void SegEraseShoveR_(CntrWork_<CntrWorkTplArgList> const& self_work,
                               SegWork_<Elem>& l_seg_work,
                               SegWork_<Elem>& r_seg_work, size_t lr_cnt,
                               size_t ers_cnt, size_t shove_cnt,
                               Acceptor&& acceptor) {
    (CheckCntrSegWorkMatched_)(self_work, l_seg_work);
    (CheckCntrSegWorkMatched_)(self_work, r_seg_work);

    size_t r_vac{ self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < shove_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(lr_cnt <= l_seg_work.ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(ers_cnt <=
                                            l_seg_work.ca.elem_cnt - lr_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <= r_vac);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(shove_cnt <=
                                            l_seg_work.ca.elem_cnt - ers_cnt);

    l_seg_work.is_dirty = true;
    r_seg_work.is_dirty = true;

#if EnStaging
    if (r_seg_work.color == ref_color) {
        (MaterializeRefSeg_)(self_work, r_seg_work);
    }
#endif

#if EnStaging
    if (l_seg_work.color == dat_color)
#endif
    {
        seg_utils::SegEraseShoveR(l_seg_work.ca, r_seg_work.ca, lr_cnt, ers_cnt,
                                  shove_cnt, acceptor);

        l_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - l_seg_work.ca.elem_cnt;
        r_seg_work.elem_vac =
            self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;

        return;
    }

#if EnStaging
    size_t cnt_a{ comparison_utils::BasicMin(lr_cnt, shove_cnt) };
    size_t cnt_b{ ers_cnt };
    size_t cnt_c{ shove_cnt - cnt_a };

    seq_cntr_utils::provider::Provider<Origin const&> origin_provider{
        .cntr{ self_work.origin },
        .cursor{},
        .cnt{ seq_cntr::max_max_elem_cnt },
    };

    if (0 < cnt_a) {
        seq_cntr::Refer(self_work.origin,
                        l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - cnt_a,
                        &origin_provider.cursor, nullptr);

        seq_cntr::PushL(r_seg_work.ca, cnt_a, origin_provider, nullptr,
                        nullptr);
    }

    {
        seq_cntr::Refer(
            self_work.origin,
            l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - lr_cnt - ers_cnt,
            &origin_provider.cursor, nullptr);

        seq_cntr::Read(self_work.origin, &origin_provider.cursor, ers_cnt,
                       acceptor, &origin_provider.cursor);
    }

    if (0 < cnt_c) {
        seq_cntr::Refer(
            self_work.origin,
            l_seg_work.ref_beg + l_seg_work.ca.elem_cnt - cnt_a - cnt_b - cnt_c,
            &origin_provider.cursor, nullptr);

        seq_cntr::PushL(r_seg_work.ca, cnt_c, origin_provider, nullptr,
                        nullptr);
    }

    l_seg_work.ca.elem_cnt -= cnt_a + cnt_b + cnt_c;
    l_seg_work.elem_vac =
        self_work.seg_elem_slot_cnt -
        comparison_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                   self_work.seg_elem_slot_cnt);

    r_seg_work.elem_vac = self_work.seg_elem_slot_cnt - r_seg_work.ca.elem_cnt;
#endif
}

template <CntrWorkTplParamList>
constexpr utils::Choose2Result Merge2_(
    CntrWork_<CntrWorkTplArgList> const& self_work, SegWork_<Elem>& l_seg_work,
    SegWork_<Elem>& r_seg_work) {
#if EnStaging
    using OriginCursor = typename CntrWork_<CntrWorkTplArgList>::OriginCursor;
#endif

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!l_seg_work.is_null);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(!r_seg_work.is_null);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < l_seg_work.ca.elem_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < r_seg_work.ca.elem_cnt);

    unsigned long long random_seed{ utils::GetRandom() };

#if EnStaging
    size_t total_elem_cnt{ l_seg_work.ca.elem_cnt + r_seg_work.ca.elem_cnt };

    if (l_seg_work.color == ref_color && r_seg_work.color == ref_color) {
        OriginCursor origin_cursor;

        Elem* data{ static_cast<Elem*>(
            (AllocateData_)(self_work.data_size, self_work.data_alctr)) };

        lin_seq_endpoint::acceptor::Acceptor<
            lifecycle::ConstructingMemToObjView<Elem>>
            data_acceptor{
                .data{ data },
                .elem_stride{ sizeof(Elem) },
                .elem_cnt{ seq_cntr::max_max_elem_cnt },
            };

        seq_cntr::Refer(self_work.origin, l_seg_work.ref_beg, &origin_cursor,
                        nullptr);

        seq_cntr::Read(self_work.origin, &origin_cursor, l_seg_work.ca.elem_cnt,
                       data_acceptor, nullptr);

        seq_cntr::Refer(self_work.origin, r_seg_work.ref_beg, &origin_cursor,
                        nullptr);

        seq_cntr::Read(self_work.origin, &origin_cursor, r_seg_work.ca.elem_cnt,
                       data_acceptor, nullptr);

        SegWork_<Elem>* dst_seg_work;
        SegWork_<Elem>* other_seg_work;

        utils::Choose2Result side{ utils::Choose2(&random_seed) };

        switch (side) {
        case utils::Choose2Result::Case0:
            dst_seg_work   = &l_seg_work;
            other_seg_work = &r_seg_work;
            break;

        case utils::Choose2Result::Case1:
            dst_seg_work   = &r_seg_work;
            other_seg_work = &l_seg_work;
            break;

        default: ZETA_Core_DebugUtils_Diag_Unreachable();
        }

        dst_seg_work->color       = dat_color;
        dst_seg_work->ca.data     = data;
        dst_seg_work->ca.elem_cnt = total_elem_cnt;
        dst_seg_work->elem_vac = self_work.seg_elem_slot_cnt - total_elem_cnt;

        other_seg_work->ca.elem_cnt = 0;
        other_seg_work->elem_vac    = self_work.seg_elem_slot_cnt;

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

    utils::Choose2Result side{ utils::Choose2(
        l_based_merge_cost <= r_based_merge_cost,
        r_based_merge_cost <= l_based_merge_cost, &random_seed) };

    switch (side) {
    case utils::Choose2Result::Case0:
        (SegShoveL_)(self_work, l_seg_work, r_seg_work, r_seg_work.ca.elem_cnt);
        break;

    case utils::Choose2Result::Case1:
        (SegShoveR_)(self_work, l_seg_work, r_seg_work, l_seg_work.ca.elem_cnt);
        break;

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

    return side;
}

struct ElemCntBalancer_ {
    size_t total_seg_cnt;
    size_t res_seg_cnt;
    size_t base_seg_elem_cnt;
    size_t res;
    size_t err;

    constexpr ElemCntBalancer_(size_t elem_cnt, size_t seg_elem_slot_cnt) {
        this->total_seg_cnt = this->res_seg_cnt =
            integral_math::CeilDiv(elem_cnt, seg_elem_slot_cnt);

        this->base_seg_elem_cnt = elem_cnt / this->total_seg_cnt;

        this->res = elem_cnt % this->total_seg_cnt;

        this->err = 0;
    }

    constexpr size_t Fetch() {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < this->res_seg_cnt);

        --this->res_seg_cnt;
        this->err += this->res;

        if (this->err < this->total_seg_cnt) { return this->base_seg_elem_cnt; }

        this->err -= this->total_seg_cnt;

        return this->base_seg_elem_cnt + 1;
    }
};

template <CntrTplParamList>
constexpr void CheckCntr_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& self) {
    size_t seg_elem_slot_cnt{ self.seg_elem_slot_cnt };
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_elem_slot_cnt <=
                                            max_seg_elem_slot_cnt);
}

template <CntrTplParamList>
constexpr void CheckCursor_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& self, Cursor const* cursor) {
#if EnStaging
    using Elem = typename Cntr<CntrTplArgList>::Elem;
#endif

    (CheckCntr_)(self);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor != nullptr);

    Cursor re_cursor;

    resolving::Packet<Elem, Elem> re_resolving_packet{
        resolving::Packet<Elem, Elem>::MakeRef()
    };

    self.Refer(seq_cntr::Tag{}, cursor->idx, &re_cursor, &re_resolving_packet);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor->idx == re_cursor.idx);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor->n == re_cursor.n);

#if ZETA_Core_DebugEnable
    ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor->stamp == re_cursor.stamp);
#endif

    re_resolving_packet.AssertResultIsResolved();

    if (cursor->seg_idx != re_cursor.seg_idx ||
        re_resolving_packet.dst_ref.ptr != re_cursor.elem_ptr) {
        ZETA_Core_DebugUtils_Logging_ImmLogVar(self.lb);
        ZETA_Core_DebugUtils_Logging_ImmLogVar(self.rb);
        ZETA_Core_DebugUtils_Logging_ImmLogVar(cursor->n);
        ZETA_Core_DebugUtils_Logging_ImmLogVar(cursor->seg_idx);
        ZETA_Core_DebugUtils_Logging_ImmLogVar(re_cursor.seg_idx);
        ZETA_Core_DebugUtils_Logging_ImmLogVar(re_resolving_packet.dst_ref.ptr);
        ZETA_Core_DebugUtils_Logging_ImmLogVar(re_cursor.elem_ptr);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor->seg_idx ==
                                                re_cursor.seg_idx);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            re_resolving_packet.dst_ref.ptr == re_cursor.elem_ptr);
    }

    if (re_cursor.n == self.lb || re_cursor.n == self.rb) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            static_cast<bool>(!re_resolving_packet.state.has_resolved_ref));
    } else {
#if EnStaging
        if ((GetNColor_)(static_cast<Node*>(cursor->n)) == ref_color) {
            if (re_resolving_packet.state.has_resolved_ref) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    re_resolving_packet.dst_ref.semantics !=
                    lifecycle::StorageSemantics::ConstLeftObj);
            }

            switch (re_resolving_packet.dst_ref.semantics) {
            case lifecycle::StorageSemantics::LeftObj:
            case lifecycle::StorageSemantics::ConstLeftObj:
            case lifecycle::StorageSemantics::RightObj:
            case lifecycle::StorageSemantics::ConstRightObj: break;

            case lifecycle::StorageSemantics::Null:
            case lifecycle::StorageSemantics::ConstructingMemToObj:
            case lifecycle::StorageSemantics::DestructingObjToMem:
            default: ZETA_Core_DebugUtils_Diag_Unreachable();
            }
        } else
#endif
        {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(
                static_cast<bool>(re_resolving_packet.state.has_resolved_ref));
        }
    }

    if (cursor->elem_ptr_is_resolved) {
        if (cursor->elem_ptr != re_resolving_packet.dst_ref.ptr) {
            ZETA_Core_DebugUtils_Logging_ImmLogVar(self.lb);
            ZETA_Core_DebugUtils_Logging_ImmLogVar(self.rb);

            ZETA_Core_DebugUtils_Logging_ImmLogVar(cursor->idx);
            ZETA_Core_DebugUtils_Logging_ImmLogVar(re_cursor.idx);

            ZETA_Core_DebugUtils_Logging_ImmLogVar(cursor->n);
            ZETA_Core_DebugUtils_Logging_ImmLogVar(re_cursor.n);

            ZETA_Core_DebugUtils_Logging_ImmLogVar(cursor->seg_idx);
            ZETA_Core_DebugUtils_Logging_ImmLogVar(re_cursor.seg_idx);

            ZETA_Core_DebugUtils_Logging_ImmLogVar(cursor->elem_ptr);
            ZETA_Core_DebugUtils_Logging_ImmLogVar(re_cursor.elem_ptr);

            ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor->elem_ptr ==
                                                    re_cursor.elem_ptr);
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(cursor->elem_ptr_is_resolved ==
                                                re_cursor.elem_ptr_is_resolved);
    }
}

enum struct ReadWriteOp_ : unsigned char {
    Write,
    ReadWrite,
};

template <lifecycle::StorageSemantics cntr_semantics, CntrTplParamList,
          ReadWriteOp_ op,
          typename Endpoint>
constexpr void ReadWrite_  // NOLINT(misc-use-internal-linkage)
    (lifecycle::StorageView<cntr_semantics, Cntr<CntrTplArgList>> self_view,
     meta::ValueWrapper<ReadWriteOp_, op>, Cursor* pos_cursor, size_t cnt,
     Endpoint&& endpoint, Cursor* dst_cursor)
    requires requires {
        requires(cntr_semantics == lifecycle::StorageSemantics::LeftObj ||
                 cntr_semantics == lifecycle::StorageSemantics::ConstLeftObj ||
                 cntr_semantics == lifecycle::StorageSemantics::RightObj ||
                 cntr_semantics == lifecycle::StorageSemantics::ConstRightObj);

        requires(op == ReadWriteOp_::Write || op == ReadWriteOp_::ReadWrite);

        requires meta::Implies(
            op == ReadWriteOp_::Write,
            seq_endpoint::provider::IsProviderLike<
                Endpoint, typename Cntr<CntrTplArgList>::Elem>);

        requires meta::Implies(
            op == ReadWriteOp_::ReadWrite,
            seq_endpoint::acceptor::IsAcceptorLike<
                Endpoint, typename Cntr<CntrTplArgList>::Elem>);
    }
{
#if EnStaging
    using Elem         = typename Cntr<CntrTplArgList>::Elem;
    using OriginCursor = typename Cntr<CntrTplArgList>::OriginCursor;
#endif

    using OpCircularArrayRef = meta::GetTypeWrapperType<decltype([]() {
        if constexpr (cntr_semantics == lifecycle::StorageSemantics::LeftObj) {
            return meta::TypeWrapper<circular_array::Cntr<Elem>&>{};
        } else if constexpr (cntr_semantics ==
                             lifecycle::StorageSemantics::ConstLeftObj) {
            return meta::TypeWrapper<circular_array::Cntr<Elem> const&>{};
        } else if constexpr (cntr_semantics ==
                             lifecycle::StorageSemantics::RightObj) {
            return meta::TypeWrapper<circular_array::Cntr<Elem>&&>{};
        } else if constexpr (cntr_semantics ==
                             lifecycle::StorageSemantics::ConstRightObj) {
            return meta::TypeWrapper<circular_array::Cntr<Elem> const&&>{};
        } else {
            static_assert(false);
        }
    }())>;

    static_assert(op == ReadWriteOp_::Write || op == ReadWriteOp_::ReadWrite);

    auto&& self{ self_view.GetRef() };

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

#if ZETA_Core_DebugEnable
    pos_cursor->stamp = self.stamp;
#endif

    if (cnt == 0) {
        if (dst_cursor != nullptr) { *dst_cursor = *pos_cursor; }
        return;
    }

    auto const self_work{ (MakeCntrWork_)(self) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(seq_cntr::op_check::CanDerefer(
        pos_cursor->idx, cnt, self_work.elem_cnt));

    size_t seg_elem_slot_cnt{ self_work.seg_elem_slot_cnt };

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
    OriginCursor origin_cursor;

    bool any_written{ false };

    unsigned long long random_seed{ utils::GetRandom() };
#endif

    CircularArray<Elem> ca{
        .data{},
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{ seg_elem_slot_cnt },
        .slot_cnt{ seg_elem_slot_cnt },
        .rot{},
#if ZETA_Core_DebugEnable
        .stamp{ debug_utils::revision::GetStamp() },
#endif
    };

    for (;;) {
        seg = (NToSeg_)(n);

#if EnStaging
        if ((GetNColor_)(n) == dat_color)
#endif
        {
            size_t cur_cnt{ comparison_utils::BasicMin(
                cnt, seg->dat.elem_cnt - seg_idx) };

            ca.data = static_cast<Elem*>(seg->dat.data);
            ca.rot  = seg->dat.rot;

            if constexpr (op == ReadWriteOp_::Write) {
                static_cast<OpCircularArrayRef>(ca).IdxWrite(seg_idx, cur_cnt,
                                                             endpoint);
            } else if constexpr (op == ReadWriteOp_::ReadWrite) {
                static_cast<OpCircularArrayRef>(ca).IdxReadWrite(
                    seg_idx, cur_cnt, endpoint);
            } else {
                static_assert(false);
            }

#if EnStaging
            if (!any_written) {
                any_written                      = true;
                pos_cursor->n                    = n;
                pos_cursor->seg_idx              = seg_idx;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = circular_array::ReferElem(
                    static_cast<Elem*>(seg->dat.data), sizeof(Elem),
                    self_work.seg_elem_slot_cnt, seg->dat.rot, seg_idx);
            }
#endif

            seg_idx += cur_cnt;

            if (seg_idx == seg->dat.elem_cnt) {
#if EnStaging
                l_n = n;
#endif

                n   = r_n == nullptr ? bin_tree::StepR(n) : r_n;
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
        size_t cnt_b{ comparison_utils::BasicMin(cnt, seg_elem_cnt - seg_idx) };
        size_t cnt_c{ seg_elem_cnt - cnt_a - cnt_b };

        size_t needed_seg_cnt{ integral_math::CeilDiv(
            cnt_b, self_work.seg_elem_slot_cnt) };

        if (integral_math::CeilDiv(seg_elem_cnt, self_work.seg_elem_slot_cnt) ==
            needed_seg_cnt) {
            goto WRITE;
        }

        {
            bool mat_ab_is_ok{ integral_math::CeilDiv(
                                   cnt_a + cnt_b,
                                   self_work.seg_elem_slot_cnt) ==
                               needed_seg_cnt };

            bool mat_bc_is_ok{ integral_math::CeilDiv(
                                   cnt_c + cnt_b,
                                   self_work.seg_elem_slot_cnt) ==
                               needed_seg_cnt };

            switch (static_cast<unsigned>(mat_ab_is_ok) * 0b10 +
                    static_cast<unsigned>(mat_bc_is_ok) * 0b01) {
            case 0b00: goto MAT_B;
            case 0b01: goto MAT_BC;
            case 0b10: goto MAT_AB;
            case 0b11:
                switch (utils::Choose2(cnt_a <= cnt_c, cnt_c <= cnt_a,
                                       &random_seed)) {
                case utils::Choose2Result::Case0: goto MAT_AB;
                case utils::Choose2Result::Case1: goto MAT_BC;
                default: ZETA_Core_DebugUtils_Diag_Unreachable();
                }
            default: ZETA_Core_DebugUtils_Diag_Unreachable();
            }
        }

    MAT_AB:
        {
            Seg* new_seg{ (CreateRefSeg_)(self_work.seg_alctr) };

            self.root = rbtree::InsertR(n, &new_seg->n);

            new_seg->ref.beg      = seg->ref.beg + cnt_a + cnt_b;
            new_seg->ref.elem_cnt = cnt_c;

            seg->ref.elem_cnt = cnt_a + cnt_b;

            bin_tree::AddDiffSize(&new_seg->n, new_seg->ref.elem_cnt);

            r_n = &new_seg->n;

            goto WRITE;
        }

    MAT_BC:
        {
            Seg* new_seg{ (CreateRefSeg_)(self_work.seg_alctr) };

            self.root = rbtree::InsertL(n, &new_seg->n);

            new_seg->ref.beg      = seg->ref.beg;
            new_seg->ref.elem_cnt = cnt_a;

            seg->ref.beg += cnt_a;
            seg->ref.elem_cnt = cnt_b + cnt_c;

            bin_tree::AddDiffSize(&new_seg->n, new_seg->ref.elem_cnt);

            l_n = &new_seg->n;

            seg_idx = 0;

            goto WRITE;
        }

    MAT_B:
        {
            Seg* new_l_seg{ (CreateRefSeg_)(self_work.seg_alctr) };
            Seg* new_r_seg{ (CreateRefSeg_)(self_work.seg_alctr) };

            self.root = rbtree::InsertL(n, &new_l_seg->n);
            self.root = rbtree::InsertR(n, &new_r_seg->n);

            new_l_seg->ref.beg      = seg->ref.beg;
            new_l_seg->ref.elem_cnt = cnt_a;

            new_r_seg->ref.beg      = seg->ref.beg + cnt_a + cnt_b;
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

    WRITE:
        {
            ElemCntBalancer_ balancer{ seg->ref.elem_cnt,
                                       self_work.seg_elem_slot_cnt };

            for (;;) {
                size_t new_seg_elem_cnt{ balancer.Fetch() };

                size_t cur_ref_beg{ seg->ref.beg };

                if (balancer.res_seg_cnt == 0) {
                    l_n = n;
                    n   = r_n == nullptr ? bin_tree::StepR(n) : r_n;
                    r_n = nullptr;

                    (SetNColor_)(l_n, dat_color);

                    Seg* l_seg{ (NToSeg_)(l_n) };

                    l_seg->dat.data = (AllocateData_)(self_work.data_size,
                                                      self_work.data_alctr);

                    l_seg->dat.rot = 0;

                    bin_tree::AddDiffSize(l_n, new_seg_elem_cnt - seg_elem_cnt);
                } else {
                    Seg* new_seg{ (CreateDatSeg_)(self_work.data_size,
                                                  self_work.seg_alctr,
                                                  self_work.data_alctr) };

                    self.root = l_n == nullptr
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

                size_t cur_cnt{ comparison_utils::BasicMin(
                    cnt, new_seg_elem_cnt - seg_idx) };

                Elem* data{ static_cast<Elem*>(l_seg->dat.data) };

                if constexpr (op == ReadWriteOp_::Write) {
                    if (0 < seg_idx) {
                        seq_cntr::Refer(self_work.origin, cur_ref_beg,
                                        &origin_cursor, nullptr);

                        seq_cntr::Read(
                            self_work.origin, &origin_cursor, seg_idx,
                            lin_seq_endpoint::acceptor::Acceptor<
                                lifecycle::ConstructingMemToObjView<Elem>>{
                                .data{ data },
                                .elem_stride{ sizeof(Elem) },
                                .elem_cnt{ seq_cntr::max_max_elem_cnt },
                            },
                            nullptr);
                    }

                    if (seg_idx + cur_cnt < new_seg_elem_cnt) {
                        seq_cntr::Refer(self_work.origin,
                                        cur_ref_beg + seg_idx + cur_cnt,
                                        &origin_cursor, nullptr);

                        seq_cntr::Read(
                            self_work.origin, &origin_cursor,
                            new_seg_elem_cnt - seg_idx - cur_cnt,
                            lin_seq_endpoint::acceptor::Acceptor<
                                lifecycle::ConstructingMemToObjView<Elem>>{
                                .data{ data + (seg_idx + cur_cnt) },
                                .elem_stride{ sizeof(Elem) },
                                .elem_cnt{ seq_cntr::max_max_elem_cnt },
                            },
                            nullptr);
                    }

                    seq_endpoint::provider::Transfer(
                        endpoint,
                        lifecycle::ConstructingMemToObjView<Elem>{ data +
                                                                   seg_idx },
                        sizeof(Elem), cur_cnt);
                } else if constexpr (op == ReadWriteOp_::ReadWrite) {
                    seq_cntr::Refer(self_work.origin, cur_ref_beg,
                                    &origin_cursor, nullptr);

                    seq_cntr::Read(
                        self_work.origin, &origin_cursor, new_seg_elem_cnt,
                        lin_seq_endpoint::acceptor::Acceptor<
                            lifecycle::ConstructingMemToObjView<Elem>>{
                            .data{ data },
                            .elem_stride{ sizeof(Elem) },
                            .elem_cnt{ seq_cntr::max_max_elem_cnt },
                        },
                        nullptr);

                    seq_endpoint::acceptor::Transfer(
                        endpoint,
                        lifecycle::StorageView<cntr_semantics, Elem>{ data +
                                                                      seg_idx },
                        sizeof(Elem), cur_cnt);
                } else {
                    static_assert(false);
                }

                if (!any_written) {
                    any_written                      = true;
                    pos_cursor->n                    = l_n;
                    pos_cursor->seg_idx              = seg_idx;
                    pos_cursor->elem_ptr_is_resolved = true;
                    pos_cursor->elem_ptr             = data + seg_idx;
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

    dst_cursor->idx                  = end_idx;
    dst_cursor->n                    = n;
    dst_cursor->seg_idx              = seg_idx;
    dst_cursor->elem_ptr_is_resolved = false;
    dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <CntrTplParamList>
constexpr void InitTree_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& self) {
    self.lb->Reset(1);
    self.rb->Reset(1);

    Node* root{ nullptr };

    root = rbtree::InsertR(root, self.lb);
    root = rbtree::InsertR(root, self.rb);

    self.root = root;
}

#if EnStaging

template <CntrTplParamList>
constexpr void RefOrigin_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& self) {
    auto& origin{ meta::GetInstRefFromLike(self.origin_like) };

    size_t origin_elem_cnt{ seq_cntr::GetElemCnt(origin) };

    if (origin_elem_cnt == 0) { return; }

    Seg* seg{ (CreateRefSeg_)(meta::GetInstRefFromLike(self.seg_alctr_like)) };

    bin_tree::SetAccSize(&seg->n, origin_elem_cnt);
    seg->ref.beg      = 0;
    seg->ref.elem_cnt = origin_elem_cnt;

    self.root = rbtree::Insert(self.lb, self.rb, &seg->n);
}

#endif

template <CntrTplParamList, seq_endpoint::acceptor::IsAcceptor<
                                typename Namespace::Cntr<CntrTplArgList>::Elem>
                                Acceptor>
constexpr void EraseAll_(Cntr<CntrTplArgList>& self, Acceptor&& acceptor) {
#if EnStaging
    using Elem         = typename Cntr<CntrTplArgList>::Elem;
    using OriginCursor = typename Cntr<CntrTplArgList>::OriginCursor;
#endif

    size_t seg_elem_slot_cnt{ self.seg_elem_slot_cnt };

    Node* lb{ self.lb };
    Node* rb{ self.rb };

#if EnStaging
    auto const& origin{ meta::GetInstRefFromLike(self.origin_like) };
#endif

    auto& seg_alctr{ meta::GetInstRefFromLike(self.seg_alctr_like) };
    auto& data_alctr{ meta::GetInstRefFromLike(self.data_alctr_like) };

    constexpr size_t buffer_capacity{ rbtree::max_height * 4 };

    Node* buffer[buffer_capacity];
    size_t buffer_i{ 0 };

#if EnStaging
    OriginCursor origin_cursor;
#endif

    CircularArray<Elem> ca{
        .data{},
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{},
        .slot_cnt{ seg_elem_slot_cnt },
        .rot{},

#if ZETA_Core_DebugEnable
        .stamp{ debug_utils::revision::GetStamp() },
#endif
    };

    buffer[buffer_i++] = self.root;

    while (0 < buffer_i) {
        Node* n{ buffer[--buffer_i] };

        Node* nl{ bin_tree::GetL(n) };
        Node* nr{ bin_tree::GetR(n) };

        if (nl != nullptr) {
            bin_tree::SetL(n, static_cast<Node*>(nullptr));

            buffer[buffer_i++] = n;
            buffer[buffer_i++] = nl;

            continue;
        }

        if (nr != nullptr) { buffer[buffer_i++] = nr; }

        if (lb == n || rb == n) { continue; }

        Seg* seg{ (NToSeg_)(n) };

#if EnStaging
        if ((GetNColor_)(n) == ref_color) {
            seq_cntr::Refer(origin, seg->ref.beg, &origin_cursor, nullptr);

            seq_cntr::Read(origin, &origin_cursor, seg->ref.elem_cnt, acceptor,
                           nullptr);
        } else
#endif
        {

            ca.data     = static_cast<Elem*>(seg->dat.data);
            ca.elem_cnt = seg->dat.elem_cnt;
            ca.rot      = seg->dat.rot;

            seq_cntr::EraseAll(ca, acceptor);
        }

        detail::DestroySeg_(seg, seg_alctr, data_alctr);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(seg);
    }
}

template <
#if EnStaging
    typename SrcOriginLike,
#else
    meta::IsObjectType Elem,
#endif
    typename SrcSegAllocatorLike, typename SrcDataAllocatorLike,
    typename SegAllocator, typename DataAllocator>
constexpr Node* Copy_(
    size_t seg_elem_slot_cnt, Node* lb, Node* rb,
    Cntr<
#if EnStaging
        SrcOriginLike,
#else
        Elem,
#endif
        SrcSegAllocatorLike, SrcDataAllocatorLike> const& src_cntr,
    SegAllocator& seg_alctr, DataAllocator& data_alctr) {
#if EnStaging
    using Elem = decltype(src_cntr)::Elem;
#endif

    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_elem_slot_cnt <=
                                            max_seg_elem_slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(lb != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(rb != nullptr);

    size_t data_size{ sizeof(Elem) * seg_elem_slot_cnt };

    lb->Reset(1);
    rb->Reset(1);

    Node* root{ nullptr };

    root = rbtree::InsertR(root, lb);
    root = rbtree::InsertR(root, rb);

    Node* last_n{ lb };

    Node* src_n{ bin_tree::StepR(src_cntr.lb) };

    SegWork_<Elem> src_seg_work;
    src_seg_work.SetBasics((MakeCntrWork_)(src_cntr));

    while (src_n != src_cntr.rb) {
#if EnStaging
        if ((GetNColor_)(src_n) == ref_color) {
            Seg* src_seg{ (NToSeg_)(src_n) };

            Seg* dst_seg{ (CreateRefSeg_)(seg_alctr) };

            dst_seg->ref.beg      = src_seg->ref.beg;
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
            Seg* dst_seg{ (CreateDatSeg_)(data_size, seg_alctr, data_alctr) };
            root   = rbtree::Insert(last_n, rb, &dst_seg->n);
            last_n = &dst_seg->n;

            dst_data = dst_seg->dat.data;
            dst_idx  = 0;
        }

        size_t src_idx{ 0 };
        src_seg_work.Load((NToSeg_)(src_n));

        ElemCntBalancer_ balancer{ acc_elem_cnt, seg_elem_slot_cnt };

        size_t res_cnt{ balancer.Fetch() };

        for (;;) {
            size_t cur_cnt{ comparison_utils::BasicMin(
                res_cnt, src_seg_work.ca.elem_cnt - src_idx) };

            src_seg_work.ca.IdxRead(
                src_idx, cur_cnt,
                lin_seq_endpoint::acceptor::Acceptor<
                    lifecycle::ConstructingMemToObjView<Elem>>{
                    .data{ dst_data },
                    .elem_stride{ static_cast<ptrdiff_t>(sizeof(Elem)) },
                    .elem_cnt{ seq_cntr::max_max_elem_cnt },
                });

            dst_data = static_cast<char*>(dst_data) + sizeof(Elem) * cur_cnt;
            dst_idx += cur_cnt;

            src_idx += cur_cnt;

            res_cnt -= cur_cnt;

            if (res_cnt == 0) {
                (NToSeg_)(last_n)->dat.elem_cnt =
                    static_cast<unsigned short>(dst_idx);

                bin_tree::AddDiffSize(last_n, dst_idx);

                if (balancer.res_seg_cnt == 0) {
                    ZETA_Core_DebugUtils_Diag_PromiseAssert(
                        src_idx == src_seg_work.ca.elem_cnt);

                    src_n = bin_tree::StepR(src_n);

                    break;
                }

                Seg* dst_seg{ (CreateDatSeg_)(data_size, seg_alctr,
                                              data_alctr) };
                root   = rbtree::Insert(last_n, rb, &dst_seg->n);
                last_n = &dst_seg->n;

                dst_data = dst_seg->dat.data;
                dst_idx  = 0;

                res_cnt = balancer.Fetch();
            }

            if (src_idx == src_seg_work.ca.elem_cnt) {
                src_n   = bin_tree::StepR(src_n);
                src_idx = 0;
                src_seg_work.Load((NToSeg_)(src_n));
            }
        }
    }

    return root;
}

}  // namespace Namespace::detail

constexpr Namespace::Cursor::Cursor(seq_cntr::CursorLimit const& src_cursor) {
    *this = src_cursor;
}

constexpr Namespace::Cursor::operator seq_cntr::CursorLimit(
    this Cursor const& self) {
    seq_cntr::CursorLimit ret;

    utils::MemCopy(&ret, &self, sizeof(Cursor));

    return ret;
}

constexpr Namespace::Cursor& Namespace::Cursor::operator=(
    seq_cntr::CursorLimit const& src_cursor) {
    utils::MemCopy(this, &src_cursor, sizeof(Cursor));
    return *this;
}

template <CntrTplParamList>
template <
#if EnStaging
    typename OriginLikeConstructArg,
#endif
    typename SegAllocatorLikeConstructArg,
    typename DataAllocatorLikeConstructArg>
constexpr Namespace::Cntr<CntrTplArgList>::Cntr(
    size_t seg_elem_slot_cnt,
#if EnStaging
    OriginLikeConstructArg&& origin_like_construct_arg,
#endif
    SegAllocatorLikeConstructArg&& seg_alctr_like_construct_arg,
    DataAllocatorLikeConstructArg&& data_alctr_like_construct_arg)
    :
#if EnStaging
      origin_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          OriginLike, OriginLikeConstructArg, origin_like_construct_arg) },
#endif
      seg_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          SegAllocatorLike, SegAllocatorLikeConstructArg,
          seg_alctr_like_construct_arg) },
      data_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          DataAllocatorLike, DataAllocatorLikeConstructArg,
          data_alctr_like_construct_arg) } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_elem_slot_cnt <=
                                            max_seg_elem_slot_cnt);

    auto& seg_alctr{ meta::GetInstRefFromLike(this->seg_alctr_like) };

    this->seg_elem_slot_cnt = seg_elem_slot_cnt;

    this->lb =
        new (allocator::SafeAllocate(seg_alctr, alignof(Node), sizeof(Node)))
            Node{ 0 };

    this->rb =
        new (allocator::SafeAllocate(seg_alctr, alignof(Node), sizeof(Node)))
            Node{ 0 };

    detail::InitTree_(*this);

#if EnStaging
    detail::RefOrigin_(*this);
#endif
}

template <CntrTplParamList>
template <
#if EnStaging
    typename OriginLikeConstructArg,
#endif
    typename SegAllocatorLikeConstructArg,
    typename DataAllocatorLikeConstructArg, typename SrcOriginLike,
    typename SrcSegAllocatorLike, typename SrcDataAllocatorLike>
constexpr Namespace::Cntr<CntrTplArgList>::Cntr(
    size_t seg_elem_slot_cnt,
#if EnStaging
    OriginLikeConstructArg&& origin_like_construct_arg,
#endif
    SegAllocatorLikeConstructArg&& seg_alctr_like_construct_arg,
    DataAllocatorLikeConstructArg&& data_alctr_like_construct_arg,
    Cntr<
#if EnStaging
        SrcOriginLike,
#else
        Elem,
#endif
        SrcSegAllocatorLike, SrcDataAllocatorLike> const& src_cntr)
    :
#if EnStaging
      origin_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          OriginLike, OriginLikeConstructArg, origin_like_construct_arg) },
#endif
      seg_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          SegAllocatorLike, SegAllocatorLikeConstructArg,
          seg_alctr_like_construct_arg) },
      data_alctr_like{ ZETA_Core_Lifecycle_UnpackConstructArgs(
          DataAllocatorLike, DataAllocatorLikeConstructArg,
          data_alctr_like_construct_arg) } {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < seg_elem_slot_cnt);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(seg_elem_slot_cnt <=
                                            max_seg_elem_slot_cnt);

#if EnStaging
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::GetReferedInstPtr(
            meta::GetInstRefFromLike(this->origin_like)) ==
        seq_cntr::GetReferedInstPtr(
            meta::GetInstRefFromLike(src_cntr.origin_like)));
#endif

    auto& seg_alctr{ meta::GetInstRefFromLike(this->seg_alctr_like) };
    auto& data_alctr{ meta::GetInstRefFromLike(this->data_alctr_like) };

    this->seg_elem_slot_cnt = seg_elem_slot_cnt;

    this->lb =
        new (allocator::SafeAllocate(seg_alctr, alignof(Node), sizeof(Node)))
            Node{ 0 };

    this->rb =
        new (allocator::SafeAllocate(seg_alctr, alignof(Node), sizeof(Node)))
            Node{ 0 };

    this->root = detail::Copy_(seg_elem_slot_cnt, this->lb, this->rb, src_cntr,
                               seg_alctr, data_alctr);
}

template <CntrTplParamList>
constexpr Namespace::Cntr<CntrTplArgList>::~Cntr() {
    detail::CheckCntr_(*this);

    auto& seg_alctr{ meta::GetInstRefFromLike(this->seg_alctr_like) };

    detail::EraseAll_(*this, seq_endpoint::acceptor::BasicAcceptor{});

    lifecycle::InvokeDestructor(*this->lb);
    lifecycle::InvokeDestructor(*this->rb);

    allocator::Deallocate(seg_alctr, this->lb);
    allocator::Deallocate(seg_alctr, this->rb);
}

template <CntrTplParamList>
template <meta::IsAnySame<Namespace::Cntr<CntrTplArgList>&,
                          Namespace::Cntr<CntrTplArgList>&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
Namespace::Cntr<CntrTplArgList>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::FlagBuilder{
        .GetElemCnt{ true },
        .GetMaxElemCnt{ true },

        .GetLBCursor{ true },
        .GetRBCursor{ true },

        .PeekL{ true },
        .PeekR{ true },

        .Refer{ true },
        .Derefer{ true },

        .Read{ true },
        .Write{ true },
        .ReadWrite{ true },

        .PushL{ true },
        .PushR{ true },
        .Insert{ true },

        .PopL{ true },
        .PopR{ true },
        .Erase{ true },
        .EraseAll{ true },

        .CopyCursor{ true },

        .AreEqualCursor{ true },
        .CompareCursor{ true },
        .GetCursorDist{ true },
        .GetCursorIdx{ true },

        .CursorStepL{ true },
        .CursorStepR{ true },

        .CursorAdvanceL{ true },
        .CursorAdvanceR{ true },
    }();
}

template <CntrTplParamList>
template <meta::IsAnySame<Namespace::Cntr<CntrTplArgList> const&,
                          Namespace::Cntr<CntrTplArgList> const&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
Namespace::Cntr<CntrTplArgList>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return (GetStaticEnabledCapabilityFlag)(seq_cntr::Tag{},
                                            meta::TypeWrapper<Cntr&>{}) &
           seq_cntr::capability::const_cap_flag;
}

template <CntrTplParamList>
template <meta::IsAnySame<Namespace::Cntr<CntrTplArgList>&,
                          Namespace::Cntr<CntrTplArgList>&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
Namespace::Cntr<CntrTplArgList>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::empty_cap_flag;
}

template <CntrTplParamList>
template <meta::IsAnySame<Namespace::Cntr<CntrTplArgList> const&,
                          Namespace::Cntr<CntrTplArgList> const&&>
              TargetCntr>
constexpr seq_cntr::capability::Flag
Namespace::Cntr<CntrTplArgList>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<TargetCntr>) {
    return seq_cntr::capability::non_const_cap_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag
Namespace::Cntr<CntrTplArgList>::GetDynamicEnabledCapabilityFlag(
    seq_cntr::Tag) {
    return seq_cntr::capability::empty_cap_flag;
}

template <CntrTplParamList>
constexpr seq_cntr::capability::Flag
Namespace::Cntr<CntrTplArgList>::GetDynamicDisabledCapabilityFlag(
    seq_cntr::Tag) {
    return seq_cntr::capability::empty_cap_flag;
}

template <CntrTplParamList>
constexpr void* Namespace::Cntr<CntrTplArgList>::GetReferedInstPtr(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return const_cast<void*>(static_cast<void const*>(&self));
}

template <CntrTplParamList>
template <meta::IsAnySame<
    Namespace::Cntr<CntrTplArgList>&, Namespace::Cntr<CntrTplArgList> const&,
    Namespace::Cntr<CntrTplArgList>&&, Namespace::Cntr<CntrTplArgList> const&&>
              TargetCntr>
constexpr meta::TypeWrapper<Namespace::Cursor>
Namespace::Cntr<CntrTplArgList>::GetCursorType(seq_cntr::Tag,
                                               meta::TypeWrapper<TargetCntr>) {
    return {};
}

template <CntrTplParamList>
template <meta::IsAnySame<
    Namespace::Cntr<CntrTplArgList>&, Namespace::Cntr<CntrTplArgList> const&,
    Namespace::Cntr<CntrTplArgList>&&, Namespace::Cntr<CntrTplArgList> const&&>
              TargetCntr>
constexpr meta::TypeWrapper<typename Namespace::Cntr<CntrTplArgList>::Elem>
Namespace::Cntr<CntrTplArgList>::GetElemType(seq_cntr::Tag,
                                             meta::TypeWrapper<TargetCntr>) {
    return {};
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetElemSize(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return sizeof(Elem);
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetElemCnt(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return bin_tree::GetAccSize(self.root) - 2;
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetMaxElemCnt(
    this Cntr const& self, seq_cntr::Tag) {
    detail::CheckCntr_(self);

    return ZETA_Core_max_capacity;
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::GetLBCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->idx                  = static_cast<size_t>(-1);
    dst_cursor->n                    = self.lb;
    dst_cursor->seg_idx              = 0;
    dst_cursor->elem_ptr_is_resolved = true;
    dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::GetRBCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor* dst_cursor) {
    detail::CheckCntr_(self);

    if (dst_cursor == nullptr) { return; }

    dst_cursor->idx                  = self.GetElemCnt(seq_cntr::Tag{});
    dst_cursor->n                    = self.rb;
    dst_cursor->seg_idx              = 0;
    dst_cursor->elem_ptr_is_resolved = true;
    dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::PeekL(
    this auto&& self, seq_cntr::Tag, Cursor* dst_cursor,
    resolving::Packet<Elem, Elem>* resolving_packet) {
    detail::CheckCntr_(self);

    auto const self_work{ detail::MakeCntrWork_(
        meta::Forward<decltype(self)>(self)) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr ||
                                            resolving_packet != nullptr);

    if (resolving_packet != nullptr) {
        resolving_packet->AssertResultIsNotResolved();
    }

    Node* n{ bin_tree::StepR(self.lb) };

    if (self.rb == n) {
        if (dst_cursor != nullptr) {
            dst_cursor->idx                  = 0;
            dst_cursor->n                    = n;
            dst_cursor->seg_idx              = 0;
            dst_cursor->elem_ptr_is_resolved = true;
            dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
        }

        if (resolving_packet != nullptr) {
            resolving_packet->state.resolved = true;
        }

        return;
    }

    Seg* seg{ detail::NToSeg_(n) };

#if EnStaging
    if (detail::GetNColor_(n) == ref_color) {
        if (resolving_packet == nullptr ||
            resolving_packet->intent == resolving::Intent::Null) {
            if (dst_cursor != nullptr) {
                dst_cursor->idx                  = 0;
                dst_cursor->n                    = n;
                dst_cursor->seg_idx              = 0;
                dst_cursor->elem_ptr_is_resolved = false;
                dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
                dst_cursor->stamp = self.stamp;
#endif
            }

            if (resolving_packet != nullptr) {
                resolving_packet->state.resolved = true;
            }

            return;
        }

        seq_cntr::Refer(self_work.origin, seg->ref.beg, nullptr,
                        resolving_packet);

        if (dst_cursor != nullptr) {
            dst_cursor->idx                  = 0;
            dst_cursor->n                    = n;
            dst_cursor->seg_idx              = 0;
            dst_cursor->elem_ptr_is_resolved = true;
            dst_cursor->elem_ptr             = resolving_packet->dst_ref.ptr;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
        }

        return;
    }
#endif

    resolving::Packet<Elem, Elem> ref_resolving_packet{
        resolving::Packet<Elem, Elem>::MakeRef()
    };

    CircularArray<Elem> ca{
        .data{ static_cast<Elem*>(seg->dat.data) },
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{ seg->dat.elem_cnt },
        .slot_cnt{ self_work.seg_elem_slot_cnt },
        .rot{ seg->dat.rot },

#if ZETA_Core_DebugEnable
        .stamp{ debug_utils::revision::GetStamp() },
#endif
    };

    seq_cntr::PeekL(ca, nullptr, &ref_resolving_packet);

    if (dst_cursor != nullptr) {
        dst_cursor->idx                  = 0;
        dst_cursor->n                    = n;
        dst_cursor->seg_idx              = 0;
        dst_cursor->elem_ptr_is_resolved = true;
        dst_cursor->elem_ptr             = ref_resolving_packet.dst_ref.ptr;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    if (resolving_packet != nullptr &&
        resolving_packet->intent != resolving::Intent::Null) {
        resolving_packet->ResolveFromView(ref_resolving_packet.dst_ref,
                                          meta::AutoValueWrapper<true>{},
                                          meta::AutoValueWrapper<true>{});
    }
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::PeekR(
    this auto&& self, seq_cntr::Tag, Cursor* dst_cursor,
    resolving::Packet<Elem, Elem>* resolving_packet) {
    detail::CheckCntr_(self);

    auto const self_work{ detail::MakeCntrWork_(self) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr ||
                                            resolving_packet != nullptr);

    if (resolving_packet != nullptr) {
        resolving_packet->AssertResultIsNotResolved();
    }

    Node* n{ bin_tree::StepL(self.rb) };

    if (self.lb == n) {
        if (dst_cursor != nullptr) {
            dst_cursor->idx                  = static_cast<size_t>(-1);
            dst_cursor->n                    = n;
            dst_cursor->seg_idx              = 0;
            dst_cursor->elem_ptr_is_resolved = true;
            dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
        }

        if (resolving_packet != nullptr) {
            resolving_packet->state.resolved = true;
        }

        return;
    }

    Seg* seg{ detail::NToSeg_(n) };

    size_t seg_elem_cnt;

#if EnStaging
    if (detail::GetNColor_(n) == ref_color) {
        seg_elem_cnt = seg->ref.elem_cnt;

        if (resolving_packet == nullptr ||
            resolving_packet->intent == resolving::Intent::Null) {
            if (dst_cursor != nullptr) {
                dst_cursor->idx                  = self_work.elem_cnt - 1;
                dst_cursor->n                    = n;
                dst_cursor->seg_idx              = seg_elem_cnt - 1;
                dst_cursor->elem_ptr_is_resolved = false;
                dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
                dst_cursor->stamp = self.stamp;
#endif
            }

            if (resolving_packet != nullptr) {
                resolving_packet->state.resolved = true;
            }

            return;
        }

        seq_cntr::Refer(self_work.origin, seg->ref.beg + seg_elem_cnt - 1,
                        nullptr, resolving_packet);

        if (dst_cursor != nullptr) {
            dst_cursor->idx                  = self_work.elem_cnt - 1;
            dst_cursor->n                    = n;
            dst_cursor->seg_idx              = seg_elem_cnt - 1;
            dst_cursor->elem_ptr_is_resolved = true;
            dst_cursor->elem_ptr             = resolving_packet->dst_ref.ptr;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
        }

        return;
    }
#endif

    seg_elem_cnt = seg->dat.elem_cnt;

    resolving::Packet<Elem, Elem> ref_resolving_packet{
        resolving::Packet<Elem, Elem>::MakeRef()
    };

    CircularArray<Elem> ca{
        .data{ static_cast<Elem*>(seg->dat.data) },
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{ seg_elem_cnt },
        .slot_cnt{ self_work.seg_elem_slot_cnt },
        .rot{ seg->dat.rot },

#if ZETA_Core_DebugEnable
        .stamp{ debug_utils::revision::GetStamp() },
#endif
    };

    seq_cntr::PeekR(ca, nullptr, &ref_resolving_packet);

    if (dst_cursor != nullptr) {
        dst_cursor->idx                  = self_work.elem_cnt - 1;
        dst_cursor->n                    = n;
        dst_cursor->seg_idx              = seg_elem_cnt - 1;
        dst_cursor->elem_ptr_is_resolved = true;
        dst_cursor->elem_ptr             = ref_resolving_packet.dst_ref.ptr;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    if (resolving_packet != nullptr &&
        resolving_packet->intent != resolving::Intent::Null) {
        resolving_packet->ResolveFromView(ref_resolving_packet.dst_ref,
                                          meta::AutoValueWrapper<true>{},
                                          meta::AutoValueWrapper<true>{});
    }
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::Refer(
    this auto&& self, seq_cntr::Tag, size_t idx, Cursor* dst_cursor,
    resolving::Packet<Elem, Elem>* resolving_packet) {
    detail::CheckCntr_(self);

    auto const self_work{ detail::MakeCntrWork_(self) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanRefer(idx, 1, self_work.elem_cnt));

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_cursor != nullptr ||
                                            resolving_packet != nullptr);

    if (resolving_packet != nullptr) {
        resolving_packet->AssertResultIsNotResolved();
    }

    auto [n, seg_idx]{ bin_tree::AccessL(self.root, idx + 1) };

    if (self_work.elem_cnt <= idx) {
        if (dst_cursor != nullptr) {
            dst_cursor->idx                  = idx;
            dst_cursor->n                    = n;
            dst_cursor->seg_idx              = seg_idx;
            dst_cursor->elem_ptr_is_resolved = true;
            dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
        }

        if (resolving_packet != nullptr) {
            resolving_packet->state.resolved = true;
        }

        return;
    }

    Seg* seg{ detail::NToSeg_(n) };

#if EnStaging
    if (detail::GetNColor_(n) == ref_color) {
        if (resolving_packet == nullptr ||
            resolving_packet->intent == resolving::Intent::Null) {
            if (dst_cursor != nullptr) {
                dst_cursor->idx                  = idx;
                dst_cursor->n                    = n;
                dst_cursor->seg_idx              = seg_idx;
                dst_cursor->elem_ptr_is_resolved = false;
                dst_cursor->elem_ptr             = nullptr;

#if ZETA_Core_DebugEnable
                dst_cursor->stamp = self.stamp;
#endif
            }

            if (resolving_packet != nullptr) {
                resolving_packet->state.resolved = true;
            }

            return;
        }

        seq_cntr::Refer(self_work.origin, seg->ref.beg + seg_idx, nullptr,
                        resolving_packet);

        if (dst_cursor != nullptr) {
            dst_cursor->idx                  = idx;
            dst_cursor->n                    = n;
            dst_cursor->seg_idx              = seg_idx;
            dst_cursor->elem_ptr_is_resolved = true;
            dst_cursor->elem_ptr             = resolving_packet->dst_ref.ptr;

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif
        }

        return;
    }
#endif

    resolving::Packet<Elem, Elem> ref_resolving_packet{
        resolving::Packet<Elem, Elem>::MakeRef()
    };

    CircularArray<Elem> ca{
        .data{ static_cast<Elem*>(seg->dat.data) },
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{ seg->dat.elem_cnt },
        .slot_cnt{ self_work.seg_elem_slot_cnt },
        .rot{ seg->dat.rot },

#if ZETA_Core_DebugEnable
        .stamp{ debug_utils::revision::GetStamp() },
#endif
    };

    seq_cntr::Refer(ca, seg_idx, nullptr, &ref_resolving_packet);

    if (dst_cursor != nullptr) {
        dst_cursor->idx                  = idx;
        dst_cursor->n                    = n;
        dst_cursor->seg_idx              = seg_idx;
        dst_cursor->elem_ptr_is_resolved = true;
        dst_cursor->elem_ptr             = ref_resolving_packet.dst_ref.ptr;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    if (resolving_packet != nullptr &&
        resolving_packet->intent != resolving::Intent::Null) {
        resolving_packet->ResolveFromView(ref_resolving_packet.dst_ref,
                                          meta::AutoValueWrapper<true>{},
                                          meta::AutoValueWrapper<true>{});
    }
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::Derefer(
    this auto&& self, seq_cntr::Tag, Cursor const* pos_cursor,
    resolving::Packet<Elem, Elem>* resolving_packet) {
    detail::CheckCursor_(self, pos_cursor);

    auto const self_work{ detail::MakeCntrWork_(self) };

    Node* n{ static_cast<Node*>(pos_cursor->n) };
    size_t seg_idx{ pos_cursor->seg_idx };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(resolving_packet != nullptr);

    if (self.lb == n || self.rb == n ||
        resolving_packet->intent == resolving::Intent::Null) {
        resolving_packet->state.resolved = true;

        return;
    }

    if (!pos_cursor->elem_ptr_is_resolved) {
        resolving::Packet<Elem, Elem> ref_resolving_packet{
            resolving::Packet<Elem, Elem>::MakeRef()
        };

        Seg* seg{ detail::NToSeg_(n) };

#if EnStaging
        if (detail::GetNColor_(n) == ref_color) {
            seq_cntr::Refer(self_work.origin, seg->ref.beg + seg_idx, nullptr,
                            &ref_resolving_packet);

        } else
#endif
        {
            CircularArray<Elem> ca{
                .data{ static_cast<Elem*>(seg->dat.data) },
                .elem_stride{ sizeof(Elem) },
                .elem_cnt{ self_work.seg_elem_slot_cnt },
                .slot_cnt{ self_work.seg_elem_slot_cnt },
                .rot{ seg->dat.rot },

#if ZETA_Core_DebugEnable
                .stamp{ debug_utils::revision::GetStamp() },
#endif
            };

            seq_cntr::Refer(static_cast<meta::CopyConstVolatileReference<
                                decltype(self), CircularArray<Elem>>>(ca),
                            seg_idx, nullptr, &ref_resolving_packet);
        }

        pos_cursor->elem_ptr_is_resolved = true;
        pos_cursor->elem_ptr             = ref_resolving_packet.dst_ref.ptr;
    }

    resolving_packet->ResolveFromView(
        lifecycle::ConstLeftObjView<Elem>{
            static_cast<Elem*>(pos_cursor->elem_ptr) },
        meta::AutoValueWrapper<true>{}, meta::AutoValueWrapper<true>{});
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Acceptor>
constexpr void Namespace::Cntr<CntrTplArgList>::Read(
    this auto&& self, seq_cntr::Tag, Cursor const* pos_cursor, size_t cnt,
    Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(seq_cntr::op_check::CanDerefer(
        pos_cursor->idx, cnt, self.GetElemCnt(seq_cntr::Tag{})));

    auto const self_work{ detail::MakeCntrWork_(self) };

    Node* n{ static_cast<Node*>(pos_cursor->n) };
    Seg* seg;

    size_t seg_idx{ pos_cursor->seg_idx };

    if (dst_cursor != nullptr) {
        dst_cursor->idx = pos_cursor->idx + cnt;

#if ZETA_Core_DebugEnable
        dst_cursor->stamp = self.stamp;
#endif
    }

    if (cnt == 0) {
        if (dst_cursor != nullptr) {
            dst_cursor->n                    = n;
            dst_cursor->seg_idx              = seg_idx;
            dst_cursor->elem_ptr_is_resolved = pos_cursor->elem_ptr_is_resolved;
            dst_cursor->elem_ptr             = pos_cursor->elem_ptr;
        }

        return;
    }

#if EnStaging
    OriginCursor origin_cursor;
#endif

    CircularArray<Elem> ca{
        .data{ nullptr },
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{ 0 },
        .slot_cnt{ self_work.seg_elem_slot_cnt },
        .rot{ 0 },

#if ZETA_Core_DebugEnable
        .stamp{ debug_utils::revision::GetStamp() },
#endif
    };

    while (0 < cnt) {
        seg = detail::NToSeg_(n);

        size_t seg_elem_cnt;
        size_t cur_cnt;

#if EnStaging
        if (detail::GetNColor_(n) == ref_color) {
            seg_elem_cnt = seg->ref.elem_cnt;
            cur_cnt = comparison_utils::BasicMin(cnt, seg_elem_cnt - seg_idx);

            seq_cntr::Refer(self_work.origin, seg->ref.beg + seg_idx,
                            &origin_cursor, nullptr);

            seq_cntr::Read(self_work.origin, &origin_cursor, cur_cnt, acceptor,
                           nullptr);
        } else
#endif
        {
            seg_elem_cnt = seg->dat.elem_cnt;
            cur_cnt = comparison_utils::BasicMin(cnt, seg_elem_cnt - seg_idx);

            ca.data     = static_cast<Elem*>(seg->dat.data);
            ca.elem_cnt = seg_elem_cnt;
            ca.rot      = seg->dat.rot;

            ca.IdxRead(seg_idx, cur_cnt, acceptor);
        }

        seg_idx += cur_cnt;

        if (seg_idx == seg_elem_cnt) {
            n       = bin_tree::StepR(n);
            seg_idx = 0;
        }

        cnt -= cur_cnt;
    }

    if (dst_cursor == nullptr) { return; }

    dst_cursor->n       = n;
    dst_cursor->seg_idx = seg_idx;

    if (self.rb == n
#if EnStaging
        || detail::GetNColor_(n) == ref_color
#endif
    ) {
        dst_cursor->elem_ptr_is_resolved = false;
        dst_cursor->elem_ptr             = nullptr;
        return;
    }

    seg = detail::NToSeg_(n);

    ca.data     = static_cast<Elem*>(seg->dat.data);
    ca.elem_cnt = seg->dat.elem_cnt;
    ca.rot      = seg->dat.rot;

    dst_cursor->elem_ptr_is_resolved = true;

    dst_cursor->elem_ptr = ({
        resolving::Packet<Elem, Elem> tmp{
            resolving::Packet<Elem, Elem>::MakeRef()
        };

        seq_cntr::Refer(ca, seg_idx, nullptr, &tmp);

        tmp.dst_ref.ptr;
    });
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Provider>
constexpr void Namespace::Cntr<CntrTplArgList>::Write(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Provider&& provider, Cursor* dst_cursor) {
    detail::ReadWrite_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOp_::Write>{}, pos_cursor, cnt,
        provider, dst_cursor);
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Acceptor>
constexpr void Namespace::Cntr<CntrTplArgList>::ReadWrite(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Acceptor&& acceptor, Cursor* dst_cursor) {
    detail::ReadWrite_(
        lifecycle::MakeStorageView(meta::Forward<decltype(self)>(self)),
        meta::AutoValueWrapper<detail::ReadWriteOp_::ReadWrite>{}, pos_cursor,
        cnt, acceptor, dst_cursor);
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Provider>
constexpr void Namespace::Cntr<CntrTplArgList>::PushL(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    size_t cnt, Provider&& provider, Cursor* dst_beg_cursor,
    Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    Cursor pos_cursor;
    self.PeekL(seq_cntr::Tag{}, &pos_cursor, nullptr);

    self.Insert(seq_cntr::Tag{}, &pos_cursor, cnt, provider, dst_end_cursor);

    if (dst_beg_cursor != nullptr) { *dst_beg_cursor = pos_cursor; }
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Provider>
constexpr void Namespace::Cntr<CntrTplArgList>::PushR(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    size_t cnt, Provider&& provider, Cursor* dst_beg_cursor,
    Cursor* dst_end_cursor) {
    detail::CheckCntr_(self);

    Cursor pos_cursor;
    self.GetRBCursor(seq_cntr::Tag{}, &pos_cursor);

    self.Insert(seq_cntr::Tag{}, &pos_cursor, cnt, provider, dst_end_cursor);

    if (dst_beg_cursor != nullptr) { *dst_beg_cursor = pos_cursor; }
}

template <CntrTplParamList>
template <seq_endpoint::provider::IsProvider<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Provider>
constexpr void Namespace::Cntr<CntrTplArgList>::Insert(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Provider&& provider, Cursor* dst_cursor) {
    detail::CheckCursor_(self, pos_cursor);

    auto const self_work{ detail::MakeCntrWork_(self) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanInsert(pos_cursor->idx, cnt, self_work.elem_cnt,
                                      self.GetMaxElemCnt(seq_cntr::Tag{})));

#if ZETA_Core_DebugEnable
    self.stamp = debug_utils::revision::GetStamp();
#endif

#if ZETA_Core_DebugEnable
    pos_cursor->stamp = self.stamp;
#endif

    if (cnt == 0) {
        if (dst_cursor != nullptr) { *dst_cursor = *pos_cursor; }
        return;
    }

    Node* m_n{ static_cast<Node*>(pos_cursor->n) };
    size_t seg_idx{ pos_cursor->seg_idx };
    size_t end_idx{ pos_cursor->idx + cnt };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.lb != m_n);

    unsigned long long random_seed{ utils::GetRandom() };

    Node* l_n;
    Node* r_n;

    detail::SegWork_<Elem> l_seg_work;
    detail::SegWork_<Elem> m_seg_work;
    detail::SegWork_<Elem> r_seg_work;

    l_seg_work.SetBasics(self_work);
    m_seg_work.SetBasics(self_work);
    r_seg_work.SetBasics(self_work);

    if (seg_idx == 0) {
        /*

        if the insertion point is between segments, the state has already be a
        nice form to insert (optionally) new segments to store more data.

        */

        l_n = bin_tree::StepL(m_n);

        r_n = m_n;

        m_n = nullptr;
        m_seg_work.SetNull();

        if (self.lb == l_n) {
            l_seg_work.SetNull();
        } else {
            l_seg_work.Load(detail::NToSeg_(l_n));
        }

        if (self.rb == r_n) {
            r_seg_work.SetNull();
        } else {
            r_seg_work.Load(detail::NToSeg_(r_n));
        }
    } else {
        m_seg_work.Load(detail::NToSeg_(m_n));

        size_t ml_elem_cnt{ seg_idx };
        size_t mr_elem_cnt{ m_seg_work.ca.elem_cnt - seg_idx };

        if (cnt <= m_seg_work.elem_vac) {
#if EnStaging
            if (m_seg_work.color == ref_color) {
                detail::AugMaterializeRefSeg_(
                    self_work, m_seg_work, ml_elem_cnt, cnt, mr_elem_cnt,
                    seq_endpoint::acceptor::BasicAcceptor{}, provider);

                pos_cursor->elem_ptr_is_resolved = true;

                pos_cursor->elem_ptr = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(m_seg_work.ca, ml_elem_cnt, nullptr, &tmp);

                    tmp.dst_ref.ptr;
                });
            } else
#endif
            {
                m_seg_work.ca.IdxInsert(ml_elem_cnt, cnt, provider);

                pos_cursor->elem_ptr_is_resolved = true;

                pos_cursor->elem_ptr = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(m_seg_work.ca, ml_elem_cnt, nullptr, &tmp);

                    tmp.dst_ref.ptr;
                });
            }

            m_seg_work.Store();

            if (dst_cursor != nullptr) {
                dst_cursor->idx                  = end_idx;
                dst_cursor->n                    = m_n;
                dst_cursor->seg_idx              = ml_elem_cnt + cnt;
                dst_cursor->elem_ptr_is_resolved = true;
                dst_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(m_seg_work.ca, ml_elem_cnt + cnt, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });

#if ZETA_Core_DebugEnable
                dst_cursor->stamp = self.stamp;
#endif
            }

            return;
        }

        l_n = bin_tree::StepL(m_n);
        r_n = bin_tree::StepR(m_n);

        if (self.lb == l_n) {
            l_seg_work.SetNull();
        } else {
            l_seg_work.Load(detail::NToSeg_(l_n));
        }

        if (self.rb == r_n) {
            r_seg_work.SetNull();
        } else {
            r_seg_work.Load(detail::NToSeg_(r_n));
        }

        struct {
            bool is_ok;
            size_t shove_cnt;
        } insert_into_l_m_vars, insert_into_m_r_vars;

#if EnStaging
        if (m_seg_work.color == ref_color) {
            insert_into_l_m_vars.is_ok =
                ml_elem_cnt + cnt <= l_seg_work.elem_vac;

            if (insert_into_l_m_vars.is_ok) {
                insert_into_l_m_vars.shove_cnt = ml_elem_cnt + cnt;
            }

            insert_into_m_r_vars.is_ok =
                mr_elem_cnt + cnt <= r_seg_work.elem_vac;

            if (insert_into_m_r_vars.is_ok) {
                insert_into_m_r_vars.shove_cnt = cnt + mr_elem_cnt;
            }
        } else
#endif
        {
            insert_into_l_m_vars.is_ok =
                cnt <= l_seg_work.elem_vac + m_seg_work.elem_vac;

            if (insert_into_l_m_vars.is_ok) {
                insert_into_l_m_vars.shove_cnt =
                    (m_seg_work.ca.elem_cnt + cnt - l_seg_work.ca.elem_cnt) / 2;
            }

            insert_into_m_r_vars.is_ok =
                cnt <= m_seg_work.elem_vac + r_seg_work.elem_vac;

            if (insert_into_m_r_vars.is_ok) {
                insert_into_m_r_vars.shove_cnt =
                    (m_seg_work.ca.elem_cnt + cnt - r_seg_work.ca.elem_cnt) / 2;
            }
        }

        // choose the better side to insert

        switch (({
            bool cond_l{ insert_into_l_m_vars.is_ok &&
                         l_seg_work.ca.elem_cnt <= r_seg_work.ca.elem_cnt };

            bool cond_r{ insert_into_m_r_vars.is_ok &&
                         r_seg_work.ca.elem_cnt <= l_seg_work.ca.elem_cnt };

            if (!cond_l && !cond_r) { goto SPLIT_M; }

            utils::Choose2(cond_l, cond_r, &random_seed);
        })) {
        case utils::Choose2Result::Case0: goto INSERT_INTO_L_M;
        case utils::Choose2Result::Case1: goto INSERT_INTO_M_R;
        default: ZETA_Core_DebugUtils_Diag_Unreachable();
        }

    INSERT_INTO_L_M:
        {
            // use l + m to hold all old + new data

            detail::SegInsertShoveL_(self_work, l_seg_work, m_seg_work,
                                     ml_elem_cnt, cnt,
                                     insert_into_l_m_vars.shove_cnt, provider);

            l_seg_work.Store();
            m_seg_work.Store();

            if (m_seg_work.ca.elem_cnt < cnt + mr_elem_cnt) {
                pos_cursor->n       = l_n;
                pos_cursor->seg_idx = l_seg_work.ca.elem_cnt +
                                      m_seg_work.ca.elem_cnt - cnt -
                                      mr_elem_cnt;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(l_seg_work.ca, pos_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });
            } else {
                pos_cursor->n = m_n;
                pos_cursor->seg_idx =
                    m_seg_work.ca.elem_cnt - cnt - mr_elem_cnt;

#if EnStaging
                if (m_seg_work.color == ref_color) {
                    pos_cursor->elem_ptr_is_resolved = false;
                    pos_cursor->elem_ptr             = nullptr;
                } else
#endif
                {
                    pos_cursor->elem_ptr_is_resolved = true;

                    pos_cursor->elem_ptr = ({
                        resolving::Packet<Elem, Elem> tmp{
                            resolving::Packet<Elem, Elem>::MakeRef()
                        };

                        seq_cntr::Refer(m_seg_work.ca, pos_cursor->seg_idx,
                                        nullptr, &tmp);

                        tmp.dst_ref.ptr;
                    });
                }
            }

            if (dst_cursor == nullptr) { return; }

            dst_cursor->idx = end_idx;

            if (m_seg_work.ca.elem_cnt < mr_elem_cnt) {
                dst_cursor->n       = l_n;
                dst_cursor->seg_idx = l_seg_work.ca.elem_cnt +
                                      m_seg_work.ca.elem_cnt - mr_elem_cnt;
                dst_cursor->elem_ptr_is_resolved = true;
                dst_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(l_seg_work.ca, dst_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });
            } else {
                dst_cursor->n       = m_n;
                dst_cursor->seg_idx = m_seg_work.ca.elem_cnt - mr_elem_cnt;

#if EnStaging
                if (m_seg_work.color == ref_color) {
                    dst_cursor->elem_ptr_is_resolved = false;
                    dst_cursor->elem_ptr             = nullptr;
                } else
#endif
                {
                    dst_cursor->elem_ptr_is_resolved = true;

                    dst_cursor->elem_ptr = ({
                        resolving::Packet<Elem, Elem> tmp{
                            resolving::Packet<Elem, Elem>::MakeRef()
                        };

                        seq_cntr::Refer(m_seg_work.ca, dst_cursor->seg_idx,
                                        nullptr, &tmp);

                        tmp.dst_ref.ptr;
                    });
                }
            }

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif

            return;
        }

    INSERT_INTO_M_R:
        {
            // use m + r to hold all old + new data

            /*

            m_seg_work.elem_vac = seg_elem_slot_cnt - m_seg_work.ca.elem_cnt
            r_seg_work.elem_vac = seg_elem_slot_cnt - r_seg_work.ca.elem_cnt

            cnt <= m_seg_work.elem_vac + r_seg_work.elem_vac

            shove_cnt = (m_seg_work.ca.elem_cnt + cnt - r_seg_work.ca.elem_cnt)
            / 2

            =>

            shove_cnt <= r_seg_work.elem_vac ?

            */

            detail::SegInsertShoveR_(self_work, m_seg_work, r_seg_work,
                                     mr_elem_cnt, cnt,
                                     insert_into_m_r_vars.shove_cnt, provider);

            m_seg_work.Store();
            r_seg_work.Store();

            if (ml_elem_cnt < m_seg_work.ca.elem_cnt) {
                pos_cursor->n                    = m_n;
                pos_cursor->seg_idx              = ml_elem_cnt;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(m_seg_work.ca, pos_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });
            } else {
                pos_cursor->n       = r_n;
                pos_cursor->seg_idx = ml_elem_cnt - m_seg_work.ca.elem_cnt;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(r_seg_work.ca, pos_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });
            }

            if (dst_cursor == nullptr) { return; }

            dst_cursor->idx = end_idx;

            if (ml_elem_cnt + cnt < m_seg_work.ca.elem_cnt) {
                dst_cursor->n                    = m_n;
                dst_cursor->seg_idx              = ml_elem_cnt + cnt;
                dst_cursor->elem_ptr_is_resolved = true;
                dst_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(m_seg_work.ca, dst_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });
            } else {
                dst_cursor->n = r_n;
                dst_cursor->seg_idx =
                    ml_elem_cnt + cnt - m_seg_work.ca.elem_cnt;
                dst_cursor->elem_ptr_is_resolved = true;
                dst_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(r_seg_work.ca, dst_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });
            }

#if ZETA_Core_DebugEnable
            dst_cursor->stamp = self.stamp;
#endif

            return;
        }

    SPLIT_M:
        {
            /*

            the aim is to transform the insertion point to be between segments.

            case 0: push ml to l
            case 1: push mr to r
            case 2: insert a seg between l and m, push ml to new seg
            case 3: insert a seg between m and r, push mr to new seg

            */

            switch (static_cast<unsigned>(ml_elem_cnt <= l_seg_work.elem_vac) *
                        0b10 +
                    static_cast<unsigned>(mr_elem_cnt <= r_seg_work.elem_vac) *
                        0b01) {
            case 0b01: goto SPLIT_M__SHOVE_MR_TO_R;
            case 0b10: goto SPLIT_M__SHOVE_ML_TO_L;
            case 0b11:
                switch (({
                    size_t l_ml_elem_cnt{ l_seg_work.ca.elem_cnt +
                                          ml_elem_cnt };
                    size_t mr_r_elem_cnt{ mr_elem_cnt +
                                          r_seg_work.ca.elem_cnt };

                    utils::Choose2(l_ml_elem_cnt <= mr_r_elem_cnt,
                                   mr_r_elem_cnt <= l_ml_elem_cnt,
                                   &random_seed);
                })) {
                case utils::Choose2Result::Case0: goto SPLIT_M__SHOVE_ML_TO_L;
                case utils::Choose2Result::Case1: goto SPLIT_M__SHOVE_MR_TO_R;
                default: ZETA_Core_DebugUtils_Diag_Unreachable();
                }
            case 0b00:
                switch (utils::Choose2(ml_elem_cnt <= mr_elem_cnt,
                                       mr_elem_cnt <= ml_elem_cnt,
                                       &random_seed)) {
                case utils::Choose2Result::Case0: goto SPLIT_M__SPLIT_ML;
                case utils::Choose2Result::Case1: goto SPLIT_M__SPLIT_MR;
                default: ZETA_Core_DebugUtils_Diag_Unreachable();
                }
            default: ZETA_Core_DebugUtils_Diag_Unreachable();
            }

        SPLIT_M__SHOVE_ML_TO_L:
            {
                detail::SegShoveL_(self_work, l_seg_work, m_seg_work,
                                   ml_elem_cnt);

                r_n = m_n;
                r_seg_work.CopyFromSameBasics(m_seg_work);

                m_n = nullptr;
                m_seg_work.SetNull();

                goto INSERT_BETWEEN_L_R;
            }

        SPLIT_M__SHOVE_MR_TO_R:
            {
                detail::SegShoveR_(self_work, m_seg_work, r_seg_work,
                                   mr_elem_cnt);

                l_n = m_n;
                l_seg_work.CopyFromSameBasics(m_seg_work);

                m_n = nullptr;
                m_seg_work.SetNull();

                goto INSERT_BETWEEN_L_R;
            }

        SPLIT_M__SPLIT_ML:
            {
#if EnStaging
                if (m_seg_work.color == ref_color) {
                    Seg* new_l_seg{ detail::CreateRefSeg_(
                        self_work.seg_alctr) };

                    self.root = rbtree::Insert(l_n, m_n, &new_l_seg->n);

                    l_n                    = &new_l_seg->n;
                    l_seg_work.is_null     = false;
                    l_seg_work.is_dirty    = true;
                    l_seg_work.seg         = new_l_seg;
                    l_seg_work.color       = ref_color;
                    l_seg_work.ref_beg     = m_seg_work.ref_beg;
                    l_seg_work.ca.elem_cnt = ml_elem_cnt;
                    l_seg_work.elem_vac =
                        self_work.seg_elem_slot_cnt -
                        comparison_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                                   self_work.seg_elem_slot_cnt);

                    r_n                    = m_n;
                    r_seg_work.is_null     = false;
                    r_seg_work.is_dirty    = true;
                    r_seg_work.seg         = m_seg_work.seg;
                    r_seg_work.color       = ref_color;
                    r_seg_work.ref_beg     = m_seg_work.ref_beg + ml_elem_cnt;
                    r_seg_work.ca.elem_cnt = mr_elem_cnt;
                    r_seg_work.elem_vac =
                        self_work.seg_elem_slot_cnt -
                        comparison_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                                   self_work.seg_elem_slot_cnt);
                } else
#endif
                {
                    Seg* new_l_seg{ detail::CreateDatSeg_(
                        self_work.data_size, self_work.seg_alctr,
                        self_work.data_alctr) };

                    self.root = rbtree::Insert(l_n, m_n, &new_l_seg->n);

                    l_n = &new_l_seg->n;
                    l_seg_work.Load(new_l_seg);

                    r_n = m_n;
                    r_seg_work.CopyFromSameBasics(m_seg_work);

                    detail::SegShoveL_(self_work, l_seg_work, r_seg_work,
                                       ml_elem_cnt);
                }

                goto INSERT_BETWEEN_L_R;
            }

        SPLIT_M__SPLIT_MR:
            {
#if EnStaging
                if (m_seg_work.color == ref_color) {
                    Seg* new_r_seg{ detail::CreateRefSeg_(
                        self_work.seg_alctr) };

                    self.root = rbtree::Insert(m_n, r_n, &new_r_seg->n);

                    l_n                    = m_n;
                    l_seg_work.is_null     = false;
                    l_seg_work.is_dirty    = true;
                    l_seg_work.seg         = m_seg_work.seg;
                    l_seg_work.color       = ref_color;
                    l_seg_work.ref_beg     = m_seg_work.ref_beg;
                    l_seg_work.ca.elem_cnt = ml_elem_cnt;
                    l_seg_work.elem_vac =
                        self_work.seg_elem_slot_cnt -
                        comparison_utils::BasicMin(l_seg_work.ca.elem_cnt,
                                                   self_work.seg_elem_slot_cnt);

                    r_n                    = &new_r_seg->n;
                    r_seg_work.is_null     = false;
                    r_seg_work.is_dirty    = true;
                    r_seg_work.seg         = new_r_seg;
                    r_seg_work.color       = ref_color;
                    r_seg_work.ref_beg     = m_seg_work.ref_beg + ml_elem_cnt;
                    r_seg_work.ca.elem_cnt = mr_elem_cnt;
                    r_seg_work.elem_vac =
                        self_work.seg_elem_slot_cnt -
                        comparison_utils::BasicMin(r_seg_work.ca.elem_cnt,
                                                   self_work.seg_elem_slot_cnt);
                } else
#endif
                {
                    Seg* new_r_seg{ detail::CreateDatSeg_(
                        self_work.data_size, self_work.seg_alctr,
                        self_work.data_alctr) };

                    self.root = rbtree::Insert(m_n, r_n, &new_r_seg->n);

                    l_n = m_n;
                    l_seg_work.CopyFromSameBasics(m_seg_work);

                    r_n = &new_r_seg->n;
                    r_seg_work.Load(new_r_seg);

                    detail::SegShoveR_(self_work, l_seg_work, r_seg_work,
                                       mr_elem_cnt);
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

    size_t extra_res_cnt{ cnt % self_work.seg_elem_slot_cnt };
    size_t extra_vac_cnt{ extra_res_cnt == 0
                              ? 0
                              : self_work.seg_elem_slot_cnt - extra_res_cnt };

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
                               &random_seed)) {
        case utils::Choose2Result::Case0:
            l_shove = true;
            r_shove = false;
            break;

        case utils::Choose2Result::Case1:
            l_shove = false;
            r_shove = true;
            break;

        default: ZETA_Core_DebugUtils_Diag_Unreachable();
        }
    }

    detail::ElemCntBalancer_ balancer{
        (l_shove ? l_seg_work.ca.elem_cnt : 0) + cnt +
            (r_shove ? r_seg_work.ca.elem_cnt : 0),
        self_work.seg_elem_slot_cnt
    };

    bool pos_cursor_is_set{ false };

    if (l_shove) {
        size_t new_l_elem_cnt{ balancer.Fetch() };

#if EnStaging
        if (l_seg_work.color == ref_color) {
            detail::MaterializeRefSeg_(self_work, l_seg_work);
        }
#endif

        l_seg_work.is_dirty = true;

        pos_cursor->n       = l_n;
        pos_cursor->seg_idx = l_seg_work.ca.elem_cnt;

        seq_cntr::PushR(l_seg_work.ca, new_l_elem_cnt - l_seg_work.ca.elem_cnt,
                        provider, nullptr, nullptr);

        pos_cursor->elem_ptr_is_resolved = true;

        pos_cursor->elem_ptr = ({
            resolving::Packet<Elem, Elem> tmp{
                resolving::Packet<Elem, Elem>::MakeRef()
            };

            seq_cntr::Refer(l_seg_work.ca, pos_cursor->seg_idx, nullptr, &tmp);

            tmp.dst_ref.ptr;
        });

        pos_cursor_is_set = true;
    }

    if (!l_seg_work.is_null && l_seg_work.is_dirty) { l_seg_work.Store(); }

    {
        Node* prv_n{ l_n };

        while ((r_shove ? 1 : 0) < balancer.res_seg_cnt) {
            size_t cur_cnt{ balancer.Fetch() };

            Seg* new_seg{ detail::CreateDatSeg_(self_work.data_size,
                                                self_work.seg_alctr,
                                                self_work.data_alctr) };

            self.root = rbtree::Insert(prv_n, r_n, &new_seg->n);

            seq_endpoint::provider::Transfer(
                provider,
                lifecycle::ConstructingMemToObjView<Elem>{
                    static_cast<Elem*>(new_seg->dat.data) },
                sizeof(Elem), cur_cnt);

            new_seg->dat.elem_cnt = static_cast<unsigned short>(cur_cnt);

            bin_tree::AddDiffSize(&new_seg->n, cur_cnt);

            prv_n = &new_seg->n;

            if (!pos_cursor_is_set) {
                pos_cursor->n                    = &new_seg->n;
                pos_cursor->seg_idx              = 0;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = new_seg->dat.data;

                pos_cursor_is_set = true;
            }
        }
    }

    size_t old_r_elem_cnt{ r_seg_work.ca.elem_cnt };

    if (r_shove) {
        size_t new_r_elem_cnt{ balancer.Fetch() };

#if EnStaging
        if (r_seg_work.color == ref_color) {
            detail::MaterializeRefSeg_(self_work, r_seg_work);
        }
#endif

        r_seg_work.is_dirty = true;

        seq_cntr::PushL(r_seg_work.ca, new_r_elem_cnt - r_seg_work.ca.elem_cnt,
                        provider, nullptr, nullptr);

        if (!pos_cursor_is_set) {
            pos_cursor->n                    = r_n;
            pos_cursor->seg_idx              = 0;
            pos_cursor->elem_ptr_is_resolved = true;
            pos_cursor->elem_ptr             = ({
                resolving::Packet<Elem, Elem> tmp{
                    resolving::Packet<Elem, Elem>::MakeRef()
                };

                seq_cntr::PeekL(r_seg_work.ca, nullptr, &tmp);

                tmp.dst_ref.ptr;
            });

            pos_cursor_is_set = true;
        }
    }

    if (!r_seg_work.is_null && r_seg_work.is_dirty) { r_seg_work.Store(); }

    if (dst_cursor == nullptr) { return; }

    size_t dst_seg_idx{ r_seg_work.ca.elem_cnt - old_r_elem_cnt };

    dst_cursor->n       = r_n;
    dst_cursor->idx     = end_idx;
    dst_cursor->seg_idx = dst_seg_idx;

#if ZETA_Core_DebugEnable
    dst_cursor->stamp = self.stamp;
#endif

    if (r_seg_work.is_null) {
        dst_cursor->elem_ptr_is_resolved = true;
        dst_cursor->elem_ptr             = nullptr;

        return;
    }

#if EnStaging
    if (r_seg_work.color == ref_color) {
        dst_cursor->elem_ptr_is_resolved = false;
        dst_cursor->elem_ptr             = nullptr;
    } else
#endif
    {
        dst_cursor->elem_ptr_is_resolved = true;

        dst_cursor->elem_ptr = ({
            resolving::Packet<Elem, Elem> tmp{
                resolving::Packet<Elem, Elem>::MakeRef()
            };

            seq_cntr::Refer(r_seg_work.ca, dst_seg_idx, nullptr, &tmp);

            tmp.dst_ref.ptr;
        });
    }
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Acceptor>
constexpr void Namespace::Cntr<CntrTplArgList>::PopL(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    size_t cnt, Acceptor&& acceptor, Cursor* dst_cursor) {
    size_t elem_cnt{ sizeof(Elem) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPopL(cnt, elem_cnt));

    Cursor pos_cursor;
    self.PeekL(seq_cntr::Tag{}, &pos_cursor, nullptr);

    self.Erase(seq_cntr::Tag{}, &pos_cursor, cnt, acceptor);

    if (dst_cursor != nullptr) { *dst_cursor = pos_cursor; }
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Acceptor>
constexpr void Namespace::Cntr<CntrTplArgList>::PopR(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    size_t cnt, Acceptor&& acceptor, Cursor* dst_cursor) {
    size_t elem_cnt{ self.GetElemCnt(seq_cntr::Tag{}) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanPopR(cnt, elem_cnt));

    Cursor pos_cursor;
    self.Refer(seq_cntr::Tag{}, elem_cnt - cnt, &pos_cursor, nullptr);

    self.Erase(seq_cntr::Tag{}, &pos_cursor, cnt, acceptor);

    if (dst_cursor != nullptr) { *dst_cursor = pos_cursor; }
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Acceptor>
constexpr void Namespace::Cntr<CntrTplArgList>::Erase(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Cursor* pos_cursor, size_t cnt, Acceptor&& acceptor) {
    detail::CheckCursor_(self, pos_cursor);

    if (cnt == 0) { return; }

    auto const self_work{ detail::MakeCntrWork_(self) };

    unsigned long long random_seed{ utils::GetRandom() };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanErase(pos_cursor->idx, cnt, self_work.elem_cnt));

    Node* m_n{ static_cast<Node*>(pos_cursor->n) };
    size_t seg_idx{ pos_cursor->seg_idx };

    detail::SegWork_<Elem> seg_works[4];
    size_t seg_work_cnt{ 0 };

    for (int i{ 0 }; i < 4; ++i) { seg_works[i].SetBasics(self_work); }

#if EnStaging
    OriginCursor origin_cursor;
#endif

    Node* l_n{ bin_tree::StepL(m_n) };
    Node* r_n;

    if (self.lb != l_n) {
        seg_works[seg_work_cnt++].Load(detail::NToSeg_(l_n));
    }

    seg_works[seg_work_cnt++].Load(detail::NToSeg_(m_n));

    if (seg_idx + cnt < seg_works[seg_work_cnt - 1].ca.elem_cnt) {
        detail::SegWork_<Elem> l_seg_work;
        detail::SegWork_<Elem> m_seg_work{ seg_works[seg_work_cnt - 1] };
        detail::SegWork_<Elem> r_seg_work;

        if (seg_work_cnt < 2) {
            l_seg_work.SetBasics(self_work);
            l_seg_work.SetNull();
        } else {
            l_seg_work = seg_works[seg_work_cnt - 2];
        }

        r_n = bin_tree::StepR(m_n);

        r_seg_work.SetBasics(self_work);

        if (self.rb == r_n) {
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
                                self_work.seg_elem_slot_cnt };

            bool m_r_is_ok{ !r_seg_work.is_null &&
                            ml_elem_cnt + mr_elem_cnt +
                                    r_seg_work.ca.elem_cnt <=
                                self_work.seg_elem_slot_cnt };

            static_cast<int>(l_m_is_ok) * 0b10 +
                static_cast<int>(m_r_is_ok) * 0b01;
        })) {
        case 0b00: goto INTER_M_ERASE__M_ERASE;
        case 0b01: goto INTER_M_ERASE__M_R_ERASE_MERGE;
        case 0b10: goto INTER_M_ERASE__L_M_ERASE_MERGE;
        default:
        }

        switch (({
            size_t l_ml_elem_cnt{ l_seg_work.ca.elem_cnt + ml_elem_cnt };
            size_t mr_r_elem_cnt{ mr_elem_cnt + r_seg_work.ca.elem_cnt };

            utils::Choose2(l_ml_elem_cnt <= mr_r_elem_cnt,
                           mr_r_elem_cnt <= l_ml_elem_cnt, &random_seed);
        })) {
        case utils::Choose2Result::Case0: goto INTER_M_ERASE__L_M_ERASE_MERGE;
        case utils::Choose2Result::Case1: goto INTER_M_ERASE__M_R_ERASE_MERGE;
        default: ZETA_Core_DebugUtils_Diag_Unreachable();
        }

    INTER_M_ERASE__L_M_ERASE_MERGE:
        {
            switch (({
                size_t l_based_merge_cost{ EnStagingTernary(
                                               l_seg_work.color == ref_color,
                                               l_seg_work.ca.elem_cnt, 0) +
                                           ml_elem_cnt + mr_elem_cnt };

                size_t m_based_merge_cost{ l_seg_work.ca.elem_cnt +
                                           EnStagingTernary(
                                               m_seg_work.color == ref_color,
                                               ml_elem_cnt + mr_elem_cnt,
                                               comparison_utils::BasicMin(
                                                   ml_elem_cnt, mr_elem_cnt)) };

                utils::Choose2(l_based_merge_cost <= m_based_merge_cost,
                               m_based_merge_cost <= l_based_merge_cost,
                               &random_seed);
            })) {
            case utils::Choose2Result::Case0:
                goto INTER_M_ERASE__L_M_ERASE_MERGE__L_BASED;
            case utils::Choose2Result::Case1:
                goto INTER_M_ERASE__L_M_ERASE_MERGE__M_BASED;
            default: ZETA_Core_DebugUtils_Diag_Unreachable();
            }

        INTER_M_ERASE__L_M_ERASE_MERGE__L_BASED:
            {
                detail::SegEraseShoveL_(self_work, l_seg_work, m_seg_work,
                                        ml_elem_cnt, cnt,
                                        ml_elem_cnt + mr_elem_cnt, acceptor);

                self.root = rbtree::Extract(m_n);
                detail::DestroySeg_(m_seg_work.seg, self_work.seg_alctr,
                                    self_work.data_alctr);

                l_seg_work.Store();

                pos_cursor->n       = l_n;
                pos_cursor->seg_idx = l_seg_work.ca.elem_cnt - mr_elem_cnt;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(l_seg_work.ca, pos_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });

                return;
            }

        INTER_M_ERASE__L_M_ERASE_MERGE__M_BASED:
            {
#if EnStaging
                if (m_seg_work.color == ref_color) {
                    detail::AugMaterializeRefSeg_(
                        self_work, m_seg_work, ml_elem_cnt, 0, mr_elem_cnt,
                        acceptor, seq_endpoint::provider::BasicProvider{});
                } else
#endif
                {
                    m_seg_work.ca.IdxErase(ml_elem_cnt, cnt, acceptor, nullptr);
                }

                detail::SegShoveR_(self_work, l_seg_work, m_seg_work,
                                   l_seg_work.ca.elem_cnt);

                self.root = rbtree::Extract(l_n);
                detail::DestroySeg_(l_seg_work.seg, self_work.seg_alctr,
                                    self_work.data_alctr);

                m_seg_work.Store();

                pos_cursor->n       = m_n;
                pos_cursor->seg_idx = m_seg_work.ca.elem_cnt - mr_elem_cnt;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(m_seg_work.ca, pos_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });

                return;
            }
        }

    INTER_M_ERASE__M_R_ERASE_MERGE:
        {
            switch (({
                size_t m_based_merge_cost{ EnStagingTernary(
                                               m_seg_work.color == ref_color,
                                               ml_elem_cnt + mr_elem_cnt,
                                               comparison_utils::BasicMin(
                                                   ml_elem_cnt, mr_elem_cnt)) +
                                           r_seg_work.ca.elem_cnt };

                size_t r_based_merge_cost{ EnStagingTernary(
                                               r_seg_work.color == ref_color,
                                               r_seg_work.ca.elem_cnt, 0) +
                                           ml_elem_cnt + mr_elem_cnt };

                utils::Choose2(m_based_merge_cost <= r_based_merge_cost,
                               r_based_merge_cost <= m_based_merge_cost,
                               &random_seed);
            })) {
            case utils::Choose2Result::Case0:
                goto INTER_M_ERASE__M_R_ERASE_MERGE__M_BASED;
            case utils::Choose2Result::Case1:
                goto INTER_M_ERASE__M_R_ERASE_MERGE__R_BASED;
            default: ZETA_Core_DebugUtils_Diag_Unreachable();
            }

        INTER_M_ERASE__M_R_ERASE_MERGE__M_BASED:
            {
#if EnStaging
                if (m_seg_work.color == ref_color) {
                    detail::AugMaterializeRefSeg_(
                        self_work, m_seg_work, ml_elem_cnt, 0, mr_elem_cnt,
                        acceptor, seq_endpoint::provider::BasicProvider{});
                } else
#endif
                {
                    m_seg_work.ca.IdxErase(seg_idx, cnt, acceptor, nullptr);
                }

                detail::SegShoveL_(self_work, m_seg_work, r_seg_work,
                                   r_seg_work.ca.elem_cnt);

                self.root = rbtree::Extract(r_n);
                detail::DestroySeg_(r_seg_work.seg, self_work.seg_alctr,
                                    self_work.data_alctr);

                m_seg_work.Store();

                pos_cursor->n                    = m_n;
                pos_cursor->seg_idx              = ml_elem_cnt;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(m_seg_work.ca, pos_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });

                return;
            }

        INTER_M_ERASE__M_R_ERASE_MERGE__R_BASED:
            {
                detail::SegEraseShoveR_(self_work, m_seg_work, r_seg_work,
                                        mr_elem_cnt, cnt,
                                        ml_elem_cnt + mr_elem_cnt, acceptor);

                self.root = rbtree::Extract(m_n);
                detail::DestroySeg_(m_seg_work.seg, self_work.seg_alctr,
                                    self_work.data_alctr);

                r_seg_work.Store();

                pos_cursor->n                    = r_n;
                pos_cursor->seg_idx              = ml_elem_cnt;
                pos_cursor->elem_ptr_is_resolved = true;
                pos_cursor->elem_ptr             = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(r_seg_work.ca, pos_cursor->seg_idx, nullptr,
                                    &tmp);

                    tmp.dst_ref.ptr;
                });

                return;
            }
        }

    INTER_M_ERASE__M_ERASE:
        {
#if EnStaging
            if (m_seg_work.color == dat_color)
#endif
            {
                m_seg_work.ca.IdxErase(ml_elem_cnt, cnt, acceptor, nullptr);

                m_seg_work.Store();

                pos_cursor->elem_ptr_is_resolved = true;

                pos_cursor->elem_ptr = ({
                    resolving::Packet<Elem, Elem> tmp{
                        resolving::Packet<Elem, Elem>::MakeRef()
                    };

                    seq_cntr::Refer(m_seg_work.ca, ml_elem_cnt, nullptr, &tmp);

                    tmp.dst_ref.ptr;
                });

                return;
            }

            pos_cursor->elem_ptr_is_resolved = false;
            pos_cursor->elem_ptr             = nullptr;

#if EnStaging
            {
                size_t target_idx{ m_seg_work.ref_beg + ml_elem_cnt };

                seq_cntr::Refer(self_work.origin, target_idx, &origin_cursor,
                                nullptr);

                seq_cntr::Read(self_work.origin, &origin_cursor, cnt, acceptor,
                               nullptr);
            }

            switch (static_cast<unsigned>(ml_elem_cnt == 0) * 0b10 +
                    static_cast<unsigned>(mr_elem_cnt == 0) * 0b01) {
            case 0b11:
                self.root = rbtree::Extract(m_n);
                detail::DestroyRefSeg_(m_seg_work.seg, self_work.seg_alctr);

                pos_cursor->n       = r_n;
                pos_cursor->seg_idx = 0;

                return;

            case 0b10:
                m_seg_work.ref_beg += cnt;
                m_seg_work.ca.elem_cnt = mr_elem_cnt;
                m_seg_work.Store();

                pos_cursor->elem_ptr_is_resolved = false;
                pos_cursor->elem_ptr             = nullptr;

                return;

            case 0b01:
                m_seg_work.ca.elem_cnt = ml_elem_cnt;
                m_seg_work.Store();

                pos_cursor->elem_ptr_is_resolved = false;
                pos_cursor->elem_ptr             = nullptr;

                return;

            default:
            }

            Seg* new_seg{ detail::CreateRefSeg_(self_work.seg_alctr) };

            switch (utils::Choose2(&random_seed)) {
            case utils::Choose2Result::Case0: {
                new_seg->ref.beg      = m_seg_work.ref_beg;
                new_seg->ref.elem_cnt = ml_elem_cnt;

                m_seg_work.ref_beg += ml_elem_cnt + cnt;
                m_seg_work.ca.elem_cnt = mr_elem_cnt;

                self.root = rbtree::Insert(l_n, m_n, &new_seg->n);

                pos_cursor->seg_idx = 0;

                break;
            }

            case utils::Choose2Result::Case1: {
                new_seg->ref.beg      = m_seg_work.ref_beg + ml_elem_cnt + cnt;
                new_seg->ref.elem_cnt = mr_elem_cnt;

                m_seg_work.ca.elem_cnt = ml_elem_cnt;

                self.root = rbtree::Insert(m_n, r_n, &new_seg->n);

                pos_cursor->n       = &new_seg->n;
                pos_cursor->seg_idx = 0;

                break;
            }

            default: ZETA_Core_DebugUtils_Diag_Unreachable();
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
        detail::SegWork_<Elem>& m_seg_work{ seg_works[seg_work_cnt - 1] };

        cnt -= m_seg_work.ca.elem_cnt - seg_idx;

#if EnStaging
        if (m_seg_work.color == ref_color) {
            size_t target_idx{ m_seg_work.ref_beg + seg_idx };

            seq_cntr::Refer(self_work.origin, target_idx, &origin_cursor,
                            nullptr);

            seq_cntr::Read(self_work.origin, &origin_cursor,
                           m_seg_work.ca.elem_cnt - seg_idx, acceptor, nullptr);

            m_seg_work.ca.elem_cnt = seg_idx;
        } else
#endif
        {
            m_seg_work.ca.IdxErase(seg_idx, m_seg_work.ca.elem_cnt - seg_idx,
                                   acceptor, nullptr);
        }

        m_seg_work.is_dirty = true;

        m_n     = bin_tree::StepR(m_n);
        seg_idx = 0;
    }

    while (0 < cnt) {
        Seg* m_seg{ detail::NToSeg_(m_n) };

#if EnStaging
        if (detail::GetNColor_(m_n) == ref_color) {
            {
                size_t target_idx{ m_seg->ref.beg };

                seq_cntr::Refer(self_work.origin, target_idx, &origin_cursor,
                                nullptr);

                seq_cntr::Read(
                    self_work.origin, &origin_cursor,
                    comparison_utils::BasicMin(m_seg->ref.elem_cnt, cnt),
                    acceptor, nullptr);
            }

            if (m_seg->ref.elem_cnt <= cnt) {
                cnt -= m_seg->ref.elem_cnt;

                Node* nxt_m_n{ bin_tree::StepR(m_n) };

                self.root = rbtree::Extract(m_n);
                detail::DestroyRefSeg_(m_seg, self_work.seg_alctr);

                m_n = nxt_m_n;

                continue;
            }

            detail::SegWork_<Elem>& seg_work{ seg_works[seg_work_cnt++] };
            seg_work.Load(m_seg);

            seg_work.is_dirty = true;
            seg_work.ref_beg += cnt;
            seg_work.ca.elem_cnt -= cnt;

            break;
        }
#endif

        detail::SegWork_<Elem>& m_seg_work{ seg_works[seg_work_cnt] };
        m_seg_work.Load(m_seg);

        if (cnt < m_seg_work.ca.elem_cnt) {
            ++seg_work_cnt;

            m_seg_work.is_dirty = true;
            m_seg_work.ca.IdxErase(0, cnt, acceptor, nullptr);

            break;
        }

        cnt -= m_seg->dat.elem_cnt;

        m_seg_work.ca.IdxErase(0, m_seg_work.ca.elem_cnt, acceptor, nullptr);

        Node* nxt_m_n{ bin_tree::StepR(m_n) };

        self.root = rbtree::Extract(m_n);
        detail::DestroyDatSeg_(m_seg, self_work.seg_alctr,
                               self_work.data_alctr);

        m_n = nxt_m_n;
    }

    if (0 < seg_work_cnt && &seg_works[seg_work_cnt - 1].seg->n == m_n) {
        Node* nxt_m_n{ bin_tree::StepR(m_n) };

        if (self.rb != nxt_m_n) {
            detail::SegWork_<Elem>& seg_work{ seg_works[seg_work_cnt++] };
            seg_work.Load(detail::NToSeg_(nxt_m_n));
        }
    } else if (self.rb != m_n) {
        Seg* m_seg{ detail::NToSeg_(m_n) };

#if EnStaging
        if (0 < seg_work_cnt && detail::GetNColor_(m_n) == ref_color &&
            seg_works[seg_work_cnt - 1].ref_beg +
                    seg_works[seg_work_cnt - 1].ca.elem_cnt ==
                m_seg->ref.beg) {
            size_t old_l_elem_cnt{ seg_works[0].ca.elem_cnt };

            Seg* dst_seg;
            Seg* other_seg;

            switch (utils::Choose2(&random_seed)) {
            case utils::Choose2Result::Case0:
                dst_seg   = seg_works[0].seg;
                other_seg = m_seg;
                break;

            case utils::Choose2Result::Case1:
                dst_seg   = m_seg;
                other_seg = seg_works[0].seg;
                break;

            default: ZETA_Core_DebugUtils_Diag_Unreachable();
            }

            seg_works[0].is_dirty = true;
            seg_works[0].seg      = dst_seg;
            seg_works[0].ca.elem_cnt += m_seg->ref.elem_cnt;

            self.root = rbtree::Extract(&other_seg->n);
            detail::DestroyRefSeg_(other_seg, self_work.seg_alctr);

            seg_works[0].Store();

            pos_cursor->n                    = &seg_works[0].seg->n;
            pos_cursor->seg_idx              = old_l_elem_cnt;
            pos_cursor->elem_ptr_is_resolved = false;
            pos_cursor->elem_ptr             = nullptr;

            return;
        }
#endif

        detail::SegWork_<Elem>& seg_work{ seg_works[seg_work_cnt++] };
        seg_work.Load(m_seg);
    }

    Node* ret_n{ m_n };
    size_t ret_seg_idx{ 0 };

    auto merge_then_update_ret{ [&](unsigned i, unsigned j) {
        size_t old_i_seg_elem_cnt{ seg_works[i].ca.elem_cnt };

        switch (detail::Merge2_(self_work, seg_works[i], seg_works[j])) {
        case utils::Choose2Result::Case0:
            if (ret_n == &seg_works[j].seg->n) {
                ret_n = &seg_works[i].seg->n;
                ret_seg_idx += old_i_seg_elem_cnt;
            }

            break;

        case utils::Choose2Result::Case1:
            if (ret_n == &seg_works[i].seg->n) {
                ret_n = &seg_works[j].seg->n;
            } else if (ret_n == &seg_works[j].seg->n) {
                ret_seg_idx += old_i_seg_elem_cnt;
            }

            break;

        default: ZETA_Core_DebugUtils_Diag_Unreachable();
        }
    } };

    switch (seg_work_cnt) {
    case 0:
    case 1: break;

    case 2: {
        size_t elem_vac_01{ seg_works[0].elem_vac + seg_works[1].elem_vac };

        if (self_work.seg_elem_slot_cnt <= elem_vac_01) {
            merge_then_update_ret(0, 1);
        }

        break;
    }

    case 3: {
        size_t elem_vac_01{ seg_works[0].elem_vac + seg_works[1].elem_vac };
        size_t elem_vac_12{ seg_works[1].elem_vac + seg_works[2].elem_vac };

        size_t max_elem_vac{ comparison_utils::BasicMax(elem_vac_01,
                                                        elem_vac_12) };

        if (self_work.seg_elem_slot_cnt <= max_elem_vac) {
            unsigned k{ static_cast<unsigned>(
                utils::Choose2(elem_vac_01 == max_elem_vac,
                               elem_vac_12 == max_elem_vac, &random_seed)) };

            merge_then_update_ret(k, k + 1);
        }

        break;
    }

    case 4: {
        size_t elem_vac_01{ seg_works[0].elem_vac + seg_works[1].elem_vac };
        size_t elem_vac_12{ seg_works[1].elem_vac + seg_works[2].elem_vac };
        size_t elem_vac_23{ seg_works[2].elem_vac + seg_works[3].elem_vac };

        if (self_work.seg_elem_slot_cnt <= elem_vac_01 &&
            self_work.seg_elem_slot_cnt <= elem_vac_23) {
            merge_then_update_ret(0, 1);
            merge_then_update_ret(2, 3);
        } else {
            size_t max_elem_vac{ comparison_utils::BasicMax(
                elem_vac_01, elem_vac_12, elem_vac_23) };

            if (self_work.seg_elem_slot_cnt <= max_elem_vac) {
                unsigned k{ static_cast<unsigned>(utils::Choose3(
                    elem_vac_01 == max_elem_vac, elem_vac_12 == max_elem_vac,
                    elem_vac_23 == max_elem_vac, &random_seed)) };

                merge_then_update_ret(k, k + 1);
            }
        }

        break;
    }

    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

    for (size_t i{ 0 }; i < seg_work_cnt; ++i) {
        if (seg_works[i].ca.elem_cnt == 0) {
            self.root = rbtree::Extract(&seg_works[i].seg->n);
            detail::DestroySeg_(seg_works[i].seg, self_work.seg_alctr,
                                self_work.data_alctr);
        }
    }

    for (size_t i{ 0 }; i < seg_work_cnt; ++i) {
        if (0 < seg_works[i].ca.elem_cnt && seg_works[i].is_dirty) {
            seg_works[i].Store();
        }
    }

    pos_cursor->n       = ret_n;
    pos_cursor->seg_idx = ret_seg_idx;

    if (self.rb == ret_n) {
        pos_cursor->elem_ptr_is_resolved = false;
        pos_cursor->elem_ptr             = nullptr;
        return;
    }

    for (size_t i{ 0 }; i < seg_work_cnt; ++i) {
        if (&seg_works[i].seg->n != pos_cursor->n) { continue; }

#if EnStaging
        if (seg_works[i].color == ref_color) {
            pos_cursor->elem_ptr_is_resolved = false;
            pos_cursor->elem_ptr             = nullptr;
        } else
#endif
        {
            pos_cursor->elem_ptr_is_resolved = true;

            pos_cursor->elem_ptr = ({
                resolving::Packet<Elem, Elem> tmp{
                    resolving::Packet<Elem, Elem>::MakeRef()
                };

                seq_cntr::Refer(seg_works[i].ca, ret_seg_idx, nullptr, &tmp);

                tmp.dst_ref.ptr;
            });
        }

        break;
    }
}

template <CntrTplParamList>
template <seq_endpoint::acceptor::IsAcceptor<
    typename Namespace::Cntr<CntrTplArgList>::Elem>
              Acceptor>
constexpr void Namespace::Cntr<CntrTplArgList>::EraseAll(
    this meta::IsNonConstWithoutReference auto&& self, seq_cntr::Tag,
    Acceptor&& acceptor) {
    detail::CheckCntr_(self);

    detail::EraseAll_(self, acceptor);

    detail::InitTree_(self);
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::Reset(this Cntr& self) {
    detail::CheckCntr_(self);

    detail::EraseAll_(self, seq_endpoint::acceptor::BasicAcceptor{});

    detail::InitTree_(self);

#if EnStaging
    detail::RefOrigin_(self);
#endif
}

#if EnStaging

template <CntrTplParamList>
template <typename OriginOriginLike, typename OriginSegAllocator,
          typename OriginDataAllocator, typename NewOriginLikeConstructArg>
constexpr void Namespace::Cntr<CntrTplArgList>::Collapse(
    this Cntr& self,
    Cntr<OriginOriginLike, OriginSegAllocator, OriginDataAllocator> const&
        origin_cntr,
    NewOriginLikeConstructArg&& new_origin_like_construct_arg) {
    detail::CheckCntr_(self);

    auto& origin{ meta::GetInstRefFromLike(self.origin_like) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::GetReferedInstPtr(origin) ==
        seq_cntr::GetReferedInstPtr(origin_cntr));

    self.origin_like.~decltype(self.origin_like)();

    new (&self.origin_like)
        OriginOriginLike{ ZETA_Core_Lifecycle_UnpackConstructArgs(
            OriginOriginLike, NewOriginLikeConstructArg,
            new_origin_like_construct_arg) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::GetReferedInstPtr(origin) ==
        seq_cntr::GetReferedInstPtr(
            meta::GetInstRefFromLike(origin_cntr.origin_like)));

    auto const self_work{ detail::MakeCntrWork_(self) };

    Node* n{ bin_tree::StepR(self.lb) };

    Cursor origin_cursor;
    seq_cntr::PeekL(origin_cntr, &origin_cursor, nullptr);

    CircularArray<Elem> origin_ca{
        .data{ nullptr },
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{ 0 },
        .slot_cnt{ origin_cntr.seg_elem_slot_cnt },
        .rot{ 0 },
    };

    while (n != self.rb) {
        Seg* seg{ detail::NToSeg_(n) };

        if (detail::GetNColor_(n) == dat_color) {
            n = bin_tree::StepR(n);
            continue;
        }

        origin_cntr.CursorAdvanceR(seq_cntr::Tag{}, &origin_cursor,
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
                n            = bin_tree::StepR(n);
                continue;
            }

            Seg* new_ref_seg{ detail::CreateRefSeg_(self_work.seg_alctr) };

            new_ref_seg->ref.beg      = origin_seg->ref.beg + origin_seg_idx;
            new_ref_seg->ref.elem_cnt = origin_res_size;

            bin_tree::AddDiffSize(&new_ref_seg->n, new_ref_seg->ref.elem_cnt);

            seg->ref.beg += origin_res_size;
            seg->ref.elem_cnt -= origin_res_size;

            bin_tree::AddDiffSize(n, -origin_res_size);

            self.root = rbtree::InsertL(n, &new_ref_seg->n);

            continue;
        }

        origin_ca.data     = origin_seg->dat.data;
        origin_ca.elem_cnt = origin_seg->dat.elem_cnt;
        origin_ca.rot      = origin_seg->dat.rot;

        bool cur_seg_will_exhausted{ seg->ref.elem_cnt <= origin_res_size };

        detail::ElemCntBalancer_ balancer{
            comparison_utils::BasicMin(seg->ref.elem_cnt, origin_res_size),
            self_work.seg_elem_slot_cnt
        };

        size_t src_seg_idx{ origin_seg_idx };

        size_t old_seg_ref_elem_cnt{ seg->ref.elem_cnt };

        while (0 < balancer.res_seg_cnt) {
            size_t cur_cnt{ balancer.Fetch() };

            void* dst_data;

            if (cur_cnt == seg->ref.elem_cnt) {
                detail::SetNColor_(n, dat_color);

                seg->dat.data     = detail::AllocateData_(self_work.data_size,
                                                          self_work.data_alctr);
                seg->dat.elem_cnt = static_cast<unsigned short>(cur_cnt);
                seg->dat.rot      = 0;

                bin_tree::AddDiffSize(n, cur_cnt - old_seg_ref_elem_cnt);

                dst_data = seg->dat.data;
            } else {
                Seg* new_dat_seg{ detail::CreateDatSeg_(self_work.data_size,
                                                        self_work.seg_alctr,
                                                        self_work.data_alctr) };

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

                self.root = rbtree::InsertL(n, &new_dat_seg->n);
            }

            origin_ca.IdxRead(src_seg_idx, cur_cnt,
                              lin_seq_endpoint::acceptor::Acceptor<
                                  lifecycle::ConstructingMemToObjView<Elem>>{
                                  .data{ dst_data },
                                  .elem_stride{ sizeof(Elem) },
                                  .elem_cnt{ seq_cntr::max_max_elem_cnt },
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
        return comparison::BasicCompare(
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
    return comparison::BasicCompare(
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

            ghtn->Construct();

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

        allocator::Deallocate(cntr.seg_alctr_like, seg);

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
    auto& origin{ meta::GetInstRefFromLike(cntr.origin_like) };
    DataAllocatorLike* data_alctr{
meta::GetInstPtrFromLike(cntr.data_alctr_like) };

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
    auto* origin{ GetInstrPtr(cntr.origin_like) };

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

        generic_hash_table::Construct(&ght);

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

    CircularArray<Elem> ca{
        .data { nullptr },
        .elem_size { origin->elem_size},
        .elem_stride { stride},
        .elem_cnt { 0 },
        .slot_cnt { cntr.seg_elem_slot_cnt},
        .rot { 0 },
    };

    WriteWBSeg_(cntr, ca, wb_segs, seg_cnt, 0, push_l_cnt);

    allocator::Deallocate(allocator::weak_lifo_allocator, lb_wb_seg);

    if (0 < pop_l_cnt) { SeqCntrPopL(origin, pop_l_cnt); }

    if (0 < pop_r_cnt) { SeqCntrPopR(origin, pop_r_cnt); }

    ConstructTree_(cntr);
    RefOrigin_(cntr);
}

template <CntrTplParamList>
void WriteBack_Random_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList>& cntr, unsigned long long cost_coeff_read,
     unsigned long long cost_coeff_write, unsigned long long cost_coeff_insert,
     unsigned long long cost_coeff_erase) {
    auto* origin{ GetInstrPtr(cntr.origin_like) };
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

    CircularArray<Elem> ca{
        .data { nullptr },
        .elem_size { origin->elem_size},
        .elem_stride { stride},
        .elem_cnt { 0 },
        .slot_cnt { cntr.seg_elem_slot_cnt},
        .rot { 0 },
    };

    if (ref_seg_cnt == 0) {
        if (origin_size < size) {
            SeqCntrPopR(origin, size - origin_size);
        } else if (size < origin_size) {
            SeqCntrPushR(origin, origin_size - size, nullptr);
        }

        WriteWBSeg_(cntr, ca, wb_segs, seg_cnt, 0, 0);

        allocator::Deallocate(allocator::weak_lifo_allocator, wb_segs - 1);

        ConstructTree_(cntr);
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

#if ZETA_Core_DebugEnable
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

#if ZETA_Core_DebugEnable
    for (size_t i{ 1 }; i <= ref_seg_cnt + 1; ++i) {
        unsigned long long ans_cost{ ZETA_Core_ullong_max };

        for (size_t j{ i }, j_end{ 0 }; j_end < j--;) {
            size_t sum_arr{ acc_arr[i] - acc_arr[j] };

            unsigned long long cur_cost{ dp_best_cost[j] +
                                         (sum_arr <= ZETA_Core_size_max / 2
                                              ? cost_coeff_erase * sum_arr
                                              : cost_coeff_insert * -sum_arr) +
                                         acc_brr[i - 1] - acc_brr[j] };

            ans_cost = comparison_utils::BasicMin(ans_cost, cur_cost);
        }

        dp_best_cost[i] = ans_cost;
    }
#endif

    for (size_t i{ 1 }; i <= ref_seg_cnt + 1; ++i) {
        unsigned long long ans_cost{ ZETA_Core_ullong_max };
        unsigned long long ans_prv{ 0 };

        for (size_t j{ i }, j_end{ comparison_utils::BasicMax(8ULL, i) - 8 };
j_end < j--;) { size_t sum_arr{ acc_arr[i] - acc_arr[j] };

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

#if ZETA_Core_DebugEnable
    {
        unsigned long long best_cost{ dp_best_cost[ref_seg_cnt + 1] };
        unsigned long long better_cost{ dp_cost[ref_seg_cnt + 1] };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(best_cost <= 512 || better_cost
<= best_cost * 2);
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

#if ZETA_Core_DebugEnable
    allocator::Deallocate(allocator::weak_lifo_allocator, dp_best_cost);
#endif

    allocator::Deallocate(allocator::weak_lifo_allocator, dp_cost);
    allocator::Deallocate(allocator::weak_lifo_allocator, dp_prv);

    ConstructTree_(cntr);
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

    auto* origin{ GetInstrPtr(cntr.origin_like) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(write_back_strategy ==
WriteBackStrategy::L
||    // write_back_strategy == WriteBackStrategy::R ||    //
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
constexpr void Namespace::Cntr<CntrTplArgList>::CopyCursor(
    this Cntr const& cntr, seq_cntr::Tag, Cursor const* src_cursor,
    Cursor* dst_cursor) {
    detail::CheckCursor_(cntr, src_cursor);
    detail::CheckCursor_(cntr, dst_cursor);

    *dst_cursor = *src_cursor;
}

template <CntrTplParamList>
constexpr bool Namespace::Cntr<CntrTplArgList>::AreEqualCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) ==
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_b);
}

template <CntrTplParamList>
constexpr comparison::Ordering Namespace::Cntr<CntrTplArgList>::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return comparison::BasicCompare(
        comparison::OpTags::Order{},
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_a) + 1,
        self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) + 1);
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetCursorDist(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor_a,
    Cursor const* cursor_b) {
    return self.GetCursorIdx(seq_cntr::Tag{}, cursor_b) -
           self.GetCursorIdx(seq_cntr::Tag{}, cursor_a);
}

template <CntrTplParamList>
constexpr size_t Namespace::Cntr<CntrTplArgList>::GetCursorIdx(
    this Cntr const& self, seq_cntr::Tag, Cursor const* cursor) {
    detail::CheckCursor_(self, cursor);

    return cursor->idx;
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::CursorStepL(
    this Cntr const& self, seq_cntr::Tag, Cursor* cursor) {
    self.CursorAdvanceL(seq_cntr::Tag{}, cursor, 1);
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::CursorStepR(
    this Cntr const& self, seq_cntr::Tag, Cursor* cursor) {
    self.CursorAdvanceR(seq_cntr::Tag{}, cursor, 1);
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::CursorAdvanceL(
    this Cntr const& self, seq_cntr::Tag, Cursor* cursor, size_t step) {
    detail::CheckCursor_(self, cursor);

    if (step == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= cursor->idx + 1);

    Node* n{ static_cast<Node*>(cursor->n) };

    size_t n_size;

    n_size = self.lb == n || self.rb == n ? 1 : ({
        Seg* seg{ detail::NToSeg_(n) };
        EnStagingTernary(detail::GetNColor_(n) == ref_color, seg->ref.elem_cnt,
                         seg->dat.elem_cnt);
    });

    auto [dst_n, dst_seg_idx]{ bin_tree::AdvanceL(
        n, n_size - 1 - cursor->seg_idx + step) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_n != nullptr);

    cursor->idx -= step;
    cursor->n = dst_n;

    if (self.lb == dst_n) {
        cursor->seg_idx              = 0;
        cursor->elem_ptr_is_resolved = true;
        cursor->elem_ptr             = nullptr;
        return;
    }

    Seg* dst_seg{ detail::NToSeg_(dst_n) };

    cursor->seg_idx =
        EnStagingTernary(detail::GetNColor_(dst_n) == ref_color,
                         dst_seg->ref.elem_cnt, dst_seg->dat.elem_cnt) -
        1 - dst_seg_idx;

#if EnStaging
    if (detail::GetNColor_(dst_n) == ref_color) {
        cursor->elem_ptr_is_resolved = false;
        cursor->elem_ptr             = nullptr;
        return;
    }
#endif

    CircularArray<Elem> ca{
        .data{ static_cast<Elem*>(dst_seg->dat.data) },
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{ dst_seg->dat.elem_cnt },
        .slot_cnt{ self.seg_elem_slot_cnt },
        .rot{ dst_seg->dat.rot },

#if ZETA_Core_DebugEnable
        .stamp{ debug_utils::revision::GetStamp() },
#endif
    };

    cursor->elem_ptr_is_resolved = true;

    cursor->elem_ptr = ({
        resolving::Packet<Elem, Elem> tmp{
            resolving::Packet<Elem, Elem>::MakeRef()
        };

        seq_cntr::Refer(ca, cursor->seg_idx, nullptr, &tmp);

        tmp.dst_ref.ptr;
    });
}

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::CursorAdvanceR(
    this Cntr const& self, seq_cntr::Tag, Cursor* cursor, size_t step) {
    detail::CheckCursor_(self, cursor);

    if (step == 0) { return; }

    size_t elem_cnt{ self.GetElemCnt(seq_cntr::Tag{}) };

    Node* rb{ self.rb };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(step <= elem_cnt - cursor->idx);

    auto [dst_n, dst_seg_idx]{ bin_tree::AdvanceR(static_cast<Node*>(cursor->n),
                                                  cursor->seg_idx + step) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_n != nullptr);

    cursor->idx += step;
    cursor->n       = dst_n;
    cursor->seg_idx = dst_seg_idx;

    if (rb == dst_n) {
        cursor->elem_ptr_is_resolved = true;
        cursor->elem_ptr             = nullptr;
        return;
    }

#if EnStaging
    if (detail::GetNColor_(dst_n) == ref_color) {
        cursor->elem_ptr_is_resolved = false;
        cursor->elem_ptr             = nullptr;
        return;
    }
#endif

    Seg* dst_seg{ detail::NToSeg_(dst_n) };

    CircularArray<Elem> ca{
        .data{ static_cast<Elem*>(dst_seg->dat.data) },
        .elem_stride{ sizeof(Elem) },
        .elem_cnt{ dst_seg->dat.elem_cnt },
        .slot_cnt{ self.seg_elem_slot_cnt },
        .rot{ dst_seg->dat.rot },

#if ZETA_Core_DebugEnable
        .stamp{ debug_utils::revision::GetStamp() },
#endif
    };

    cursor->elem_ptr_is_resolved = true;

    cursor->elem_ptr = ({
        resolving::Packet<Elem, Elem> tmp{
            resolving::Packet<Elem, Elem>::MakeRef()
        };

        seq_cntr::Refer(ca, cursor->seg_idx, nullptr, &tmp);

        tmp.dst_ref.ptr;
    });
}

namespace Namespace::detail {

template <CntrTplParamList>
constexpr void PrintState_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& self, Node* n) {
    Node* l_n{ bin_tree::GetL(n) };
    Node* r_n{ bin_tree::GetR(n) };

    if (l_n != nullptr) { (PrintState_)(self, l_n); }

    if (self.lb != n && self.rb != n) {
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

    if (r_n != nullptr) { (PrintState_)(self, r_n); }
}

}  // namespace Namespace::detail

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::PrintState(
    this Cntr const& self) {
    detail::CheckCntr_(self);

    detail::PrintState_(self, self.root);
}

namespace Namespace::detail {

template <CntrTplParamList>
constexpr Stats GetStats_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& self, Node* n) {
    Stats ret{
#if EnStaging
        .ref_seg_cnt{ 0 },
#endif
        .dat_seg_cnt{ 0 },
#if EnStaging
        .ref_size{ 0 },
#endif
        .dat_size{ 0 },
    };

    for (;;) {
        if (self.lb != n && self.rb != n) {
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

        Stats l_stats{ (GetStats_)(self, l_n) };

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
constexpr Namespace::Stats Namespace::Cntr<CntrTplArgList>::GetStats(
    this Cntr const& self) {
    detail::CheckCntr_(self);

    Node* root{ self.root };

    if (root == nullptr) {
        return {
#if EnStaging
            .ref_seg_cnt{ 0 },
#endif

            .dat_seg_cnt{ 0 },

#if EnStaging
            .ref_size{ 0 },
#endif

            .dat_size{ 0 },
        };
    }

    return detail::GetStats_(self, self.root);
}

namespace Namespace::detail {

struct SanityCheckRet_ {
    bool b;
    size_t ref_beg;
    size_t ref_end;
};

template <CntrTplParamList>
constexpr SanityCheckRet_ SanityCheck_  // NOLINT(misc-use-internal-linkage)
    (Cntr<CntrTplArgList> const& self,
     debug_utils::mem_recording::MemRecorder& seg_mr,
     debug_utils::mem_recording::MemRecorder& data_mr, Node* n) {
#if EnStaging
    using Elem = Cntr<CntrTplArgList>::Elem;

    auto& origin{ meta::GetInstRefFromLike(self.origin_like) };
    size_t origin_elem_cnt{ seq_cntr::GetElemCnt(origin) };
#endif

    size_t seg_elem_slot_cnt{ self.seg_elem_slot_cnt };
    size_t data_size{ sizeof(Elem) * seg_elem_slot_cnt };

    Node* l_n{ bin_tree::StepL(n) };
    Node* r_n{ bin_tree::StepR(n) };

    if (self.lb == n) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_n == nullptr);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(r_n != nullptr);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetSize(n) == 1);

        seg_mr.Add(self.lb, sizeof(Node));
    } else if (self.rb == n) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_n != nullptr);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(r_n == nullptr);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(bin_tree::GetSize(n) == 1);

        seg_mr.Add(self.rb, sizeof(Node));
    }
#if EnStaging
    else if (GetNColor_(n) == ref_color) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_n != nullptr);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(r_n != nullptr);

        Seg* seg{ (NToSeg_)(n) };

        seg_mr.Add(seg, sizeof(Seg));

        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < seg->ref.elem_cnt);

        ZETA_Core_DebugUtils_Diag_PromiseAssert(seg->ref.beg < origin_elem_cnt);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            seg->ref.beg + seg->ref.elem_cnt <= origin_elem_cnt);

        size_t size{ seg->ref.elem_cnt };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(size == bin_tree::GetSize(n));

        if (self.lb != l_n) {
            Seg* l_seg{ (NToSeg_)(l_n) };

            if (GetNColor_(&l_seg->n) == ref_color) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    l_seg->ref.beg + l_seg->ref.elem_cnt < seg->ref.beg);
            }
        }

        if (self.rb != r_n) {
            Seg* r_seg{ (NToSeg_)(r_n) };

            if ((GetNColor_)(&r_seg->n) == ref_color) {
                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    seg->ref.beg + seg->ref.elem_cnt < r_seg->ref.beg);
            }
        }
    }
#endif
    else {
#if EnStaging
        ZETA_Core_DebugUtils_Diag_PromiseAssert((GetNColor_)(n) == dat_color);
#endif

        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_n != nullptr);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(r_n != nullptr);

        Seg* seg{ (NToSeg_)(n) };

        seg_mr.Add(seg, sizeof(Seg));

        size_t elem_cnt{ seg->dat.elem_cnt };

        ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt ==
                                                bin_tree::GetSize(n));

        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < elem_cnt);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_cnt <= seg_elem_slot_cnt);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(seg->dat.rot <
                                                seg_elem_slot_cnt);

        data_mr.Add(seg->dat.data, data_size);

        size_t l_vac;
        size_t r_vac;

        if (self.lb == l_n) {
            l_vac = 0;
        } else {
            Seg* l_seg{ (NToSeg_)(l_n) };

            l_vac =
                seg_elem_slot_cnt -
                EnStagingTernary(GetNColor_(l_n) == ref_color,
                                 comparison_utils::BasicMin(l_seg->ref.elem_cnt,
                                                            seg_elem_slot_cnt),
                                 l_seg->dat.elem_cnt);
        }

        if (self.rb == r_n) {
            r_vac = 0;
        } else {
            Seg* r_seg{ (NToSeg_)(r_n) };

            r_vac =
                seg_elem_slot_cnt -
                EnStagingTernary(GetNColor_(r_n) == ref_color,
                                 comparison_utils::BasicMin(r_seg->ref.elem_cnt,
                                                            seg_elem_slot_cnt),
                                 r_seg->dat.elem_cnt);
        }

        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 <= l_vac);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(0 <= r_vac);
    }

    Node* sub_l_n{ bin_tree::GetL(n) };
    Node* sub_r_n{ bin_tree::GetR(n) };

    SanityCheckRet_ l_check_ret{ false, 0, 0 };
    SanityCheckRet_ r_check_ret{ false, 0, 0 };

    if (sub_l_n != nullptr) {
        l_check_ret = (SanityCheck_)(self, seg_mr, data_mr, sub_l_n);
    }

    if (sub_r_n != nullptr) {
        r_check_ret = (SanityCheck_)(self, seg_mr, data_mr, sub_r_n);
    }

    SanityCheckRet_ ret{ false, 0, 0 };

    if (l_check_ret.b && r_check_ret.b) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(l_check_ret.ref_end <=
                                                r_check_ret.ref_beg);

        ret.b       = true;
        ret.ref_beg = l_check_ret.ref_beg;
        ret.ref_end = r_check_ret.ref_end;
    } else if (l_check_ret.b) {
        ret = l_check_ret;
    } else if (r_check_ret.b) {
        ret = r_check_ret;
    }

    if (self.lb == n || self.rb == n) { return ret; }

#if EnStaging
    if (GetNColor_(n) == ref_color) {
        Seg* seg{ (NToSeg_)(n) };

        size_t cur_ref_beg{ seg->ref.beg };
        size_t cur_ref_end{ seg->ref.beg + seg->ref.elem_cnt };

        if (l_check_ret.b) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(l_check_ret.ref_end <=
                                                    cur_ref_beg);
        }
        if (r_check_ret.b) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(cur_ref_end <=
                                                    r_check_ret.ref_beg);
        }

        if (ret.b) {
            ret.ref_beg = comparison_utils::BasicMin(ret.ref_beg, cur_ref_beg);
            ret.ref_end = comparison_utils::BasicMax(ret.ref_end, cur_ref_end);
        } else {
            ret.b       = true;
            ret.ref_beg = cur_ref_beg;
            ret.ref_end = cur_ref_end;
        }
    }
#endif

    return ret;
}

}  // namespace Namespace::detail

template <CntrTplParamList>
constexpr void Namespace::Cntr<CntrTplArgList>::SanityCheck(
    this Cntr const& self, debug_utils::mem_recording::MemRecorder& seg_mr,
    debug_utils::mem_recording::MemRecorder& data_mr) {
#if !ZETA_Core_DebugEnable
    ZETA_Core_Unused(cntr);
    ZETA_Core_Unused(dst_seg);
    ZETA_Core_Unused(dst_data);
#else
    detail::CheckCntr_(self);

    bin_tree::Sanitize(self.root);

    {
        debug_utils::mem_recording::MemRecorder rbtn_mr;
        rbtree::SanityCheck(rbtn_mr, self.root);
    }

    detail::SanityCheck_(self, seg_mr, data_mr, self.root);
#endif
}

}  // namespace zeta::core

#pragma pop_macro("Namespace")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")
#pragma pop_macro("EnStagingTernary")

// NOLINTEND(cppcoreguidelines-pro-type-union-access)
