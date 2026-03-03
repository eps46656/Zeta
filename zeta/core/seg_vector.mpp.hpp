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
#include <zeta/core/value_wrapper.hpp>

#if EnStaging

#pragma push_macro("NameSpace")
#define NameSpace staging_seg_vector

#else

#pragma push_macro("NameSpace")
#define NameSpace seg_vector

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

namespace zeta::core::NameSpace {

constexpr size_t max_seg_capacity{ integral::RangeMaxOf<unsigned short> };

#if EnStaging
static constexpr unsigned ref_color{ 1 };
static constexpr unsigned dat_color{ 2 };
#endif

using CircularArray = circular_array::Cntr;

using BinTreeNode =
#if EnStaging
    basic_bin_tree_node::Node<void*,
                              value_wrapper::TrueType,   // PColor
                              value_wrapper::TrueType,   // LColor
                              value_wrapper::FalseType,  // RColor
                              value_wrapper::TrueType    // AccSize
                              >;
#else
    basic_bin_tree_node::Node<void*,
                              value_wrapper::TrueType,   // PColor
                              value_wrapper::TrueType,   // LColor
                              value_wrapper::FalseType,  // RColor
                              value_wrapper::TrueType    // AccSize
                              >;
#endif

struct TreeNode : public BinTreeNode {};

}  // namespace zeta::core::NameSpace

namespace zeta::core {

template <>
struct bin_tree::Traits<NameSpace::TreeNode const> {
    static constexpr bool IsConst();

    static constexpr bool HasAccSize();

    static NameSpace::TreeNode const* GetP(NameSpace::TreeNode const* n);
    static NameSpace::TreeNode const* GetL(NameSpace::TreeNode const* n);
    static NameSpace::TreeNode const* GetR(NameSpace::TreeNode const* n);

    static constexpr size_t GetNullAccSize();

    static size_t GetAccSize(NameSpace::TreeNode const* n);
};

template <>
struct bin_tree::Traits<NameSpace::TreeNode>
    : public bin_tree::Traits<NameSpace::TreeNode const> {
    static constexpr bool IsConst();

    static NameSpace::TreeNode* GetP(NameSpace::TreeNode* n);
    static NameSpace::TreeNode* GetL(NameSpace::TreeNode* n);
    static NameSpace::TreeNode* GetR(NameSpace::TreeNode* n);

    static void SetP(NameSpace::TreeNode* n, NameSpace::TreeNode* m);
    static void SetL(NameSpace::TreeNode* n, NameSpace::TreeNode* m);
    static void SetR(NameSpace::TreeNode* n, NameSpace::TreeNode* m);

    static void SetAccSize(NameSpace::TreeNode* n, size_t acc_size);
};

template <>
struct rbtree::Traits<NameSpace::TreeNode const> {
    static unsigned GetColor(NameSpace::TreeNode const* n);
};

template <>
struct rbtree::Traits<NameSpace::TreeNode>
    : public rbtree::Traits<NameSpace::TreeNode const> {
    static void SetColor(NameSpace::TreeNode* n, unsigned color);
};

}  // namespace zeta::core

namespace zeta::core::NameSpace {

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

#if EnStaging
    OriginLike origin;
#endif

    size_t width;
    size_t stride;
    size_t seg_capacity;

    TreeNode* root;

    TreeNode* lb;
    TreeNode* rb;

    SegAllocatorLike seg_alctr;
    DataAllocatorLike data_alctr;
};

