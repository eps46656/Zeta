#if ZETA_Core_Editor
#define EnStaging 1
#endif

#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

#pragma push_macro("Skip")

#if EnStaging

#if defined(ZETA_MacroGuard__seg_vector_mpp_hpp__staging_seg_vector)
#define Skip 1
#else
#define ZETA_MacroGuard__seg_vector_mpp_hpp__staging_seg_vector
#define Skip 0
#endif

#else

#if defined(ZETA_MacroGuard__seg_vector_mpp_hpp__seg_vector)
#define Skip 1
#else
#define ZETA_MacroGuard__seg_vector_mpp_hpp__seg_vector
#define Skip 0
#endif

#endif

#if !Skip

#include <zeta/core/allocator.hpp>
#include <zeta/core/basic_bin_tree_node.hpp>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pool_allocator.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.hpp>

#if EnStaging

#pragma push_macro("Namespace")
#define Namespace staging_seg_vector

#else

#pragma push_macro("Namespace")
#define Namespace seg_vector

#endif

#if EnStaging

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList(suffix)                                    \
    typename OriginLike##suffix, typename SegAllocatorLike##suffix, \
        typename DataAllocatorLike##suffix

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList OriginLike, SegAllocatorLike, DataAllocatorLike

#else

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList(suffix) \
    typename SegAllocatorLike##suffix, typename DataAllocatorLike##suffix

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList SegAllocatorLike, DataAllocatorLike

#endif

namespace zeta::core::Namespace {

constexpr size_t max_seg_elem_slot_cnt{ integral::RangeMaxOf<unsigned short> };

#if EnStaging
static constexpr unsigned ref_color{ 1 };
static constexpr unsigned dat_color{ 2 };
static constexpr unsigned null_color{ 3 };
#endif

using CircularArray = circular_array::Cntr;

using Node =
#if EnStaging
    basic_bin_tree_node::Node<
        void*,
        meta::AutoValueWrapper<true>,                // PColor
        meta::AutoValueWrapper<true>,                // LColor
        meta::AutoValueWrapper<false>,               // RColor
        meta::AutoValueWrapper<true>,                // AccSize
        basic_bin_tree_node::PrimaryColorTagEnum::P  // PrimaryColor
        >;
#else
    basic_bin_tree_node::Node<
        void*,
        meta::AutoValueWrapper<true>,                // PColor
        meta::AutoValueWrapper<true>,                // LColor
        meta::AutoValueWrapper<false>,               // RColor
        meta::AutoValueWrapper<true>,                // AccSize
        basic_bin_tree_node::PrimaryColorTagEnum::P  // PrimaryColor
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
};

struct Cursor {
    void const* cntr;
    size_t idx;
    void* n;
    size_t seg_idx;

    void* elem_ptr;
    bool elem_ptr_is_valid;

    bool operator==(Cursor const&) const = default;
    bool operator!=(Cursor const&) const = default;
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

template <CntrTplParamList(_)>
struct Cntr {
#if EnStaging
    using OriginLike = OriginLike_;
#endif

    using SegAllocatorLike = SegAllocatorLike_;
    using DataAllocatorLike = DataAllocatorLike_;

    size_t elem_size;
    size_t elem_stride;
    size_t seg_elem_slot_cnt;

    Node* lb;
    Node* rb;

    Node* root;

#if EnStaging
    OriginLike origin_like;
#endif

    SegAllocatorLike seg_alctr_like;
    DataAllocatorLike data_alctr_like;

    template <
#if EnStaging
        typename OriginLikeInitArg,
#endif
        typename SegAllocatorLikeInitArg, typename DataAllocatorLikeInitArg>
    constexpr Cntr(
#if !EnStaging
        size_t elem_size,
#endif
        size_t elem_stride, size_t seg_elem_slot_cnt,
#if EnStaging
        OriginLikeInitArg&& origin_like_init_arg,
#endif
        SegAllocatorLikeInitArg&& seg_alctr_like_init_arg,
        DataAllocatorLikeInitArg&& data_alctr_like_init_arg);

