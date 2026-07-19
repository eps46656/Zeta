#pragma once

#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_cntr_ref.hpp>

namespace zeta::core {

#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(capability_flag, capability)                 \
    (((capability_flag) & (static_cast<seq_cntr::CapabilityFlag>(1) \
                           << seq_cntr::CapabilityEnum::capability)) != 0)

#pragma push_macro("CallMethod_")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod_(method_ptr, capability, method, ...)                      \
    {                                                                         \
        ZETA_Core_DebugAssert(                                                \
            TestCapability(ref.dynamic_enabled_capability_flag, capability)); \
                                                                              \
        auto method_ptr{ ref.vtable->method };                                \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                         \
                                                                              \
        return method_ptr(ref.cntr, __VA_ARGS__);                             \
    }                                                                         \
    ZETA_Core_StaticAssert(true);

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(capability, method, ...) \
    CallMethod_(ZETA_Core_TmpName, capability, method, __VA_ARGS__)

inline seq_cntr::CapabilityFlag seq_cntr_ref::GetDynamicDisabledCapabilityFlag(
    Ref& ref) {
    return ref.dynamic_disabled_capability_flag;
}

inline size_t seq_cntr_ref::GetCursorSize(Ref const& ref) {
    return ref.cursor_size;
}

inline size_t seq_cntr_ref::GetElemSize(Ref const& ref) { return ref.width; }

inline size_t seq_cntr_ref::GetElemCnt(Ref const& ref) {
    CallMethod(GetElemCnt, GetElemCnt);
}

inline size_t seq_cntr_ref::GetMaxElemCnt(Ref const& ref) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt);
}

