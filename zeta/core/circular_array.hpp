#pragma once

#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::circular_array {

struct Cntr {
    void* data;
    size_t elem_size;
    size_t elem_stride;
    size_t elem_cnt;
    size_t elem_capacity;
    size_t idx_offset;
};

struct Cursor {
    Cntr const* cntr;
    size_t idx;
    void* elem;
};

void* ReferElem(void* data, size_t elem_stride, size_t elem_capacity,
                size_t idx_offset, size_t idx);

size_t GetLongestContSucr(size_t elem_cnt, size_t elem_capacity,
                          size_t idx_offset, size_t idx);

size_t GetLongestContPred(size_t elem_cnt, size_t elem_capacity,
                          size_t idx_offset, size_t idx);

template <typename SrcSeqCntr>
void AssignFromSeqCntr(Cntr* cntr, size_t dst_beg,
                       SrcSeqCntr const* src_seq_cntr,
                       void* src_seq_cntr_cursor, size_t cnt);

void AssignFromCircularArray(Cntr* dst_cntr, size_t dst_beg,
                             Cntr const* src_cntr, size_t src_beg, size_t cnt);

void Init(Cntr const* cntr);

void Deinit(Cntr* cntr);

constexpr size_t GetCursorSize(Cntr const* cntr);

size_t GetElemSize(Cntr const* cntr);

size_t GetElemStride(Cntr const* cntr);

size_t GetIdxOffset(Cntr const* cntr);

size_t GetElemCount(Cntr const* cntr);

size_t GetMaxElemCnt(Cntr const* cntr);

void GetLBCursor(Cntr const* cntr, Cursor* dst_cursor);

void GetRBCursor(Cntr const* cntr, Cursor* dst_cursor);

void* PeekL(Cntr const* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
            void* dst_elem);

void* PeekR(Cntr const* cntr, bool lazy_copy_elem, Cursor* dst_cursor,
            void* dst_elem);

void* Access(Cntr const* cntr, size_t idx, bool lazy_copy_elem,
             Cursor* dst_cursor, void* dst_elem);

void* Derefer(Cntr const* cntr, Cursor const* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

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

}  // namespace zeta::core::circular_array

namespace zeta::core {

template <>
struct seq_cntr::CntrTraits<circular_array::Cntr const, void> {
    static void* GetReferedInst(circular_array::Cntr const* cntr);

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        circular_array::Cntr const* cntr);

    static constexpr seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        circular_array::Cntr const* cntr);

    static size_t GetCursorSize(circular_array::Cntr const* cntr);

    static size_t GetElemSize(circular_array::Cntr const* cntr);

    static size_t GetElemCnt(circular_array::Cntr const* cntr);

    static size_t GetMaxElemCnt(circular_array::Cntr const* cntr);

    static void GetLBCursor(circular_array::Cntr const* cntr, void* dst_cursor);

    static void GetRBCursor(circular_array::Cntr const* cntr, void* dst_cursor);

    static void* PeekL(circular_array::Cntr const* cntr, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void* PeekR(circular_array::Cntr const* cntr, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void* Access(circular_array::Cntr const* cntr, size_t idx,
                        bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(circular_array::Cntr const* cntr,
                         void const* pos_cursor, bool lazy_copy_elem,
                         void* dst_elem);

    template <typename Reader>
    static void Read(circular_array::Cntr const* cntr, void const* pos_cursor,
                     size_t cnt, Reader&& reader, void* dst_cursor);

    static void CopyCursor(circular_array::Cntr const* cntr,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(circular_array::Cntr const* cntr,
                               void const* cursor_a, void const* cursor_b);

    static int CompareCursor(circular_array::Cntr const* cntr,
                             void const* cursor_a, void const* cursor_b);

    static size_t GetCursorDist(circular_array::Cntr const* cntr,
                                void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(circular_array::Cntr const* cntr,
                               void const* cursor);

    static void CursorStepL(circular_array::Cntr const* cntr, void* cursor);

    static void CursorStepR(circular_array::Cntr const* cntr, void* cursor);

    static void CursorAdvanceL(circular_array::Cntr const* cntr, void* cursor,
                               size_t step);

    static void CursorAdvanceR(circular_array::Cntr const* cntr, void* cursor,
                               size_t step);
};

template <>
struct seq_cntr::CntrTraits<circular_array::Cntr, void>
    : public seq_cntr::CntrTraits<circular_array::Cntr const, void> {
    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    template <typename Writer>
    static void Write(circular_array::Cntr* cntr, void* pos_cursor, size_t cnt,
                      Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(circular_array::Cntr* cntr, void* pos_cursor,
                          size_t cnt, ReaderWriter&& reader_writer,
                          void* dst_cursor);

    template <typename Writer>
    static void* PushL(circular_array::Cntr* cntr, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* PushR(circular_array::Cntr* cntr, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* Insert(circular_array::Cntr* cntr, void* pos_cursor,
                        size_t cnt, Writer&& writer, void* dst_cursor);

    static void PopL(circular_array::Cntr* cntr, size_t cnt);

    static void PopR(circular_array::Cntr* cntr, size_t cnt);

    static void Erase(circular_array::Cntr* cntr, void* pos_cursor, size_t cnt);

    static void EraseAll(circular_array::Cntr* cntr);
};

}  // namespace zeta::core
