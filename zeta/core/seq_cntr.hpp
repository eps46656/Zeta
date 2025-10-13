#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

#define ZETA_Core_SeqCntr_AllocaCursor_(tmp_seq_cntr, seq_cntr)           \
    ({                                                                    \
        auto tmp_seq_cntr{ seq_cntr };                                    \
        ZETA_Core_DebugAssert(tmp_seq_cntr != nullptr);                   \
                                                                          \
        __builtin_alloca_with_align(tmp_seq_cntr->cursor_size,            \
                                    __CHAR_BIT__ * alignof(max_align_t)); \
    })

#define ZETA_Core_SeqCntr_AllocaCursor(seq_cntr) \
    ZETA_Core_SeqCntr_AllocaCursor_(ZETA_Core_TmpName, seq_cntr)

namespace zeta::core::seq_cntr {

using FnReader = FunctionRef<void(void const*, size_t, size_t)>;

using FnWriter = FunctionRef<void(void*, size_t, size_t)>;

using FnReaderWriter = FunctionRef<void(void*, size_t, size_t)>;

// -----------------------------------------------------------------------------

bool IsReferable(size_t idx, size_t cnt, size_t size);

bool IsDereferable(size_t idx, size_t cnt, size_t size);

bool IsInsertable(size_t idx, size_t cnt, size_t size, size_t capacity);

bool IsErasable(size_t idx, size_t cnt, size_t size);

// -----------------------------------------------------------------------------

template <typename DstSeqCntrImpl, typename SrcSeqCntrImpl>
static void RangeAssign(DstSeqCntrImpl* dst_cntr, SrcSeqCntrImpl* src_cntr,
                        size_t dst_beg, size_t src_beg, size_t cnt);

// -----------------------------------------------------------------------------

struct SeqCntrVTable {
    void (*Deinit)(void* cntr);

    // -------------------------------------------------------------------------

    size_t (*GetSize)(void const* cntr);

    size_t (*GetCapacity)(void const* cntr);

    // -------------------------------------------------------------------------

    void (*GetLBCursor)(void const* cntr, void* dst_cursor);

    void (*GetRBCursor)(void const* cntr, void* dst_cursor);

    // -------------------------------------------------------------------------

    void* (*PeekL)(void* cntr, void* dst_cursor, void* dst_elem);

    void const* (*ConstPeekL)(void const* cntr, void* dst_cursor,
                              void* dst_elem);

    void* (*PeekR)(void* cntr, void* dst_cursor, void* dst_elem);

    void const* (*ConstPeekR)(void const* cntr, void* dst_cursor,
                              void* dst_elem);

    void* (*Access)(void* cntr, size_t idx, void* dst_cursor, void* dst_elem);

    void const* (*ConstAccess)(void const* cntr, size_t idx, void* dst_cursor,
                               void* dst_elem);

    void* (*Refer)(void* cntr, void const* pos_cursor);

    void const* (*ConstRefer)(void const* cntr, void const* pos_cursor);

    // -------------------------------------------------------------------------

    void (*FnRead)(void const* cntr, void const* pos_cursor, size_t cnt,
                   FnReader reader, void* dst_cursor);

    void (*FnWrite)(void* cntr, void* pos_cursor, size_t cnt, FnWriter writer,
                    void* dst_cursor);

