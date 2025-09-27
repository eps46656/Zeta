#pragma once

#include <deque>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core {

struct DebugDeque {
    struct Cursor {
        DebugDeque const* dd;
        size_t idx;
    };

    // -------------------------------------------------------------------------

    std::deque<void*>* deque;

    size_t width;

    // -------------------------------------------------------------------------

    template <bool EnWrite, typename ReaderWriter>
    static void CoreReadWrite(void* dd, void const* pos_cursor, size_t cnt,
                              ReaderWriter&& reader_writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void Init(void* dd);

    static void Deinit(void* dd);

    // -------------------------------------------------------------------------

    static size_t GetSize(void const* dd);

    static size_t GetCapacity(void const* dd);

    // -------------------------------------------------------------------------

    static void GetLBCursor(void const* dd, void* dst_cursor);

    static void GetRBCursor(void const* dd, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* PeekL(void* dd, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekL(void const* dd, void* dst_cursor,
                                  void* dst_elem);

    static void* PeekR(void* dd, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekR(void const* dd, void* dst_cursor,
                                  void* dst_elem);

    static void* Access(void* dd, size_t idx, void* dst_cursor, void* dst_elem);

    static void const* ConstAccess(void const* dd, size_t idx, void* dst_cursor,
                                   void* dst_elem);

    static void* Refer(void* dd, void const* pos_cursor);

    static void const* ConstRefer(void const* dd, void const* pos_cursor);

    // -------------------------------------------------------------------------

    template <typename Reader>
    static void TplRead(void const* dd, void const* pos_cursor, size_t cnt,
                        Reader&& reader, void* dst_cursor);

    template <typename Writer>
    static void TplWrite(void* dd, void const* pos_cursor, size_t cnt,
                         Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void TplReadWrite(void* dd, void const* pos_cursor, size_t cnt,
                             ReaderWriter&& reader_writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void FnRead(void const* dd, void const* pos_cursor, size_t cnt,
                       SeqCntr::FnReader reader, void* dst_cursor);

    static void FnWrite(void* dd, void* pos_cursor, size_t cnt,
                        SeqCntr::FnWriter writer, void* dst_cursor);

    static void FnReadWrite(void* dd, void const* pos_cursor, size_t cnt,
                            SeqCntr::FnReaderWriter reader_writer,
                            void* dst_cursor);

    // -------------------------------------------------------------------------

    static void MemRead(void const* dd, void const* pos_cursor, size_t cnt,
                        void* dst, size_t dst_stride, void* dst_cursor);

    static void MemWrite(void* dd, void* pos_cursor, size_t cnt,
                         void const* src, size_t src_stride, void* dst_cursor);

    // -------------------------------------------------------------------------

    template <typename Writer>
    static void* TplPushL(void* dd, size_t cnt, Writer&& writer,
                          void* dst_cursor);

    template <typename Writer>
    static void* TplPushR(void* dd, size_t cnt, Writer&& writer,
                          void* dst_cursor);

    template <typename Writer>
    static void* TplInsert(void* dd, void* pos_cursor, size_t cnt,
                           Writer&& writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* FnPushL(void* dd, size_t cnt, SeqCntr::FnWriter writer,
                         void* dst_cursor);

    static void* FnPushR(void* dd, size_t cnt, SeqCntr::FnWriter writer,
                         void* dst_cursor);

    static void* FnInsert(void* dd, void* pos_cursor, size_t cnt,
                          SeqCntr::FnWriter writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* MemPushL(void* dd, size_t cnt, void const* src,
                          size_t src_stride, void* dst_cursor);

    static void* MemPushR(void* dd, size_t cnt, void const* src,
                          size_t src_stride, void* dst_cursor);

    static void* MemInsert(void* dd, void* pos_cursor, size_t cnt,
                           void const* src, size_t src_stride,
                           void* dst_cursor);

    // -------------------------------------------------------------------------

    static void PopL(void* dd, size_t cnt);

    static void PopR(void* dd, size_t cnt);

    static void Erase(void* dd, void* pos_cursor, size_t cnt);

    static void EraseAll(void* dd);

    // -------------------------------------------------------------------------

    static void CopyCursor(void const* dd, void* dst_cursor,
                           void const* src_cursor);

    static bool AreEqualCursor(void const* dd, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(void const* dd, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(void const* dd, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(void const* dd, void const* cursor);

    static void CursorStepL(void const* dd, void* cursor);

    static void CursorStepR(void const* dd, void* cursor);

    static void CursorAdvanceL(void const* dd, void* cursor, size_t step);

    static void CursorAdvanceR(void const* dd, void* cursor, size_t step);

    // -------------------------------------------------------------------------

    static void CheckCntr(void const* dd);

    static void CheckCursor(void const* dd, void const* cursor);

    // -------------------------------------------------------------------------

    static SeqCntr ToSeqCntr(void* dd);

    static SeqCntr ToSeqCntr(void const* dd);
};

}  // namespace zeta::core
