#pragma once

#include <zeta/core/elem_stream.ipp>
#include <zeta/core/elem_stream_ref.ipp>
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
#define CallMethod_(method_ptr, cap_name, method, ...)                       \
    {                                                                        \
        ZETA_Core_DebugAssert(                                               \
            TestCapability(cntr.dynamic_enabled_capability_flag, cap_name)); \
                                                                             \
        auto method_ptr{ cntr.vtable->method };                              \
        ZETA_Core_DebugAssert(method_ptr != nullptr);                        \
                                                                             \
        return method_ptr(cntr.target_cntr, __VA_ARGS__);                    \
    }                                                                        \
    ZETA_Core_StaticAssert(true);

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method, ...) \
    CallMethod_(ZETA_Core_TmpName, cap_name, method, __VA_ARGS__)

template <seq_cntr::IsSeqCntr TargetCntr>
constexpr seq_cntr_ref::Cntr::Cntr(TargetCntr& target_cntr) {
    this->Set(target_cntr);
}

constexpr void seq_cntr_ref::Cntr::Init(this Cntr& cntr,
                                        Cntr const& other_cntr) {
    cntr = other_cntr;
}

template <seq_cntr::IsSeqCntr TargetCntr>
constexpr void seq_cntr_ref::Cntr::Init(this Cntr& cntr,
                                        TargetCntr& target_cntr) {
    cntr.Set(target_cntr);
}

constexpr void seq_cntr_ref::Cntr::Set(this Cntr& cntr,
                                       Cntr const& other_cntr) {
    cntr = other_cntr;
}

template <seq_cntr::IsSeqCntr TargetCntr>
constexpr void seq_cntr_ref::Cntr::Set(this Cntr& cntr,
                                       TargetCntr& target_cntr) {
    cntr.cursor_size = seq_cntr::GetCursorSize(target_cntr);
    cntr.width = seq_cntr::GetElemSize(target_cntr);
    cntr.capacity = seq_cntr::GetMaxElemCnt(target_cntr);
    cntr.dynamic_enabled_capability_flag =
        seq_cntr::GetStaticEnabledCapabilityFlag<Cntr>() |
        seq_cntr::GetDynamicEnabledCapabilityFlag(target_cntr);
    cntr.dynamic_disabled_capability_flag =
        seq_cntr::GetStaticDisabledCapabilityFlag<Cntr>() |
        seq_cntr::GetDynamicDisabledCapabilityFlag(target_cntr);
    cntr.vtable = &seq_cntr::GetVTable<Cntr>();
    cntr.target_cntr =
        const_cast<void*>(static_cast<void const*>(&target_cntr));
}

constexpr void seq_cntr_ref::Cntr::Deinit(this Cntr&) {}

constexpr void* seq_cntr_ref::Cntr::GetReferedInstPtr(this Cntr const& cntr) {
    return cntr.target_cntr;
}

constexpr size_t seq_cntr_ref::Cntr::GetCursorSize(this Cntr const& cntr) {
    return cntr.cursor_size;
}

constexpr size_t seq_cntr_ref::Cntr::GetElemSize(this Cntr const& cntr) {
    return cntr.width;
}

constexpr size_t seq_cntr_ref::Cntr::GetElemCnt(this Cntr const& cntr) {
    CallMethod(GetElemCnt, get_elem_cnt);
}

constexpr size_t seq_cntr_ref::Cntr::GetMaxElemCnt(this Cntr const& cntr) {
    CallMethod(GetMaxElemCnt, get_max_elem_cnt);
}

constexpr void seq_cntr_ref::Cntr::GetLBCursor(this Cntr const& cntr,
                                               void* dst_cursor) {
    CallMethod(GetLBCursor, get_lb_cursor, dst_cursor);
}

constexpr void seq_cntr_ref::Cntr::GetRBCursor(this Cntr const& cntr,
                                               void* dst_cursor) {
    CallMethod(GetRBCursor, get_rb_cursor, dst_cursor);
}

