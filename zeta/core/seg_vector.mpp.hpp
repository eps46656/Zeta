#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

#define EnStaging 1

#include <zeta/core/allocator.hpp>
#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/pool_allocator.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("NameSpace")

#if EnStaging

#define NameSpace staging_seg_vector

#else

#define NameSpace seg_vector

#endif

#if EnStaging

#define CntrTplDeclParamList                                                 \
    typename OriginOperator, typename Origin, typename SegAllocatorOperator, \
        typename SegAllocator, typename DataAllocatorOperator,               \
        typename DataAllocator

#define CntrTplArgList                                          \
    OriginOperator, Origin, SegAllocatorOperator, SegAllocator, \
        DataAllocatorOperator, DataAllocator

#else

#define CntrTplDeclParamList                              \
    typename SegAllocatorOperator, typename SegAllocator, \
        typename DataAllocatorOperator, typename DataAllocator

#define CntrTplArgList \
    SegAllocatorOperator, SegAllocator, DataAllocatorOperator, DataAllocator

#endif

namespace zeta::core::NameSpace {

#if EnStaging
static constexpr unsigned ref_color{ 1 };
static constexpr unsigned dat_color{ 2 };
#endif

using TreeNode =
#if EnStaging
    BinTreeNodeTpl<void*,
                   value_wrapper::StaticValueWrapper<true>,   // PColor
                   value_wrapper::StaticValueWrapper<true>,   // LColor
                   value_wrapper::StaticValueWrapper<false>,  // RColor
                   value_wrapper::StaticValueWrapper<true>    // AccSize
                   >;
#else
    BinTreeNodeTpl<void*,
                   value_wrapper::StaticValueWrapper<true>,   // PColor
                   value_wrapper::StaticValueWrapper<true>,   // LColor
                   value_wrapper::StaticValueWrapper<false>,  // RColor
                   value_wrapper::StaticValueWrapper<true>    // AccSize
                   >;
#endif

constexpr struct TreeNodeOperator {
    static constexpr bool en_acc_size{ true };

    static constexpr size_t null_acc_size{ 0 };

    static TreeNode* GetP(TreeNode* n);
    static TreeNode* GetL(TreeNode* n);
    static TreeNode* GetR(TreeNode* n);

    static void SetP(TreeNode* n, TreeNode* m);
    static void SetL(TreeNode* n, TreeNode* m);
    static void SetR(TreeNode* n, TreeNode* m);

    static unsigned GetColor(TreeNode* n);
    static void SetColor(TreeNode* n, unsigned color);

    static size_t GetAccSize(TreeNode* n);
    static void SetAccSize(TreeNode* n, size_t acc_size);
} tn_opr;

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

struct WBSeg {
    size_t beg;
    size_t size;

    size_t dst_idx;
    size_t acc_ref;

    void* data;
    unsigned short offset;
};

struct OffsetCntNode {
    GenericHashTableNode ghtn;

    size_t offset;
    size_t cnt;

    struct NodeHash {
        size_t operator()(GenericHashTableNode const* ghtn,
                          unsigned long long salt) const;
    };

    struct NodeCompare {
        int operator()(GenericHashTableNode const* a,
                       GenericHashTableNode const* b) const;
    };
};

using OffsetCntGenericHashTable =
    GenericHashTable<typename OffsetCntNode::NodeHash,
                     typename OffsetCntNode::NodeCompare,
                     allocator::Ref<value_wrapper::FalseType>>;
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

struct SanitizeRet {
    bool b;
    size_t ref_beg;
    size_t ref_end;
};

template <CntrTplDeclParamList>
struct Cntr {
#if EnStaging
    OriginOperator const* origin_opr;
    Origin* origin;
#endif

#if !EnStaging
    unsigned short width;
#endif

    size_t stride;

    size_t seg_capacity;

    TreeNode* root;

    TreeNode* lb;
    TreeNode* rb;