namespace ops {

template <typename SegAllocatorLike, typename DataAllocatorLike>
void* CopyTree(size_t width, size_t stride, size_t seg_capacity,
               TreeNode* src_root, TreeNode* src_lb, TreeNode* src_rb,
               TreeNode* dst_lb, TreeNode* dst_rb, SegAllocatorLike&& seg_alctr,
               DataAllocatorLike&& data_alctr);

template <typename SegAllocatorLike, typename DataAllocatorLike>
utils::Pair<TreeNode*, TreeNode*> EraseTree(TreeNode* root,
                                            SegAllocatorLike&& seg_alctr,
                                            DataAllocatorLike&& data_alctr);

template <CntrTplParamList>
void Init(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void CopyInit(Cntr<CntrTplArgList>* cntr, Cntr<CntrTplArgList> const* src_cntr);

template <CntrTplParamList>
void Deinit(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
constexpr size_t GetCursorSize(Cntr<CntrTplArgList> const*);

template <CntrTplParamList>
size_t GetWidth(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetSize(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
size_t GetCapacity(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
void GetLBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void GetRBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor);

template <CntrTplParamList>
void* PeekL(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* PeekR(Cntr<CntrTplArgList> const* cntr, bool lazy_copy_elem,
            Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Access(Cntr<CntrTplArgList> const* cntr, size_t idx, bool lazy_copy_elem,
             Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList>
void* Derefer(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
              bool lazy_copy_elem, void* dst_elem);

template <CntrTplParamList, typename Predictor>
void* FindFirst(Cntr<CntrTplArgList>* cntr, Predictor&& predictor,
                Cursor* dst_cursor, void* dst_elem);

template <CntrTplParamList, typename Reader>
void Read(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
          size_t cnt, Reader&& reader, Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
static void Write(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
                  Writer&& writer, Cursor* dst_cursor);

template <CntrTplParamList, typename ReaderWriter>
void ReadWrite(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* PushL(Cntr<CntrTplArgList>* cntr, size_t cnt, Writer&& writer,
            Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* PushR(Cntr<CntrTplArgList>* cntr, size_t cnt, Writer&& writer,
            Cursor* dst_cursor);

template <CntrTplParamList, typename Writer>
void* Insert(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
             Writer&& writer, Cursor* dst_cursor);

template <CntrTplParamList>
void PopL(Cntr<CntrTplArgList>* cntr, size_t cnt);

template <CntrTplParamList>
void PopR(Cntr<CntrTplArgList>* cntr, size_t cnt);

template <CntrTplParamList>
void Erase(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt);

template <CntrTplParamList>
void EraseAll(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void Reset(Cntr<CntrTplArgList>* cntr);

#if EnStaging

template <CntrTplParamList>
void Collapse(Cntr<CntrTplArgList>* cntr);

template <CntrTplParamList>
void WriteBack(Cntr<CntrTplArgList>* cntr, int write_back_strategy,
               unsigned long long cost_coeff_read,
               unsigned long long cost_coeff_write,
               unsigned long long cost_coeff_insert,
               unsigned long long cost_coeff_erase);
#endif

template <CntrTplParamList>
void CopyCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* src_cursor,
                Cursor* dst_cursor);

template <CntrTplParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b);

template <CntrTplParamList>
int CompareCursor(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                  Cursor const* cursor_b);

template <CntrTplParamList>
size_t GetCursorDist(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor_a,
                     Cursor const* cursor_b);

template <CntrTplParamList>
size_t GetCursorIdx(Cntr<CntrTplArgList> const* cntr, Cursor const* cursor);

template <CntrTplParamList>
void CursorStepL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor);

template <CntrTplParamList>
void CursorStepR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor);

template <CntrTplParamList>
void CursorAdvanceL(Cntr<CntrTplArgList> const* cntr, Cursor* cursor,
                    size_t step);

template <CntrTplParamList>
void CursorAdvanceR(Cntr<CntrTplArgList> const* cntr, Cursor* cursor,
                    size_t step);

template <CntrTplParamList>
void PrintState(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
Stats GetStats(Cntr<CntrTplArgList> const* cntr);

template <CntrTplParamList>
void Sanitize(Cntr<CntrTplArgList> const* cntr,
              mem_recorder::MemRecorder* dst_seg,
              mem_recorder::MemRecorder* dst_data);

};  // namespace ops

}  // namespace zeta::core::NameSpace

namespace zeta::core {

template <CntrTplParamList>
struct seq_cntr::Traits<NameSpace::Cntr<CntrTplArgList> const, void> {
    static void* GetReferedInst(NameSpace::Cntr<CntrTplArgList> const* cntr);

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        NameSpace::Cntr<CntrTplArgList> const* cntr);

    static constexpr seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        NameSpace::Cntr<CntrTplArgList> const* cntr);

    static size_t GetCursorSize(NameSpace::Cntr<CntrTplArgList> const* cntr);

    static size_t GetWidth(NameSpace::Cntr<CntrTplArgList> const* cntr);

    static size_t GetSize(NameSpace::Cntr<CntrTplArgList> const* cntr);

    static size_t GetCapacity(NameSpace::Cntr<CntrTplArgList> const* cntr);

    static void GetLBCursor(NameSpace::Cntr<CntrTplArgList> const* cntr,
                            void* dst_cursor);

    static void GetRBCursor(NameSpace::Cntr<CntrTplArgList> const* cntr,
                            void* dst_cursor);

    static void* PeekL(NameSpace::Cntr<CntrTplArgList> const* cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* PeekR(NameSpace::Cntr<CntrTplArgList> const* cntr,
                       bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Access(NameSpace::Cntr<CntrTplArgList> const* cntr, size_t idx,
                        bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(NameSpace::Cntr<CntrTplArgList> const* cntr,
                         void const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem);

    template <typename Reader>
    static void Read(NameSpace::Cntr<CntrTplArgList> const* cntr,
                     void const* pos_cursor, size_t cnt, Reader&& reader,
                     void* dst_cursor);

    static void CopyCursor(NameSpace::Cntr<CntrTplArgList> const* cntr,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(NameSpace::Cntr<CntrTplArgList> const* cntr,
                               void const* cursor_a, void const* cursor_b);

    static int CompareCursor(NameSpace::Cntr<CntrTplArgList> const* cntr,
                             void const* cursor_a, void const* cursor_b);

    static size_t GetCursorDist(NameSpace::Cntr<CntrTplArgList> const* cntr,
                                void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(NameSpace::Cntr<CntrTplArgList> const* cntr,
                               void const* cursor);

    static void CursorStepL(NameSpace::Cntr<CntrTplArgList> const* cntr,
                            void* cursor);

    static void CursorStepR(NameSpace::Cntr<CntrTplArgList> const* cntr,
                            void* cursor);

    static void CursorAdvanceL(NameSpace::Cntr<CntrTplArgList> const* cntr,
                               void* cursor, size_t step);

    static void CursorAdvanceR(NameSpace::Cntr<CntrTplArgList> const* cntr,
                               void* cursor, size_t step);
};

template <CntrTplParamList>
struct seq_cntr::Traits<NameSpace::Cntr<CntrTplArgList>, void>
    : public seq_cntr::Traits<NameSpace::Cntr<CntrTplArgList> const, void> {
    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    template <typename Writer>
    static void Write(NameSpace::Cntr<CntrTplArgList>* cntr, void* pos_cursor,
                      size_t cnt, Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(NameSpace::Cntr<CntrTplArgList>* cntr,
                          void* pos_cursor, size_t cnt,
                          ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer>
    static void* PushL(NameSpace::Cntr<CntrTplArgList>* cntr, size_t cnt,
                       Writer&& writer, void* dst_cursor);

    template <typename Writer>
    static void* PushR(NameSpace::Cntr<CntrTplArgList>* cntr, size_t cnt,
                       Writer&& writer, void* dst_cursor);

    template <typename Writer>
    static void* Insert(NameSpace::Cntr<CntrTplArgList>* cntr, void* pos_cursor,
                        size_t cnt, Writer&& writer, void* dst_cursor);

    static void PopL(NameSpace::Cntr<CntrTplArgList>* cntr, size_t cnt);

    static void PopR(NameSpace::Cntr<CntrTplArgList>* cntr, size_t cnt);

    static void Erase(NameSpace::Cntr<CntrTplArgList>* cntr, void* pos_cursor,
                      size_t cnt);

    static void EraseAll(NameSpace::Cntr<CntrTplArgList>* cntr);
};

}  // namespace zeta::core

#pragma pop_macro("NameSpace")
#pragma pop_macro("CntrTplDeclParamList")
#pragma pop_macro("CntrTplParamList")
#pragma pop_macro("CntrTplArgList")

#endif

#pragma pop_macro("Skip")
