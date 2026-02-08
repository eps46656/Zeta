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
#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pool_allocator.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("NameSpace")

#if EnStaging

#define NameSpace staging_seg_vector

#else

#define NameSpace seg_vector

#endif

#if EnStaging

#define CntrTplDeclParamList \
    typename OriginLike, typename SegAllocatorLike, typename DataAllocatorLike

#define CntrTplArgList OriginLike, SegAllocatorLike, DataAllocatorLike

#else

#define CntrTplDeclParamList \
    typename SegAllocatorLike, typename DataAllocatorLike

#define CntrTplArgList SegAllocatorLike, DataAllocatorLike

#endif

namespace zeta::core::NameSpace {

#if EnStaging
static constexpr unsigned ref_color{ 1 };
static constexpr unsigned dat_color{ 2 };
#endif

using TreeNode =
#if EnStaging
    BinTreeNodeTpl<void*,
                   value_wrapper::TrueType,   // PColor
                   value_wrapper::TrueType,   // LColor
                   value_wrapper::FalseType,  // RColor
                   value_wrapper::TrueType    // AccSize
                   >;
#else
    BinTreeNodeTpl<void*,
                   value_wrapper::TrueType,   // PColor
                   value_wrapper::TrueType,   // LColor
                   value_wrapper::FalseType,  // RColor
                   value_wrapper::TrueType    // AccSize
                   >;
#endif

struct TreeNodeView {
    static constexpr bool IsConst(type_wrapper::TypeWrapper<TreeNode*>);
    static constexpr bool IsConst(type_wrapper::TypeWrapper<TreeNode const*>);

    static constexpr bool IsAccSizeEnabled(
        type_wrapper::TypeWrapper<TreeNode const*>);

    static constexpr size_t GetNullAccSize(
        type_wrapper::TypeWrapper<TreeNode const*>);

    static TreeNodeView* GetP(TreeNodeView* n);
    static TreeNodeView* GetL(TreeNodeView* n);
    static TreeNodeView* GetR(TreeNodeView* n);

    static TreeNodeView const* GetP(TreeNodeView const* n);
    static TreeNodeView const* GetL(TreeNodeView const* n);
    static TreeNodeView const* GetR(TreeNodeView const* n);

    static void SetP(TreeNodeView* n, TreeNodeView* m);
    static void SetL(TreeNodeView* n, TreeNodeView* m);
    static void SetR(TreeNodeView* n, TreeNodeView* m);

    static unsigned GetColor(TreeNodeView const* n);
    static void SetColor(TreeNodeView* n, unsigned color);

    static size_t GetAccSize(TreeNodeView const* n);
    static void SetAccSize(TreeNodeView* n, size_t acc_size);
};

struct Seg {
    ZETA_Core_DebugStructPadding;

    TreeNode n;

    ZETA_Core_DebugStructPadding;

    union {
#if EnStaging
        struct {
            size_t beg;
            size_t size;
        } ref;
#endif

        struct {
            void* data;
            unsigned short offset;
            unsigned short size;
        } dat;
    };
};

struct Cursor {
    void const* cntr;
    size_t idx;
    TreeNode* n;
    size_t seg_idx;
    void* ref;

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

template <typename OriginLike>
struct CntrCore {
#if !EnStaging
    ZETA_Core_StaticAssert(IsAnyOf<OriginLike, void>);
#endif

#if EnStaging
    OriginLike origin;
#endif

    unsigned short width;
    size_t stride;
    size_t seg_capacity;

    TreeNode* root;

