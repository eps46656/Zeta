#if ZETA_Core_Editor
#define EnStaging 1
#endif

#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

#if EnStaging

#if !defined(ZETA_MacroGuard__seg_vector_mpp_hpp__staging_seg_vector)
#define ZETA_MacroGuard__seg_vector_mpp_hpp__staging_seg_vector 1
#endif

#else

#if !defined(ZETA_MacroGuard__seg_vector_mpp_hpp__seg_vector)
#define ZETA_MacroGuard__seg_vector_mpp_hpp__seg_vector 1
#endif

#endif

#if ZETA_MacroGuard__seg_vector_mpp_hpp__staging_seg_vector == 1 || \
    ZETA_MacroGuard__seg_vector_mpp_hpp__seg_vector == 1

#if ZETA_MacroGuard__seg_vector_mpp_hpp__staging_seg_vector == 1
#undef ZETA_MacroGuard__seg_vector_mpp_hpp__staging_seg_vector
#define ZETA_MacroGuard__seg_vector_mpp_hpp__staging_seg_vector 2
#endif

#if ZETA_MacroGuard__seg_vector_mpp_hpp__seg_vector == 1
#undef ZETA_MacroGuard__seg_vector_mpp_hpp__seg_vector
#define ZETA_MacroGuard__seg_vector_mpp_hpp__seg_vector 2
#endif

#include <zeta/core/allocator.hpp>
#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
// #include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pool_allocator.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.hpp>

ZETA_Core_ClangdPreambleBarrier;

#if EnStaging

#pragma push_macro("Namespace")
#define Namespace staging_seg_vector

#else

#pragma push_macro("Namespace")
#define Namespace seg_vector

#endif

namespace zeta::core::Namespace {

constexpr size_t max_seg_elem_slot_cnt{ integral::RangeMaxOf<unsigned short> };

#if EnStaging
static constexpr unsigned ref_color{ 1 };
static constexpr unsigned dat_color{ 2 };
static constexpr unsigned null_color{ 3 };
#endif

template <meta::IsObjectType Elem>
using CircularArray = circular_array::Cntr<Elem>;

using Node =
#if EnStaging
    basic_bin_tree_node::Node<
        void*,
        true,                                 // PColor
        true,                                 // LColor
        false,                                // RColor
        true,                                 // AccSize
        basic_bin_tree_node::PrimaryColor::P  // PrimaryColor
        >;
#else
    basic_bin_tree_node::Node<
        void*,
        true,                                 // PColor
        true,                                 // LColor
        false,                                // RColor
        true,                                 // AccSize
        basic_bin_tree_node::PrimaryColor::P  // PrimaryColor
        >;
#endif

struct Seg {
    ZETA_Core_DebugStructPadding;

    Node n;

    ZETA_Core_DebugStructPadding;

    union {
#if EnStaging
        struct {
            size_t beg;
            size_t elem_cnt;
        } ref;
#endif

        struct {
            void* data;
            unsigned short elem_cnt;
            unsigned short rot;
        } dat;
    };

    constexpr Seg();
};

struct Cursor {
    size_t idx;
    void* n;
    size_t seg_idx;

    mutable bool elem_ptr_is_resolved;
    mutable void* elem_ptr;

#if ZETA_Core_DebugEnable
    mutable debug_utils::revision::Stamp stamp;
#endif

    constexpr Cursor() = default;

    constexpr Cursor(Cursor const& src_cursor) = default;

    constexpr Cursor(seq_cntr::CursorLimit const& src_cursor);

    constexpr operator seq_cntr::CursorLimit(this Cursor const& self);

    constexpr Cursor& operator=(Cursor const& src_cursor) = default;

    constexpr Cursor& operator=(seq_cntr::CursorLimit const& src_cursor);

    constexpr bool operator==(Cursor const&) const = default;
    constexpr bool operator!=(Cursor const&) const = default;
};

#if EnStaging
struct WriteBackStrategy {
    static constexpr int L{ 0b001 };
    static constexpr int R{ 0b010 };
    static constexpr int LR{ 0b011 };
    static constexpr int Random{ 0b100 };
    static constexpr int Auto{ 0b111 };
};
#endif

struct Stats {
#if EnStaging
    size_t ref_seg_cnt;
#endif

    size_t dat_seg_cnt;

#if EnStaging
    size_t ref_size;
#endif

    size_t dat_size;
};

template <
#if EnStaging
    seq_cntr::IsSeqCntrLike OriginLike_,
#else
    meta::IsNonConstObjectType Elem_,
#endif
    allocator::IsAllocatorLike SegAllocatorLike_,
    allocator::IsAllocatorLike DataAllocatorLike_>
struct Cntr {
#if EnStaging
    using Elem = meta::GetTypeWrapperType<decltype(seq_cntr::GetElemType(
        meta::TypeWrapper<meta::GetInstTypeFromLike<OriginLike_>&>{}))>;
#else
    using Elem = Elem_;
#endif

#if EnStaging
    using OriginLike = OriginLike_;

    using Origin = meta::GetInstTypeFromLike<OriginLike>;

    using OriginCursor =
        meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
            meta::TypeWrapper<meta::GetInstTypeFromLike<OriginLike_>&>{}))>;
#endif

    using SegAllocatorLike  = SegAllocatorLike_;
    using DataAllocatorLike = DataAllocatorLike_;

    size_t seg_elem_slot_cnt;

    Node* lb;
    Node* rb;

    Node* root;

#if EnStaging
    OriginLike origin_like;
#endif

    SegAllocatorLike seg_alctr_like;
    DataAllocatorLike data_alctr_like;

