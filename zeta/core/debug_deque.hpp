#pragma once

#include <deque>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::debug_deque {

struct Cntr;
struct Cursor;

struct Cntr {
    size_t elem_size;

    std::deque<void*>* deque;

    constexpr Cntr(size_t elem_size);

    constexpr ~Cntr();

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticEnabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr seq_cntr::capability::Flag GetStaticDisabledCapabilityFlag(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        seq_cntr::Tag);

    static constexpr seq_cntr::capability::Flag
        GetDynamicDisabledCapabilityFlag(seq_cntr::Tag);

    constexpr void* GetReferedInstPtr(this Cntr const& cntr, seq_cntr::Tag);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr>);

    static constexpr meta::TypeWrapper<Cursor> GetCursorType(
        seq_cntr::Tag, meta::TypeWrapper<Cntr const>);

    static constexpr size_t GetCursorSize(seq_cntr::Tag);

    constexpr size_t GetElemSize(this Cntr const& cntr, seq_cntr::Tag);

    constexpr size_t GetElemCnt(this Cntr const& cntr, seq_cntr::Tag);

    constexpr size_t GetMaxElemCnt(this Cntr const& cntr, seq_cntr::Tag);

    constexpr void GetLBCursor(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* dst_cursor);

    constexpr void PeekL(this auto& cntr, seq_cntr::Tag, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this auto& cntr, seq_cntr::Tag, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Refer(this auto& cntr, seq_cntr::Tag, size_t idx,
                         bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this auto& cntr, seq_cntr::Tag,
                           Cursor const* pos_cursor, bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <typename Reader>
    constexpr void Read(this Cntr const& cntr, seq_cntr::Tag,
                        Cursor const* pos_cursor, size_t cnt, Reader&& reader,
                        Cursor* dst_cursor);

    template <typename Writer>
    constexpr void Write(this Cntr& cntr, seq_cntr::Tag,
                         Cursor const* pos_cursor, size_t cnt, Writer&& writer,
                         Cursor* dst_cursor);

    template <typename ReaderWriter>
    constexpr void ReadWrite(this Cntr& cntr, seq_cntr::Tag,
                             Cursor const* pos_cursor, size_t cnt,
                             ReaderWriter&& reader_writer, Cursor* dst_cursor);

    template <typename Writer>
    constexpr void PushL(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                         Writer&& writer, Cursor* dst_cursor);

    template <typename Writer>
    constexpr void PushR(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                         Writer&& writer, Cursor* dst_cursor);

    template <typename Writer>
    constexpr void Insert(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                          size_t cnt, Writer&& writer, Cursor* dst_cursor);

    template <typename Reader>
    constexpr void PopL(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                        Reader&& reader);

    template <typename Reader>
    constexpr void PopR(this Cntr& cntr, seq_cntr::Tag, size_t cnt,
                        Reader&& reader);

    template <typename Reader>
    constexpr void Erase(this Cntr& cntr, seq_cntr::Tag, Cursor* pos_cursor,
                         size_t cnt, Reader&& reader);

    constexpr void EraseAll(this Cntr& cntr, seq_cntr::Tag);

    constexpr void CopyCursor(this Cntr const& cntr, seq_cntr::Tag,
                              Cursor const* src_cursor, Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& cntr,
                                                 seq_cntr::Tag,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& cntr, seq_cntr::Tag,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& cntr, seq_cntr::Tag,
                               Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);

    constexpr void CursorAdvanceR(this Cntr const& cntr, seq_cntr::Tag,
                                  Cursor* cursor, size_t step);
};

struct Cursor {
    Cntr const* cntr;
    size_t idx;
};

}  // namespace zeta::core::debug_deque