    template <
#if EnStaging
        typename OriginLikeInitArg,
#endif
        typename SegAllocatorLikeInitArg, typename DataAllocatorLikeInitArg,
        typename SrcOriginLike, typename SrcSegAllocatorLike,
        typename SrcDataAllocatorLike>
    constexpr Cntr(
        size_t elem_stride, size_t seg_elem_slot_cnt,
#if EnStaging
        OriginLikeInitArg&& origin_like_init_arg,
#endif
        SegAllocatorLikeInitArg&& seg_alctr_like_init_arg,
        DataAllocatorLikeInitArg&& data_alctr_like_init_arg,
        Cntr<
#if EnStaging
            SrcOriginLike,
#endif
            SrcSegAllocatorLike, SrcDataAllocatorLike> const& src_cntr);

#if EnStaging
    template <typename OriginLikeInitArg>
#endif
    constexpr Cntr(Cntr const& src_cntr);

    constexpr ~Cntr();

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr::Tag);

    static constexpr seq_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& cntr, seq_cntr::Tag);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr size_t GetCursorSize(seq_cntr::Tag);

    constexpr size_t GetElemSize(this Cntr const& cntr, seq_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& cntr, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& cntr, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void PeekL(this auto& cntr, seq_cntr::Tag, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this auto& cntr, seq_cntr::Tag, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Refer(this auto& cntr, seq_cntr::Tag, size_t idx,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this auto& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                           bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <typename Predictor>
    constexpr void* FindFirst(this Cntr& cntr, seq_cntr::Tag,
                              Predictor&& predictor, Cursor* dst_cursor,
                              void* dst_elem);

    template <typename Reader>
    constexpr void Read(this Cntr const& cntr, seq_cntr::Tag,
                        Cursor* pos_cursor, size_t cnt, Reader&& reader,
                        Cursor* dst_cursor);

    template <typename Writer>
    constexpr void Write(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                         size_t cnt, Writer&& writer, Cursor* dst_cursor);

    template <typename ReaderWriter>
    constexpr void ReadWrite(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                             size_t cnt, ReaderWriter&& reader_writer,
                             Cursor* dst_cursor);

    template <typename Writer>
    constexpr void PushL(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                         Writer&& writer, Cursor* dst_cursor);

    template <typename Writer>
    constexpr void PushR(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                         Writer&& writer, Cursor* dst_cursor);

    template <typename Writer>
    constexpr void Insert(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                          size_t cnt, Writer&& writer, Cursor* dst_cursor);

    template <typename Reader>
    constexpr void PopL(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                        Reader&& reader);

    template <typename Reader>
    constexpr void PopR(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                        Reader&& reader);

    template <typename Reader>
    constexpr void Erase(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                         size_t cnt, Reader&& reader);

    constexpr void EraseAll(this Cntr& cntr, seq_cntr::Tag);

    constexpr void Reset(this Cntr& cntr);

#if EnStaging
    template <typename OriginOriginLike, typename OriginSegAllocator,
              typename OriginDataAllocator, typename NewOriginLikeInitArg>
    constexpr void Collapse(this Cntr& cntr,
                            Cntr<OriginOriginLike, OriginSegAllocator,
                                 OriginDataAllocator> const& origin_cntr,
                            NewOriginLikeInitArg&& new_origin_like_init_arg);

    constexpr void WriteBack(this Cntr& cntr, int write_back_strategy,
                             unsigned long long cost_coeff_read,
                             unsigned long long cost_coeff_write,
                             unsigned long long cost_coeff_insert,
                             unsigned long long cost_coeff_erase);
#endif

    constexpr void CopyCursor(this Cntr const& cntr, seq_cntr::Tag,
                              Cursor* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor* cursor_a, Cursor* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& cntr,
                                                 seq_cntr::Tag,
                                                 Cursor* cursor_a,
                                                 Cursor* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& cntr, seq_cntr::Tag,
                                   Cursor* cursor_a, Cursor* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor* cursor);

    constexpr void CursorStepL(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void PrintState(this Cntr const& cntr);

    constexpr Stats GetStats(this Cntr const& cntr);

    constexpr void Sanitize(this Cntr const& cntr,
                            mem_recorder::MemRecorder* dst_seg,
                            mem_recorder::MemRecorder* dst_data);
};

}  // namespace zeta::core::Namespace

#pragma pop_macro("Namespace")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")

#endif

#pragma pop_macro("Skip")
