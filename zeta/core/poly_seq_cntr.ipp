#pragma once

#include <zeta/core/debug_utils/sanity.ipp>
#include <zeta/core/poly_seq_cntr.hpp>
#include <zeta/core/poly_seq_endpoint.ipp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_endpoint.ipp>

namespace zeta::core {

#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)        \
    (((cap_flag) &                                \
      (static_cast<seq_cntr::capability::Flag>(1) \
       << meta::ToUnderlying(seq_cntr::capability::Kind::cap_name))) != 0)

#pragma push_macro("CallMethod_")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod_(method_ptr, cap_name, method, ...)                        \
    {                                                                         \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_cntr != nullptr); \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(self.vtable != nullptr);      \
                                                                              \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(                              \
            TestCapability(self.dynamic_enabled_capability_flag, cap_name));  \
                                                                              \
        auto method_ptr{ self.vtable->method };                               \
        ZETA_Core_DebugUtils_Diag_PromiseAssert(method_ptr != nullptr);       \
                                                                              \
        return method_ptr(self.target_cntr, __VA_ARGS__);                     \
    }                                                                         \
    static_assert(true);

#pragma push_macro("CallMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CallMethod(cap_name, method, ...) \
    CallMethod_(ZETA_Core_TmpName, cap_name, method, __VA_ARGS__)

constexpr poly_seq_cntr::Cntr::Cntr()
    : cursor_size{ 0 },
      width{ 0 },
      capacity{ 0 },
      dynamic_enabled_capability_flag{
          seq_cntr::capability::empty_capability_flag
      },
      dynamic_disabled_capability_flag{
          seq_cntr::capability::empty_capability_flag
      },
      vtable{ nullptr },
      target_cntr{ nullptr } {
    debug_utils::sanity::RegisterSanityCheckFunc(
        this, poly_seq_cntr::Cntr::SanityCheck);
}

template <seq_cntr::IsSeqCntr TargetCntr>
constexpr poly_seq_cntr::Cntr::Cntr(TargetCntr& target_cntr) {
    debug_utils::sanity::RegisterSanityCheckFunc(
        this, poly_seq_cntr::Cntr::SanityCheck);

    this->Set(target_cntr);
}

constexpr poly_seq_cntr::Cntr::~Cntr() {
    debug_utils::sanity::UnregisterSanityCheckFunc(this);
}

template <seq_cntr::IsSeqCntr TargetCntr>
constexpr void poly_seq_cntr::Cntr::Set(this Cntr& self,
                                        TargetCntr& target_cntr) {
    self.cursor_size = seq_cntr::GetCursorSize(target_cntr);

    self.width = seq_cntr::GetElemSize(target_cntr);

    self.capacity = seq_cntr::GetMaxElemCnt(target_cntr);

    self.dynamic_enabled_capability_flag =
        seq_cntr::GetStaticEnabledCapabilityFlag<TargetCntr>() |
        seq_cntr::GetDynamicEnabledCapabilityFlag(target_cntr);

    self.dynamic_disabled_capability_flag =
        seq_cntr::GetStaticDisabledCapabilityFlag<TargetCntr>() |
        seq_cntr::GetDynamicDisabledCapabilityFlag(target_cntr);

    self.vtable = &seq_cntr::GetVTable<TargetCntr>();

    self.target_cntr =
        const_cast<void*>(static_cast<void const*>(&target_cntr));
}

constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr::GetStaticEnabledCapabilityFlag(seq_cntr::Tag,
                                                    meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr::GetStaticDisabledCapabilityFlag(seq_cntr::Tag,
                                                     meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return seq_cntr::capability::non_const_capability_flag;
}

constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr::GetDynamicEnabledCapabilityFlag(this Cntr& self,
                                                     seq_cntr::Tag) {
    return self.dynamic_enabled_capability_flag;
}

constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr::GetDynamicEnabledCapabilityFlag(this Cntr const& self,
                                                     seq_cntr::Tag) {
    return self.dynamic_enabled_capability_flag &
           seq_cntr::capability::const_capability_flag;
}

constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr::GetDynamicDisabledCapabilityFlag(this Cntr& self,
                                                      seq_cntr::Tag) {
    return self.dynamic_disabled_capability_flag;
}

constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr::GetDynamicDisabledCapabilityFlag(this Cntr const& self,
                                                      seq_cntr::Tag) {
    return self.dynamic_disabled_capability_flag &
           seq_cntr::capability::const_capability_flag;
}

constexpr void* poly_seq_cntr::Cntr::GetReferedInstPtr(this Cntr const& self,
                                                       seq_cntr::Tag) {
    return self.target_cntr;
}

constexpr meta::TypeWrapper<void> poly_seq_cntr::Cntr::GetCursorType(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return {};
}

constexpr meta::TypeWrapper<void> poly_seq_cntr::Cntr::GetCursorType(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return {};
}

constexpr size_t poly_seq_cntr::Cntr::GetCursorSize(this Cntr const& self,
                                                    seq_cntr::Tag) {
    return self.cursor_size;
}

constexpr size_t poly_seq_cntr::Cntr::GetElemSize(this Cntr const& self,
                                                  seq_cntr::Tag) {
    return self.width;
}

constexpr size_t poly_seq_cntr::Cntr::GetElemCnt(this Cntr const& self,
                                                 seq_cntr::Tag) {
    CallMethod(GetElemCnt, get_elem_cnt);
}

constexpr size_t poly_seq_cntr::Cntr::GetMaxElemCnt(this Cntr const& self,
                                                    seq_cntr::Tag) {
    CallMethod(GetMaxElemCnt, get_max_elem_cnt);
}

constexpr void poly_seq_cntr::Cntr::GetLBCursor(this Cntr const& self,
                                                seq_cntr::Tag,
                                                void* dst_cursor) {
    CallMethod(GetLBCursor, get_lb_cursor, dst_cursor);
}

constexpr void poly_seq_cntr::Cntr::GetRBCursor(this Cntr const& self,
                                                seq_cntr::Tag,
                                                void* dst_cursor) {
    CallMethod(GetRBCursor, get_rb_cursor, dst_cursor);
}

constexpr void poly_seq_cntr::Cntr::PeekL(
    this Cntr const& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekL, peek_l, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void poly_seq_cntr::Cntr::PeekR(
    this Cntr const& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(PeekR, peek_r, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void poly_seq_cntr::Cntr::Refer(
    this Cntr const& self, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_cursor,
    void* dst_elem) {
    CallMethod(Refer, refer, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem);
}

constexpr void poly_seq_cntr::Cntr::Derefer(
    this Cntr const& self, seq_cntr::Tag, void* pos_cursor, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, void* dst_elem) {
    CallMethod(Derefer, derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem);
}

template <typename Reader>
constexpr void poly_seq_cntr::Cntr::Read(this Cntr const& self, seq_cntr::Tag,
                                         void* pos_cursor, size_t cnt,
                                         Reader&& reader, void* dst_cursor) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(Read, read.empty, pos_cursor, cnt, reader, dst_cursor);
    } else if constexpr (meta::IsSame<meta::RemoveCVRef<Reader>,
                                      seq_cntr::LinSeqReader>) {
        CallMethod(Read, read.lin_seq, pos_cursor, cnt, reader, dst_cursor);
    } else {
        CallMethod(Read, read.poly, pos_cursor, cnt, reader, dst_cursor);
    }
}

template <typename Writer>
constexpr void poly_seq_cntr::Cntr::Write(this Cntr& self, seq_cntr::Tag,
                                          void* pos_cursor, size_t cnt,
                                          Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(Write, write.empty, pos_cursor, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(Write, write.lin_seq, pos_cursor, cnt, writer, dst_cursor);
    } else {
        CallMethod(Write, write.poly, pos_cursor, cnt, writer, dst_cursor);
    }
}

template <typename ReaderWriter>
constexpr void poly_seq_cntr::Cntr::ReadWrite(this Cntr& self, seq_cntr::Tag,
                                              void* pos_cursor, size_t cnt,
                                              ReaderWriter&& reader_writer,
                                              void* dst_cursor) {
    CallMethod(ReadWrite, read_write.poly, pos_cursor, cnt, reader_writer,
               dst_cursor);
}

template <typename Writer>
constexpr void poly_seq_cntr::Cntr::PushL(this Cntr& self, seq_cntr::Tag,
                                          size_t cnt, Writer&& writer,
                                          void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(PushL, push_l.empty, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(PushL, push_l.lin_seq, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushL, push_l.poly, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
constexpr void poly_seq_cntr::Cntr::PushR(this Cntr& self, seq_cntr::Tag,
                                          size_t cnt, Writer&& writer,
                                          void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(PushR, push_r.empty, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(PushR, push_r.lin_seq, cnt, writer, dst_cursor);
    } else {
        CallMethod(PushR, push_r.poly, cnt, writer, dst_cursor);
    }
}

template <typename Writer>
constexpr void poly_seq_cntr::Cntr::Insert(this Cntr& self, seq_cntr::Tag,
                                           void* pos_cursor, size_t cnt,
                                           Writer&& writer, void* dst_cursor) {
    using RawWriter = meta::RemoveCVRef<Writer>;

    if constexpr (meta::IsSame<RawWriter, seq_cntr::EmptyWriter>) {
        CallMethod(Insert, insert.empty, pos_cursor, cnt, writer, dst_cursor);
    } else if constexpr (meta::IsSame<RawWriter, seq_cntr::LinSeqWriter>) {
        CallMethod(Insert, insert.lin_seq, pos_cursor, cnt, writer, dst_cursor);
    } else {
        CallMethod(Insert, insert.poly, pos_cursor, cnt, writer, dst_cursor);
    }
}

template <typename Reader>
constexpr void poly_seq_cntr::Cntr::PopL(this Cntr& self, seq_cntr::Tag,
                                         size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(PopL, pop_l.empty, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(PopL, pop_l.lin_seq, cnt, reader);
    } else {
        CallMethod(PopL, pop_l.poly, cnt, reader);
    }
}

template <typename Reader>
constexpr void poly_seq_cntr::Cntr::PopR(this Cntr& self, seq_cntr::Tag,
                                         size_t cnt, Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(PopR, pop_r.empty, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(PopR, pop_r.lin_seq, cnt, reader);
    } else {
        CallMethod(PopR, pop_r.poly, cnt, reader);
    }
}

template <typename Reader>
constexpr void poly_seq_cntr::Cntr::Erase(this Cntr& self, seq_cntr::Tag,
                                          void* pos_cursor, size_t cnt,
                                          Reader&& reader) {
    using RawReader = meta::RemoveCVRef<Reader>;

    if constexpr (meta::IsSame<RawReader, seq_cntr::EmptyReader>) {
        CallMethod(Erase, erase.empty, pos_cursor, cnt, reader);
    } else if constexpr (meta::IsSame<RawReader, seq_cntr::LinSeqReader>) {
        CallMethod(Erase, erase.lin_seq, pos_cursor, cnt, reader);
    } else {
        CallMethod(Erase, erase.poly, pos_cursor, cnt, reader);
    }
}

constexpr void poly_seq_cntr::Cntr::EraseAll(this Cntr& self, seq_cntr::Tag) {
    CallMethod(EraseAll, erase_all);
}

constexpr void poly_seq_cntr::Cntr::CopyCursor(this Cntr const& self,
                                               seq_cntr::Tag, void* src_cursor,
                                               void* dst_cursor) {
    CallMethod(CopyCursor, copy_cursor, src_cursor, dst_cursor);
}

constexpr bool poly_seq_cntr::Cntr::AreEqualCursor(this Cntr const& self,
                                                   seq_cntr::Tag,
                                                   void* cursor_a,
                                                   void* cursor_b) {
    CallMethod(AreEqualCursor, are_equal_cursor, cursor_a, cursor_b);
}

constexpr comparison::Ordering poly_seq_cntr::Cntr::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, void* cursor_a, void* cursor_b) {
    CallMethod(CompareCursor, compare_cursor, cursor_a, cursor_b);
}

constexpr size_t poly_seq_cntr::Cntr::GetCursorDist(this Cntr const& self,
                                                    seq_cntr::Tag,
                                                    void* cursor_a,
                                                    void* cursor_b) {
    CallMethod(GetCursorDist, get_cursor_dist, cursor_a, cursor_b);
}

constexpr size_t poly_seq_cntr::Cntr::GetCursorIdx(this Cntr const& self,
                                                   seq_cntr::Tag,
                                                   void* cursor) {
    CallMethod(GetCursorIdx, get_cursor_idx, cursor);
}

constexpr void poly_seq_cntr::Cntr::CursorStepL(this Cntr const& self,
                                                seq_cntr::Tag, void* cursor) {
    CallMethod(CursorStepL, cursor_step_l, cursor);
}

constexpr void poly_seq_cntr::Cntr::CursorStepR(this Cntr const& self,
                                                seq_cntr::Tag, void* cursor) {
    CallMethod(CursorStepR, cursor_step_r, cursor);
}

constexpr void poly_seq_cntr::Cntr::CursorAdvanceL(this Cntr const& self,
                                                   seq_cntr::Tag, void* cursor,
                                                   size_t step) {
    CallMethod(CursorAdvanceL, cursor_advance_l, cursor, step);
}

constexpr void poly_seq_cntr::Cntr::CursorAdvanceR(this Cntr const& self,
                                                   seq_cntr::Tag, void* cursor,
                                                   size_t step) {
    CallMethod(CursorAdvanceR, cursor_advance_r, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

constexpr void poly_seq_cntr::Cntr::Check(this Cntr const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(0 < self.width);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.vtable != nullptr);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.target_cntr != nullptr);

    seq_cntr::capability::Flag enabled_capability_flag{
        self.dynamic_enabled_capability_flag
    };

#pragma push_macro("CheckMethod")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CheckMethod(capability, method)                         \
    ZETA_Core_DebugUtils_Diag_PromiseAssert(                    \
        !TestCapability(enabled_capability_flag, capability) || \
        self.vtable->method != nullptr);

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
    CheckMethod(Read, read.poly);

    CheckMethod(Write, write.empty);
    CheckMethod(Write, write.lin_seq);
    CheckMethod(Write, write.poly);

    CheckMethod(ReadWrite, read_write.poly);

    CheckMethod(PushL, push_l.empty);
    CheckMethod(PushL, push_l.lin_seq);
    CheckMethod(PushL, push_l.poly);

    CheckMethod(PushR, push_r.empty);
    CheckMethod(PushR, push_r.lin_seq);
    CheckMethod(PushR, push_r.poly);

    CheckMethod(Insert, insert.empty);
    CheckMethod(Insert, insert.lin_seq);
    CheckMethod(Insert, insert.poly);

    CheckMethod(PopL, pop_l.empty);
    CheckMethod(PopL, pop_l.lin_seq);
    CheckMethod(PopL, pop_l.poly);

    CheckMethod(PopR, pop_r.empty);
    CheckMethod(PopR, pop_r.lin_seq);
    CheckMethod(PopR, pop_r.poly);

    CheckMethod(Erase, erase.empty);
    CheckMethod(Erase, erase.lin_seq);
    CheckMethod(Erase, erase.poly);

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

constexpr void poly_seq_cntr::Cntr::SanityCheck(
    void const* self_, debug_utils::sanity::SanityCheckScope scope) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self_ != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        scope != debug_utils::sanity::SanityCheckScope::Basic ||
        scope != debug_utils::sanity::SanityCheckScope::Complete);

    auto& self{ *static_cast<poly_seq_cntr::Cntr const*>(self_) };

    self.Check();

    debug_utils::sanity::ExpandFinishedSanityCheckScope(
        self_, debug_utils::sanity::SanityCheckScope::Basic);

    if (scope == debug_utils::sanity::SanityCheckScope::Complete) {
        debug_utils::sanity::SanityCheck(self.target_cntr, scope);
    }
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
