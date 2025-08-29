#pragma once

#include <zeta/core/seq_cntr.hpp>

namespace zeta::core {

struct CircularArray;
struct CircularArray_Cursor;

// -----------------------------------------------------------------------------

struct CircularArray : public seq_cntr::SeqCntr {
    void* data;
    size_t width;
    size_t stride;
    size_t offset;
    size_t size;
    size_t capacity;

    // -------------------------------------------------------------------------

    void Init();

    void Deinit() override;

    // -------------------------------------------------------------------------

    size_t GetCursorSize() const override;

    size_t GetWidth() const override;

    size_t GetSride() const;

    size_t GetOffset() const;

    size_t GetSize() const override;

    size_t GetCapacity() const override;

    // -------------------------------------------------------------------------

    void GetLBCursor(void* dst_cursor) const override;

    void GetRBCursor(void* dst_cursor) const override;

    // -------------------------------------------------------------------------

    void* PeekL(void* dst_cursor, void* dst_elem) override;

    void const* PeekL(void* dst_cursor, void* dst_elem) const override;

    void* PeekR(void* dst_cursor, void* dst_elem) override;

    void const* PeekR(void* dst_cursor, void* dst_elem) const override;

    void* Access(size_t idx, void* dst_cursor, void* dst_elem) override;

    void const* Access(size_t idx, void* dst_cursor,
                       void* dst_elem) const override;

    void* Refer(void const* pos_cursor) override;

    void const* Refer(void const* pos_cursor) const override;

    // -------------------------------------------------------------------------

    template <typename Reader>
    void Read(void const* pos_cursor, size_t cnt, const Reader& reader,
              void* dst_cursor) const;

    template <typename Writer>
    void Write(void* pos_cursor, size_t cnt, const Writer& writer,
               void* dst_cursor);

    template <typename ReaderWriter>
    void ReadWrite(void* pos_cursor, size_t cnt,
                   const ReaderWriter& reader_writer, void* dst_cursor);

    // -------------------------------------------------------------------------

    void Read(void const* pos_cursor, size_t cnt, void* dst, size_t dst_stride,
              void* dst_cursor) const override;

    void Write(void* pos_cursor, size_t cnt, void const* src, size_t src_stride,
               void* dst_cursor) override;

    // -------------------------------------------------------------------------

    template <typename Writer>
    void* PushL(size_t cnt, const Writer& writer, void* dst_cursor);

    template <typename Writer>
    void* PushR(size_t cnt, const Writer& writer, void* dst_cursor);

    template <typename Writer>
    void* Insert(void const* pos_cursor, size_t cnt, const Writer& writer,
                 void* dst_cursor);

    // -------------------------------------------------------------------------

    void* PushL(size_t cnt, void const* src, size_t src_stride,
                void* dst_cursor) override;

    void* PushR(size_t cnt, void const* src, size_t src_stride,
                void* dst_cursor) override;

    void* Insert(void const* pos_cursor, size_t cnt, void const* src,
                 size_t src_stride, void* dst_cursor) override;

    // -------------------------------------------------------------------------

    void PopL(size_t cnt) override;

    void PopR(size_t cnt) override;

    void Erase(void* pos_cursor, size_t cnt) override;

    void EraseAll() override;

    // -------------------------------------------------------------------------

    void Cursor_Copy(void* dst_cursor, void const* src_cursor) const;

    bool Cursor_AreEqual(void const* cursor_a,
                         void const* cursor_b) const override;

    int Cursor_Compare(void const* cursor_a,
                       void const* cursor_b) const override;

    size_t Cursor_GetDist(void const* cursor_a,
                          void const* cursor_b) const override;

    size_t Cursor_GetIdx(void const* cursor) const override;

    void Cursor_StepL(void* cursor) const override;

    void Cursor_StepR(void* cursor) const override;

    void Cursor_AdvanceL(void* cursor, size_t step) const override;

    void Cursor_AdvanceR(void* cursor, size_t step) const override;

    // -------------------------------------------------------------------------

    void Check() const;

    void CheckCursor(void const* cursor) const;

    // -------------------------------------------------------------------------

    void Assign(size_t dst_beg, CircularArray* src_ca, size_t src_beg,
                size_t cnt);

    void Assign(size_t dst_beg, SeqCntr* src_ca, size_t src_beg, size_t cnt);

    // -------------------------------------------------------------------------

    static void* Refer_(void* data, size_t stride, size_t offset, size_t idx,
                        size_t capacity);

    static size_t GetLongestContPred_(size_t offset, size_t idx, size_t size,
                                      size_t capacity);

    static size_t GetLongestContSucr_(size_t offset, size_t idx, size_t size,
                                      size_t capacity);

    template <bool EnWrite, typename ReaderWriter>
    void ReadWriteTemp_(void* pos_cursor, size_t cnt,
                        const ReaderWriter& reader_writer, void* dst_cursor);

    void AssignFromSeqCntr_(size_t dst_beg, SeqCntr const* src_seq_cntr,
                            void const* src_seq_cntr_cursor, size_t cnt);

    void AssignFromCircularArray_(size_t dst_beg, CircularArray const* src_ca,
                                  size_t src_beg, size_t cnt);
};

// -----------------------------------------------------------------------------

struct CircularArray_Cursor {
    CircularArray const* ca;
    size_t idx;
    void* ref;
};

}  // namespace zeta::core