    TreeNode* lb;
    TreeNode* rb;
};

template <CntrTplDeclParamList>
struct Cntr : public CntrCore
#if EnStaging
              <OriginLike>
#else
              <void>
#endif
{
    SegAllocatorLike seg_alctr;

    DataAllocatorLike data_alctr;
};

namespace ops {

using CircularArray = zeta::core::circular_array::Cntr;

template <CntrTplDeclParamList>
void Init(Cntr<CntrTplArgList>* cntr);

template <CntrTplDeclParamList>
void Deinit(Cntr<CntrTplArgList>* cntr);

template <typename OriginLike>
constexpr size_t GetCursorSize(CntrCore<OriginLike> const*);

template <typename OriginLike>
size_t GetWidth(CntrCore<OriginLike> const* cntr);

template <typename OriginLike>
size_t GetSize(CntrCore<OriginLike> const* cntr);

template <typename OriginLike>
size_t GetCapacity(CntrCore<OriginLike> const* cntr);

template <typename OriginLike>
void GetLBCursor(CntrCore<OriginLike> const* cntr, Cursor* dst_cursor);

template <typename OriginLike>
void GetRBCursor(CntrCore<OriginLike> const* cntr, Cursor* dst_cursor);

template <CntrTplDeclParamList>
void* PeekL(Cntr<CntrTplArgList>* cntr, Cursor* dst_cursor, bool lazy_copy_elem,
            void* dst_elem);

template <CntrTplDeclParamList>
void const* PeekL(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor,
                  bool lazy_copy_elem, void* dst_elem);

template <CntrTplDeclParamList>
void* PeekR(Cntr<CntrTplArgList>* cntr, Cursor* dst_cursor, bool lazy_copy_elem,
            void* dst_elem);

template <CntrTplDeclParamList>
void const* PeekR(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor,
                  bool lazy_copy_elem, void* dst_elem);

template <CntrTplDeclParamList>
void* Access(Cntr<CntrTplArgList>* cntr, size_t idx, Cursor* dst_cursor,
             void* dst_elem);

template <CntrTplDeclParamList>
void const* Access(Cntr<CntrTplArgList> const* cntr, size_t idx,
                   Cursor* dst_cursor, void* dst_elem);

template <CntrTplDeclParamList>
void* Derefer(Cntr<CntrTplArgList>* cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem);

template <CntrTplDeclParamList>
void const* Derefer(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
                    bool lazy_copy_elem, void* dst_elem);

template <CntrTplDeclParamList, typename Predictor>
void* FindFirst(Cntr<CntrTplArgList>* cntr, Predictor&& predictor,
                Cursor* dst_cursor, void* dst_elem);

template <CntrTplDeclParamList, typename Reader>
void Read(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
          size_t cnt, Reader&& reader, Cursor* dst_cursor);

template <CntrTplDeclParamList, typename Writer>
static void Write(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
                  Writer&& writer, Cursor* dst_cursor);

template <CntrTplDeclParamList, typename ReaderWriter>
void ReadWrite(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, Cursor* dst_cursor);

template <CntrTplDeclParamList, typename Writer>
void* PushL(Cntr<CntrTplArgList>* cntr, size_t cnt, Writer&& writer,
            Cursor* dst_cursor);

template <CntrTplDeclParamList, typename Writer>
void* PushR(Cntr<CntrTplArgList>* cntr, size_t cnt, Writer&& writer,
            Cursor* dst_cursor);

template <CntrTplDeclParamList, typename Writer>
void* Insert(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
             Writer&& writer, Cursor* dst_cursor);

template <CntrTplDeclParamList>
void PopL(Cntr<CntrTplArgList>* cntr, size_t cnt);

template <CntrTplDeclParamList>
void PopR(Cntr<CntrTplArgList>* cntr, size_t cnt);

template <CntrTplDeclParamList>
void Erase(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt);

template <CntrTplDeclParamList>
void EraseAll(Cntr<CntrTplArgList>* cntr);

template <CntrTplDeclParamList>
void Reset(Cntr<CntrTplArgList>* cntr);

template <CntrTplDeclParamList, typename SrcOriginLike>
void Copy(Cntr<CntrTplArgList>* cntr, CntrCore<SrcOriginLike>* src_sv);

#if EnStaging

template <CntrTplDeclParamList>
void Collapse(Cntr<CntrTplArgList>* cntr);

template <CntrTplDeclParamList>
void WriteBack(Cntr<CntrTplArgList>* cntr, int write_back_strategy,
               unsigned long long cost_coeff_read,
               unsigned long long cost_coeff_write,
               unsigned long long cost_coeff_insert,
               unsigned long long cost_coeff_erase);
#endif

template <CntrTplDeclParamList>
void CopyCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor,
                Cursor const* src_cursor);

template <CntrTplDeclParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b);

template <CntrTplDeclParamList>
int CompareCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                  Cursor const* cursor_b);

template <CntrTplDeclParamList>
size_t GetCursorDist(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                     Cursor const* cursor_b);

template <CntrTplDeclParamList>
size_t GetCursorIdx(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor);

template <CntrTplDeclParamList>
void CursorStepL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor);

template <CntrTplDeclParamList>
void CursorStepR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor);

template <CntrTplDeclParamList>
void CursorAdvanceL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor,
                    size_t step);

template <CntrTplDeclParamList>
void CursorAdvanceR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor,
                    size_t step);

template <typename OriginLike>
void PrintState(CntrCore<OriginLike> const* cntr);

