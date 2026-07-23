#pragma once

#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_cntr_ref.hpp>

namespace zeta::core {

#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(capability_flag, capability)                            \
    (((capability_flag) & (static_cast<seq_cntr::CapabilityFlag>(1)            \
                           << seq_cntr::CapabilityEnum::capability::value)) != \
     0)

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

template <seq_cntr::IsSeqCntr Cntr>
seq_cntr_ref::Ref::Ref(Cntr& cntr)
    : cursor_size{ seq_cntr::GetCursorSize(cntr) },
      width{ seq_cntr::GetElemSize(cntr) },
      capacity{ seq_cntr::GetMaxElemCnt(cntr) },
      dynamic_enabled_capability_flag{
          seq_cntr::GetStaticEnabledCapabilityFlag<Cntr>() |
          seq_cntr::GetDynamicEnabledCapabilityFlag(cntr)
      },
      dynamic_disabled_capability_flag{
          seq_cntr::GetStaticDisabledCapabilityFlag<Cntr>() |
          seq_cntr::GetDynamicDisabledCapabilityFlag(cntr)
      },
      vtable{ &seq_cntr::GetVTable<Cntr>() },
      cntr{ const_cast<void*>(static_cast<void const*>(&cntr)) } {}

inline void* seq_cntr_ref::Ref::GetReferedInstPtr(this Ref const& ref) {
    return ref.cntr;
}

inline size_t seq_cntr_ref::Ref::GetCursorSize(this Ref const& ref) {
    return ref.cursor_size;
}

inline size_t seq_cntr_ref::Ref::GetElemSize(this Ref const& ref) {
    return ref.width;
}

inline size_t seq_cntr_ref::Ref::GetElemCnt(this Ref const& ref) {
    CallMethod(GetElemCnt, GetElemCnt);
}

inline size_t seq_cntr_ref::Ref::GetMaxElemCnt(this Ref const& ref) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt);
}