#if ZETA_Core_DebugEnable
    debug_utils::revision::Stamp stamp;
#endif

    template <
#if EnStaging
        typename OriginLikeConstructArg,
#endif
        typename SegAllocatorLikeConstructArg,
        typename DataAllocatorLikeConstructArg>
    constexpr Cntr(
        size_t seg_elem_slot_cnt,
#if EnStaging
        OriginLikeConstructArg&& origin_like_construct_arg,
#endif
        SegAllocatorLikeConstructArg&& seg_alctr_like_construct_arg,
        DataAllocatorLikeConstructArg&& data_alctr_like_construct_arg);

    template <
#if EnStaging
        typename OriginLikeConstructArg,
#endif
        typename SegAllocatorLikeConstructArg,
        typename DataAllocatorLikeConstructArg, typename SrcOriginLike,
        typename SrcSegAllocatorLike, typename SrcDataAllocatorLike>
    constexpr Cntr(
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
            SrcSegAllocatorLike, SrcDataAllocatorLike> const& src_cntr);

#if EnStaging
    template <typename OriginLikeConstructArg>
#endif
    constexpr Cntr(Cntr const& src_cntr);

    constexpr ~Cntr();

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr&, Cntr&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <meta::IsAnySame<Cntr const&, Cntr const&&> TargetCntr>
    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr::Tag);

    static constexpr seq_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& self, seq_cntr::Tag);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    template <
        meta::IsAnySame<Cntr&, Cntr const&, Cntr&&, Cntr const&&> TargetCntr>
    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<TargetCntr>);

    constexpr size_t GetElemSize(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& self, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& self, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void PeekL(this auto&& self, seq_cntr::Tag, Cursor* dst_cursor,
                         resolving::Packet<Elem, Elem>* resolving_packet);

    constexpr void PeekR(this auto&& self, seq_cntr::Tag, Cursor* dst_cursor,
                         resolving::Packet<Elem, Elem>* resolving_packet);

    constexpr void Refer(this auto&& self, seq_cntr::Tag, size_t idx,
                         Cursor* dst_cursor,
                         resolving::Packet<Elem, Elem>* resolving_packet);

    constexpr void Derefer(this auto&& self, seq_cntr::Tag,
                           Cursor const* pos_cursor,
                           resolving::Packet<Elem, Elem>* resolving_packet);

    template <typename Predictor>
    constexpr void* FindFirst(this auto&& self, seq_cntr::Tag,
                              Predictor&& predictor, Cursor* dst_cursor,
                              resolving::Packet<Elem, Elem>* resolving_packet);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Read(this auto&& self, seq_cntr::Tag,
                        Cursor const* pos_cursor, size_t cnt,
                        Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Write(this meta::IsNonConstWithoutReference auto&& self,
                         seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                         Provider&& provider, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void ReadWrite(this meta::IsNonConstWithoutReference auto&& self,
                             seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                             Acceptor&& acceptor, Cursor* dst_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushL(this meta::IsNonConstWithoutReference auto&& self,
                         seq_cntr::Tag, size_t cnt, Provider&& provider,
                         Cursor* dst_beg_cursor, Cursor* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void PushR(this meta::IsNonConstWithoutReference auto&& self,
                         seq_cntr::Tag, size_t cnt, Provider&& provider,
                         Cursor* dst_beg_cursor, Cursor* dst_end_cursor);

    template <seq_endpoint::provider::IsProvider<Elem> Provider>
    constexpr void Insert(this meta::IsNonConstWithoutReference auto&& self,
                          seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                          Provider&& provider, Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopL(this meta::IsNonConstWithoutReference auto&& self,
                        seq_cntr::Tag, size_t cnt, Acceptor&& acceptor,
                        Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void PopR(this meta::IsNonConstWithoutReference auto&& self,
                        seq_cntr::Tag, size_t cnt, Acceptor&& acceptor,
                        Cursor* dst_cursor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void Erase(this meta::IsNonConstWithoutReference auto&& self,
                         seq_cntr::Tag, Cursor* pos_cursor, size_t cnt,
                         Acceptor&& acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
    constexpr void EraseAll(this meta::IsNonConstWithoutReference auto&& self,
                            seq_cntr::Tag, Acceptor&& acceptor);

    constexpr void Reset(this Cntr& self);

#if EnStaging
    template <typename OriginOriginLike, typename OriginSegAllocator,
              typename OriginDataAllocator, typename NewOriginLikeConstructArg>
    constexpr void Collapse(
        this Cntr& self,
        Cntr<OriginOriginLike, OriginSegAllocator, OriginDataAllocator> const&
            origin_cntr,
        NewOriginLikeConstructArg&& new_origin_like_construct_arg);

    constexpr void WriteBack(this Cntr& self, int write_back_strategy,
                             unsigned long long cost_coeff_read,
                             unsigned long long cost_coeff_write,
                             unsigned long long cost_coeff_insert,
                             unsigned long long cost_coeff_erase);
#endif

    constexpr void CopyCursor(this Cntr const& self, seq_cntr::Tag,
                              Cursor const* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& self,
                                                 seq_cntr::Tag,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& self, seq_cntr::Tag,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& self, seq_cntr::Tag,
                                  Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& self, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& self, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void PrintState(this Cntr const& self);

    constexpr Stats GetStats(this Cntr const& self);

    constexpr void SanityCheck(
        this Cntr const& self, debug_utils::mem_recording::MemRecorder& seg_mr,
        debug_utils::mem_recording::MemRecorder& data_mr);
};

}  // namespace zeta::core::Namespace

#pragma pop_macro("Namespace")

#endif
