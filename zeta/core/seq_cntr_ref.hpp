#pragma once

#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::seq_cntr_ref {

struct Ref {
    size_t cursor_size;

    size_t width;
    size_t capacity;

    seq_cntr::AbilityFlag dynamic_enabled_ability_flag;
    seq_cntr::AbilityFlag dynamic_disabled_ability_flag;

    seq_cntr::VTable const* vtable;

    void* cntr;
};

constexpr seq_cntr::AbilityFlag GetEnabledAbilityFlag(Ref const*);

constexpr seq_cntr::AbilityFlag GetDisabledAbilityFlag(Ref*);

constexpr seq_cntr::AbilityFlag GetDisabledAbilityFlag(Ref const*);

seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(Ref*);

seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(Ref const*);

seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(Ref*);

seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(Ref const*);

size_t GetCursorSize(Ref const*);

size_t GetElemSize(Ref const* ref);

size_t GetSride(Ref const* ref);

size_t GetOffset(Ref const* ref);

size_t GetElemCnt(Ref const* ref);

size_t GetMaxElemCnt(Ref const* ref);

void GetLBCursor(Ref const* ref, void* dst_cursor);

void GetRBCursor(Ref const* ref, void* dst_cursor);

void* PeekL(Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

void* PeekR(Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
            void* dst_elem);

void* Access(Ref const* ref, size_t idx, bool lazy_copy_elem, void* dst_cursor,
             void* dst_elem);

void* Derefer(Ref const* ref, void const* pos_cursor, bool lazy_copy_elem,
              void* dst_elem);

template <typename Reader>
void Read(Ref const* ref, void const* pos_cursor, size_t cnt, Reader&& reader,
          void* dst_cursor);

void Read(Ref const* ref, void const* pos_cursor, size_t cnt,
          seq_cntr::MemReader reader, void* dst_cursor);

template <typename Writer>
void Write(Ref* ref, void* pos_cursor, size_t cnt, Writer&& writer,
           void* dst_cursor);

void Write(Ref* ref, void* pos_cursor, size_t cnt, seq_cntr::MemWriter writer,
           void* dst_cursor);

template <typename ReaderWriter>
void ReadWrite(Ref* ref, void* pos_cursor, size_t cnt,
               ReaderWriter&& reader_writer, void* dst_cursor);

template <typename Writer>
void* PushL(Ref* ref, size_t cnt, Writer&& writer, void* dst_cursor);

void* PushL(Ref* ref, size_t cnt, seq_cntr::MemWriter writer, void* dst_cursor);

template <typename Writer>
void* PushR(Ref* ref, size_t cnt, Writer&& writer, void* dst_cursor);

void* PushR(Ref* ref, size_t cnt, seq_cntr::MemWriter writer, void* dst_cursor);

template <typename Writer>
void* Insert(Ref* ref, void* pos_cursor, size_t cnt, Writer&& writer,
             void* dst_cursor);

void* Insert(Ref* ref, void* pos_cursor, size_t cnt, seq_cntr::MemWriter writer,
             void* dst_cursor);

void PopL(Ref* ref, size_t cnt);

void PopR(Ref* ref, size_t cnt);

void Erase(Ref* ref, void* pos_cursor, size_t cnt);

void EraseAll(Ref* ref);

void CopyCursor(Ref const* ref, void const* src_cursor, void* dst_cursor);

bool AreEqualCursor(Ref const* ref, void const* cursor_a, void const* cursor_b);

int CompareCursor(Ref const* ref, void const* cursor_a, void const* cursor_b);

size_t GetCursorDist(Ref const* ref, void const* cursor_a,
                     void const* cursor_b);

size_t GetCursorIdx(Ref const* ref, void const* cursor);

void CursorStepL(Ref const* ref, void* cursor);

void CursorStepR(Ref const* ref, void* cursor);

void CursorAdvanceL(Ref const* ref, void* cursor, size_t step);

void CursorAdvanceR(Ref const* ref, void* cursor, size_t step);

void CheckRef(Ref* ref);

template <typename SeqCntrLike>
Ref MakeRef(SeqCntrLike&& cntr);

}  // namespace zeta::core::seq_cntr_ref

