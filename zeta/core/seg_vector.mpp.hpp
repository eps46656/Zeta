#if !defined(EnStaging)
#error "EnStaging is not defined."
#endif

#include <zeta/core/allocator.hpp>
#include <zeta/core/bin_tree_node_tpl.hpp>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/debug_utils.hpp>
#include <zeta/core/generic_hash_table.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/mem_check_utils.hpp>
#include <zeta/core/pool_allocator.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("Cntr")

#if EnStaging

#define Cntr StagingVector

#else

#define Cntr SegVector

#endif

namespace zeta::core {

template <typename SegAllocator, typename DataAllocator>
struct Cntr {
    using TreeNode =
#if EnStaging
        BinTreeNodeTpl<void*,
                       value_wrapper::StaticValueWrapper<true>,  // PColor
                       value_wrapper::StaticValueWrapper<true>,  // LColor
                       value_wrapper::StaticValueWrapper<true>,  // RColor
                       value_wrapper::StaticValueWrapper<true>   // AccSize
                       >;
#else
        BinTreeNodeTpl<void*,
                       value_wrapper::StaticValueWrapper<true>,   // PColor
                       value_wrapper::StaticValueWrapper<false>,  // LColor
                       value_wrapper::StaticValueWrapper<false>,  // RColor
                       value_wrapper::StaticValueWrapper<true>    // AccSize
                       >;
#endif

#if EnStaging
    static constexpr int ref_color{ 1 };
    static constexpr int dat_color{ 2 };
#endif