inline void seq_cntr_ref::Ref::GetLBCursor(this Ref const& ref,
                                           void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

inline void seq_cntr_ref::Ref::GetRBCursor(this Ref const& ref,
                                           void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

inline void seq_cntr_ref::Ref::PeekL(this Ref const& ref, bool lazy_copy_elem,
                                     seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                     void* dst_cursor, void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

inline void seq_cntr_ref::Ref::PeekR(this Ref const& ref, bool lazy_copy_elem,
                                     seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                     void* dst_cursor, void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

inline void seq_cntr_ref::Ref::Refer(this Ref const& ref, size_t idx,
                                     bool lazy_copy_elem,
                                     seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                     void* dst_cursor, void* dst_elem) {
    CallMethod(Refer, Refer, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

inline void seq_cntr_ref::Ref::Derefer(this Ref const& ref, void* pos_cursor,
                                       bool lazy_copy_elem,
                                       seq_cntr::ElemPtrView* dst_elem_ptr_view,
                                       void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem);
}

template <typename Reader>
void seq_cntr_ref::Ref::Read(this Ref const& ref, void* pos_cursor, size_t cnt,
                             Reader&& reader, void* dst_cursor) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, elem_stream::EmptyAcceptor>) {
        CallMethod(Read, Read_EmptyAcceptor, pos_cursor, cnt, reader,
                   dst_cursor);
    } else if constexpr (meta::IsSame<meta::RemoveCVRef<Reader>,
                                      lin_seq_elem_stream::Acceptor>) {
        CallMethod(Read, Read_LinSeqAcceptor, pos_cursor, cnt, reader,
                   dst_cursor);
    } else {
        CallMethod(Read, Read_FnAcceptor, pos_cursor, cnt, reader, dst_cursor);
    }
}

template <typename Writer>
void seq_cntr_ref::Ref::Write(this Ref& ref, void* pos_cursor, size_t cnt,
                              Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, elem_stream::EmptyProvider>) {
        CallMethod(Write, Write_EmptyProvider, pos_cursor, cnt, writer,
                   dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter,
                                      lin_seq_elem_stream::Provider>) {
        CallMethod(Write, Write_LinSeqProvider, pos_cursor, cnt, writer,
                   dst_cursor);
    } else {
        CallMethod(Write, Write_FnProvider, pos_cursor, cnt, writer,
                   dst_cursor);
    }
}

template <typename ReaderWriter>
void seq_cntr_ref::Ref::ReadWrite(this Ref& ref, void* pos_cursor, size_t cnt,
                                  ReaderWriter&& reader_writer,
                                  void* dst_cursor) {
    CallMethod(ReadWrite, ReadWrite_FnProvider, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

template <typename Writer>
void seq_cntr_ref::Ref::PushL(this Ref& ref, size_t cnt, Writer&& writer,
                              void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, elem_stream::EmptyProvider>) {
        CallMethod(PushL, PushL_EmptyProvider, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter,
                                      lin_seq_elem_stream::Provider>) {
        CallMethod(PushL, PushL_LinSeqProvider, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushL, PushL_FnProvider, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
void seq_cntr_ref::Ref::PushR(this Ref& ref, size_t cnt, Writer&& writer,
                              void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, elem_stream::EmptyProvider>) {
        CallMethod(PushR, PushR_EmptyProvider, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter,
                                      lin_seq_elem_stream::Provider>) {
        CallMethod(PushR, PushR_LinSeqProvider, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushR, PushR_FnProvider, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
void seq_cntr_ref::Ref::Insert(this Ref& ref, void* pos_cursor, size_t cnt,
                               Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, elem_stream::EmptyProvider>) {
        CallMethod(Insert, Insert_EmptyProvider, pos_cursor, cnt, writer,
                   dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter,
                                      lin_seq_elem_stream::Provider>) {
        CallMethod(Insert, Insert_LinSeqProvider, pos_cursor, cnt, writer,
                   dst_cursor);
    } else {
        CallMethod(Insert, Insert_FnProvider, pos_cursor, cnt, writer,
                   dst_cursor);
    }
}

template <typename Reader>
void seq_cntr_ref::Ref::PopL(this Ref& ref, size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, elem_stream::EmptyAcceptor>) {
        CallMethod(PopL, PopL_EmptyAcceptor, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader,
                                      lin_seq_elem_stream::Acceptor>) {
        CallMethod(PopL, PopL_LinSeqAcceptor, cnt, reader);
    } else {
        CallMethod(PopL, PopL_FnAcceptor, cnt, reader);
    }
}

template <typename Reader>
void seq_cntr_ref::Ref::PopR(this Ref& ref, size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, elem_stream::EmptyAcceptor>) {
        CallMethod(PopR, PopR_EmptyAcceptor, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader,
                                      lin_seq_elem_stream::Acceptor>) {
        CallMethod(PopR, PopR_LinSeqAcceptor, cnt, reader);
    } else {
        CallMethod(PopR, PopR_FnAcceptor, cnt, reader);
    }
}

template <typename Reader>
void seq_cntr_ref::Ref::Erase(this Ref& ref, void* pos_cursor, size_t cnt,
                              Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, elem_stream::EmptyAcceptor>) {
        CallMethod(Erase, Erase_EmptyAcceptor, pos_cursor, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader,
                                      lin_seq_elem_stream::Acceptor>) {
        CallMethod(Erase, Erase_LinSeqAcceptor, pos_cursor, cnt, reader);
    } else {
        CallMethod(Erase, Erase_FnAcceptor, pos_cursor, cnt, reader);
    }
}

inline void seq_cntr_ref::Ref::EraseAll(this Ref& ref) {
    CallMethod(EraseAll, EraseAll);
}

inline void seq_cntr_ref::Ref::CopyCursor(this Ref const& ref, void* src_cursor,
                                          void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

inline bool seq_cntr_ref::Ref::AreEqualCursor(this Ref const& ref,
                                              void* cursor_a, void* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

inline int seq_cntr_ref::Ref::CompareCursor(this Ref const& ref, void* cursor_a,
                                            void* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

inline size_t seq_cntr_ref::Ref::GetCursorDist(this Ref const& ref,
                                               void* cursor_a, void* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

inline size_t seq_cntr_ref::Ref::GetCursorIdx(this Ref const& ref,
                                              void* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

inline void seq_cntr_ref::Ref::CursorStepL(this Ref const& ref, void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

inline void seq_cntr_ref::Ref::CursorStepR(this Ref const& ref, void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

inline void seq_cntr_ref::Ref::CursorAdvanceL(this Ref const& ref, void* cursor,
                                              size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

inline void seq_cntr_ref::Ref::CursorAdvanceR(this Ref const& ref, void* cursor,
                                              size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

inline void seq_cntr_ref::Ref::Check(this Ref const& ref) {
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

    CheckMethod(Read, Read_EmptyAcceptor);
    CheckMethod(Read, Read_LinSeqAcceptor);
    CheckMethod(Read, Read_FnAcceptor);

    CheckMethod(Write, Write_EmptyProvider);
    CheckMethod(Write, Write_LinSeqProvider);
    CheckMethod(Write, Write_FnProvider);

    CheckMethod(ReadWrite, ReadWrite_FnProvider);

    CheckMethod(PushL, PushL_EmptyProvider);
    CheckMethod(PushL, PushL_LinSeqProvider);
    CheckMethod(PushL, PushL_FnProvider);

    CheckMethod(PushR, PushR_EmptyProvider);
    CheckMethod(PushR, PushR_LinSeqProvider);
    CheckMethod(PushR, PushR_FnProvider);

    CheckMethod(Insert, Insert_EmptyProvider);
    CheckMethod(Insert, Insert_LinSeqProvider);
    CheckMethod(Insert, Insert_FnProvider);

    CheckMethod(PopL, PopL_EmptyAcceptor);
    CheckMethod(PopL, PopL_LinSeqAcceptor);
    CheckMethod(PopL, PopL_FnAcceptor);

    CheckMethod(PopR, PopR_EmptyAcceptor);
    CheckMethod(PopR, PopR_LinSeqAcceptor);
    CheckMethod(PopR, PopR_FnAcceptor);

    CheckMethod(Erase, Erase_EmptyAcceptor);
    CheckMethod(Erase, Erase_LinSeqAcceptor);
    CheckMethod(Erase, Erase_FnAcceptor);

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

constexpr seq_cntr::CapabilityFlag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::empty_capability_flag;
}

constexpr seq_cntr::CapabilityFlag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::non_const_capability_flag;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref>::GetDynamicEnabledCapabilityFlag(
    seq_cntr_ref::Ref& ref) {
    return ref.dynamic_enabled_capability_flag;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetDynamicEnabledCapabilityFlag(
    seq_cntr_ref::Ref const& ref) {
    return ref.dynamic_enabled_capability_flag &
           seq_cntr::const_capability_flag;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref>::GetDynamicDisabledCapabilityFlag(
    seq_cntr_ref::Ref& ref) {
    return ref.dynamic_disabled_capability_flag;
}

constexpr seq_cntr::CapabilityFlag
seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetDynamicDisabledCapabilityFlag(
    seq_cntr_ref::Ref const& ref) {
    return ref.dynamic_disabled_capability_flag |
           seq_cntr::non_const_capability_flag;
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