constexpr void seq_cntr_ref::Cntr::PeekL(
    this Cntr const& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekL, peek_l, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void seq_cntr_ref::Cntr::PeekR(
    this Cntr const& cntr, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekR, peek_r, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void seq_cntr_ref::Cntr::Refer(
    this Cntr const& cntr, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(Refer, refer, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void seq_cntr_ref::Cntr::Derefer(
    this Cntr const& cntr, void* pos_cursor, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    CallMethod(Derefer, derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem);
}

template <typename Reader>
constexpr void seq_cntr_ref::Cntr::Read(this Cntr const& cntr, void* pos_cursor,
                                        size_t cnt, Reader&& reader,
                                        void* dst_cursor) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(Read, read.empty, pos_cursor, cnt, reader, dst_cursor);
    } else if constexpr (meta::IsSame<meta::RemoveCVRef<Reader>,
                                      seq_cntr::LinSeqReader>) {
        CallMethod(Read, read.lin_seq, pos_cursor, cnt, reader, dst_cursor);
    } else {
        CallMethod(Read, read.fn, pos_cursor, cnt, reader, dst_cursor);
    }
}

template <typename Writer>
constexpr void seq_cntr_ref::Cntr::Write(this Cntr& cntr, void* pos_cursor,
                                         size_t cnt, Writer&& writer,
                                         void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(Write, write.empty, pos_cursor, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(Write, write.lin_seq, pos_cursor, cnt, writer, dst_cursor);
    } else {
        CallMethod(Write, write.fn, pos_cursor, cnt, writer, dst_cursor);
    }
}

template <typename ReaderWriter>
constexpr void seq_cntr_ref::Cntr::ReadWrite(this Cntr& cntr, void* pos_cursor,
                                             size_t cnt,
                                             ReaderWriter&& reader_writer,
                                             void* dst_cursor) {
    CallMethod(ReadWrite, read_write.fn, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

template <typename Writer>
constexpr void seq_cntr_ref::Cntr::PushL(this Cntr& cntr, size_t cnt,
                                         Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(PushL, push_l.empty, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(PushL, push_l.lin_seq, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushL, push_l.fn, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
constexpr void seq_cntr_ref::Cntr::PushR(this Cntr& cntr, size_t cnt,
                                         Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(PushR, push_r.empty, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(PushR, push_r.lin_seq, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushR, push_r.fn, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
constexpr void seq_cntr_ref::Cntr::Insert(this Cntr& cntr, void* pos_cursor,
                                          size_t cnt, Writer&& writer,
                                          void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(Insert, insert.empty, pos_cursor, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(Insert, insert.lin_seq, pos_cursor, cnt, writer, dst_cursor);
    } else {
        CallMethod(Insert, insert.fn, pos_cursor, cnt, writer, dst_cursor);
    }
}

template <typename Reader>
constexpr void seq_cntr_ref::Cntr::PopL(this Cntr& cntr, size_t cnt,
                                        Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(PopL, pop_l.empty, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(PopL, pop_l.lin_seq, cnt, reader);
    } else {
        CallMethod(PopL, pop_l.fn, cnt, reader);
    }
}

template <typename Reader>
constexpr void seq_cntr_ref::Cntr::PopR(this Cntr& cntr, size_t cnt,
                                        Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(PopR, pop_r.empty, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(PopR, pop_r.lin_seq, cnt, reader);
    } else {
        CallMethod(PopR, pop_r.fn, cnt, reader);
    }
}

template <typename Reader>
constexpr void seq_cntr_ref::Cntr::Erase(this Cntr& cntr, void* pos_cursor,
                                         size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(Erase, erase.empty, pos_cursor, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(Erase, erase.lin_seq, pos_cursor, cnt, reader);
    } else {
        CallMethod(Erase, erase.fn, pos_cursor, cnt, reader);
    }
}

constexpr void seq_cntr_ref::Cntr::EraseAll(this Cntr& cntr) {
    CallMethod(EraseAll, erase_all);
}

constexpr void seq_cntr_ref::Cntr::CopyCursor(this Cntr const& cntr,
                                              void* src_cursor,
                                              void* dst_cursor) {
    CallMethod(CopyCursor, copy_cursor, src_cursor, dst_cursor);
}

constexpr bool seq_cntr_ref::Cntr::AreEqualCursor(this Cntr const& cntr,
                                                  void* cursor_a,
                                                  void* cursor_b) {
    CallMethod(AreEqualCursor, are_equal_cursor, cursor_a, cursor_b);
}

constexpr comparison::Ordering seq_cntr_ref::Cntr::CompareCursor(
    this Cntr const& cntr, void* cursor_a, void* cursor_b) {
    CallMethod(CompareCursor, compare_cursor, cursor_a, cursor_b);
}

constexpr size_t seq_cntr_ref::Cntr::GetCursorDist(this Cntr const& cntr,
                                                   void* cursor_a,
                                                   void* cursor_b) {
    CallMethod(GetCursorDist, get_cursor_dist, cursor_a, cursor_b);
}

constexpr size_t seq_cntr_ref::Cntr::GetCursorIdx(this Cntr const& cntr,
                                                  void* cursor) {
    CallMethod(GetCursorIdx, get_cursor_idx, cursor);
}

constexpr void seq_cntr_ref::Cntr::CursorStepL(this Cntr const& cntr,
                                               void* cursor) {
    CallMethod(CursorStepL, cursor_step_l, cursor);
}

constexpr void seq_cntr_ref::Cntr::CursorStepR(this Cntr const& cntr,
                                               void* cursor) {
    CallMethod(CursorStepR, cursor_step_r, cursor);
}

constexpr void seq_cntr_ref::Cntr::CursorAdvanceL(this Cntr const& cntr,
                                                  void* cursor, size_t step) {
    CallMethod(CursorAdvanceL, cursor_advance_l, cursor, step);
}

constexpr void seq_cntr_ref::Cntr::CursorAdvanceR(this Cntr const& cntr,
                                                  void* cursor, size_t step) {
    CallMethod(CursorAdvanceR, cursor_advance_r, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

constexpr void seq_cntr_ref::Cntr::Check(this Cntr const& cntr) {
    ZETA_Core_DebugAssert(0 < cntr.width);
    ZETA_Core_DebugAssert(cntr.vtable != nullptr);
    ZETA_Core_DebugAssert(cntr.target_cntr != nullptr);

    seq_cntr::capability::Flag enabled_capability_flag{
        cntr.dynamic_enabled_capability_flag
    };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(capability, method)                         \
    ZETA_Core_DebugAssert(                                      \
        !TestCapability(enabled_capability_flag, capability) || \
        cntr.vtable->method != nullptr);

    CheckMethod(GetElemCnt, get_elem_cnt);
    CheckMethod(GetMaxElemCnt, get_max_elem_cnt);

    CheckMethod(GetLBCursor, get_lb_cursor);
    CheckMethod(GetRBCursor, get_rb_cursor);
    CheckMethod(PeekL, peek_l);
    CheckMethod(PeekR, peek_r);
    CheckMethod(Refer, refer);
    CheckMethod(Derefer, derefer);

    CheckMethod(Read, read.empty);
    CheckMethod(Read, read.lin_seq);
    CheckMethod(Read, read.fn);

    CheckMethod(Write, write.empty);
    CheckMethod(Write, write.lin_seq);
    CheckMethod(Write, write.fn);

    CheckMethod(ReadWrite, read_write.fn);

    CheckMethod(PushL, push_l.empty);
    CheckMethod(PushL, push_l.lin_seq);
    CheckMethod(PushL, push_l.fn);

    CheckMethod(PushR, push_r.empty);
    CheckMethod(PushR, push_r.lin_seq);
    CheckMethod(PushR, push_r.fn);

    CheckMethod(Insert, insert.empty);
    CheckMethod(Insert, insert.lin_seq);
    CheckMethod(Insert, insert.fn);

    CheckMethod(PopL, pop_l.empty);
    CheckMethod(PopL, pop_l.lin_seq);
    CheckMethod(PopL, pop_l.fn);

    CheckMethod(PopR, pop_r.empty);
    CheckMethod(PopR, pop_r.lin_seq);
    CheckMethod(PopR, pop_r.fn);

    CheckMethod(Erase, erase.empty);
    CheckMethod(Erase, erase.lin_seq);
    CheckMethod(Erase, erase.fn);

    CheckMethod(EraseAll, erase_all);

    CheckMethod(CopyCursor, copy_cursor);
    CheckMethod(AreEqualCursor, are_equal_cursor);
    CheckMethod(CompareCursor, compare_cursor);
    CheckMethod(GetCursorDist, get_cursor_dist);
    CheckMethod(GetCursorIdx, get_cursor_idx);
    CheckMethod(CursorStepL, cursor_step_l);
    CheckMethod(CursorStepR, cursor_step_r);
    CheckMethod(CursorAdvanceL, cursor_advance_l);
    CheckMethod(CursorAdvanceR, cursor_advance_r);

#pragma pop_macro("CheckMethod")
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Cntr>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Cntr>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Cntr>::GetDynamicEnabledCapabilityFlag(
    seq_cntr_ref::Cntr& cntr) {
    return cntr.dynamic_enabled_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Cntr>::GetDynamicDisabledCapabilityFlag(
    seq_cntr_ref::Cntr& cntr) {
    return cntr.dynamic_disabled_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    seq_cntr_ref::Cntr const>::GetStaticEnabledCapabilityFlag() {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::CntrTraits<
    seq_cntr_ref::Cntr const>::GetStaticDisabledCapabilityFlag() {
    return seq_cntr::capability::non_const_capability_flag;
}

constexpr seq_cntr::capability::Flag
seq_cntr::CntrTraits<seq_cntr_ref::Cntr const>::GetDynamicEnabledCapabilityFlag(
    seq_cntr_ref::Cntr const& cntr) {
    return cntr.dynamic_enabled_capability_flag &
           seq_cntr::capability::const_capability_flag;
}

constexpr seq_cntr::capability::Flag seq_cntr::
    CntrTraits<seq_cntr_ref::Cntr const>::GetDynamicDisabledCapabilityFlag(
        seq_cntr_ref::Cntr const& cntr) {
    return cntr.dynamic_disabled_capability_flag |
           seq_cntr::capability::non_const_capability_flag;
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