    static constexpr struct TreeNodeOperator {
        static constexpr bool en_acc_size{ true };

        static constexpr size_t null_acc_size{ 0 };

        TreeNode* GetP(TreeNode* n) const;
        TreeNode* GetL(TreeNode* n) const;
        TreeNode* GetR(TreeNode* n) const;

        void SetP(TreeNode* n, TreeNode* m) const;
        void SetL(TreeNode* n, TreeNode* m) const;
        void SetR(TreeNode* n, TreeNode* m) const;

        int GetColor(TreeNode* n) const;
        void SetColor(TreeNode* n, int color) const;

        size_t GetAccSize(TreeNode* n) const;
        void SetAccSize(TreeNode* n, size_t acc_size) const;
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
                         allocator::AllocatorRef>;
#endif

    struct Stats {
        size_t ref_seg_cnt;
        size_t dat_seg_cnt;
        size_t ref_size;
        size_t dat_size;
    };

    struct SanitizeRet {
        bool b;
        size_t ref_beg;
        size_t ref_end;
    };

    // -------------------------------------------------------------------------

    static size_t GetAvgCnt_(size_t cnt, size_t seg_capacity,
                             unsigned long long& random_seed);

    // -------------------------------------------------------------------------

#if EnStaging
    static int GetNColor_(TreeNode const* n);

    static void DirectlySetNColor_(TreeNode* n, int color);

    static void SetNColor_(TreeNode* n, int color);
#endif

    // -------------------------------------------------------------------------

    static Seg* NToSeg_(TreeNode* n);

    // -------------------------------------------------------------------------

#if EnStaging
    static void GetRefSegState_(size_t seg_capacity, Seg* seg,
                                CircularArray* dst_ca, size_t* dst_vacant);
#endif

    static void GetDatSegState_(size_t seg_capacity, Seg* seg,
                                CircularArray* dst_ca, size_t* dst_vacant);

    static void GetSegState_(size_t seg_capacity, Seg* seg,
#if EnStaging
                             int* dst_color,
#endif
                             CircularArray* dst_ca, size_t* dst_vacant);

    // -------------------------------------------------------------------------

    static void* AllocateData_(Cntr* cntr, PoolAllocator* data_pool_allocator);

    static Seg* AllocateSeg_(Cntr* cntr, PoolAllocator* data_pool_allocator);

#if EnStaging
    static Seg* AllocateRefSeg_(Cntr* cntr);
#endif

    static Seg* AllocateDatSeg_(Cntr* s);

    // -------------------------------------------------------------------------

#if EnStaging
    static void TransferRefSegToDatSeg_(Cntr* cntr, Seg* seg);

    static void PushRefL_(Cntr* cntr, CircularArray* ca, size_t beg,
                          size_t size);

    static void PushRefR_(Cntr* cntr, CircularArray* ca, size_t beg,
                          size_t size);

    template <typename Writer>
    static void RefShoveL_(Cntr* cntr, CircularArray* l_ca, Seg* r_seg,
                           size_t rl_size, size_t ins_cnt, size_t shove_cnt,
                           Writer&& writer);

    template <typename Writer>
    static void RefShoveR_(Cntr* cntr, Seg* l_seg, CircularArray* r_ca,
                           size_t lr_size, size_t ins_cnt, size_t shove_cnt,
                           Writer&& writer);
#endif

    static int Merge2_(Cntr* cntr, Seg* a_seg, Seg* b_seg, bool a_read_data,
                       bool b_read_data);

    // -------------------------------------------------------------------------

    template <typename CoreReader>
    static void Read_(void const* cntr, void const* pos_cursor, size_t cnt,
                      CoreReader&& core_reader, void* dst_cursor);

    template <bool EnRead, typename CoreReaderWriter>
    static void ReadWrite_(void* cntr, void* pos_cursor, size_t cnt,
                           CoreReaderWriter&& core_reader_writer,
                           void* dst_cursor);

    // -------------------------------------------------------------------------

#if EnStaging
    static void EraseRefSeg_(Cntr* cntr, Seg* seg);
#endif

    static void EraseDatSeg_(Cntr* cntr, Seg* seg);

    static void EraseSeg_(Cntr* cntr, Seg* seg);

    template <typename SegAllocatorImpl, typename DataAllocatorImpl>
    static void EraseAllSegs__(Cntr* cntr, TreeNode* n,
                               SegAllocatorImpl* seg_allocator,
                               DataAllocatorImpl* data_allocator);

    template <typename SegAllocatorImpl, typename DataAllocatorImpl>
    static void EraseAllSegs_(Cntr* cntr, SegAllocatorImpl* seg_allocator,
                              DataAllocatorImpl* data_allocator);

    // -------------------------------------------------------------------------

    static void InitTree_(Cntr* cntr);

#if EnStaging
    static void RefOrigin_(Cntr* cntr);
#endif

    // -------------------------------------------------------------------------

    static TreeNode* CopySegs_(Cntr* cntr, Cntr* src_sv, TreeNode* src_n,
                               PoolAllocator* segs, PoolAllocator* datas);

    // -------------------------------------------------------------------------

#if EnStaging
    static unsigned long long OffsetHash_(size_t offset,
                                          unsigned long long salt);

    static int OffsetOffsetCntNodeCompare_(size_t offset,
                                           GenericHashTableNode const* ghtn);

    static size_t RecordOffset_(Cntr* cntr, TreeNode* n, size_t dst_idx,
                                OffsetCntGenericHashTable* ght);

    static Pair<size_t, size_t> ToWBSeg_(Cntr* cntr, TreeNode* n, WBSeg* dst);

    static void WriteWBSeg_(Cntr* cntr, CircularArray* ca, WBSeg* wb_segs,
                            size_t segs_cnt, size_t dst_offset,
                            size_t ref_offset);

    static void WriteBack_LR_(Cntr* cntr, int write_back_strategy,
                              unsigned long long cost_coeff_read,
                              unsigned long long cost_coeff_write,
                              unsigned long long cost_coeff_insert,
                              unsigned long long cost_coeff_erase);

    static void WriteBack_Random_(Cntr* cntr,
                                  unsigned long long cost_coeff_read,
                                  unsigned long long cost_coeff_write,
                                  unsigned long long cost_coeff_insert,
                                  unsigned long long cost_coeff_erase);
#endif

    // -------------------------------------------------------------------------

    static void PrintState_(Cntr* cntr, TreeNode* n);

    static Stats GetStats_(Cntr const* cntr, TreeNode* n);

    static SanitizeRet Sanitize_(Cntr const* cntr, MemRecorder* dst_seg,
                                 MemRecorder* dst_data, TreeNode* n);

    // -------------------------------------------------------------------------

    static seq_cntr::SeqCntrVTable const seq_cntr_vtable;

    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------

    static void Init(void* cntr);

    static void Deinit(void* cntr);

    // -------------------------------------------------------------------------

    static size_t GetWidth(void const* cntr);

    static size_t GetSize(void const* cntr);

    static size_t GetCapacity(void const* cntr);

    // -------------------------------------------------------------------------

    static void GetLBCursor(void const* cntr, void* dst_cursor);

    static void GetRBCursor(void const* cntr, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* PeekL(void* cntr, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekL(void const* cntr, void* dst_cursor,
                                  void* dst_elem);

    static void* PeekR(void* cntr, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekR(void const* cntr, void* dst_cursor,
                                  void* dst_elem);

    static void* Access(void* cntr, size_t idx, void* dst_cursor,
                        void* dst_elem);

    static void const* ConstAccess(void const* cntr, size_t idx,
                                   void* dst_cursor, void* dst_elem);

    static void* Refer(void* cntr, void const* pos_cursor);

    static void const* ConstRefer(void const* cntr, void const* pos_cursor);

    // -------------------------------------------------------------------------

    template <typename Predictor>
    static void* FindFirst(void* cntr, Predictor&& predictor, void* dst_cursor,
                           void* dst_elem);

    // -------------------------------------------------------------------------

    template <typename Reader>
    static void TplRead(void const* cntr, void const* pos_cursor, size_t cnt,
                        Reader&& reader, void* dst_cursor);

    template <typename Writer>
    static void TplWrite(void* cntr, void* pos_cursor, size_t cnt,
                         Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void TplReadWrite(void* cntr, void* pos_cursor, size_t cnt,
                             ReaderWriter&& reader_writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void FnRead(void const* cntr, void const* pos_cursor, size_t cnt,
                       seq_cntr::FnReader reader, void* dst_cursor);

    static void FnWrite(void* cntr, void* pos_cursor, size_t cnt,
                        seq_cntr::FnWriter writer, void* dst_cursor);

    static void FnReadWrite(void* cntr, void* pos_cursor, size_t cnt,
                            seq_cntr::FnReaderWriter reader_writer,
                            void* dst_cursor);

    // -------------------------------------------------------------------------

    static void MemRead(void const* cntr, void const* pos_cursor, size_t cnt,
                        void* dst, size_t dst_stride, void* dst_cursor);

    static void MemWrite(void* cntr, void* pos_cursor, size_t cnt,
                         void const* src, size_t src_stride, void* dst_cursor);

    // -------------------------------------------------------------------------

    template <typename Writer>
    static void* TplPushL(void* cntr, size_t cnt, Writer&& writer,
                          void* dst_cursor);

    template <typename Writer>
    static void* TplPushR(void* cntr, size_t cnt, Writer&& writer,
                          void* dst_cursor);

    template <typename Writer>
    static void* TplInsert(void* cntr, void* pos_cursor, size_t cnt,
                           Writer&& writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* FnPushL(void* cntr, size_t cnt, seq_cntr::FnWriter writer,
                         void* dst_cursor);

    static void* FnPushR(void* cntr, size_t cnt, seq_cntr::FnWriter writer,
                         void* dst_cursor);

    static void* FnInsert(void* cntr, void* pos_cursor, size_t cnt,
                          seq_cntr::FnWriter writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* MemPushL(void* cntr, size_t cnt, void const* src,
                          size_t src_stride, void* dst_cursor);

    static void* MemPushR(void* cntr, size_t cnt, void const* src,
                          size_t src_stride, void* dst_cursor);

    static void* MemInsert(void* cntr, void* pos_cursor, size_t cnt,
                           void const* src, size_t src_stride,
                           void* dst_cursor);

    // -------------------------------------------------------------------------

    static void PopL(void* cntr, size_t cnt);

    static void PopR(void* cntr, size_t cnt);

    static void Erase(void* cntr, void* pos_cursor, size_t cnt);

    static void EraseAll(void* cntr);

    // -------------------------------------------------------------------------

    static void Reset(void* cntr);

    static void Copy(void* cntr, void* src_sv);

#if EnStaging
    static void Collapse(void* cntr);

    static void WriteBack(void* cntr, int write_back_strategy,
                          unsigned long long cost_coeff_read,
                          unsigned long long cost_coeff_write,
                          unsigned long long cost_coeff_insert,
                          unsigned long long cost_coeff_erase);
#endif

    // -------------------------------------------------------------------------

    static void CopyCursor(void const* cntr, void* dst_cursor,
                           void const* src_cursor);

    static bool AreEqualCursor(void const* cntr, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(void const* cntr, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(void const* cntr, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(void const* cntr, void const* cursor);

    static void CursorStepL(void const* cntr, void* cursor);

    static void CursorStepR(void const* cntr, void* cursor);

    static void CursorAdvanceL(void const* cntr, void* cursor, size_t step);

    static void CursorAdvanceR(void const* cntr, void* cursor, size_t step);

    // -------------------------------------------------------------------------

    static bool CheckCntr(void const* cntr);

    static bool CheckCursor(void const* cntr, void const* cursor);

    // -------------------------------------------------------------------------

    static seq_cntr::SeqCntrRef GetSeqCntrRef(void* cntr);

    static seq_cntr::ConstSeqCntrRef GetSeqCntrRef(void const* cntr);

    // -------------------------------------------------------------------------

    static void PrintState(void const* cntr);

    static Stats GetStats(void const* cntr);

    static void Sanitize(void const* cntr, MemRecorder* dst_seg,
                         MemRecorder* dst_data);

    // -------------------------------------------------------------------------

#if EnStaging
    seq_cntr::SeqCntrRef origin;
#endif

#if !EnStaging
    unsigned short width;
#endif

    size_t stride;

    size_t seg_capacity;

    TreeNode* root;

    TreeNode* lb;
    TreeNode* rb;

    SegAllocator seg_allocator;
    DataAllocator data_allocator;
};

}  // namespace zeta::core