    void (*FnReadWrite)(void* cntr, void* pos_cursor, size_t cnt,
                        FnReaderWriter reader_writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    void (*MemRead)(void const* cntr, void const* pos_cursor, size_t cnt,
                    void* dst, size_t dst_stride, void* dst_cursor);

    void (*MemWrite)(void* cntr, void* pos_cursor, size_t cnt, void const* src,
                     size_t src_stride, void* dst_cursor);

    // -------------------------------------------------------------------------

    void* (*FnPushL)(void* cntr, size_t cnt, FnWriter writer, void* dst_cursor);

    void* (*FnPushR)(void* cntr, size_t cnt, FnWriter writer, void* dst_cursor);

    void* (*FnInsert)(void* cntr, void* pos_cursor, size_t cnt, FnWriter writer,
                      void* dst_cursor);

    // -------------------------------------------------------------------------

    void* (*MemPushL)(void* cntr, size_t cnt, void const* src,
                      size_t src_stride, void* dst_cursor);

    void* (*MemPushR)(void* cntr, size_t cnt, void const* src,
                      size_t src_stride, void* dst_cursor);

    void* (*MemInsert)(void* cntr, void* pos_cursor, size_t cnt,
                       void const* src, size_t src_stride, void* dst_cursor);

    // -------------------------------------------------------------------------

    void (*PopL)(void* cntr, size_t cnt);

    void (*PopR)(void* cntr, size_t cnt);

    void (*Erase)(void* cntr, void* pos_cursor, size_t cnt);

    void (*EraseAll)(void* cntr);

    // -------------------------------------------------------------------------

    void (*CopyCursor)(void const* cntr, void* dst_cursor,
                       void const* src_cursor);

    bool (*AreEqualCursor)(void const* cntr, void const* cursor_a,
                           void const* cursor_b);

    int (*CompareCursor)(void const* cntr, void const* cursor_a,
                         void const* cursor_b);

    size_t (*GetCursorDist)(void const* cntr, void const* cursor_a,
                            void const* cursor_b);

    size_t (*GetCursorIdx)(void const* cntr, void const* cursor);

    void (*CursorStepL)(void const* cntr, void* cursor);

    void (*CursorStepR)(void const* cntr, void* cursor);

    void (*CursorAdvanceL)(void const* cntr, void* cursor, size_t step);

    void (*CursorAdvanceR)(void const* cntr, void* cursor, size_t step);
};

template <typename IsConst>
struct SeqCntrRefTpl {
    ZETA_Core_StaticAssert(
        IsAnyOf<IsConst, value_wrapper::StaticValueWrapper<false>,
                value_wrapper::StaticValueWrapper<true>>);

    // -------------------------------------------------------------------------

    static void Deinit(void* cntr);

    // -------------------------------------------------------------------------

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

    static void const* ConstAccess(void const* cntr, size_t idx, void* dst_cur,
                                   void* dst_elem);

    static void* Refer(void* cntr, void const* pos_cursor);

    static void const* ConstRefer(void const* cntr, void const* pos_cursor);

    // -------------------------------------------------------------------------

    static void FnRead(void const* cntr, void const* pos_cursor, size_t cnt,
                       FnReader reader, void* dst_cursor);

    static void FnWrite(void* cntr, void* pos_cursor, size_t cnt,
                        FnWriter writer, void* dst_cursor);

    static void FnReadWrite(void* cntr, void* pos_cursor, size_t cnt,
                            FnReaderWriter reader_writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void MemRead(void const* cntr, void const* pos_cursor, size_t cnt,
                        void* dst, size_t dst_stride, void* dst_cursor);

    static void MemWrite(void* cntr, void* pos_cursor, size_t cnt,
                         void const* src, size_t src_stride, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* FnPushL(void* cntr, size_t cnt, FnWriter writer,
                         void* dst_cursor);

    static void* FnPushR(void* cntr, size_t cnt, FnWriter writer,
                         void* dst_cursor);

    static void* FnInsert(void* cntr, void* pos_cursor, size_t cnt,
                          FnWriter writer, void* dst_cursor);

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

    // -------------------------------------------------------------------------

    Conditional<IsConst::value, void const*, void*> inst;

    unsigned short cursor_size;

    size_t width;
    size_t capacity;

    SeqCntrVTable const* vtable;
};

using SeqCntrRef = SeqCntrRefTpl<value_wrapper::StaticValueWrapper<false>>;

using ConstSeqCntrRef = SeqCntrRefTpl<value_wrapper::StaticValueWrapper<true>>;

}  // namespace zeta::core::seq_cntr
