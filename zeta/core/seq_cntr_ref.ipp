#pragma once

#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_cntr_ref.hpp>

namespace zeta::core {

#pragma push_macro("TestAbility")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestAbility(ability_flag, ability)                    \
    (((ability_flag) & (static_cast<seq_cntr::AbilityFlag>(1) \
                        << seq_cntr::AbilityEnum::ability)) != 0)

#pragma push_macro("CallMethod_")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod_(method_ptr, ability, method, ...)                 \
    {                                                                 \
        ZETA_Core_DebugAssert(                                        \
            TestAbility(ref->dynamic_enabled_ability_flag, ability)); \
                                                                      \
        auto method_ptr{ ref->vtable->method };                       \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                 \
                                                                      \
        return method_ptr(ref->cntr, __VA_ARGS__);                    \
    }                                                                 \
    ZETA_Core_StaticAssert(true);

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(ability, method, ...) \
    CallMethod_(method, ability, method, __VA_ARGS__)

inline seq_cntr::AbilityFlag seq_cntr_ref::ops::GetDynamicDisabledAbilityFlag(
    Ref* ref) {
    return ref->dynamic_disabled_ability_flag;
}

inline size_t seq_cntr_ref::ops::GetCursorSize(Ref const* ref) {
    return ref->cursor_size;
}

inline size_t seq_cntr_ref::ops::GetWidth(Ref const* ref) { return ref->width; }

inline size_t seq_cntr_ref::ops::GetSize(Ref const* ref) {
    CallMethod(GetSize, GetSize);
}

inline size_t seq_cntr_ref::ops::GetCapacity(Ref const* ref) {
    CallMethod(GetCapacity, GetCapacity);
}

inline void seq_cntr_ref::ops::GetLBCursor(Ref const* ref, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

inline void seq_cntr_ref::ops::GetRBCursor(Ref const* ref, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

inline void* seq_cntr_ref::ops::PeekL(Ref const* ref, bool lazy_copy_elem,
                                      void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr_ref::ops::PeekR(Ref const* ref, bool lazy_copy_elem,
                                      void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr_ref::ops::Access(Ref const* ref, size_t idx,
                                       bool lazy_copy_elem, void* dst_cursor,
                                       void* dst_elem) {
    CallMethod(Access, Access, idx, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr_ref::ops::Derefer(Ref const* ref, void const* pos_cursor,
                                        bool lazy_copy_elem, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename Reader>
void seq_cntr_ref::ops::Read(
    Ref const* ref, void const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Read, FnRead, pos_cursor, cnt, reader, dst_cursor);
}

inline void seq_cntr_ref::ops::Read(Ref const* ref, void const* pos_cursor,
                                    size_t cnt, seq_cntr::MemReader reader,
                                    void* dst_cursor) {
    CallMethod(Read, MemRead, pos_cursor, cnt, reader, dst_cursor);
}

template <typename Writer>
void seq_cntr_ref::ops::Write(
    Ref* ref, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Write, FnWrite, pos_cursor, cnt, writer, dst_cursor);
}

inline void seq_cntr_ref::ops::Write(Ref* ref, void* pos_cursor, size_t cnt,
                                     seq_cntr::MemWriter writer,
                                     void* dst_cursor) {
    CallMethod(Write, MemWrite, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void seq_cntr_ref::ops::ReadWrite(
    Ref* ref, void* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(ReadWrite, FnReadWrite, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

template <typename Writer>
void* seq_cntr_ref::ops::PushL(
    Ref* ref, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(PushL, FnPushL, cnt, writer, dst_cursor);
}

inline void* seq_cntr_ref::ops::PushL(Ref* ref, size_t cnt,
                                      seq_cntr::MemWriter writer,
                                      void* dst_cursor) {
    CallMethod(PushL, MemPushL, cnt, writer, dst_cursor);
}

template <typename Writer>
void* seq_cntr_ref::ops::PushR(
    Ref* ref, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(PushR, FnPushR, cnt, writer, dst_cursor);
}

inline void* seq_cntr_ref::ops::PushR(Ref* ref, size_t cnt,
                                      seq_cntr::MemWriter writer,
                                      void* dst_cursor) {
    CallMethod(PushR, MemPushR, cnt, writer, dst_cursor);
}

template <typename Writer>
void* seq_cntr_ref::ops::Insert(Ref* ref, void* pos_cursor, size_t cnt,
                                Writer&& writer, void* dst_cursor) {
    CallMethod(Insert, FnInsert, pos_cursor, cnt, meta::Forward<Writer>(writer),
               dst_cursor);
}

inline void* seq_cntr_ref::ops::Insert(Ref* ref, void* pos_cursor, size_t cnt,
                                       seq_cntr::MemWriter writer,
                                       void* dst_cursor) {
    CallMethod(Insert, MemInsert, pos_cursor, cnt, writer, dst_cursor);
}

inline void seq_cntr_ref::ops::PopL(Ref* ref, size_t cnt) {
    CallMethod(PopL, PopL, cnt);
}

inline void seq_cntr_ref::ops::PopR(Ref* ref, size_t cnt) {
    CallMethod(PopR, PopR, cnt);
}

inline void seq_cntr_ref::ops::Erase(Ref* ref, void* pos_cursor, size_t cnt) {
    CallMethod(Erase, Erase, pos_cursor, cnt);
}

inline void seq_cntr_ref::ops::EraseAll(Ref* ref) {
    CallMethod(EraseAll, EraseAll);
}

inline void seq_cntr_ref::ops::CopyCursor(Ref const* ref,
                                          void const* src_cursor,
                                          void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

inline bool seq_cntr_ref::ops::AreEqualCursor(Ref const* ref,
                                              void const* cursor_a,
                                              void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

inline int seq_cntr_ref::ops::CompareCursor(Ref const* ref,
                                            void const* cursor_a,
                                            void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

inline size_t seq_cntr_ref::ops::GetCursorDist(Ref const* ref,
                                               void const* cursor_a,
                                               void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

inline size_t seq_cntr_ref::ops::GetCursorIdx(Ref const* ref,
                                              void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

inline void seq_cntr_ref::ops::CursorStepL(Ref const* ref, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

inline void seq_cntr_ref::ops::CursorStepR(Ref const* ref, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

inline void seq_cntr_ref::ops::CursorAdvanceL(Ref const* ref, void* cursor,
                                              size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

inline void seq_cntr_ref::ops::CursorAdvanceR(Ref const* ref, void* cursor,
                                              size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

inline void seq_cntr_ref::ops::CheckRef(Ref* ref) {
    ZETA_Core_DebugAssert(ref != nullptr);
    ZETA_Core_DebugAssert(0 < ref->width);
    ZETA_Core_DebugAssert(ref->vtable != nullptr);
    ZETA_Core_DebugAssert(ref->cntr != nullptr);

    seq_cntr::AbilityFlag enabled_ability_flag{
        ref->dynamic_enabled_ability_flag
    };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(ability, method)                                     \
    ZETA_Core_DebugAssert(!TestAbility(enabled_ability_flag, ability) || \
                          ref->vtable->method != nullptr);

    CheckMethod(GetSize, GetSize);
    CheckMethod(GetCapacity, GetCapacity);

    CheckMethod(GetLBCursor, GetLBCursor);
    CheckMethod(GetRBCursor, GetRBCursor);
    CheckMethod(PeekL, PeekL);
    CheckMethod(PeekR, PeekR);
    CheckMethod(Access, Access);
    CheckMethod(Derefer, Derefer);

    CheckMethod(Read, FnRead);
    CheckMethod(Write, FnWrite);
    CheckMethod(ReadWrite, FnReadWrite);

    CheckMethod(Read, MemRead);
    CheckMethod(Write, MemWrite);

    CheckMethod(PushL, FnPushL);
    CheckMethod(PushR, FnPushR);
    CheckMethod(Insert, FnInsert);

    CheckMethod(PushL, MemPushL);
    CheckMethod(PushR, MemPushR);
    CheckMethod(Insert, MemInsert);

    CheckMethod(PopL, PopL);
    CheckMethod(PopR, PopR);
    CheckMethod(Erase, Erase);
    CheckMethod(EraseAll, EraseAll);

    CheckMethod(CopyCursor, CopyCursor);
    CheckMethod(AreEqualCursor, AreEqualCursor);
    CheckMethod(CompareCursor, CompareCursor);
    CheckMethod(GetCursorDist, GetCursorDist);
    CheckMethod(GetCursorIdx, GetCursorIdx);
    CheckMethod(CursorStepL, CursorStepL);
    CheckMethod(CursorStepR, CursorStepR);
    CheckMethod(CursorAdvanceL, CursorAdvanceL);
    CheckMethod(CursorAdvanceR, CursorAdvanceR);

#pragma pop_macro("CheckMethod")
}

template <typename SeqCntrLike>
seq_cntr_ref::Ref
    seq_cntr_ref::ops::MakeRef  // NOLINT(misc-use-internal-linkage)
    (SeqCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr_)) };
    using SeqCntr = meta::RemovePointer<decltype(cntr)>;

    seq_cntr::ops::CheckContract(cntr);

    size_t cursor_size{ seq_cntr::ops::GetCursorSize(cntr) };

    return {
        .cursor_size = cursor_size,

        .width = seq_cntr::ops::GetWidth(cntr),
        .capacity = seq_cntr::ops::GetCapacity(cntr),

        .dynamic_enabled_ability_flag =
            seq_cntr::ops::GetStaticEnabledAbilityFlag<SeqCntr>() |
            seq_cntr::ops::GetDynamicEnabledAbilityFlag(cntr),
        .dynamic_disabled_ability_flag =
            seq_cntr::ops::GetStaticDisabledAbilityFlag<SeqCntr>() |
            seq_cntr::ops::GetDynamicDisabledAbilityFlag(cntr),

        .vtable = &seq_cntr::ops::GetVTable<SeqCntr>(),

        .cntr = const_cast<void*>(static_cast<void const*>(cntr)),
    };
}

inline void* seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetReferedInstPtr(
    seq_cntr_ref::Ref const* ref) {
    return ref->cntr;
}

constexpr seq_cntr::AbilityFlag
seq_cntr::Traits<seq_cntr_ref::Ref, void>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,

        .GetWidth = true,
        .GetSize = true,
        .GetCapacity = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Access = true,
        .Derefer = true,

        .Read = true,
        .Write = true,
        .ReadWrite = true,

        .PushL = true,
        .PushR = true,
        .Insert = true,

        .PopL = true,
        .PopR = true,
        .Erase = true,
        .EraseAll = true,

        .CopyCursor = true,

        .AreEqualCursor = true,
        .CompareCursor = true,
        .GetCursorDist = true,
        .GetCursorIdx = true,

        .CursorStepL = true,
        .CursorStepR = true,

        .CursorAdvanceL = true,
        .CursorAdvanceR = true,
    }();
}

constexpr seq_cntr::AbilityFlag
seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::Traits<seq_cntr_ref::Ref,
                            void>::GetStaticEnabledAbilityFlag() &
           seq_cntr::const_ability_flag;
}

constexpr seq_cntr::AbilityFlag
seq_cntr::Traits<seq_cntr_ref::Ref, void>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag seq_cntr::Traits<
    seq_cntr_ref::Ref const, void>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::non_const_ability_flag;
}

constexpr seq_cntr::AbilityFlag
seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetDynamicEnabledAbilityFlag(
    seq_cntr_ref::Ref const*) {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag
seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetDynamicDisabledAbilityFlag(
    seq_cntr_ref::Ref const*) {
    return seq_cntr::empty_ability_flag;
}

inline size_t seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetCursorSize(
    seq_cntr_ref::Ref const* ref) {
    return seq_cntr_ref::ops::GetCursorSize(ref);
}

inline size_t seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetWidth(
    seq_cntr_ref::Ref const* ref) {
    return seq_cntr_ref::ops::GetWidth(ref);
}

inline size_t seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetSize(
    seq_cntr_ref::Ref const* ref) {
    return seq_cntr_ref::ops::GetSize(ref);
}

inline size_t seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetCapacity(
    seq_cntr_ref::Ref const* ref) {
    return seq_cntr_ref::ops::GetCapacity(ref);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetLBCursor(
    seq_cntr_ref::Ref const* ref, void* dst_cursor) {
    seq_cntr_ref::ops::GetLBCursor(ref, dst_cursor);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetRBCursor(
    seq_cntr_ref::Ref const* ref, void* dst_cursor) {
    seq_cntr_ref::ops::GetRBCursor(ref, dst_cursor);
}

inline void* seq_cntr::Traits<seq_cntr_ref::Ref const, void>::PeekL(
    seq_cntr_ref::Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return seq_cntr_ref::ops::PeekL(ref, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr::Traits<seq_cntr_ref::Ref const, void>::PeekR(
    seq_cntr_ref::Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return seq_cntr_ref::ops::PeekR(ref, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr::Traits<seq_cntr_ref::Ref const, void>::Access(
    seq_cntr_ref::Ref const* ref, size_t idx, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return seq_cntr_ref::ops::Access(ref, idx, lazy_copy_elem, dst_cursor,
                                     dst_elem);
}

inline void* seq_cntr::Traits<seq_cntr_ref::Ref const, void>::Derefer(
    seq_cntr_ref::Ref const* ref, void const* pos_cursor, bool lazy_copy_elem,
    void* dst_elem) {
    return seq_cntr_ref::ops::Derefer(ref, pos_cursor, lazy_copy_elem,
                                      dst_elem);
}

template <typename Reader>
void seq_cntr::Traits<seq_cntr_ref::Ref const, void>::Read(
    seq_cntr_ref::Ref const* ref, void const* pos_cursor, size_t cnt,
    Reader&& reader, void* dst_cursor) {
    seq_cntr_ref::ops::Read(ref, pos_cursor, cnt, meta::Forward<Reader>(reader),
                            dst_cursor);
}

template <typename Writer>
void seq_cntr::Traits<seq_cntr_ref::Ref, void>::Write(seq_cntr_ref::Ref* ref,
                                                      void* pos_cursor,
                                                      size_t cnt,
                                                      Writer&& writer,
                                                      void* dst_cursor) {
    seq_cntr_ref::ops::Write(ref, pos_cursor, cnt,
                             meta::Forward<Writer>(writer), dst_cursor);
}

template <typename ReaderWriter>
void seq_cntr::Traits<seq_cntr_ref::Ref, void>::ReadWrite(
    seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt,
    ReaderWriter&& reader_writer, void* dst_cursor) {
    seq_cntr_ref::ops::ReadWrite(ref, pos_cursor, cnt,
                                 meta::Forward<ReaderWriter>(reader_writer),
                                 dst_cursor);
}

template <typename Writer>
void* seq_cntr::Traits<seq_cntr_ref::Ref, void>::PushL(seq_cntr_ref::Ref* ref,
                                                       size_t cnt,
                                                       Writer&& writer,
                                                       void* dst_cursor) {
    return seq_cntr_ref::ops::PushL(ref, cnt, meta::Forward<Writer>(writer),
                                    dst_cursor);
}

template <typename Writer>
void* seq_cntr::Traits<seq_cntr_ref::Ref, void>::PushR(seq_cntr_ref::Ref* ref,
                                                       size_t cnt,
                                                       Writer&& writer,
                                                       void* dst_cursor) {
    return seq_cntr_ref::ops::PushR(ref, cnt, meta::Forward<Writer>(writer),
                                    dst_cursor);
}

template <typename Writer>
void* seq_cntr::Traits<seq_cntr_ref::Ref, void>::Insert(seq_cntr_ref::Ref* ref,
                                                        void* pos_cursor,
                                                        size_t cnt,
                                                        Writer&& writer,
                                                        void* dst_cursor) {
    return seq_cntr_ref::ops::Insert(ref, pos_cursor, cnt,
                                     meta::Forward<Writer>(writer), dst_cursor);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref, void>::PopL(
    seq_cntr_ref::Ref* ref, size_t cnt) {
    seq_cntr_ref::ops::PopL(ref, cnt);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref, void>::PopR(
    seq_cntr_ref::Ref* ref, size_t cnt) {
    seq_cntr_ref::ops::PopR(ref, cnt);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref, void>::Erase(
    seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt) {
    seq_cntr_ref::ops::Erase(ref, pos_cursor, cnt);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref, void>::EraseAll(
    seq_cntr_ref::Ref* ref) {
    seq_cntr_ref::ops::EraseAll(ref);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref const, void>::CopyCursor(
    seq_cntr_ref::Ref const* ref, void const* src_cursor, void* dst_cursor) {
    seq_cntr_ref::ops::CopyCursor(ref, src_cursor, dst_cursor);
}

inline bool seq_cntr::Traits<seq_cntr_ref::Ref const, void>::AreEqualCursor(
    seq_cntr_ref::Ref const* ref, void const* cursor_a, void const* cursor_b) {
    return seq_cntr_ref::ops::AreEqualCursor(ref, cursor_a, cursor_b);
}

inline int seq_cntr::Traits<seq_cntr_ref::Ref const, void>::CompareCursor(
    seq_cntr_ref::Ref const* ref, void const* cursor_a, void const* cursor_b) {
    return seq_cntr_ref::ops::CompareCursor(ref, cursor_a, cursor_b);
}

inline size_t seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetCursorDist(
    seq_cntr_ref::Ref const* ref, void const* cursor_a, void const* cursor_b) {
    return seq_cntr_ref::ops::GetCursorDist(ref, cursor_a, cursor_b);
}

inline size_t seq_cntr::Traits<seq_cntr_ref::Ref const, void>::GetCursorIdx(
    seq_cntr_ref::Ref const* ref, void const* cursor) {
    return seq_cntr_ref::ops::GetCursorIdx(ref, cursor);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref const, void>::CursorStepL(
    seq_cntr_ref::Ref const* ref, void* cursor) {
    seq_cntr_ref::ops::CursorStepL(ref, cursor);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref const, void>::CursorStepR(
    seq_cntr_ref::Ref const* ref, void* cursor) {
    seq_cntr_ref::ops::CursorStepR(ref, cursor);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref const, void>::CursorAdvanceL(
    seq_cntr_ref::Ref const* ref, void* cursor, size_t step) {
    seq_cntr_ref::ops::CursorAdvanceL(ref, cursor, step);
}

inline void seq_cntr::Traits<seq_cntr_ref::Ref const, void>::CursorAdvanceR(
    seq_cntr_ref::Ref const* ref, void* cursor, size_t step) {
    seq_cntr_ref::ops::CursorAdvanceR(ref, cursor, step);
}

#pragma pop_macro("TestAbility")

}  // namespace zeta::core