template <typename OriginLike>
inline Stats GetStats(CntrCore<OriginLike> const* cntr);

template <CntrTplDeclParamList>
void Sanitize(Cntr<CntrTplArgList> const* cntr, MemRecorder* dst_seg,
              MemRecorder* dst_data);

};  // namespace ops

template <CntrTplDeclParamList>
struct SeqCntrView {
    static constexpr bool IsConst(type_wrapper::TypeWrapper<SeqCntrView*>);

    static constexpr bool IsConst(
        type_wrapper::TypeWrapper<SeqCntrView const*>);

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<SeqCntrView*>);

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag(
        type_wrapper::TypeWrapper<SeqCntrView const*>);

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag(
        type_wrapper::TypeWrapper<SeqCntrView const*>);

    static constexpr seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        SeqCntrView const*);

    static constexpr seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        SeqCntrView const*);

    static constexpr size_t GetCursorSize(SeqCntrView const*);

    static size_t GetWidth(SeqCntrView const* seq_cntr_view);

    static size_t GetSride(SeqCntrView const* seq_cntr_view);

    static size_t GetOffset(SeqCntrView const* seq_cntr_view);

    static size_t GetSize(SeqCntrView const* seq_cntr_view);

    static size_t GetCapacity(SeqCntrView const* seq_cntr_view);

    static void GetLBCursor(SeqCntrView const* seq_cntr_view, void* dst_cursor);

    static void GetRBCursor(SeqCntrView const* seq_cntr_view, void* dst_cursor);

    static void* PeekL(SeqCntrView* seq_cntr_view, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void const* PeekL(SeqCntrView const* seq_cntr_view,
                             bool lazy_copy_elem, void* dst_cursor,
                             void* dst_elem);

    static void* PeekR(SeqCntrView* seq_cntr_view, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void const* PeekR(SeqCntrView const* seq_cntr_view,
                             bool lazy_copy_elem, void* dst_cursor,
                             void* dst_elem);

    static void* Access(SeqCntrView* seq_cntr_view, size_t idx,
                        bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void const* Access(SeqCntrView const* seq_cntr_view, size_t idx,
                              bool lazy_copy_elem, void* dst_cursor,
                              void* dst_elem);

    static void* Derefer(SeqCntrView* seq_cntr_view, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    static void const* Derefer(SeqCntrView const* seq_cntr_view,
                               void const* pos_cursor, bool lazy_copy_elem,
                               void* dst_elem);

    template <typename Reader>
    static void Read(SeqCntrView const* seq_cntr_view, void const* pos_cursor,
                     size_t cnt, Reader&& reader, void* dst_cursor);

    template <typename Writer>
    static void Write(SeqCntrView* seq_cntr_view, void* pos_cursor, size_t cnt,
                      Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(SeqCntrView* seq_cntr_view, void* pos_cursor,
                          size_t cnt, ReaderWriter&& reader_writer,
                          void* dst_cursor);

    template <typename Writer>
    static void* PushL(SeqCntrView* seq_cntr_view, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* PushR(SeqCntrView* seq_cntr_view, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* Insert(SeqCntrView* seq_cntr_view, void* pos_cursor,
                        size_t cnt, Writer&& writer, void* dst_cursor);

    static void PopL(SeqCntrView* seq_cntr_view, size_t cnt);

    static void PopR(SeqCntrView* seq_cntr_view, size_t cnt);

    static void Erase(SeqCntrView* seq_cntr_view, void* pos_cursor, size_t cnt);

    static void EraseAll(SeqCntrView* seq_cntr_view);

    static void CopyCursor(SeqCntrView const* seq_cntr_view,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(SeqCntrView const* seq_cntr_view,
                               void const* cursor_a, void const* cursor_b);

    static int CompareCursor(SeqCntrView const* seq_cntr_view,
                             void const* cursor_a, void const* cursor_b);

    static size_t GetCursorDist(SeqCntrView const* seq_cntr_view,
                                void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(SeqCntrView const* seq_cntr_view,
                               void const* cursor);

    static void CursorStepL(SeqCntrView const* seq_cntr_view, void* cursor);

    static void CursorStepR(SeqCntrView const* seq_cntr_view, void* cursor);

    static void CursorAdvanceL(SeqCntrView const* seq_cntr_view, void* cursor,
                               size_t step);

    static void CursorAdvanceR(SeqCntrView const* seq_cntr_view, void* cursor,
                               size_t step);
};

}  // namespace zeta::core::NameSpace

#pragma pop_macro("NameSpace")

#endif

#pragma pop_macro("Skip")
