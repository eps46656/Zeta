#pragma once

#include <deque>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::debug_deque {

struct Cntr;
struct Cursor;

struct Cntr {
    std::deque<void*>* deque;

    size_t elem_size;

    constexpr void Init(this Cntr& cntr);

    constexpr void Deinit(this Cntr& cntr);

    constexpr void* GetReferedInstPtr(this Cntr const& cntr);

    constexpr size_t GetCursorSize(this Cntr const& cntr);

    constexpr size_t GetElemSize(this Cntr const& cntr);

    constexpr size_t GetElemCnt(this Cntr const& cntr);

    constexpr size_t GetMaxElemCnt(this Cntr const& cntr);

    constexpr void GetLBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    constexpr void GetRBCursor(this Cntr const& cntr, Cursor* dst_cursor);

    constexpr void PeekL(this auto& cntr, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void PeekR(this auto& cntr, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Refer(this auto& cntr, size_t idx, bool lazy_copy_elem,
                         seq_cntr::ElemPtrView* dst_elem_ptr_view,
                         Cursor* dst_cursor, void* dst_elem);

    constexpr void Derefer(this auto& cntr, Cursor const* pos_cursor,
                           bool lazy_copy_elem,
                           seq_cntr::ElemPtrView* dst_elem_ptr_view,
                           void* dst_elem);

    template <typename Reader>
    constexpr void Read(this Cntr const& cntr, Cursor const* pos_cursor,
                        size_t cnt, Reader&& reader, Cursor* dst_cursor);

    template <typename Writer>
    constexpr void Write(this Cntr& cntr, Cursor const* pos_cursor, size_t cnt,
                         Writer&& writer, Cursor* dst_cursor);

    template <typename ReaderWriter>
    constexpr void ReadWrite(this Cntr& cntr, Cursor const* pos_cursor,
                             size_t cnt, ReaderWriter&& reader_writer,
                             Cursor* dst_cursor);

    template <typename Writer>
    constexpr void PushL(this Cntr& cntr, size_t cnt, Writer&& writer,
                         Cursor* dst_cursor);

    template <typename Writer>
    constexpr void PushR(this Cntr& cntr, size_t cnt, Writer&& writer,
                         Cursor* dst_cursor);

    template <typename Writer>
    constexpr void Insert(this Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                          Writer&& writer, Cursor* dst_cursor);

    template <typename Reader>
    constexpr void PopL(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <typename Reader>
    constexpr void PopR(this Cntr& cntr, size_t cnt, Reader&& reader);

    template <typename Reader>
    constexpr void Erase(this Cntr& cntr, Cursor* pos_cursor, size_t cnt,
                         Reader&& reader);

    constexpr void EraseAll(this Cntr& cntr);

    constexpr void CopyCursor(this Cntr const& cntr, Cursor const* src_cursor,
                              Cursor* dst_cursor);

    constexpr bool AreEqualCursor(this Cntr const& cntr, Cursor const* cursor_a,
                                  Cursor const* cursor_b);

    constexpr comparison::Ordering CompareCursor(this Cntr const& cntr,
                                                 Cursor const* cursor_a,
                                                 Cursor const* cursor_b);

    constexpr size_t GetCursorDist(this Cntr const& cntr,
                                   Cursor const* cursor_a,
                                   Cursor const* cursor_b);

    constexpr size_t GetCursorIdx(this Cntr const& cntr, Cursor const* cursor);

    constexpr void CursorStepL(this Cntr const& cntr, Cursor* cursor);

    constexpr void CursorStepR(this Cntr const& cntr, Cursor* cursor);

    constexpr void CursorAdvanceL(this Cntr const& cntr, Cursor* cursor,
                                  size_t step);

    constexpr void CursorAdvanceR(this Cntr const& cntr, Cursor* cursor,
                                  size_t step);
};

struct Cursor {
    Cntr const* cntr;
    size_t idx;
};

}  // namespace zeta::core::debug_deque

namespace zeta::core {

template <>
struct seq_cntr::CntrTraits<debug_deque::Cntr>
    : public seq_cntr::MemberFuncCntrTraitsAdapter<debug_deque::Cntr,
                                                   debug_deque::Cursor> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        debug_deque::Cntr& cntr);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(debug_deque::Cntr& cntr);
};

template <>
struct seq_cntr::CntrTraits<debug_deque::Cntr const>
    : public seq_cntr::MemberFuncCntrTraitsAdapter<debug_deque::Cntr const,
                                                   debug_deque::Cursor> {
    static constexpr seq_cntr::capability::Flag
    GetStaticEnabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag
    GetStaticDisabledCapabilityFlag();

    static constexpr seq_cntr::capability::Flag GetDynamicEnabledCapabilityFlag(
        debug_deque::Cntr const& cntr);

    static constexpr seq_cntr::capability::Flag
    GetDynamicDisabledCapabilityFlag(debug_deque::Cntr const& cntr);
};

}  // namespace zeta::core
