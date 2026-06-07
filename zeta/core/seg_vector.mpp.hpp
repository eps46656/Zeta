#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

// #define EnStaging 1

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
#include <zeta/core/value_wrapper.hpp>

#if EnStaging

#pragma push_macro("Namespace")
#define Namespace staging_seg_vector

#else

#pragma push_macro("Namespace")
#define Namespace seg_vector

#endif

#if EnStaging

#pragma push_macro("CntrTplDeclParamList")
#define CntrTplDeclParamList                          \
    typename OriginLike_, typename SegAllocatorLike_, \
        typename DataAllocatorLike_

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList \
    typename OriginLike, typename SegAllocatorLike, typename DataAllocatorLike

#pragma push_macro("CntrTplArgList")
#define CntrTplArgList OriginLike, SegAllocatorLike, DataAllocatorLike

#else

#pragma push_macro("CntrTplDeclParamList")
#define CntrTplDeclParamList \
    typename SegAllocatorLike_, typename DataAllocatorLike_

#pragma push_macro("CntrTplParamList")
#define CntrTplParamList typename SegAllocatorLike, typename DataAllocatorLike

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
        value_wrapper::TrueType,                     // PColor
        value_wrapper::TrueType,                     // LColor
        value_wrapper::FalseType,                    // RColor
        value_wrapper::TrueType,                     // AccSize
        basic_bin_tree_node::PrimaryColorTagEnum::P  // PrimaryColor
        >;
#else
    basic_bin_tree_node::Node<
        void*,
        value_wrapper::TrueType,                     // PColor
        value_wrapper::TrueType,                     // LColor
        value_wrapper::FalseType,                    // RColor
        value_wrapper::TrueType,                     // AccSize
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
    void* elem;

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

template <CntrTplDeclParamList>
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
};

template <CntrTplParamList,
#if EnStaging
          typename OriginLikeInitArg,
#endif
          typename SegAllocatorLikeInitArg, typename DataAllocatorLikeInitArg>
void Init(Cntr<CntrTplArgList>& cntr,
#if !EnStaging
          size_t elem_size,
#endif
          size_t elem_stride, size_t seg_elem_slot_cnt,
#if EnStaging
          OriginLikeInitArg&& origin_like_init_arg,
#endif
          SegAllocatorLikeInitArg&& seg_alctr_like_init_arg,
          DataAllocatorLikeInitArg&& data_alctr_like_init_arg);

template <CntrTplParamList,
#if EnStaging
          typename OriginLikeInitArg,
#endif
          typename SegAllocatorLikeInitArg, typename DataAllocatorLikeInitArg,
          typename SrcOriginLike, typename SrcSegAllocatorLike,
          typename SrcDataAllocatorLike>
void Init(Cntr<CntrTplArgList>& cntr, size_t elem_stride,
          size_t seg_elem_slot_cnt,
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

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>& cntr);

template <CntrTplParamList
#if EnStaging
          ,
          typename OriginLikeInitArg
#endif
          >
void Copy(Cntr<CntrTplArgList>& cntr,
#if EnStaging
          OriginLikeInitArg&& origin_like_init_arg,
#endif
          Cntr<CntrTplArgList> const& src_cntr);

template <CntrTplParamList>
constexpr size_t GetCursorSize(Cntr<CntrTplArgList> const&);