inline void seq_cntr_ref::GetLBCursor(Ref const& ref, void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

inline void seq_cntr_ref::GetRBCursor(Ref const& ref, void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

inline void seq_cntr_ref::PeekL(Ref const& ref, bool lazy_copy_elem,
                                seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

inline void seq_cntr_ref::PeekR(Ref const& ref, bool lazy_copy_elem,
                                seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

inline void seq_cntr_ref::Refer(Ref const& ref, size_t idx, bool lazy_copy_elem,
                                seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                void* dst_cursor, void* dst_elem) {
    CallMethod(Refer, Refer, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

inline void seq_cntr_ref::Derefer(Ref const& ref, void* pos_cursor,
                                  bool lazy_copy_elem,
                                  seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                  void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem);
}

template <typename Reader>
void seq_cntr_ref::Read(Ref const& ref, void* pos_cursor, size_t cnt,
                        Reader&& reader, void* dst_cursor) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(Read, EmptyRead, pos_cursor, cnt, reader, dst_cursor);
    } else if constexpr (meta::IsSame<meta::RemoveCVRef<Reader>,
                                      seq_cntr::MemReader>) {
        CallMethod(Read, MemRead, pos_cursor, cnt, reader, dst_cursor);
    } else {
        CallMethod(Read, FnRead, pos_cursor, cnt, reader, dst_cursor);
    }
}

template <typename Writer>
void seq_cntr_ref::Write(Ref& ref, void* pos_cursor, size_t cnt,
                         Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(Write, EmptyWrite, pos_cursor, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::MemWriter>) {
        CallMethod(Write, MemWrite, pos_cursor, cnt, writer, dst_cursor);
    } else {
        CallMethod(Write, FnWrite, pos_cursor, cnt, writer, dst_cursor);
    }
}

template <typename ReaderWriter>
void seq_cntr_ref::ReadWrite(Ref& ref, void* pos_cursor, size_t cnt,
                             ReaderWriter&& reader_writer, void* dst_cursor) {
    CallMethod(ReadWrite, FnReadWrite, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

template <typename Writer>
void seq_cntr_ref::PushL(Ref& ref, size_t cnt, Writer&& writer,
                         void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(PushL, EmptyPushL, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::MemWriter>) {
        CallMethod(PushL, MemPushL, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushL, FnPushL, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
void seq_cntr_ref::PushR(Ref& ref, size_t cnt, Writer&& writer,
                         void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(PushR, EmptyPushR, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::MemWriter>) {
        CallMethod(PushR, MemPushR, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushR, FnPushR, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
void seq_cntr_ref::Insert(Ref& ref, void* pos_cursor, size_t cnt,
                          Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(Insert, EmptyInsert, pos_cursor, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::MemWriter>) {
        CallMethod(Insert, MemInsert, pos_cursor, cnt, writer, dst_cursor);
    } else {
        CallMethod(Insert, FnInsert, pos_cursor, cnt, writer, dst_cursor);
    }
}

template <typename Reader>
void seq_cntr_ref::PopL(Ref& ref, size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(PopL, EmptyPopL, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::MemReader>) {
        CallMethod(PopL, MemPopL, cnt, reader);
    } else {
        CallMethod(PopL, FnPopL, cnt, reader);
    }
}

template <typename Reader>
void seq_cntr_ref::PopR(Ref& ref, size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(PopR, EmptyPopR, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::MemReader>) {
        CallMethod(PopR, MemPopR, cnt, reader);
    } else {
        CallMethod(PopR, FnPopR, cnt, reader);
    }
}

template <typename Reader>
void seq_cntr_ref::Erase(Ref& ref, void* pos_cursor, size_t cnt,
                         Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(Erase, EmptyErase, pos_cursor, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::MemReader>) {
        CallMethod(Erase, MemErase, pos_cursor, cnt, reader);
    } else {
        CallMethod(Erase, FnErase, pos_cursor, cnt, reader);
    }
}

inline void seq_cntr_ref::EraseAll(Ref& ref) { CallMethod(EraseAll, EraseAll); }

inline void seq_cntr_ref::CopyCursor(Ref const& ref, void* src_cursor,
                                     void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

inline bool seq_cntr_ref::AreEqualCursor(Ref const& ref, void* cursor_a,
                                         void* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

inline int seq_cntr_ref::CompareCursor(Ref const& ref, void* cursor_a,
                                       void* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

inline size_t seq_cntr_ref::GetCursorDist(Ref const& ref, void* cursor_a,
                                          void* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

inline size_t seq_cntr_ref::GetCursorIdx(Ref const& ref, void* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

inline void seq_cntr_ref::CursorStepL(Ref const& ref, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

inline void seq_cntr_ref::CursorStepR(Ref const& ref, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

inline void seq_cntr_ref::CursorAdvanceL(Ref const& ref, void* cursor,
                                         size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

inline void seq_cntr_ref::CursorAdvanceR(Ref const& ref, void* cursor,
                                         size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

inline void seq_cntr_ref::CheckRef(Ref& ref) {
    ZETA_Core_DebugAssert(0 < ref.width);
    ZETA_Core_DebugAssert(ref.vtable != nullptr);
    ZETA_Core_DebugAssert(ref.cntr != nullptr);

    seq_cntr::CapabilityFlag enabled_capability_flag{
        ref.dynamic_enabled_capability_flag
    };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(capability, method)                         \
    ZETA_Core_DebugAssert(                                      \
        !TestCapability(enabled_capability_flag, capability) || \
        ref.vtable->method != nullptr);

    CheckMethod(GetElemCnt, GetElemCnt);
    CheckMethod(GetMaxElemCnt, GetMaxElemCnt);

    CheckMethod(GetLBCursor, GetLBCursor);
    CheckMethod(GetRBCursor, GetRBCursor);
    CheckMethod(PeekL, PeekL);
    CheckMethod(PeekR, PeekR);
    CheckMethod(Refer, Refer);
    CheckMethod(Derefer, Derefer);

    CheckMethod(Read, EmptyRead);
    CheckMethod(Read, MemRead);
    CheckMethod(Read, FnRead);

    CheckMethod(Write, EmptyWrite);
    CheckMethod(Write, MemWrite);
    CheckMethod(Write, FnWrite);

    CheckMethod(ReadWrite, FnReadWrite);

    CheckMethod(PushL, EmptyPushL);
    CheckMethod(PushL, MemPushL);
    CheckMethod(PushL, FnPushL);

    CheckMethod(PushR, EmptyPushR);
    CheckMethod(PushR, MemPushR);
    CheckMethod(PushR, FnPushR);

    CheckMethod(Insert, EmptyInsert);
    CheckMethod(Insert, MemInsert);
    CheckMethod(Insert, FnInsert);

    CheckMethod(PopL, EmptyPopL);
    CheckMethod(PopL, MemPopL);
    CheckMethod(PopL, FnPopL);

    CheckMethod(PopR, EmptyPopR);
    CheckMethod(PopR, MemPopR);
    CheckMethod(PopR, FnPopR);

    CheckMethod(Erase, EmptyErase);
    CheckMethod(Erase, MemErase);
    CheckMethod(Erase, FnErase);

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

template <typename Cntr>
seq_cntr_ref::Ref seq_cntr_ref::MakeRef  // NOLINT(misc-use-internal-linkage)
    (Cntr& cntr) {
    seq_cntr::CheckContract(cntr);

    size_t cursor_size{ seq_cntr::GetCursorSize(cntr) };

    return {
        .cursor_size = cursor_size,

        .width = seq_cntr::GetElemSize(cntr),
        .capacity = seq_cntr::GetMaxElemCnt(cntr),

        .dynamic_enabled_capability_flag =
            seq_cntr::GetStaticEnabledCapabilityFlag<Cntr>() |
            seq_cntr::GetDynamicEnabledCapabilityFlag(cntr),
        .dynamic_disabled_capability_flag =
            seq_cntr::GetStaticDisabledCapabilityFlag<Cntr>() |
            seq_cntr::GetDynamicDisabledCapabilityFlag(cntr),

        .vtable = &seq_cntr::GetVTable<Cntr>(),

        .cntr = const_cast<void*>(static_cast<void const*>(&cntr)),
    };
}

inline void* seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetReferedInstPtr(
    seq_cntr_ref::Ref const& ref) {
    return ref.cntr;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::CapabilityFlagBuilder{
        .GetCursorSize = true,

        .GetElemSize = true,
        .GetElemCnt = true,
        .GetMaxElemCnt = true,

        .GetLBCursor = true,
        .GetRBCursor = true,

        .PeekL = true,
        .PeekR = true,

        .Refer = true,
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

constexpr seq_cntr::CapabilityFlag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::CntrTraits<
               seq_cntr_ref::Ref>::GetStaticEnabledCapabilityFlag() &
           seq_cntr::const_capability_flag;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::empty_capability_flag;
}

constexpr seq_cntr::CapabilityFlag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::non_const_capability_flag;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetDynamicEnabledCapabilityFlag(
    seq_cntr_ref::Ref const&) {
    return seq_cntr::empty_capability_flag;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetDynamicDisabledCapabilityFlag(
    seq_cntr_ref::Ref const&) {
    return seq_cntr::empty_capability_flag;
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetCursorSize(
    seq_cntr_ref::Ref const& ref) {
    return seq_cntr_ref::GetCursorSize(ref);
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetElemSize(
    seq_cntr_ref::Ref const& ref) {
    return seq_cntr_ref::GetElemSize(ref);
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetElemCnt(
    seq_cntr_ref::Ref const& ref) {
    return seq_cntr_ref::GetElemCnt(ref);
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetMaxElemCnt(
    seq_cntr_ref::Ref const& ref) {
    return seq_cntr_ref::GetMaxElemCnt(ref);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetLBCursor(
    seq_cntr_ref::Ref const& ref, void* dst_cursor) {
    seq_cntr_ref::GetLBCursor(ref, dst_cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetRBCursor(
    seq_cntr_ref::Ref const& ref, void* dst_cursor) {
    seq_cntr_ref::GetRBCursor(ref, dst_cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::PeekL(
    seq_cntr_ref::Ref const& ref, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    return seq_cntr_ref::PeekL(ref, lazy_copy_elem, dst_elem_ptr_view,
                               dst_cursor, dst_elem);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::PeekR(
    seq_cntr_ref::Ref const& ref, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    return seq_cntr_ref::PeekR(ref, lazy_copy_elem, dst_elem_ptr_view,
                               dst_cursor, dst_elem);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::Refer(
    seq_cntr_ref::Ref const& ref, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    return seq_cntr_ref::Refer(ref, idx, lazy_copy_elem, dst_elem_ptr_view,
                               dst_cursor, dst_elem);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::Derefer(
    seq_cntr_ref::Ref const& ref, void* pos_cursor, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    return seq_cntr_ref::Derefer(ref, pos_cursor, lazy_copy_elem,
                                 dst_elem_ptr_view, dst_elem);
}

template <typename Reader>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::Read(
    seq_cntr_ref::Ref const& ref, void* pos_cursor, size_t cnt, Reader&& reader,
    void* dst_cursor) {
    seq_cntr_ref::Read(ref, pos_cursor, cnt, reader, dst_cursor);
}

template <typename Writer>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::Write(seq_cntr_ref::Ref& ref,
                                                    void* pos_cursor,
                                                    size_t cnt, Writer&& writer,
                                                    void* dst_cursor) {
    seq_cntr_ref::Write(ref, pos_cursor, cnt, writer, dst_cursor);
}

template <typename ReaderWriter>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::ReadWrite(
    seq_cntr_ref::Ref& ref, void* pos_cursor, size_t cnt,
    ReaderWriter&& reader_writer, void* dst_cursor) {
    seq_cntr_ref::ReadWrite(ref, pos_cursor, cnt, reader_writer, dst_cursor);
}

template <typename Writer>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::PushL(seq_cntr_ref::Ref& ref,
                                                    size_t cnt, Writer&& writer,
                                                    void* dst_cursor) {
    seq_cntr_ref::PushL(ref, cnt, writer, dst_cursor);
}

template <typename Writer>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::PushR(seq_cntr_ref::Ref& ref,
                                                    size_t cnt, Writer&& writer,
                                                    void* dst_cursor) {
    seq_cntr_ref::PushR(ref, cnt, writer, dst_cursor);
}

template <typename Writer>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::Insert(seq_cntr_ref::Ref& ref,
                                                     void* pos_cursor,
                                                     size_t cnt,
                                                     Writer&& writer,
                                                     void* dst_cursor) {
    seq_cntr_ref::Insert(ref, pos_cursor, cnt, writer, dst_cursor);
}

template <typename Reader>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::PopL(seq_cntr_ref::Ref& ref,
                                                   size_t cnt,
                                                   Reader&& reader) {
    seq_cntr_ref::PopL(ref, cnt, reader);
}

template <typename Reader>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::PopR(seq_cntr_ref::Ref& ref,
                                                   size_t cnt,
                                                   Reader&& reader) {
    seq_cntr_ref::PopR(ref, cnt, reader);
}

template <typename Reader>
void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::Erase(seq_cntr_ref::Ref& ref,
                                                    void* pos_cursor,
                                                    size_t cnt,
                                                    Reader&& reader) {
    seq_cntr_ref::Erase(ref, pos_cursor, cnt, reader);
}

void seq_cntr::CntrTraits<seq_cntr_ref::Ref>::EraseAll(seq_cntr_ref::Ref& ref) {
    seq_cntr_ref::EraseAll(ref);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::CopyCursor(
    seq_cntr_ref::Ref const& ref, void* src_cursor, void* dst_cursor) {
    seq_cntr_ref::CopyCursor(ref, src_cursor, dst_cursor);
}

inline bool seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::AreEqualCursor(
    seq_cntr_ref::Ref const& ref, void* cursor_a, void* cursor_b) {
    return seq_cntr_ref::AreEqualCursor(ref, cursor_a, cursor_b);
}

inline int seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::CompareCursor(
    seq_cntr_ref::Ref const& ref, void* cursor_a, void* cursor_b) {
    return seq_cntr_ref::CompareCursor(ref, cursor_a, cursor_b);
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetCursorDist(
    seq_cntr_ref::Ref const& ref, void* cursor_a, void* cursor_b) {
    return seq_cntr_ref::GetCursorDist(ref, cursor_a, cursor_b);
}

inline size_t seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetCursorIdx(
    seq_cntr_ref::Ref const& ref, void* cursor) {
    return seq_cntr_ref::GetCursorIdx(ref, cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::CursorStepL(
    seq_cntr_ref::Ref const& ref, void* cursor) {
    seq_cntr_ref::CursorStepL(ref, cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::CursorStepR(
    seq_cntr_ref::Ref const& ref, void* cursor) {
    seq_cntr_ref::CursorStepR(ref, cursor);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::CursorAdvanceL(
    seq_cntr_ref::Ref const& ref, void* cursor, size_t step) {
    seq_cntr_ref::CursorAdvanceL(ref, cursor, step);
}

inline void seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::CursorAdvanceR(
    seq_cntr_ref::Ref const& ref, void* cursor, size_t step) {
    seq_cntr_ref::CursorAdvanceR(ref, cursor, step);
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
