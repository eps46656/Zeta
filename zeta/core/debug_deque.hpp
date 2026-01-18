#pragma once

#include <deque>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/type_wrapper.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::debug_deque {

struct Cntr {
    std::deque<void*>* deque;

    unsigned short width;
};

struct Cursor {
    Cntr const* cntr;
    size_t idx;
};

namespace ops {

template <bool EnWrite, typename ReaderWriter>
void ReadWrite_(Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
                ReaderWriter&& reader_writer, Cursor* dst_cursor);

void Init(Cntr* cntr);

void Deinit(Cntr* cntr);

size_t GetCursorSize(Cntr const* cntr);

size_t GetWidth(Cntr const* cntr);

size_t GetSize(Cntr const* cntr);

size_t GetCapacity(Cntr const* cntr);

void GetLBCursor(Cntr const* cntr, Cursor* dst_cursor);

void GetRBCursor(Cntr const* cntr, Cursor* dst_cursor);

void* PeekL(Cntr* cntr, Cursor* dst_cursor, void* dst_elem);

void const* PeekL(Cntr const* cntr, Cursor* dst_cursor, void* dst_elem);

void* PeekR(Cntr* cntr, Cursor* dst_cursor, void* dst_elem);

void const* PeekR(Cntr const* cntr, Cursor* dst_cursor, void* dst_elem);

void* Access(Cntr* cntr, size_t idx, Cursor* dst_cursor, void* dst_elem);

void const* Access(Cntr const* cntr, size_t idx, Cursor* dst_cursor,
                   void* dst_elem);

void* Derefer(Cntr* cntr, Cursor const* pos_cursor, void* dst_elem);

void const* Derefer(Cntr const* cntr, Cursor const* pos_cursor, void* dst_elem);

template <typename Reader>
void Read(Cntr const* cntr, Cursor const* pos_cursor, size_t cnt,
          Reader&& reader, Cursor* dst_cursor);

template <typename Writer>
void Write(Cntr* cntr, Cursor const* pos_cursor, size_t cnt, Writer&& writer,
           Cursor* dst_cursor);

template <typename ReaderWriter>
void ReadWrite(Cntr* cntr, Cursor const* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, Cursor* dst_cursor);

template <typename Writer>
void* PushL(Cntr* cntr, size_t cnt, Writer&& writer, Cursor* dst_cursor);

template <typename Writer>
void* PushR(Cntr* cntr, size_t cnt, Writer&& writer, Cursor* dst_cursor);

template <typename Writer>
void* Insert(Cntr* cntr, Cursor* pos_cursor, size_t cnt, Writer&& writer,
             Cursor* dst_cursor);

void PopL(Cntr* cntr, size_t cnt);

void PopR(Cntr* cntr, size_t cnt);

void Erase(Cntr* cntr, Cursor* pos_cursor, size_t cnt);

void EraseAll(Cntr* cntr);

void CopyCursor(Cntr const* cntr, Cursor* dst_cursor, Cursor const* src_cursor);

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

struct CheckResultCode {
    static constexpr int NumBase{ __COUNTER__ + 1 };

    static constexpr int Success{ __COUNTER__ - NumBase };

    static constexpr int NullCntr{ __COUNTER__ - NumBase };
    static constexpr int NullCursor{ __COUNTER__ - NumBase };

    static constexpr int ZeroWidth{ __COUNTER__ - NumBase };

    static constexpr int CntrCursorMismatch{ __COUNTER__ - NumBase };

    static constexpr int CursorIdxOutOfRange{ __COUNTER__ - NumBase };

    static constexpr int CursorRefMismatch{ __COUNTER__ - NumBase };
};

int CheckCntr(Cntr const* cntr);

int CheckCursor(Cntr const* cntr, Cursor const* cursor);

}  // namespace ops

template <typename CntrImpl>
struct SeqCntrView {
    ZETA_Core_StaticAssert(IsAnyOf<CntrImpl, Cntr, Cntr const>);

    static constexpr bool IsConst();

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        CntrImpl*);

    static constexpr seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        CntrImpl*);

    static constexpr size_t GetCursorSize(CntrImpl* cntr);

    static size_t GetWidth(CntrImpl* cntr);

    static size_t GetSize(CntrImpl* cntr);

    static size_t GetCapacity(CntrImpl* cntr);

    static void GetLBCursor(CntrImpl* cntr, void* dst_cursor);

    static void GetRBCursor(CntrImpl* cntr, void* dst_cursor);

    static Conditional<IsConst(), void const*, void*> PeekL(CntrImpl* cntr,
                                                            void* dst_cursor,
                                                            void* dst_elem);

    static Conditional<IsConst(), void const*, void*> PeekR(CntrImpl* cntr,
                                                            void* dst_cursor,
                                                            void* dst_elem);

    static Conditional<IsConst(), void const*, void*> Access(CntrImpl* cntr,
                                                             size_t idx,
                                                             void* dst_cursor,
                                                             void* dst_elem);

    static Conditional<IsConst(), void const*, void*> Derefer(
        CntrImpl* cntr, void const* pos_cursor, void* dst_elem);

    template <typename Reader>
    static void Read(CntrImpl* cntr, void const* pos_cursor, size_t cnt,
                     Reader&& reader, void* dst_cursor);

    template <typename Writer, typename = EnableIf<!IsConst()>>
    static void Write(CntrImpl* cntr, void const* pos_cursor, size_t cnt,
                      Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter, typename = EnableIf<!IsConst()>>
    static void ReadWrite(CntrImpl* cntr, void const* pos_cursor, size_t cnt,
                          ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer, typename = EnableIf<!IsConst()>>
    static void* PushL(CntrImpl* cntr, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer, typename = EnableIf<!IsConst()>>
    static void* PushR(CntrImpl* cntr, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer, typename = EnableIf<!IsConst()>>
    static void* Insert(CntrImpl* cntr, void* pos_cursor, size_t cnt,
                        Writer&& writer, void* dst_cursor);

    template <typename _ = void, typename = EnableIf<!IsConst(), _>>
    static void PopL(CntrImpl* cntr, size_t cnt);

    template <typename _ = void, typename = EnableIf<!IsConst(), _>>
    static void PopR(CntrImpl* cntr, size_t cnt);

    template <typename _ = void, typename = EnableIf<!IsConst(), _>>
    static void Erase(CntrImpl* cntr, void* pos_cursor, size_t cnt);

    template <typename _ = void, typename = EnableIf<!IsConst(), _>>
    static void EraseAll(CntrImpl* cntr);

    static void CopyCursor(CntrImpl* cntr, void* dst_cursor,
                           void const* src_cursor);

    static bool AreEqualCursor(CntrImpl* cntr, void const* cursor_a,
                               void const* cursor_b);

    static int CompareCursor(CntrImpl* cntr, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(CntrImpl* cntr, void const* cursor_a,
                                void const* cursor_b);

    static size_t GetCursorIdx(CntrImpl* cntr, void const* cursor);

    static void CursorStepL(CntrImpl* cntr, void* cursor);

    static void CursorStepR(CntrImpl* cntr, void* cursor);

    static void CursorAdvanceL(CntrImpl* cntr, void* cursor, size_t step);

    static void CursorAdvanceR(CntrImpl* cntr, void* cursor, size_t step);
};

}  // namespace zeta::core::debug_deque