template <CntrTplParamList>
size_t GetElemSize(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetElemCnt(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
size_t GetMaxElemCnt(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
void GetLBCursor(Cntr<CntrTplArgList> const& cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void GetRBCursor(Cntr<CntrTplArgList> const& cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void* PeekL(Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* PeekR(Cntr<CntrTplArgList> const& cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Access(Cntr<CntrTplArgList> const& cntr, size_t idx, bool lazy_copy_elem,
             Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Derefer(Cntr<CntrTplArgList> const& cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList, typename Predictor>
void* FindFirst(Cntr<CntrTplArgList>& cntr, Predictor&& predictor,
                Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList, typename Reader>
void Read(Cntr<CntrTplArgList> const& cntr, Cursor const* pos_cursor,
          size_t cnt, Reader&& reader, Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
static void Write(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
                  Writer&& writer, Cursor* dst_cursor);

template <CntrTplParamList, typename ReaderWriter>
void ReadWrite(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* PushL(Cntr<CntrTplArgList>& cntr, size_t cnt, Writer&& writer,
            Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* PushR(Cntr<CntrTplArgList>& cntr, size_t cnt, Writer&& writer,
            Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* Insert(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt,
             Writer&& writer, Cursor* dst_cursor);

template <CntrTplParamList>
void PopL(Cntr<CntrTplArgList>& cntr, size_t cnt);

template <CntrTplParamList>
void PopR(Cntr<CntrTplArgList>& cntr, size_t cnt);

template <CntrTplParamList>
void Erase(Cntr<CntrTplArgList>& cntr, Cursor* pos_cursor, size_t cnt);

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>& cntr);

template <CntrTplParamList>
void Reset(Cntr<CntrTplArgList>& cntr);

#if EnStaging

template <CntrTplParamList, typename OriginOriginLike,
          typename OriginSegAllocator, typename OriginDataAllocator,
          typename NewOriginLikeInitArg>
void Collapse(Cntr<CntrTplArgList>& cntr,
              Cntr<OriginOriginLike, OriginSegAllocator,
                   OriginDataAllocator> const& origin_cntr,
              NewOriginLikeInitArg&& new_origin_like_init_arg);

template <CntrTplParamList>
void WriteBack(Cntr<CntrTplArgList>& cntr, int write_back_strategy,
               unsigned long long cost_coeff_read,
               unsigned long long cost_coeff_write,
               unsigned long long cost_coeff_insert,
               unsigned long long cost_coeff_erase);
#endif

template <CntrTplParamList>
void CopyCursor(Cntr<CntrTplArgList> const& cntr, Cursor const* src_cursor,
                Cursor* dst_cursor);

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b);

template <CntrTplParamList>
int CompareCursor(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
                  Cursor const* cursor_b);

template <CntrTplParamList>
size_t GetCursorDist(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor_a,
                     Cursor const* cursor_b);

template <CntrTplParamList>
size_t GetCursorIdx(Cntr<CntrTplArgList> const& cntr, Cursor const* cursor);

template <CntrTplParamList>
void CursorStepL(Cntr<CntrTplArgList> const& cntr, Cursor* cursor);

template <CntrTplParamList>
void CursorStepR(Cntr<CntrTplArgList> const& cntr, Cursor* cursor);

template <CntrTplParamList>
void CursorAdvanceL(Cntr<CntrTplArgList> const& cntr, Cursor* cursor,
                    size_t step);

template <CntrTplParamList>
void CursorAdvanceR(Cntr<CntrTplArgList> const& cntr, Cursor* cursor,
                    size_t step);

template <CntrTplParamList>
void PrintState(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
Stats GetStats(Cntr<CntrTplArgList> const& cntr);

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList> const& cntr,
              mem_recorder::MemRecorder* dst_seg,
              mem_recorder::MemRecorder* dst_data);

}  // namespace zeta::core::Namespace

namespace zeta::core {

template <CntrTplParamList>
struct lifecycle::Traits<Namespace::Cntr<CntrTplArgList>> {
    template <typename... Args>
    static void Init(Namespace::Cntr<CntrTplArgList>& cntr, Args&&... args);

    static void Deinit(Namespace::Cntr<CntrTplArgList>& cntr);
};

template <CntrTplParamList>
struct seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const> {
    static void* GetReferedInstPtr(Namespace::Cntr<CntrTplArgList> const& cntr);

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        Namespace::Cntr<CntrTplArgList> const& cntr);

    static constexpr seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        Namespace::Cntr<CntrTplArgList> const& cntr);

    static size_t GetCursorSize(Namespace::Cntr<CntrTplArgList> const& cntr);

    static size_t GetElemSize(Namespace::Cntr<CntrTplArgList> const& cntr);

    static size_t GetElemCnt(Namespace::Cntr<CntrTplArgList> const& cntr);

    static size_t GetMaxElemCnt(Namespace::Cntr<CntrTplArgList> const& cntr);

    static void GetLBCursor(Namespace::Cntr<CntrTplArgList> const& cntr,
                            void* dst_cursor);

    static void GetRBCursor(Namespace::Cntr<CntrTplArgList> const& cntr,
                            void* dst_cursor);

    static void* PeekL(Namespace::Cntr<CntrTplArgList> const& cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* PeekR(Namespace::Cntr<CntrTplArgList> const& cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Access(Namespace::Cntr<CntrTplArgList> const& cntr, size_t idx,
                        bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(Namespace::Cntr<CntrTplArgList> const& cntr,
                         void const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem);

    template <typename Reader>
    static void Read(Namespace::Cntr<CntrTplArgList> const& cntr,
                     void const* pos_cursor, size_t cnt, Reader&& reader,
                     void* dst_cursor);

    static void CopyCursor(Namespace::Cntr<CntrTplArgList> const& cntr,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(Namespace::Cntr<CntrTplArgList> const& cntr,
                               void const* cursor_a, void const* cursor_b);

    static int CompareCursor(Namespace::Cntr<CntrTplArgList> const& cntr,
                             void const* cursor_a, void const* cursor_b);

    static size_t GetCursorDist(Namespace::Cntr<CntrTplArgList> const& cntr,
                                void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(Namespace::Cntr<CntrTplArgList> const& cntr,
                               void const* cursor);

    static void CursorStepL(Namespace::Cntr<CntrTplArgList> const& cntr,
                            void* cursor);

    static void CursorStepR(Namespace::Cntr<CntrTplArgList> const& cntr,
                            void* cursor);

    static void CursorAdvanceL(Namespace::Cntr<CntrTplArgList> const& cntr,
                               void* cursor, size_t step);

    static void CursorAdvanceR(Namespace::Cntr<CntrTplArgList> const& cntr,
                               void* cursor, size_t step);
};

template <CntrTplParamList>
struct seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList>>
    : public seq_cntr::CntrTraits<Namespace::Cntr<CntrTplArgList> const> {
    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    template <typename Writer>
    static void Write(Namespace::Cntr<CntrTplArgList>& cntr, void* pos_cursor,
                      size_t cnt, Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(Namespace::Cntr<CntrTplArgList>& cntr,
                          void* pos_cursor, size_t cnt,
                          ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer>
    static void* PushL(Namespace::Cntr<CntrTplArgList>& cntr, size_t cnt,
                       Writer&& writer, void* dst_cursor);

    template <typename Writer>
    static void* PushR(Namespace::Cntr<CntrTplArgList>& cntr, size_t cnt,
                       Writer&& writer, void* dst_cursor);

    template <typename Writer>
    static void* Insert(Namespace::Cntr<CntrTplArgList>& cntr, void* pos_cursor,
                        size_t cnt, Writer&& writer, void* dst_cursor);

    static void PopL(Namespace::Cntr<CntrTplArgList>& cntr, size_t cnt);

    static void PopR(Namespace::Cntr<CntrTplArgList>& cntr, size_t cnt);

    static void Erase(Namespace::Cntr<CntrTplArgList>& cntr, void* pos_cursor,
                      size_t cnt);

    static void EraseAll(Namespace::Cntr<CntrTplArgList>& cntr);
};

}  // namespace zeta::core

#pragma pop_macro("Namespace")
#pragma pop_macro("CntrTplDeclParamList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")

#endif

#pragma pop_macro("Skip")
