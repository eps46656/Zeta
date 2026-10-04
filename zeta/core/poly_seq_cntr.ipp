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

template <meta::IsContainerElem Elem>
constexpr poly_seq_cntr::Cntr<Elem>::Cntr()
    : max_elem_cnt{ 0 },
      dynamic_enabled_capability_flag{
          seq_cntr::capability::empty_capability_flag
      },
      dynamic_disabled_capability_flag{
          seq_cntr::capability::empty_capability_flag
      },
      vtable{ nullptr },
      target_cntr{ nullptr } {
    debug_utils::sanity::RegisterSanityCheckFunc(this, Cntr::SanityCheck);
}

template <meta::IsContainerElem Elem>
template <seq_cntr::IsSeqCntr TargetCntr>
constexpr poly_seq_cntr::Cntr<Elem>::Cntr(TargetCntr& target_cntr) {
    debug_utils::sanity::RegisterSanityCheckFunc(this, Cntr::SanityCheck);

    this->Set(target_cntr);
}

template <meta::IsContainerElem Elem>
constexpr poly_seq_cntr::Cntr<Elem>::~Cntr() {
    debug_utils::sanity::UnregisterSanityCheckFunc(this);
}

template <meta::IsContainerElem Elem>
template <seq_cntr::IsSeqCntr TargetCntr>
constexpr void poly_seq_cntr::Cntr<Elem>::Set(this Cntr& self,
                                              TargetCntr& target_cntr)
    requires meta::IsSame<
        Elem,
        meta::GetTypeWrapperType<decltype(seq_cntr::GetElemType<TargetCntr>())>>
{
    self.max_elem_cnt = seq_cntr::GetMaxElemCnt(target_cntr);

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

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr<Elem>::GetStaticEnabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return seq_cntr::capability::empty_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr<Elem>::GetStaticDisabledCapabilityFlag(
    seq_cntr::Tag, meta::TypeWrapper<Cntr const>) {
    return seq_cntr::capability::non_const_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr<Elem>::GetDynamicEnabledCapabilityFlag(this Cntr& self,
                                                           seq_cntr::Tag) {
    return self.dynamic_enabled_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr<Elem>::GetDynamicEnabledCapabilityFlag(
    this Cntr const& self, seq_cntr::Tag) {
    return self.dynamic_enabled_capability_flag &
           seq_cntr::capability::const_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr<Elem>::GetDynamicDisabledCapabilityFlag(this Cntr& self,
                                                            seq_cntr::Tag) {
    return self.dynamic_disabled_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr seq_cntr::capability::Flag
poly_seq_cntr::Cntr<Elem>::GetDynamicDisabledCapabilityFlag(
    this Cntr const& self, seq_cntr::Tag) {
    return self.dynamic_disabled_capability_flag &
           seq_cntr::capability::const_capability_flag;
}

template <meta::IsContainerElem Elem>
constexpr void* poly_seq_cntr::Cntr<Elem>::GetReferedInstPtr(
    this Cntr const& self, seq_cntr::Tag) {
    return self.target_cntr;
}

template <meta::IsContainerElem Elem>
constexpr meta::TypeWrapper<Elem> poly_seq_cntr::Cntr<Elem>::GetElemType(
    seq_cntr::Tag, meta::TypeWrapper<Cntr>) {
    return {};
}

template <meta::IsContainerElem Elem>
constexpr meta::TypeWrapper<seq_cntr::CursorLimit>
poly_seq_cntr::Cntr<Elem>::GetCursorType(seq_cntr::Tag,
                                         meta::TypeWrapper<Cntr>) {
    return {};
}

template <meta::IsContainerElem Elem>
constexpr meta::TypeWrapper<seq_cntr::CursorLimit>
poly_seq_cntr::Cntr<Elem>::GetCursorType(seq_cntr::Tag,
                                         meta::TypeWrapper<Cntr const>) {
    return {};
}

template <meta::IsContainerElem Elem>
constexpr size_t poly_seq_cntr::Cntr<Elem>::GetElemCnt(this Cntr const& self,
                                                       seq_cntr::Tag) {
    CallMethod(GetElemCnt, get_elem_cnt);
}

template <meta::IsContainerElem Elem>
constexpr size_t poly_seq_cntr::Cntr<Elem>::GetMaxElemCnt(this Cntr const& self,
                                                          seq_cntr::Tag) {
    CallMethod(GetMaxElemCnt, get_max_elem_cnt);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::GetLBCursor(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* dst_cursor) {
    CallMethod(GetLBCursor, get_lb_cursor, dst_cursor);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::GetRBCursor(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* dst_cursor) {
    CallMethod(GetRBCursor, get_rb_cursor, dst_cursor);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::PeekL(
    this Cntr const& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, seq_cntr::CursorLimit* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, Elem* dst_elem) {
    CallMethod(PeekL, peek_l, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem_life_state, dst_elem);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::PeekR(
    this Cntr const& self, seq_cntr::Tag, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, seq_cntr::CursorLimit* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, Elem* dst_elem) {
    CallMethod(PeekR, peek_r, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem_life_state, dst_elem);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::Refer(
    this Cntr const& self, seq_cntr::Tag, size_t idx, bool lazy_copy_elem,
    seq_cntr::ElemPtrView* dst_elem_ptr_view, seq_cntr::CursorLimit* dst_cursor,
    lifecycle::DataLifeState dst_elem_life_state, Elem* dst_elem) {
    CallMethod(Refer, refer, idx, lazy_copy_elem, dst_elem_ptr_view, dst_cursor,
               dst_elem_life_state, dst_elem);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::Derefer(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* pos_cursor,
    bool lazy_copy_elem, seq_cntr::ElemPtrView* dst_elem_ptr_view,
    lifecycle::DataLifeState dst_elem_life_state, Elem* dst_elem) {
    CallMethod(Derefer, derefer, pos_cursor, lazy_copy_elem, dst_elem_ptr_view,
               dst_elem_life_state, dst_elem);
}

template <meta::IsContainerElem Elem>
template <typename Acceptor>
constexpr void poly_seq_cntr::Cntr<Elem>::Read(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* pos_cursor,
    size_t cnt, Acceptor&& acceptor, seq_cntr::CursorLimit* dst_cursor) {
    if constexpr (seq_endpoint::acceptor::IsBasicAcceptor<Acceptor>) {
        CallMethod(Read, read.basic, pos_cursor, cnt, acceptor, dst_cursor);
    } else {
        CallMethod(Read, read.poly, pos_cursor, cnt, acceptor, dst_cursor);
    }
}

template <meta::IsContainerElem Elem>
template <typename Provider>
constexpr void poly_seq_cntr::Cntr<Elem>::Write(
    this Cntr& self, seq_cntr::Tag, seq_cntr::CursorLimit* pos_cursor,
    size_t cnt, Provider&& provider, seq_cntr::CursorLimit* dst_cursor) {
    if constexpr (seq_endpoint::provider::IsBasicProvider<Provider>) {
        CallMethod(Write, write.basic, pos_cursor, cnt, provider, dst_cursor);
    } else {
        CallMethod(Write, write.poly, pos_cursor, cnt, provider, dst_cursor);
    }
}

template <meta::IsContainerElem Elem>
template <typename Acceptor>
constexpr void poly_seq_cntr::Cntr<Elem>::ReadWrite(
    this Cntr& self, seq_cntr::Tag, seq_cntr::CursorLimit* pos_cursor,
    size_t cnt, Acceptor&& acceptor, seq_cntr::CursorLimit* dst_cursor) {
    CallMethod(ReadWrite, read_write.poly, pos_cursor, cnt, acceptor,
               dst_cursor);
}

template <meta::IsContainerElem Elem>
template <typename Provider>
constexpr void poly_seq_cntr::Cntr<Elem>::PushL(
    this Cntr& self, seq_cntr::Tag, size_t cnt, Provider&& provider,
    seq_cntr::CursorLimit* dst_beg_cursor,
    seq_cntr::CursorLimit* dst_end_cursor) {
    if constexpr (seq_endpoint::provider::IsBasicProvider<Provider>) {
        CallMethod(PushL, push_l.basic, cnt, provider, dst_beg_cursor,
                   dst_end_cursor);
    } else {
        CallMethod(PushL, push_l.poly, cnt, provider, dst_beg_cursor,
                   dst_end_cursor);
    }
}

template <meta::IsContainerElem Elem>
template <typename Provider>
constexpr void poly_seq_cntr::Cntr<Elem>::PushR(
    this Cntr& self, seq_cntr::Tag, size_t cnt, Provider&& provider,
    seq_cntr::CursorLimit* dst_beg_cursor,
    seq_cntr::CursorLimit* dst_end_cursor) {
    if constexpr (seq_endpoint::provider::IsBasicProvider<Provider>) {
        CallMethod(PushR, push_r.basic, cnt, provider, dst_beg_cursor,
                   dst_end_cursor);
    } else {
        CallMethod(PushR, push_r.poly, cnt, provider, dst_beg_cursor,
                   dst_end_cursor);
    }
}

template <meta::IsContainerElem Elem>
template <typename Provider>
constexpr void poly_seq_cntr::Cntr<Elem>::Insert(
    this Cntr& self, seq_cntr::Tag, seq_cntr::CursorLimit* pos_cursor,
    size_t cnt, Provider&& provider, seq_cntr::CursorLimit* dst_cursor) {
    if constexpr (seq_endpoint::provider::IsBasicProvider<Provider>) {
        CallMethod(Insert, insert.basic, pos_cursor, cnt, provider, dst_cursor);
    } else {
        CallMethod(Insert, insert.poly, pos_cursor, cnt, provider, dst_cursor);
    }
}

template <meta::IsContainerElem Elem>
template <typename Acceptor>
constexpr void poly_seq_cntr::Cntr<Elem>::PopL(
    this Cntr& self, seq_cntr::Tag, size_t cnt, Acceptor&& acceptor,
    seq_cntr::CursorLimit* dst_cursor) {
    if constexpr (seq_endpoint::acceptor::IsBasicAcceptor<Acceptor>) {
        CallMethod(PopL, pop_l.basic, cnt, acceptor, dst_cursor);
    } else {
        CallMethod(PopL, pop_l.poly, cnt, acceptor, dst_cursor);
    }
}

template <meta::IsContainerElem Elem>
template <typename Acceptor>
constexpr void poly_seq_cntr::Cntr<Elem>::PopR(
    this Cntr& self, seq_cntr::Tag, size_t cnt, Acceptor&& acceptor,
    seq_cntr::CursorLimit* dst_cursor) {
    if constexpr (seq_endpoint::acceptor::IsBasicAcceptor<Acceptor>) {
        CallMethod(PopR, pop_r.basic, cnt, acceptor, dst_cursor);
    } else {
        CallMethod(PopR, pop_r.poly, cnt, acceptor, dst_cursor);
    }
}

template <meta::IsContainerElem Elem>
template <typename Acceptor>
constexpr void poly_seq_cntr::Cntr<Elem>::Erase(
    this Cntr& self, seq_cntr::Tag, seq_cntr::CursorLimit* pos_cursor,
    size_t cnt, Acceptor&& acceptor) {
    if constexpr (seq_endpoint::acceptor::IsBasicAcceptor<Acceptor>) {
        CallMethod(Erase, erase.basic, pos_cursor, cnt, acceptor);
    } else {
        CallMethod(Erase, erase.poly, pos_cursor, cnt, acceptor);
    }
}

template <meta::IsContainerElem Elem>
template <typename Acceptor>
constexpr void poly_seq_cntr::Cntr<Elem>::EraseAll(this Cntr& self,
                                                   seq_cntr::Tag,
                                                   Acceptor&& acceptor) {
    if constexpr (meta::IsSame<meta::RemoveCVRef<Acceptor>,
                               seq_endpoint::acceptor::BasicAcceptor>) {
        CallMethod(EraseAll, erase_all.basic, acceptor);
    } else {
        CallMethod(EraseAll, erase_all.poly, acceptor);
    }
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::CopyCursor(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* src_cursor,
    seq_cntr::CursorLimit* dst_cursor) {
    CallMethod(CopyCursor, copy_cursor, src_cursor, dst_cursor);
}

template <meta::IsContainerElem Elem>
constexpr bool poly_seq_cntr::Cntr<Elem>::AreEqualCursor(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor_a,
    seq_cntr::CursorLimit* cursor_b) {
    CallMethod(AreEqualCursor, are_equal_cursor, cursor_a, cursor_b);
}

template <meta::IsContainerElem Elem>
constexpr comparison::Ordering poly_seq_cntr::Cntr<Elem>::CompareCursor(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor_a,
    seq_cntr::CursorLimit* cursor_b) {
    CallMethod(CompareCursor, compare_cursor, cursor_a, cursor_b);
}

template <meta::IsContainerElem Elem>
constexpr size_t poly_seq_cntr::Cntr<Elem>::GetCursorDist(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor_a,
    seq_cntr::CursorLimit* cursor_b) {
    CallMethod(GetCursorDist, get_cursor_dist, cursor_a, cursor_b);
}

template <meta::IsContainerElem Elem>
constexpr size_t poly_seq_cntr::Cntr<Elem>::GetCursorIdx(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor) {
    CallMethod(GetCursorIdx, get_cursor_idx, cursor);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::CursorStepL(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor) {
    CallMethod(CursorStepL, cursor_step_l, cursor);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::CursorStepR(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor) {
    CallMethod(CursorStepR, cursor_step_r, cursor);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::CursorAdvanceL(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor,
    size_t step) {
    CallMethod(CursorAdvanceL, cursor_advance_l, cursor, step);
}

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::CursorAdvanceR(
    this Cntr const& self, seq_cntr::Tag, seq_cntr::CursorLimit* cursor,
    size_t step) {
    CallMethod(CursorAdvanceR, cursor_advance_r, cursor, step);
}

#pragma pop_macro("CallMethod")
#pragma pop_macro("CallMethod_")

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::Check(this Cntr const& self) {
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

    CheckMethod(Read, read.basic);
    CheckMethod(Read, read.poly);

    CheckMethod(Write, write.basic);
    CheckMethod(Write, write.poly);

    CheckMethod(ReadWrite, read_write.poly);

    CheckMethod(PushL, push_l.basic);
    CheckMethod(PushL, push_l.poly);

    CheckMethod(PushR, push_r.basic);
    CheckMethod(PushR, push_r.poly);

    CheckMethod(Insert, insert.basic);
    CheckMethod(Insert, insert.poly);

    CheckMethod(PopL, pop_l.basic);
    CheckMethod(PopL, pop_l.poly);

    CheckMethod(PopR, pop_r.basic);
    CheckMethod(PopR, pop_r.poly);

    CheckMethod(Erase, erase.basic);
    CheckMethod(Erase, erase.poly);

    CheckMethod(EraseAll, erase_all.basic);
    CheckMethod(EraseAll, erase_all.poly);

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

template <meta::IsContainerElem Elem>
constexpr void poly_seq_cntr::Cntr<Elem>::SanityCheck(
    void const* self_, debug_utils::sanity::SanityCheckScope scope) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(self_ != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        scope != debug_utils::sanity::SanityCheckScope::Basic ||
        scope != debug_utils::sanity::SanityCheckScope::Complete);

    auto& self{ *static_cast<Cntr const*>(self_) };

    self.Check();

    debug_utils::sanity::ExpandFinishedSanityCheckScope(
        self_, debug_utils::sanity::SanityCheckScope::Basic);

    if (scope == debug_utils::sanity::SanityCheckScope::Complete) {
        debug_utils::sanity::SanityCheck(self.target_cntr, scope);
    }
}

#pragma pop_macro("TestCapability")

}  // namespace zeta::core
