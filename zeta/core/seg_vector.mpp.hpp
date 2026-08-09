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
    OriginLike origin;
#endif

    SegAllocatorLike seg_alctr;
    DataAllocatorLike data_alctr;

    template <
#if EnStaging
        typename OriginLikeInitArg,
#endif
        typename SegAllocatorLikeInitArg, typename DataAllocatorLikeInitArg>
    void Init(this Cntr& cntr,
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
    void Init(this Cntr& cntr, size_t elem_stride, size_t seg_elem_slot_cnt,
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

    void Deinit(this Cntr& cntr);

#if EnStaging
    template <typename OriginLikeInitArg>
#endif
    void Copy(this Cntr& cntr,
#if EnStaging
              OriginLikeInitArg&& origin_like_init_arg,
#endif
              Cntr const& src_cntr);

    void* GetReferedInstPtr(this Cntr const& cntr);

    constexpr size_t GetCursorSize(this Cntr const& cntr);

    size_t GetElemSize(this Cntr const& cntr);

    size_t GetElemCnt(this Cntr const& cntr);

    size_t GetMaxElemCnt(this Cntr const& cntr);

    void GetLBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    void GetRBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    void PeekL(this auto& cntr, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
               void* dst_elem);

    void PeekR(this auto& cntr, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
               void* dst_elem);

    void Refer(this auto& cntr, size_t idx, bool lazy_copy_elem,
               seq_cntr::ElemPtrView* dst_elem_ptr_view, Cursor* dst_cursor,
               void* dst_elem);

    void Derefer(this auto& cntr, Cursor* pos_cursor, bool lazy_copy_elem,
                 seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem);

    template <typename Predictor>
    void* FindFirst(this Cntr& cntr, Predictor&& predictor, Cursor* dst_cursor,
                    void* dst_elem);

    template <typename Reader>
    void Read(this Cntr const& cntr, Cursor* pos_cursor, size_t cnt,
              Reader&& reader, Cursor* dst_cursor);

    template <typename Writer>
    void Write(this Cntr& cntr, Cursor* pos_cursor, size_t cnt, Writer&& writer,
               Cursor* dst_cursor);

    template <typename ReaderWriter>
    void ReadWrite(this Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                   ReaderWriter&& reader_writer, Cursor* dst_cursor);

    template <typename Writer>
    void PushL(this Cntr& cntr, size_t cnt, Writer&& writer,
               Cursor* dst_cursor);

    template <typename Writer>
    void PushR(this Cntr& cntr, size_t cnt, Writer&& writer,
               Cursor* dst_cursor);

    template <typename Writer>
    void Insert(this Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                Writer&& writer, Cursor* dst_cursor);

    template <typename Reader>
    void PopL(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <typename Reader>
    void PopR(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <typename Reader>
    void Erase(this Cntr& cntr, Cursor* pos_cursor, size_t cnt,
               Reader&& reader);

    void EraseAll(this Cntr& cntr);

    void Reset(this Cntr& cntr);

#if EnStaging

    template <typename OriginOriginLike, typename OriginSegAllocator,
              typename OriginDataAllocator, typename NewOriginLikeInitArg>
    void Collapse(this Cntr& cntr,
                  Cntr<OriginOriginLike, OriginSegAllocator,
                       OriginDataAllocator> const& origin_cntr,
                  NewOriginLikeInitArg&& new_origin_like_init_arg);

    void WriteBack(this Cntr& cntr, int write_back_strategy,
                   unsigned long long cost_coeff_read,
                   unsigned long long cost_coeff_write,
                   unsigned long long cost_coeff_insert,
                   unsigned long long cost_coeff_erase);
#endif

    void CopyCursor(this Cntr const& cntr, Cursor* src_cursor,
                    Cursor* dst_cursor);

    bool AreEqualCursor(this Cntr const& cntr, Cursor* cursor_a,
                        Cursor* cursor_b);

    comparison::Ordering CompareCursor(this Cntr const& cntr, Cursor* cursor_a,
                                       Cursor* cursor_b);

    size_t GetCursorDist(this Cntr const& cntr, Cursor* cursor_a,
                         Cursor* cursor_b);

    size_t GetCursorIdx(this Cntr const& cntr, Cursor* cursor);

    void CursorStepL(this Cntr const& cntr, Cursor* cursor);

    void CursorStepR(this Cntr const& cntr, Cursor* cursor);

    void CursorAdvanceL(this Cntr const& cntr, Cursor* cursor, size_t step);

    void CursorAdvanceR(this Cntr const& cntr, Cursor* cursor, size_t step);

    void PrintState(this Cntr const& cntr);

    Stats GetStats(this Cntr const& cntr);

    void Sanitize(this Cntr const& cntr, mem_recorder::MemRecorder* dst_seg,
                  mem_recorder::MemRecorder* dst_data);
};

}  // namespace zeta::core::Namespace

namespace zeta::core {

template <CntrTplParamList()>
struct lifecycle::Traits<Namespace::Cntr<CntrTplArgList>> {
    template <typename... Args>
    static void Init(Namespace::Cntr<CntrTplArgList>& cntr, Args&&... args);

    static void Deinit(Namespace::Cntr<CntrTplArgList>& cntr);
};

template <CntrTplParamList()>
struct seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>
    : public seq_cntr::MemberFuncCntrTraitsAdapter<
          Namespace::Cntr<CntrTplArgList>, Namespace::Cursor> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        Namespace::Cntr<CntrTplArgList>& cntr);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(Namespace::Cntr<CntrTplArgList>& cntr);
};

template <CntrTplParamList()>
struct seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const>
    : public seq_cntr::MemberFuncCntrTraitsAdapter<
          Namespace::Cntr<CntrTplArgList> const, Namespace::Cursor> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        Namespace::Cntr<CntrTplArgList> const& cntr);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(
        Namespace::Cntr<CntrTplArgList> const& cntr);
};

}  // namespace zeta::core

#pragma pop_macro("Namespace")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")

#endif

#pragma pop_macro("Skip")
