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

inline seq_cntr::AbilityFlag seq_cntr_ref::GetDynamicDisabledAbilityFlag(
    Ref* ref) {
    return ref->dynamic_disabled_ability_flag;
}

inline size_t seq_cntr_ref::GetCursorSize(Ref const* ref) {
    return ref->cursor_size;
}

inline size_t seq_cntr_ref::GetElemSize(Ref const* ref) { return ref->width; }

inline size_t seq_cntr_ref::GetElemCnt(Ref const* ref) {
    CallMethod(GetElemCnt, GetElemCnt);
}

inline size_t seq_cntr_ref::GetMaxElemCnt(Ref const* ref) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt);
}

inline void seq_cntr_ref::GetLBCursor(Ref const* ref, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

inline void seq_cntr_ref::GetRBCursor(Ref const* ref, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

inline void* seq_cntr_ref::PeekL(Ref const* ref, bool lazy_copy_elem,
                                 void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr_ref::PeekR(Ref const* ref, bool lazy_copy_elem,
                                 void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr_ref::Access(Ref const* ref, size_t idx,
                                  bool lazy_copy_elem, void* dst_cursor,
                                  void* dst_elem) {
    CallMethod(Access, Access, idx, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr_ref::Derefer(Ref const* ref, void const* pos_cursor,
                                   bool lazy_copy_elem, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename Reader>
void seq_cntr_ref::Read(
    Ref const* ref, void const* pos_cursor, size_t cnt,
    Reader&& reader,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Read, FnRead, pos_cursor, cnt, reader, dst_cursor);
}

inline void seq_cntr_ref::Read(Ref const* ref, void const* pos_cursor,
                               size_t cnt, seq_cntr::MemReader reader,
                               void* dst_cursor) {
    CallMethod(Read, MemRead, pos_cursor, cnt, reader, dst_cursor);
}

template <typename Writer>
void seq_cntr_ref::Write(
    Ref* ref, void* pos_cursor, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(Write, FnWrite, pos_cursor, cnt, writer, dst_cursor);
}

inline void seq_cntr_ref::Write(Ref* ref, void* pos_cursor, size_t cnt,
                                seq_cntr::MemWriter writer, void* dst_cursor) {
    CallMethod(Write, MemWrite, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void seq_cntr_ref::ReadWrite(
    Ref* ref, void* pos_cursor, size_t cnt,
    ReaderWriter&&
        reader_writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(ReadWrite, FnReadWrite, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

template <typename Writer>
void* seq_cntr_ref::PushL(
    Ref* ref, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(PushL, FnPushL, cnt, writer, dst_cursor);
}

inline void* seq_cntr_ref::PushL(Ref* ref, size_t cnt,
                                 seq_cntr::MemWriter writer, void* dst_cursor) {
    CallMethod(PushL, MemPushL, cnt, writer, dst_cursor);
}

template <typename Writer>
void* seq_cntr_ref::PushR(
    Ref* ref, size_t cnt,
    Writer&& writer,  // NOLINT(cppcoreguidelines-missing-std-forward)
    void* dst_cursor) {
    CallMethod(PushR, FnPushR, cnt, writer, dst_cursor);
}

inline void* seq_cntr_ref::PushR(Ref* ref, size_t cnt,
                                 seq_cntr::MemWriter writer, void* dst_cursor) {
    CallMethod(PushR, MemPushR, cnt, writer, dst_cursor);
}

template <typename Writer>
void* seq_cntr_ref::Insert(Ref* ref, void* pos_cursor, size_t cnt,
                           Writer&& writer, void* dst_cursor) {
    CallMethod(Insert, FnInsert, pos_cursor, cnt, meta::Forward<Writer>(writer),
               dst_cursor);
}

inline void* seq_cntr_ref::Insert(Ref* ref, void* pos_cursor, size_t cnt,
                                  seq_cntr::MemWriter writer,
                                  void* dst_cursor) {
    CallMethod(Insert, MemInsert, pos_cursor, cnt, writer, dst_cursor);
}

inline void seq_cntr_ref::PopL(Ref* ref, size_t cnt) {
    CallMethod(PopL, PopL, cnt);
}

inline void seq_cntr_ref::PopR(Ref* ref, size_t cnt) {
    CallMethod(PopR, PopR, cnt);
}

inline void seq_cntr_ref::Erase(Ref* ref, void* pos_cursor, size_t cnt) {
    CallMethod(Erase, Erase, pos_cursor, cnt);
}

inline void seq_cntr_ref::EraseAll(Ref* ref) { CallMethod(EraseAll, EraseAll); }

inline void seq_cntr_ref::CopyCursor(Ref const* ref, void const* src_cursor,
                                     void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

inline bool seq_cntr_ref::AreEqualCursor(Ref const* ref, void const* cursor_a,
                                         void const* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

inline int seq_cntr_ref::CompareCursor(Ref const* ref, void const* cursor_a,
                                       void const* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

inline size_t seq_cntr_ref::GetCursorDist(Ref const* ref, void const* cursor_a,
                                          void const* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

inline size_t seq_cntr_ref::GetCursorIdx(Ref const* ref, void const* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

inline void seq_cntr_ref::CursorStepL(Ref const* ref, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

inline void seq_cntr_ref::CursorStepR(Ref const* ref, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

inline void seq_cntr_ref::CursorAdvanceL(Ref const* ref, void* cursor,
                                         size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

inline void seq_cntr_ref::CursorAdvanceR(Ref const* ref, void* cursor,
                                         size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

inline void seq_cntr_ref::CheckRef(Ref* ref) {
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

    CheckMethod(GetElemCnt, GetElemCnt);
    CheckMethod(GetMaxElemCnt, GetMaxElemCnt);

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
seq_cntr_ref::Ref seq_cntr_ref::MakeRef  // NOLINT(misc-use-internal-linkage)
    (SeqCntrLike&& cntr_) {
    auto* cntr{ utils::GetInstPtr(meta::Forward<SeqCntrLike>(cntr_)) };
    using SeqCntr = meta::RemovePointer<decltype(cntr)>;

    seq_cntr::CheckContract(cntr);

    size_t cursor_size{ seq_cntr::GetCursorSize(cntr) };

    return {
        .cursor_size = cursor_size,

        .width = seq_cntr::GetElemSize(cntr),
        .capacity = seq_cntr::GetMaxElemCnt(cntr),

        .dynamic_enabled_ability_flag =
            seq_cntr::GetStaticEnabledAbilityFlag<SeqCntr>() |
            seq_cntr::GetDynamicEnabledAbilityFlag(cntr),
        .dynamic_disabled_ability_flag =
            seq_cntr::GetStaticDisabledAbilityFlag<SeqCntr>() |
            seq_cntr::GetDynamicDisabledAbilityFlag(cntr),

        .vtable = &seq_cntr::GetVTable<SeqCntr>(),

        .cntr = const_cast<void*>(static_cast<void const*>(cntr)),
    };
}

inline void*
seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetReferedInstPtr(
    seq_cntr_ref::Ref const* ref) {
    return ref->cntr;
}

constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::AbilityFlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

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

constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const, void>::GetStaticEnabledAbilityFlag() {
    return seq_cntr::CntrTraits<seq_cntr_ref::Ref,
                                void>::GetStaticEnabledAbilityFlag() &
           seq_cntr::const_ability_flag;
}

constexpr seq_cntr::AbilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const, void>::GetStaticDisabledAbilityFlag() {
    return seq_cntr::non_const_ability_flag;
}

constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const,
    void>::GetDynamicEnabledAbilityFlag(seq_cntr_ref::Ref const*) {
    return seq_cntr::empty_ability_flag;
}

constexpr seq_cntr::AbilityFlag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const,
    void>::GetDynamicDisabledAbilityFlag(seq_cntr_ref::Ref const*) {
    return seq_cntr::empty_ability_flag;
}

inline size_t
seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetCursorSize(
    seq_cntr_ref::Ref const* ref) {
    return seq_cntr_ref::GetCursorSize(ref);
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetElemSize(
    seq_cntr_ref::Ref const* ref) {
    return seq_cntr_ref::GetElemSize(ref);
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetElemCnt(
    seq_cntr_ref::Ref const* ref) {
    return seq_cntr_ref::GetElemCnt(ref);
}

inline size_t
seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetMaxElemCnt(
    seq_cntr_ref::Ref const* ref) {
    return seq_cntr_ref::GetMaxElemCnt(ref);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetLBCursor(
    seq_cntr_ref::Ref const* ref, void* dst_cursor) {
    seq_cntr_ref::GetLBCursor(ref, dst_cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetRBCursor(
    seq_cntr_ref::Ref const* ref, void* dst_cursor) {
    seq_cntr_ref::GetRBCursor(ref, dst_cursor);
}

inline void* seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::PeekL(
    seq_cntr_ref::Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return seq_cntr_ref::PeekL(ref, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::PeekR(
    seq_cntr_ref::Ref const* ref, bool lazy_copy_elem, void* dst_cursor,
    void* dst_elem) {
    return seq_cntr_ref::PeekR(ref, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::Access(
    seq_cntr_ref::Ref const* ref, size_t idx, bool lazy_copy_elem,
    void* dst_cursor, void* dst_elem) {
    return seq_cntr_ref::Access(ref, idx, lazy_copy_elem, dst_cursor, dst_elem);
}

inline void* seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::Derefer(
    seq_cntr_ref::Ref const* ref, void const* pos_cursor, bool lazy_copy_elem,
    void* dst_elem) {
    return seq_cntr_ref::Derefer(ref, pos_cursor, lazy_copy_elem, dst_elem);
}

template <typename Reader>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::Read(
    seq_cntr_ref::Ref const* ref, void const* pos_cursor, size_t cnt,
    Reader&& reader, void* dst_cursor) {
    seq_cntr_ref::Read(ref, pos_cursor, cnt, meta::Forward<Reader>(reader),
                       dst_cursor);
}

template <typename Writer>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::Write(
    seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt, Writer&& writer,
    void* dst_cursor) {
    seq_cntr_ref::Write(ref, pos_cursor, cnt, meta::Forward<Writer>(writer),
                        dst_cursor);
}

template <typename ReaderWriter>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::ReadWrite(
    seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt,
    ReaderWriter&& reader_writer, void* dst_cursor) {
    seq_cntr_ref::ReadWrite(ref, pos_cursor, cnt,
                            meta::Forward<ReaderWriter>(reader_writer),
                            dst_cursor);
}

template <typename Writer>
void* seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::PushL(
    seq_cntr_ref::Ref* ref, size_t cnt, Writer&& writer, void* dst_cursor) {
    return seq_cntr_ref::PushL(ref, cnt, meta::Forward<Writer>(writer),
                               dst_cursor);
}

template <typename Writer>
void* seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::PushR(
    seq_cntr_ref::Ref* ref, size_t cnt, Writer&& writer, void* dst_cursor) {
    return seq_cntr_ref::PushR(ref, cnt, meta::Forward<Writer>(writer),
                               dst_cursor);
}

template <typename Writer>
void* seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::Insert(
    seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt, Writer&& writer,
    void* dst_cursor) {
    return seq_cntr_ref::Insert(ref, pos_cursor, cnt,
                                meta::Forward<Writer>(writer), dst_cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::PopL(
    seq_cntr_ref::Ref* ref, size_t cnt) {
    seq_cntr_ref::PopL(ref, cnt);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::PopR(
    seq_cntr_ref::Ref* ref, size_t cnt) {
    seq_cntr_ref::PopR(ref, cnt);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::Erase(
    seq_cntr_ref::Ref* ref, void* pos_cursor, size_t cnt) {
    seq_cntr_ref::Erase(ref, pos_cursor, cnt);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref, void>::EraseAll(
    seq_cntr_ref::Ref* ref) {
    seq_cntr_ref::EraseAll(ref);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::CopyCursor(
    seq_cntr_ref::Ref const* ref, void const* src_cursor, void* dst_cursor) {
    seq_cntr_ref::CopyCursor(ref, src_cursor, dst_cursor);
}

inline bool seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::AreEqualCursor(
    seq_cntr_ref::Ref const* ref, void const* cursor_a, void const* cursor_b) {
    return seq_cntr_ref::AreEqualCursor(ref, cursor_a, cursor_b);
}

inline int seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::CompareCursor(
    seq_cntr_ref::Ref const* ref, void const* cursor_a, void const* cursor_b) {
    return seq_cntr_ref::CompareCursor(ref, cursor_a, cursor_b);
}

inline size_t
seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetCursorDist(
    seq_cntr_ref::Ref const* ref, void const* cursor_a, void const* cursor_b) {
    return seq_cntr_ref::GetCursorDist(ref, cursor_a, cursor_b);
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::GetCursorIdx(
    seq_cntr_ref::Ref const* ref, void const* cursor) {
    return seq_cntr_ref::GetCursorIdx(ref, cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::CursorStepL(
    seq_cntr_ref::Ref const* ref, void* cursor) {
    seq_cntr_ref::CursorStepL(ref, cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::CursorStepR(
    seq_cntr_ref::Ref const* ref, void* cursor) {
    seq_cntr_ref::CursorStepR(ref, cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::CursorAdvanceL(
    seq_cntr_ref::Ref const* ref, void* cursor, size_t step) {
    seq_cntr_ref::CursorAdvanceL(ref, cursor, step);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const, void>::CursorAdvanceR(
    seq_cntr_ref::Ref const* ref, void* cursor, size_t step) {
    seq_cntr_ref::CursorAdvanceR(ref, cursor, step);
}

#pragma pop_macro("TestAbility")

}  // namespace zeta::core
