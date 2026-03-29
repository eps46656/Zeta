#pragma once

#include <deque>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::debug_deque {

struct Cntr {
    std::deque<void*>* deque;

    size_t elem_size;
};

struct Cursor {
    Cntr const* cntr;
    size_t idx;
};

void Init(Cntr& cntr);

void Deinit(Cntr& cntr);

size_t GetCursorSize(Cntr const& cntr);

size_t GetElemSize(Cntr const& cntr);

size_t GetElemCnt(Cntr const& cntr);

size_t GetMaxElemCnt(Cntr const& cntr);

void GetLBCursor(Cntr const& cntr, Cursor* dst_cursor);

void GetRBCursor(Cntr const& cntr, Cursor* dst_cursor);

void* PeekL(Cntr const& cntr, bool lazy_cpy_elem, Cursor* dst_cursor,
            void* dst_elem);

void* PeekR(Cntr const& cntr, bool lazy_cpy_elem, Cursor* dst_cursor,
            void* dst_elem);

void* Access(Cntr const& cntr, size_t idx, bool lazy_copy_elem,
             Cursor* dst_cursor, void* dst_elem);

void* Derefer(Cntr const& cntr, Cursor const* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

template <typename Reader>
void Read(Cntr const& cntr, Cursor const* pos_cursor, size_t cnt,
          Reader&& reader, Cursor* dst_cursor);

template <typename Writer>
void Write(Cntr& cntr, Cursor const* pos_cursor, size_t cnt, Writer&& writer,
           Cursor* dst_cursor);

template <typename ReaderWriter>
void ReadWrite(Cntr& cntr, Cursor const* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, Cursor* dst_cursor);

template <typename Writer>
void* PushL(Cntr& cntr, size_t cnt, Writer&& writer, Cursor* dst_cursor);

template <typename Writer>
void* PushR(Cntr& cntr, size_t cnt, Writer&& writer, Cursor* dst_cursor);

template <typename Writer>
void* Insert(Cntr& cntr, Cursor* pos_cursor, size_t cnt, Writer&& writer,
             Cursor* dst_cursor);

void PopL(Cntr& cntr, size_t cnt);

void PopR(Cntr& cntr, size_t cnt);

void Erase(Cntr& cntr, Cursor* pos_cursor, size_t cnt);

void EraseAll(Cntr& cntr);

void CopyCursor(Cntr const& cntr, Cursor const* src_cursor, Cursor* dst_cursor);

bool AreEqualCursor(Cntr const& cntr, Cursor const* cursor_a,
                    Cursor const* cursor_b);

int CompareCursor(Cntr const& cntr, Cursor const* cursor_a,
                  Cursor const* cursor_b);

size_t GetCursorDist(Cntr const& cntr, Cursor const* cursor_a,
                     Cursor const* cursor_b);

size_t GetCursorIdx(Cntr const& cntr, Cursor const* cursor);

void CursorStepL(Cntr const& cntr, Cursor* cursor);

void CursorStepR(Cntr const& cntr, Cursor* cursor);

void CursorAdvanceL(Cntr const& cntr, Cursor* cursor, size_t step);

void CursorAdvanceR(Cntr const& cntr, Cursor* cursor, size_t step);

}  // namespace zeta::core::debug_deque

namespace zeta::core {

template <>
struct seq_cntr::CntrTraits<debug_deque::Cntr const, void> {
    static void* GetReferedInst(debug_deque::Cntr const& cntr);

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        debug_deque::Cntr const& cntr);

    static constexpr seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        debug_deque::Cntr const& cntr);

    static size_t GetCursorSize(debug_deque::Cntr const& cntr);

    static size_t GetElemSize(debug_deque::Cntr const& cntr);

    static size_t GetElemCnt(debug_deque::Cntr const& cntr);

    static size_t GetMaxElemCnt(debug_deque::Cntr const& cntr);

    static void GetLBCursor(debug_deque::Cntr const& cntr, void* dst_cursor);

    static void GetRBCursor(debug_deque::Cntr const& cntr, void* dst_cursor);

    static void* PeekL(debug_deque::Cntr const& cntr, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void* PeekR(debug_deque::Cntr const& cntr, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void* Access(debug_deque::Cntr const& cntr, size_t idx,
                        bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(debug_deque::Cntr const& cntr, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    template <typename Reader>
    static void Read(debug_deque::Cntr const& cntr, void const* pos_cursor,
                     size_t cnt, Reader&& reader, void* dst_cursor);

    static void CopyCursor(debug_deque::Cntr const& cntr,
                           void const* src_cursor, void* dst_cursor);

    static bool AreEqualCursor(debug_deque::Cntr const& cntr,
                               void const* cursor_a, void const* cursor_b);

    static int CompareCursor(debug_deque::Cntr const& cntr,
                             void const* cursor_a, void const* cursor_b);

    static size_t GetCursorDist(debug_deque::Cntr const& cntr,
                                void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(debug_deque::Cntr const& cntr,
                               void const* cursor);

    static void CursorStepL(debug_deque::Cntr const& cntr, void* cursor);

    static void CursorStepR(debug_deque::Cntr const& cntr, void* cursor);

    static void CursorAdvanceL(debug_deque::Cntr const& cntr, void* cursor,
                               size_t step);

    static void CursorAdvanceR(debug_deque::Cntr const& cntr, void* cursor,
                               size_t step);
};

template <>
struct seq_cntr::CntrTraits<debug_deque::Cntr, void>
    : public seq_cntr::CntrTraits<debug_deque::Cntr const, void> {
    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    template <typename Writer>
    static void Write(debug_deque::Cntr& cntr, void* pos_cursor, size_t cnt,
                      Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(debug_deque::Cntr& cntr, void* pos_cursor, size_t cnt,
                          ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer>
    static void* PushL(debug_deque::Cntr& cntr, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* PushR(debug_deque::Cntr& cntr, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* Insert(debug_deque::Cntr& cntr, void* pos_cursor, size_t cnt,
                        Writer&& writer, void* dst_cursor);

    static void PopL(debug_deque::Cntr& cntr, size_t cnt);

    static void PopR(debug_deque::Cntr& cntr, size_t cnt);

    static void Erase(debug_deque::Cntr& cntr, void* pos_cursor, size_t cnt);

    static void EraseAll(debug_deque::Cntr& cntr);
};

}  // namespace zeta::core