namespace zeta::core {

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void> {
    static void* GetReferedInstPtr(seq_cntr_ref::Ref const* ref);

    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetDynamicEnabledAbilityFlag(
        seq_cntr_ref::Ref const* ref);

    static constexpr seq_cntr::AbilityFlag GetDynamicDisabledAbilityFlag(
        seq_cntr_ref::Ref const* ref);

    static size_t GetCursorSize(seq_cntr_ref::Ref const* ref);

    static size_t GetElemSize(seq_cntr_ref::Ref const* ref);

    static size_t GetElemCnt(seq_cntr_ref::Ref const* ref);

    static size_t GetMaxElemCnt(seq_cntr_ref::Ref const* ref);

    static void GetLBCursor(seq_cntr_ref::Ref const* ref, void* dst_cursor);

    static void GetRBCursor(seq_cntr_ref::Ref const* ref, void* dst_cursor);

    static void* PeekL(seq_cntr_ref::Ref const* ref, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void* PeekR(seq_cntr_ref::Ref const* ref, bool lazy_copy_elem,
                       void* dst_cursor, void* dst_elem);

    static void* Access(seq_cntr_ref::Ref const* ref, size_t idx,
                        bool lazy_copy_elem, void* dst_cursor, void* dst_elem);

    static void* Derefer(seq_cntr_ref::Ref const* ref, void const* pos_cursor,
                         bool lazy_copy_elem, void* dst_elem);

    template <typename Reader>
    static void Read(seq_cntr_ref::Ref const* ref, void const* pos_cursor,
                     size_t cnt, Reader&& reader, void* dst_cursor);

    static void CopyCursor(seq_cntr_ref::Ref const* ref, void const* src_cursor,
                           void* dst_cursor);

    static bool AreEqualCursor(seq_cntr_ref::Ref const* ref,
                               void const* cursor_a, void const* cursor_b);

    static int CompareCursor(seq_cntr_ref::Ref const* ref, void const* cursor_a,
                             void const* cursor_b);

    static size_t GetCursorDist(seq_cntr_ref::Ref const* ref,
                                void const* cursor_a, void const* cursor_b);

    static size_t GetCursorIdx(seq_cntr_ref::Ref const* ref,
                               void const* cursor);

    static void CursorStepL(seq_cntr_ref::Ref const* ref, void* cursor);

    static void CursorStepR(seq_cntr_ref::Ref const* ref, void* cursor);

    static void CursorAdvanceL(seq_cntr_ref::Ref const* ref, void* cursor,
                               size_t step);

    static void CursorAdvanceR(seq_cntr_ref::Ref const* ref, void* cursor,
                               size_t step);
};

template <>
struct seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>
    : public seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void> {
    static constexpr seq_cntr::AbilityFlag GetStaticEnabledAbilityFlag();

    static constexpr seq_cntr::AbilityFlag GetStaticDisabledAbilityFlag();

    template <typename Writer>
    static void Write(seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt,
                      Writer&& writer, void* dst_cursor);

    template <typename ReaderWriter>
    static void ReadWrite(seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt,
                          ReaderWriter&& reader_writer, void* dst_cursor);

    template <typename Writer>
    static void* PushL(seq_cntr_ref::Ref* ref, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* PushR(seq_cntr_ref::Ref* ref, size_t cnt, Writer&& writer,
                       void* dst_cursor);

    template <typename Writer>
    static void* Insert(seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt,
                        Writer&& writer, void* dst_cursor);

    static void PopL(seq_cntr_ref::Ref* ref, size_t cnt);

    static void PopR(seq_cntr_ref::Ref* ref, size_t cnt);

    static void Erase(seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt);

    static void EraseAll(seq_cntr_ref::Ref* ref);
};

}  // namespace zeta::core
