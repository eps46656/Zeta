#pragma once

#include <zeta/core/seq_cntr.hpp>

namespace zeta::core {

struct CircularArray {
    struct Cursor {
        CircularArray const* ca;
        size_t idx;
        void* ref;
    };

    void* data;
    size_t width;
    size_t stride;
    size_t offset;
    size_t size;
    size_t capacity;

    // -------------------------------------------------------------------------

    static void* ReferElem(void* data, size_t stride, size_t offset, size_t idx,
                           size_t capacity);

    static size_t GetLongestContPred(size_t offset, size_t idx, size_t size,
                                     size_t capacity);

    static size_t GetLongestContSucr(size_t offset, size_t idx, size_t size,
                                     size_t capacity);

    static void AssignFromSeqCntr(void* ca, size_t dst_beg,
                                  SeqCntr const* src_seq_cntr,
                                  void* src_seq_cntr_cursor, size_t cnt);

    static void AssignFromCircularArray(void* dst_ca, size_t dst_beg,
                                        void const* src_ca, size_t src_beg,
                                        size_t cnt);

    template <bool EnWrite, typename ReaderWriter>
    static void CoreReadWrite(void* ca, size_t idx, size_t cnt,
                              ReaderWriter&& reader_writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void Init(void const* ca);

    static void Deinit(void* ca);

    // -------------------------------------------------------------------------

    static size_t GetWidth(void const* ca);

    static size_t GetSride(void const* ca);

    static size_t GetOffset(void const* ca);

    static size_t GetSize(void const* ca);

    static size_t GetCapacity(void const* ca);

    // -------------------------------------------------------------------------

    static void GetLBCursor(void const* ca, void* dst_cursor);

    static void GetRBCursor(void const* ca, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* PeekL(void* ca, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekL(void const* ca, void* dst_cursor,
                                  void* dst_elem);

    static void* PeekR(void* ca, void* dst_cursor, void* dst_elem);

    static void const* ConstPeekR(void const* ca, void* dst_cursor,
                                  void* dst_elem);

    static void* Access(void* ca, size_t idx, void* dst_cursor, void* dst_elem);

    static void const* ConstAccess(void const* ca, size_t idx, void* dst_cursor,
                                   void* dst_elem);

    static void* Refer(void* ca, void const* pos_cursor);

    static void const* ConstRefer(void const* ca, void const* pos_cursor);

    // -------------------------------------------------------------------------

    template <typename Reader>
    static void TplRead(void const* ca, void const* pos_cursor, size_t cnt,
                        Reader&& reader, void* dst_cursor);

    template <typename Writer>
    static void TplWrite(void* ca, void const* pos_cursor, size_t cnt,
                         Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void TplReadWrite(void* ca, void const* pos_cursor, size_t cnt,
                             ReaderWriter&& reader_writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void FnRead(void const* ca, void const* pos_cursor, size_t cnt,
                       SeqCntr::FnReader writer, void* dst_cursor);

    static void FnWrite(void* ca, void* pos_cursor, size_t cnt,
                        SeqCntr::FnWriter writer, void* dst_cursor);

    static void FnReadWrite(void* ca, void const* pos_cursor, size_t cnt,
                            SeqCntr::FnReaderWriter reader_writer,
                            void* dst_cursor);

    // -------------------------------------------------------------------------

    static void MemRead(void const* ca, void const* pos_cursor, size_t cnt,
                        void* dst, size_t dst_stride, void* dst_cursor);

    static void MemWrite(void* ca, void* pos_cursor, size_t cnt,
                         void const* src, size_t src_stride, void* dst_cursor);

    // -------------------------------------------------------------------------

    template <typename ReaderWriter>
    static void IdxRead(void const* ca, size_t idx, size_t cnt,
                        ReaderWriter&& reader_writer);

    template <typename ReaderWriter>
    static void IdxWrite(void* ca, size_t idx, size_t cnt,
                         ReaderWriter&& reader_writer);

    template <typename ReaderWriter>
    static void IdxReadWrite(void* ca, size_t idx, size_t cnt,
                             ReaderWriter&& reader_writer);

    // -------------------------------------------------------------------------

    template <typename Writer>
    static void* TplPushL(void* ca, size_t cnt, Writer&& writer,
                          void* dst_cursor);

    template <typename Writer>
    static void* TplPushR(void* ca, size_t cnt, Writer&& writer,
                          void* dst_cursor);

    template <typename Writer>
    static void* TplInsert(void* ca, void* pos_cursor, size_t cnt,
                           Writer&& writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* FnPushL(void* ca, size_t cnt, SeqCntr::FnWriter writer,
                         void* dst_cursor);

    static void* FnPushR(void* ca, size_t cnt, SeqCntr::FnWriter writer,
                         void* dst_cursor);

    static void* FnInsert(void* ca, void* pos_cursor, size_t cnt,
                          SeqCntr::FnWriter writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void* MemPushL(void* ca, size_t cnt, void const* src,
                          size_t src_stride, void* dst_cursor);

    static void* MemPushR(void* ca, size_t cnt, void const* src,
                          size_t src_stride, void* dst_cursor);

    static void* MemInsert(void* ca, void* pos_cursor, size_t cnt,
                           void const* src, size_t src_stride,
                           void* dst_cursor);

    // -------------------------------------------------------------------------

    template <typename Writer>
    static void* IdxInsert(void* ca, size_t idx, size_t cnt, Writer&& writer);

    // -------------------------------------------------------------------------

    static void* PushL(void* ca, size_t cnt, void const* src, size_t src_stride,
                       void* dst_cursor);

    static void* PushR(void* ca, size_t cnt, void const* src, size_t src_stride,
                       void* dst_cursor);

    static void* Insert(void* ca, void* pos_cursor, size_t cnt, void const* src,
                        size_t src_stride, void* dst_cursor);

    // -------------------------------------------------------------------------

    static void PopL(void* ca, size_t cnt);

    static void PopR(void* ca, size_t cnt);

    static void Erase(void* ca, void* pos_cursor, size_t cnt);

    static void IdxErase(void* ca, size_t idx, size_t cnt);

    static void EraseAll(void* ca);

    // -------------------------------------------------------------------------

    static void CopyCursor(void const* ca, void* dst_cursor,
                           void const* src_cursor);

    static bool AreEqualCursor(void const* ca, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(void const* ca, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(void const* ca, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(void const* ca, void const* cursor);

    static void CursorStepL(void const* ca, void* cursor);

    static void CursorStepR(void const* ca, void* cursor);

    static void CursorAdvanceL(void const* ca, void* cursor, size_t step);

    static void CursorAdvanceR(void const* ca, void* cursor, size_t step);

    // -------------------------------------------------------------------------

    static bool CheckCntr(void const* ca);

    static bool CheckCursor(void const* ca, void const* cursor);

    // -------------------------------------------------------------------------

    static SeqCntr ToSeqCntr(void* ca);

    static SeqCntr ToSeqCntr(void const* ca);
};

}  // namespace zeta::core
