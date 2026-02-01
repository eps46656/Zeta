#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/type_wrapper.hpp>

namespace zeta::core::circular_array {

struct Cntr {
    void* data;
    size_t width;
    size_t stride;
    size_t offset;
    size_t size;
    size_t capacity;
};

struct Cursor {
    Cntr const* cntr;
    size_t idx;
    void* elem;
};

namespace ops {

void* ReferElem(void* data, size_t stride, size_t offset, size_t idx,
                size_t capacity);

size_t GetLongestContSucr(size_t offset, size_t idx, size_t size,
                          size_t capacity);

size_t GetLongestContPred(size_t offset, size_t idx, size_t size,
                          size_t capacity);

template <typename SrcSeqCntr>
void AssignFromSeqCntr(Cntr* cntr, size_t dst_beg,
                       SrcSeqCntr const* src_seq_cntr,
                       void* src_seq_cntr_cursor, size_t cnt);

void AssignFromCircularArray(Cntr* dst_cntr, size_t dst_beg,
                             Cntr const* src_cntr, size_t src_beg, size_t cnt);

void Init(Cntr const* cntr);

void Deinit(Cntr* cntr);

constexpr size_t GetCursorSize(Cntr const* cntr);

size_t GetWidth(Cntr const* cntr);

size_t GetSride(Cntr const* cntr);

size_t GetOffset(Cntr const* cntr);

size_t GetSize(Cntr const* cntr);

size_t GetCapacity(Cntr const* cntr);

void GetLBCursor(Cntr const* cntr, Cursor* dst_cursor);

void GetRBCursor(Cntr const* cntr, Cursor* dst_cursor);

void* PeekL(Cntr* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
            void* dst_elem);

void const* PeekL(Cntr const* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
                  void* dst_elem);

void* PeekR(Cntr* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
            void* dst_elem);

void const* PeekR(Cntr const* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
                  void* dst_elem);

void* Access(Cntr* cntr, size_t idx, bool lazy_copy_elem, Cursor* dst_cursor,
             void* dst_elem);

void const* Access(Cntr const* cntr, size_t idx, bool lazy_copy_elem,
                   Cursor* dst_cursor, void* dst_elem);

void* Derefer(Cntr* cntr, Cursor const* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

void const* Derefer(Cntr const* cntr, Cursor const* pos_cursor,
                    bool lazy_copy_elem, void* dst_elem);

template <typename Reader>
void Read(Cntr const* cntr, Cursor const* pos_cursor, size_t cnt,
          Reader&& reader, Cursor* dst_cursor);

template <typename Writer>
void Write(Cntr* cntr, Cursor const* pos_cursor, size_t cnt, Writer&& writer,
           Cursor* dst_cursor);

template <typename ReaderWriter>
void ReadWrite(Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, Cursor* dst_cursor);

template <typename Reader>
void IdxRead(Cntr const* cntr, size_t idx, size_t cnt, Reader&& reader_writer);

template <typename Writer>
void IdxWrite(Cntr* cntr, size_t idx, size_t cnt, Writer&& reader_writer);

template <typename ReaderWriter>
void IdxReadWrite(Cntr* cntr, size_t idx, size_t cnt,
                  ReaderWriter&& reader_writer);

template <typename Writer>
void* PushL(Cntr* cntr, size_t cnt, Writer&& writer, Cursor* dst_cursor);

template <typename Writer>
void* PushR(Cntr* cntr, size_t cnt, Writer&& writer, Cursor* dst_cursor);

template <typename Writer>
void* Insert(Cntr* cntr, Cursor* pos_cursor, size_t cnt, Writer&& writer,
             Cursor* dst_cursor);

template <typename Writer>
void* IdxInsert(Cntr* cntr, size_t idx, size_t cnt, Writer&& writer);

void PopL(Cntr* cntr, size_t cnt);

void PopR(Cntr* cntr, size_t cnt);

void Erase(Cntr* cntr, Cursor* pos_cursor, size_t cnt);

void IdxErase(Cntr* cntr, size_t idx, size_t cnt);

void EraseAll(Cntr* cntr);

void CopyCursor(Cntr const* cntr, Cursor const* src_cursor, Cursor* dst_cursor);

bool AreEqualCursor(Cntr const* cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b);

int CompareCursor(Cntr const* cntr, Cursor const* cursor_a,
                  Cursor const* cursor_b);

size_t GetCursorDist(Cntr const* cntr, Cursor const* cursor_a,
                     Cursor const* cursor_b);

size_t GetCursorIdx(Cntr const* cntr, Cursor const* cursor);

void CursorStepL(Cntr const* cntr, Cursor* cursor);

void CursorStepR(Cntr const* cntr, Cursor* cursor);

void CursorAdvanceL(Cntr const* cntr, Cursor* cursor, size_t step);

void CursorAdvanceR(Cntr const* cntr, Cursor* cursor, size_t step);

}  // namespace ops

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

}  // namespace zeta::core::circular_array