    SegAllocatorOperator const* seg_alctr_opr;
    SegAllocator* seg_alctr;

    DataAllocatorOperator const* data_alctr_opr;
    DataAllocator* data_alctr;
};

namespace ops {

using CircularArray = zeta::core::circular_array::Cntr;

size_t GetAvgCnt_(size_t cnt, size_t seg_capacity,
                  unsigned long long& random_seed);

#if EnStaging
unsigned GetNColor_(TreeNode const* n);

void DirectlySetNColor_(TreeNode* n, unsigned color);

void SetNColor_(TreeNode* n, unsigned color);
#endif

Seg* NToSeg_(TreeNode* n);

#if EnStaging
void GetRefSegState_(size_t seg_capacity, Seg* seg, CircularArray* dst_ca,
                     size_t* dst_vacant);
#endif

void GetDatSegState_(size_t seg_capacity, Seg* seg, CircularArray* dst_ca,
                     size_t* dst_vacant);

void GetSegState_(size_t seg_capacity, Seg* seg,
#if EnStaging
                  unsigned* dst_color,
#endif
                  CircularArray* dst_ca, size_t* dst_vacant);

template <CntrTplDeclParamList>
Seg* AllocateSeg_(Cntr<CntrTplArgList>* cntr,
                  PoolAllocator* seg_pool_allocator);

template <CntrTplDeclParamList>
void* AllocateData_(Cntr<CntrTplArgList>* cntr,
                    PoolAllocator* data_pool_allocator);

#if EnStaging

template <CntrTplDeclParamList>
Seg* AllocateRefSeg_(Cntr<CntrTplArgList>* cntr);
#endif

template <CntrTplDeclParamList>
Seg* AllocateDatSeg_(Cntr<CntrTplArgList>* cntr);

// -------------------------------------------------------------------------

#if EnStaging

template <CntrTplDeclParamList>
void TransferRefSegToDatSeg_(Cntr<CntrTplArgList>* cntr, Seg* seg);

template <CntrTplDeclParamList>
void PushRefL_(Cntr<CntrTplArgList>* cntr, CircularArray* ca, size_t beg,
               size_t size);

template <CntrTplDeclParamList>
void PushRefR_(Cntr<CntrTplArgList>* cntr, CircularArray* ca, size_t beg,
               size_t size);

template <CntrTplDeclParamList, typename Writer>
void RefShoveL_(Cntr<CntrTplArgList>* cntr, CircularArray* l_ca, Seg* r_seg,
                size_t rl_size, size_t ins_cnt, size_t shove_cnt,
                Writer&& writer);

template <CntrTplDeclParamList, typename Writer>
void RefShoveR_(Cntr<CntrTplArgList>* cntr, Seg* l_seg, CircularArray* r_ca,
                size_t lr_size, size_t ins_cnt, size_t shove_cnt,
                Writer&& writer);
#endif

template <CntrTplDeclParamList>
int Merge2_(Cntr<CntrTplArgList>* cntr, Seg* a_seg, Seg* b_seg,
            bool a_read_data, bool b_read_data);

template <CntrTplDeclParamList, bool EnRead, typename CoreReaderWriter>
void CoreReadWrite_(Cntr<CntrTplArgList>* cntr, Cursor* pos_cursor, size_t cnt,
                    CoreReaderWriter& core_reader_writer, Cursor* dst_cursor);

// -------------------------------------------------------------------------

#if EnStaging

template <CntrTplDeclParamList>
void EraseRefSeg_(Cntr<CntrTplArgList>* cntr, Seg* seg);
#endif

template <CntrTplDeclParamList>
void EraseDatSeg_(Cntr<CntrTplArgList>* cntr, Seg* seg);

template <CntrTplDeclParamList>
void EraseSeg_(Cntr<CntrTplArgList>* cntr, Seg* seg);

template <typename SegAllocatorOperator, typename SegAllocator,
          typename DataAllocatorOperator, typename DataAllocator>
void EraseAllSegsCore_(TreeNode* lb, TreeNode* rb, TreeNode* n,
                       SegAllocatorOperator const& seg_alctr_opr,
                       SegAllocator* seg_alctr,
                       DataAllocatorOperator const& data_alctr_opr,
                       DataAllocator* data_alctr);

template <CntrTplDeclParamList>
void EraseAllSegs_(Cntr<CntrTplArgList>* cntr,
                   SegAllocatorOperator const& seg_alctr_opr,
                   SegAllocator* seg_alctr,
                   DataAllocatorOperator const& data_alctr_opr,
                   DataAllocator* data_alctr);

// -------------------------------------------------------------------------

template <CntrTplDeclParamList>
void InitTree_(Cntr<CntrTplArgList>* cntr);

#if EnStaging
template <CntrTplDeclParamList>
void RefOrigin_(Cntr<CntrTplArgList>* cntr);
#endif

// -------------------------------------------------------------------------

template <CntrTplDeclParamList>
static TreeNode* CopySegs_(Cntr<CntrTplArgList>* cntr,
                           Cntr<CntrTplArgList>* src_sv, TreeNode* src_n,
                           PoolAllocator* segs, PoolAllocator* datas);

// -------------------------------------------------------------------------

#if EnStaging

template <CntrTplDeclParamList>
static unsigned long long OffsetHash_(size_t offset, unsigned long long salt);

template <CntrTplDeclParamList>
int OffsetOffsetCntNodeCompare_(size_t offset,
                                GenericHashTableNode const* ghtn);

template <CntrTplDeclParamList>
size_t RecordOffset_(Cntr<CntrTplArgList>* cntr, TreeNode* n, size_t dst_idx,
                     OffsetCntGenericHashTable* ght);

template <CntrTplDeclParamList>
Pair<size_t, size_t> ToWBSeg_(Cntr<CntrTplArgList>* cntr, TreeNode* n,
                              WBSeg* dst);

template <CntrTplDeclParamList>
void WriteWBSeg_(Cntr<CntrTplArgList>* cntr, CircularArray* ca, WBSeg* wb_segs,
                 size_t segs_cnt, size_t dst_offset, size_t ref_offset);

template <CntrTplDeclParamList>
void WriteBack_LR_(Cntr<CntrTplArgList>* cntr, int write_back_strategy,
                   unsigned long long cost_coeff_read,
                   unsigned long long cost_coeff_write,
                   unsigned long long cost_coeff_insert,
                   unsigned long long cost_coeff_erase);

template <CntrTplDeclParamList>
void WriteBack_Random_(Cntr<CntrTplArgList>* cntr,
                       unsigned long long cost_coeff_read,
                       unsigned long long cost_coeff_write,
                       unsigned long long cost_coeff_insert,
                       unsigned long long cost_coeff_erase);
#endif

// -------------------------------------------------------------------------

template <CntrTplDeclParamList>
void PrintState_(Cntr<CntrTplArgList>* cntr, TreeNode* n);

template <CntrTplDeclParamList>
Stats GetStats_(Cntr<CntrTplArgList> const* cntr, TreeNode* n);

template <CntrTplDeclParamList>
SanitizeRet Sanitize_(Cntr<CntrTplArgList> const* cntr, MemRecorder* dst_seg,
                      MemRecorder* dst_data, TreeNode* n);

// -------------------------------------------------------------------------

template <CntrTplDeclParamList>
void Init(Cntr<CntrTplArgList>* cntr);

template <CntrTplDeclParamList>
void Deinit(Cntr<CntrTplArgList>* cntr);

template <CntrTplDeclParamList>
size_t GetWidth(Cntr<CntrTplArgList> const* cntr);

template <CntrTplDeclParamList>
size_t GetSize(Cntr<CntrTplArgList> const* cntr);

template <CntrTplDeclParamList>
size_t GetCapacity(Cntr<CntrTplArgList> const* cntr);

template <CntrTplDeclParamList>
void GetLBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor);

template <CntrTplDeclParamList>
void GetRBCursor(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor);

template <CntrTplDeclParamList>
void* PeekL(Cntr<CntrTplArgList>* cntr, Cursor* dst_cursor, void* dst_elem);

template <CntrTplDeclParamList>
void const* PeekL(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor,
                  void* dst_elem);

template <CntrTplDeclParamList>
void* PeekR(Cntr<CntrTplArgList>* cntr, Cursor* dst_cursor, void* dst_elem);

template <CntrTplDeclParamList>
void const* PeekR(Cntr<CntrTplArgList> const* cntr, Cursor* dst_cursor,
                  void* dst_elem);

template <CntrTplDeclParamList>
void* Access(Cntr<CntrTplArgList>* cntr, size_t idx, Cursor* dst_cursor,
             void* dst_elem);

template <CntrTplDeclParamList>
void const* ConstAccess(Cntr<CntrTplArgList> const* cntr, size_t idx,
                        Cursor* dst_cursor, void* dst_elem);

template <CntrTplDeclParamList>
void* Refer(Cntr<CntrTplArgList>* cntr, Cursor const* pos_cursor);

template <CntrTplDeclParamList>
void const* ConstRefer(Cntr<CntrTplArgList> const* cntr,
                       Cursor const* pos_cursor);

template <CntrTplDeclParamList, typename Predictor>
void* FindFirst(Cntr<CntrTplArgList>* cntr, Predictor&& predictor,
                Cursor* dst_cursor, void* dst_elem);

template <CntrTplDeclParamList, typename Reader>
void TplRead(Cntr<CntrTplArgList> const* cntr, Cursor const* pos_cursor,
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

template <CntrTplDeclParamList>
void Copy(Cntr<CntrTplArgList>* cntr, void* src_sv);

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
                void const* src_cursor);

template <CntrTplDeclParamList>
bool AreEqualCursor(Cntr<CntrTplArgList> const* cntr, void const* cursor_a,
                    void const* cursor_b);

template <CntrTplDeclParamList>
int CompareCursor(Cntr<CntrTplArgList> const* cntr, void const* cursor_a,
                  void const* cursor_b);

template <CntrTplDeclParamList>
size_t GetCursorDist(Cntr<CntrTplArgList> const* cntr, void const* cursor_a,
                     void const* cursor_b);

template <CntrTplDeclParamList>
size_t GetCursorIdx(Cntr<CntrTplArgList> const* cntr, void const* cursor);

template <CntrTplDeclParamList>
void CursorStepL(Cntr<CntrTplArgList> const* cntr, void* cursor);

template <CntrTplDeclParamList>
void CursorStepR(Cntr<CntrTplArgList> const* cntr, void* cursor);

template <CntrTplDeclParamList>
void CursorAdvanceL(Cntr<CntrTplArgList> const* cntr, void* cursor,
                    size_t step);

template <CntrTplDeclParamList>
void CursorAdvanceR(Cntr<CntrTplArgList> const* cntr, void* cursor,
                    size_t step);

template <CntrTplDeclParamList>
bool CheckCntr(Cntr<CntrTplArgList> const* cntr);

template <CntrTplDeclParamList>
bool CheckCursor(Cntr<CntrTplArgList> const* cntr, void const* cursor);

template <CntrTplDeclParamList>
void PrintState(Cntr<CntrTplArgList> const* cntr);

template <CntrTplDeclParamList>
Stats GetStats(Cntr<CntrTplArgList> const* cntr);

template <CntrTplDeclParamList>
void Sanitize(Cntr<CntrTplArgList> const* cntr, MemRecorder* dst_seg,
              MemRecorder* dst_data);

};  // namespace ops

}  // namespace zeta::core::NameSpace

#pragma pop_macro("NameSpace")
