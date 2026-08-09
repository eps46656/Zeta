#pragma once

#include <zeta/core/elem_stream.ipp>
#include <zeta/core/fn_elem_stream.ipp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_cntr_ref.hpp>

namespace zeta::core {

#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)        \
    (((cap_flag) &                                \
      (static_cast<seq_cntr::capability::Flag>(1) \
       << meta::ToUnderlying(seq_cntr::capability::Kind::cap_name))) != 0)

#pragma push_macro("CallMethod_")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod_(method_ptr, cap_name, method, ...)                      \
    {                                                                       \
        ZETA_Core_DebugAssert(                                              \
            TestCapability(ref.dynamic_enabled_capability_flag, cap_name)); \
                                                                            \
        auto method_ptr{ ref.vtable->method };                              \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                       \
                                                                            \
        return method_ptr(ref.cntr, __VA_ARGS__);                           \
    }                                                                       \
    ZETA_Core_StaticAssert(true);

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method, ...) \
    CallMethod_(ZETA_Core_TmpName, cap_name, method, __VA_ARGS__)

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr_ref::Ref::Ref(Cntr& cntr) {
    this->Init(cntr);
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr void seq_cntr_ref::Ref::Init(this Ref& ref, Cntr& cntr) {
    ref.cursor_size = seq_cntr::GetCursorSize(cntr);
    ref.width = seq_cntr::GetElemSize(cntr);
    ref.capacity = seq_cntr::GetMaxElemCnt(cntr);
    ref.dynamic_enabled_capability_flag =
        seq_cntr::GetStaticEnabledCapabilityFlag<Cntr>() |
        seq_cntr::GetDynamicEnabledCapabilityFlag(cntr);
    ref.dynamic_disabled_capability_flag =
        seq_cntr::GetStaticDisabledCapabilityFlag<Cntr>() |
        seq_cntr::GetDynamicDisabledCapabilityFlag(cntr);
    ref.vtable = &seq_cntr::GetVTable<Cntr>();
    ref.cntr = const_cast<void*>(static_cast<void const*>(&cntr));
}

constexpr void* seq_cntr_ref::Ref::GetReferedInstPtr(this Ref const& ref) {
    return ref.cntr;
}

constexpr size_t seq_cntr_ref::Ref::GetCursorSize(this Ref const& ref) {
    return ref.cursor_size;
}

constexpr size_t seq_cntr_ref::Ref::GetElemSize(this Ref const& ref) {
    return ref.width;
}

constexpr size_t seq_cntr_ref::Ref::GetElemCnt(this Ref const& ref) {
    CallMethod(GetElemCnt, GetElemCnt);
}

constexpr size_t seq_cntr_ref::Ref::GetMaxElemCnt(this Ref const& ref) {
    CallMethod(GetMaxElemCnt, GetMaxElemCnt);
}

constexpr void seq_cntr_ref::Ref::GetLBCursor(this Ref const& ref,
                                              void* dst_cursor) {
    CallMethod(GetLBCursor, GetLBCursor, dst_cursor);
}

constexpr void seq_cntr_ref::Ref::GetRBCursor(this Ref const& ref,
                                              void* dst_cursor) {
    CallMethod(GetRBCursor, GetRBCursor, dst_cursor);
}

constexpr void seq_cntr_ref::Ref::PeekL(
    this Ref const& ref, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekL, PeekL, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void seq_cntr_ref::Ref::PeekR(
    this Ref const& ref, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekR, PeekR, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void seq_cntr_ref::Ref::Refer(
    this Ref const& ref, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(Refer, Refer, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void seq_cntr_ref::Ref::Derefer(
    this Ref const& ref, void* pos_cursor, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    CallMethod(Derefer, Derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem);
}

template <typename Reader>
constexpr void seq_cntr_ref::Ref::Read(this Ref const& ref, void* pos_cursor,
                                       size_t cnt, Reader&& reader,
                                       void* dst_cursor) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(Read, Read_EmptyReader, pos_cursor, cnt, reader, dst_cursor);
    } else if constexpr (meta::IsSame<meta::RemoveCVRef<Reader>,
                                      seq_cntr::LinSeqReader>) {
        CallMethod(Read, Read_LinSeqReader, pos_cursor, cnt, reader,
                   dst_cursor);
    } else {
        CallMethod(Read, Read_FnReader, pos_cursor, cnt, reader, dst_cursor);
    }
}

template <typename Writer>
constexpr void seq_cntr_ref::Ref::Write(this Ref& ref, void* pos_cursor,
                                        size_t cnt, Writer&& writer,
                                        void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(Write, Write_EmptyWriter, pos_cursor, cnt, writer,
                   dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(Write, Write_LinSeqWriter, pos_cursor, cnt, writer,
                   dst_cursor);
    } else {
        CallMethod(Write, Write_FnWriter, pos_cursor, cnt, writer, dst_cursor);
    }
}

template <typename ReaderWriter>
constexpr void seq_cntr_ref::Ref::ReadWrite(this Ref& ref, void* pos_cursor,
                                            size_t cnt,
                                            ReaderWriter&& reader_writer,
                                            void* dst_cursor) {
    CallMethod(ReadWrite, ReadWrite_FnReaderWriter, pos_cursor, cnt,
               reader_writer, dst_cursor);
}

template <typename Writer>
constexpr void seq_cntr_ref::Ref::PushL(this Ref& ref, size_t cnt,
                                        Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(PushL, PushL_EmptyWriter, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(PushL, PushL_LinSeqWriter, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushL, PushL_FnWriter, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
constexpr void seq_cntr_ref::Ref::PushR(this Ref& ref, size_t cnt,
                                        Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(PushR, PushR_EmptyWriter, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(PushR, PushR_LinSeqWriter, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushR, PushR_FnWriter, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
constexpr void seq_cntr_ref::Ref::Insert(this Ref& ref, void* pos_cursor,
                                         size_t cnt, Writer&& writer,
                                         void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(Insert, Insert_EmptyWriter, pos_cursor, cnt, writer,
                   dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(Insert, Insert_LinSeqWriter, pos_cursor, cnt, writer,
                   dst_cursor);
    } else {
        CallMethod(Insert, Insert_FnWriter, pos_cursor, cnt, writer,
                   dst_cursor);
    }
}

template <typename Reader>
constexpr void seq_cntr_ref::Ref::PopL(this Ref& ref, size_t cnt,
                                       Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(PopL, PopL_EmptyReader, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(PopL, PopL_LinSeqReader, cnt, reader);
    } else {
        CallMethod(PopL, PopL_FnReader, cnt, reader);
    }
}

template <typename Reader>
constexpr void seq_cntr_ref::Ref::PopR(this Ref& ref, size_t cnt,
                                       Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(PopR, PopR_EmptyReader, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(PopR, PopR_LinSeqReader, cnt, reader);
    } else {
        CallMethod(PopR, PopR_FnReader, cnt, reader);
    }
}

template <typename Reader>
constexpr void seq_cntr_ref::Ref::Erase(this Ref& ref, void* pos_cursor,
                                        size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(Erase, Erase_EmptyReader, pos_cursor, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(Erase, Erase_LinSeqReader, pos_cursor, cnt, reader);
    } else {
        CallMethod(Erase, Erase_FnReader, pos_cursor, cnt, reader);
    }
}

constexpr void seq_cntr_ref::Ref::EraseAll(this Ref& ref) {
    CallMethod(EraseAll, EraseAll);
}

constexpr void seq_cntr_ref::Ref::CopyCursor(this Ref const& ref,
                                             void* src_cursor,
                                             void* dst_cursor) {
    CallMethod(CopyCursor, CopyCursor, src_cursor, dst_cursor);
}

constexpr bool seq_cntr_ref::Ref::AreEqualCursor(this Ref const& ref,
                                                 void* cursor_a,
                                                 void* cursor_b) {
    CallMethod(AreEqualCursor, AreEqualCursor, cursor_a, cursor_b);
}

constexpr comparison::Ordering seq_cntr_ref::Ref::CompareCursor(
    this Ref const& ref, void* cursor_a, void* cursor_b) {
    CallMethod(CompareCursor, CompareCursor, cursor_a, cursor_b);
}

constexpr size_t seq_cntr_ref::Ref::GetCursorDist(this Ref const& ref,
                                                  void* cursor_a,
                                                  void* cursor_b) {
    CallMethod(GetCursorDist, GetCursorDist, cursor_a, cursor_b);
}

constexpr size_t seq_cntr_ref::Ref::GetCursorIdx(this Ref const& ref,
                                                 void* cursor) {
    CallMethod(GetCursorIdx, GetCursorIdx, cursor);
}

constexpr void seq_cntr_ref::Ref::CursorStepL(this Ref const& ref,
                                              void* cursor) {
    CallMethod(CursorStepL, CursorStepL, cursor);
}

constexpr void seq_cntr_ref::Ref::CursorStepR(this Ref const& ref,
                                              void* cursor) {
    CallMethod(CursorStepR, CursorStepR, cursor);
}

constexpr void seq_cntr_ref::Ref::CursorAdvanceL(this Ref const& ref,
                                                 void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, CursorAdvanceL, cursor, step);
}

constexpr void seq_cntr_ref::Ref::CursorAdvanceR(this Ref const& ref,
                                                 void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, CursorAdvanceR, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

constexpr void seq_cntr_ref::Ref::Check(this Ref const& ref) {
    ZETA_Core_DebugAssert(0 < ref.width);
    ZETA_Core_DebugAssert(ref.vtable != nullptr);
    ZETA_Core_DebugAssert(ref.cntr != nullptr);

    seq_cntr::capability::Flag enabled_capability_flag{
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

    CheckMethod(Read, Read_EmptyReader);
    CheckMethod(Read, Read_LinSeqReader);
    CheckMethod(Read, Read_FnReader);

    CheckMethod(Write, Write_EmptyWriter);
    CheckMethod(Write, Write_LinSeqWriter);
    CheckMethod(Write, Write_FnWriter);

    CheckMethod(ReadWrite, ReadWrite_FnReaderWriter);

    CheckMethod(PushL, PushL_EmptyWriter);
    CheckMethod(PushL, PushL_LinSeqWriter);
    CheckMethod(PushL, PushL_FnWriter);

    CheckMethod(PushR, PushR_EmptyWriter);
    CheckMethod(PushR, PushR_LinSeqWriter);
    CheckMethod(PushR, PushR_FnWriter);

    CheckMethod(Insert, Insert_EmptyWriter);
    CheckMethod(Insert, Insert_LinSeqWriter);
    CheckMethod(Insert, Insert_FnWriter);

    CheckMethod(PopL, PopL_EmptyReader);
    CheckMethod(PopL, PopL_LinSeqReader);
    CheckMethod(PopL, PopL_FnReader);

    CheckMethod(PopR, PopR_EmptyReader);
    CheckMethod(PopR, PopR_LinSeqReader);
    CheckMethod(PopR, PopR_FnReader);

    CheckMethod(Erase, Erase_EmptyReader);
    CheckMethod(Erase, Erase_LinSeqReader);
    CheckMethod(Erase, Erase_FnReader);

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

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Ref>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Ref>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Ref>::GetDynamicEnabledCapabilityFlag(
    seq_cntr_ref::Ref& ref) {
    return ref.dynamic_enabled_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Ref>::GetDynamicDisabledCapabilityFlag(
    seq_cntr_ref::Ref& ref) {
    return ref.dynamic_disabled_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    seq_cntr_ref::Ref const>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::capability::non_const_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetDynamicEnabledCapabilityFlag(
    seq_cntr_ref::Ref const& ref) {
    return ref.dynamic_enabled_capability_flag &
           seq_cntr::capability::const_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Ref const>::GetDynamicDisabledCapabilityFlag(
    seq_cntr_ref::Ref const& ref) {
    return ref.dynamic_disabled_capability_flag |
           seq_cntr::capability::non_const_capability_flag;
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
