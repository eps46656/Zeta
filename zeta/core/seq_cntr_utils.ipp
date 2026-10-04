#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/lin_seq_endpoint.ipp>
#include <zeta/core/seq_cntr_utils.hpp>

namespace zeta::core {

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr_utils::acceptor::Acceptor<Cntr>::Acceptor(
    Cntr&& cntr, Cursor const& cursor, size_t cnt)
    : cntr{ meta::Forward<Cntr>(cntr) }, cursor{ cursor }, cnt{ cnt } {}

template <seq_cntr::IsSeqCntr Cntr>
constexpr bool seq_cntr_utils::acceptor::Acceptor<Cntr>::IsEnd(
    this Acceptor const& self, seq_cntr::Tag) {
    return self.cnt == 0;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr size_t seq_cntr_utils::acceptor::Acceptor<Cntr>::Transfer(
    this Acceptor& self, seq_cntr::Tag,
    lifecycle::DataTransferSemantics src_transfer_semantics, Elem* src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    size_t transfer_cnt{ comparison_utils::BasicMin(self.cnt, cnt) };

    seq_cntr::Write(meta::Forward<Cntr>(self.cntr), &self.cursor, transfer_cnt,
                    lin_seq_endpoint::provider::Provider<Elem>{
                        .data_transfer_semantics = src_transfer_semantics,
                        .data = src,
                        .elem_stride = src_elem_stride,
                        .elem_cnt = transfer_cnt,
                    },
                    &self.cursor);

    self.cnt -= transfer_cnt;

    return transfer_cnt;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr seq_cntr_utils::provider::Provider<Cntr>::Provider(
    Cntr&& cntr, Cursor const& cursor, size_t cnt)
    : cntr{ meta::Forward<Cntr>(cntr) }, cursor{ cursor }, cnt{ cnt } {}

template <seq_cntr::IsSeqCntr Cntr>
constexpr bool seq_cntr_utils::provider::Provider<Cntr>::IsEnd(
    this Provider const& self, seq_cntr::Tag) {
    return self.cnt == 0;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr size_t seq_cntr_utils::provider::Provider<Cntr>::Transfer(
    this Provider& self, seq_cntr::Tag, lifecycle::DataLifeState dst_life_state,
    Elem* dst, ptrdiff_t dst_elem_stride, size_t cnt) {
    size_t transfer_cnt{ comparison_utils::BasicMin(self.cnt, cnt) };

    seq_cntr::Read(meta::Forward<Cntr>(self.cntr), &self.cursor, transfer_cnt,
                   lin_seq_endpoint::acceptor::Acceptor<Elem>{
                       .data_life_state = dst_life_state,
                       .data = dst,
                       .elem_stride = dst_elem_stride,
                       .elem_cnt = transfer_cnt,
                   },
                   &self.cursor);

    self.cnt -= transfer_cnt;

    return transfer_cnt;
}

template <seq_cntr::IsSeqCntr DstCntr, seq_cntr::IsSeqCntr SrcCntr>
constexpr void seq_cntr_utils::RangeTransfer(DstCntr&& dst_cntr,
                                             SrcCntr&& src_cntr, size_t dst_beg,
                                             size_t src_beg, size_t cnt) {
    size_t dst_elem_cnt{ seq_cntr::GetElemCnt(dst_cntr) };
    size_t src_elem_cnt{ seq_cntr::GetElemCnt(src_cntr) };

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanDerefer(dst_beg, cnt, dst_elem_cnt));
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        seq_cntr::op_check::CanDerefer(src_beg, cnt, src_elem_cnt));

    if (cnt == 0) { return; }

    void const* real_dst_cntr{ seq_cntr::GetReferedInstPtr(dst_cntr) };
    void const* real_src_cntr{ seq_cntr::GetReferedInstPtr(src_cntr) };

    if (real_dst_cntr == real_src_cntr) {
        // TODO
        ZETA_Core_DebugUtils_Diag_Unreachable();
        return;
    }

    using DstCursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<DstCntr>{}))>;

    using SrcCursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<SrcCntr>{}))>;

    DstCursor dst_cursor;
    SrcCursor src_cursor;

    seq_cntr::Refer(dst_cntr, dst_beg, true, nullptr, &dst_cursor, nullptr);
    seq_cntr::Refer(src_cntr, src_beg, true, nullptr, &src_cursor, nullptr);

    provider::Provider<SrcCntr> provider{ meta::Forward<SrcCntr>(src_cntr),
                                          src_cursor, cnt };

    seq_cntr::Write(dst_cntr, &dst_cursor, cnt, provider, &dst_cursor);
}

template <seq_cntr::IsSeqCntr DstCntr, seq_cntr::IsSeqCntr SrcCntr>
constexpr void seq_cntr_utils::Transfer(DstCntr&& dst_cntr,
                                        SrcCntr&& src_cntr) {
#pragma push_macro("TestCapability")
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TestCapability(cap_flag, cap_name)        \
    (((cap_flag) &                                \
      (static_cast<seq_cntr::capability::Flag>(1) \
       << meta::ToUnderlying(seq_cntr::capability::Kind::cap_name))) != 0)

    size_t dst_elem_cnt{ seq_cntr::GetElemCnt(dst_cntr) };
    size_t src_elem_cnt{ seq_cntr::GetElemCnt(src_cntr) };

    using DstCursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<DstCntr>{}))>;

    using SrcCursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<SrcCntr>{}))>;

    DstCursor dst_cursor;
    SrcCursor src_cursor;

    seq_cntr::PeekL(src_cntr, true, nullptr, &src_cursor, nullptr);

    provider::Provider<SrcCntr> provider{ meta::Forward<SrcCntr>(src_cntr),
                                          src_cursor, src_elem_cnt };

    constexpr seq_cntr::capability::Flag static_enabled_capability_flag{
        seq_cntr::GetStaticEnabledCapabilityFlag<DstCntr>()
    };

    constexpr seq_cntr::capability::Flag static_disabled_capability_flag{
        seq_cntr::GetStaticDisabledCapabilityFlag<DstCntr>()
    };

    seq_cntr::capability::Flag dynamic_enabled_capability_flag{
        seq_cntr::GetDynamicEnabledCapabilityFlag(dst_cntr)
    };

    constexpr bool static_enabled_push_l{ TestCapability(
        static_enabled_capability_flag, PushL) };

    constexpr bool static_disabled_push_l{ TestCapability(
        static_disabled_capability_flag, PushL) };

    bool dynamic_enabled_push_l{ TestCapability(dynamic_enabled_capability_flag,
                                                PushL) };

    constexpr bool static_enabled_push_r{ TestCapability(
        static_enabled_capability_flag, PushR) };

    constexpr bool static_disabled_push_r{ TestCapability(
        static_disabled_capability_flag, PushR) };

    bool dynamic_enabled_push_r{ TestCapability(dynamic_enabled_capability_flag,
                                                PushR) };

    constexpr bool static_enabled_pop_l{ TestCapability(
        static_enabled_capability_flag, PopL) };

    constexpr bool static_disabled_pop_l{ TestCapability(
        static_disabled_capability_flag, PopL) };

    bool dynamic_enabled_pop_l{ TestCapability(dynamic_enabled_capability_flag,
                                               PopL) };

    constexpr bool static_enabled_pop_r{ TestCapability(
        static_enabled_capability_flag, PopR) };

    constexpr bool static_disabled_pop_r{ TestCapability(
        static_disabled_capability_flag, PopR) };

    bool dynamic_enabled_pop_r{ TestCapability(dynamic_enabled_capability_flag,
                                               PopR) };
    /*

       se sd de dd
    se R  L  L  L
    sd R  X  R  X
    de R  L  R  L
    dd R  X  R  X

    */

#pragma push_macro("FPushL")
#define FPushL                                                                \
    seq_cntr::PushL(dst_cntr, src_elem_cnt - dst_elem_cnt, provider, nullptr, \
                    &dst_cursor);                                             \
    seq_cntr::Write(dst_cntr, &dst_cursor, dst_elem_cnt, provider, nullptr);

#pragma push_macro("FPushR")
#define FPushR                                                                \
    seq_cntr::PeekL(dst_cntr, true, nullptr, &dst_cursor,                     \
                    lifecycle::DataLifeState::Mem, nullptr);                  \
    seq_cntr::Write(dst_cntr, &dst_cursor, dst_elem_cnt, provider, nullptr);  \
    seq_cntr::PushR(dst_cntr, src_elem_cnt - dst_elem_cnt, provider, nullptr, \
                    nullptr);

#pragma push_macro("FPopL")
#define FPopL                                                             \
    seq_cntr::PopL(dst_cntr, dst_elem_cnt - src_elem_cnt,                 \
                   seq_endpoint::acceptor::BasicAcceptor{}, &dst_cursor); \
    seq_cntr::Write(dst_cntr, &dst_cursor, dst_elem_cnt, provider, nullptr);

#pragma push_macro("FPopR")
#define FPopR                                                                \
    seq_cntr::PeekL(dst_cntr, true, nullptr, &dst_cursor,                    \
                    lifecycle::DataLifeState::Mem, nullptr);                 \
    seq_cntr::Write(dst_cntr, &dst_cursor, dst_elem_cnt, provider, nullptr); \
    seq_cntr::PopR(dst_cntr, dst_elem_cnt - src_elem_cnt,                    \
                   seq_endpoint::acceptor::BasicAcceptor{}, nullptr);

    // NOLINTBEGIN(bugprone-branch-clone)
    if (dst_elem_cnt < src_elem_cnt) {
        if constexpr (static_enabled_push_r) {
            FPushR;
        } else if constexpr (static_enabled_push_l) {
            FPushL;
        } else if constexpr (static_disabled_push_r) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(dynamic_enabled_push_l);
            FPushL;
        } else if constexpr (static_disabled_push_l) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(dynamic_enabled_push_r);
            FPushR;
        } else if (dynamic_enabled_push_r) {
            FPushR;
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(dynamic_enabled_push_l);
            FPushL;
        }
    } else if (src_elem_cnt < dst_elem_cnt) {
        if constexpr (static_enabled_pop_r) {
            FPopR;
        } else if constexpr (static_enabled_pop_l) {
            FPopL;
        } else if constexpr (static_disabled_pop_r) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(dynamic_enabled_pop_l);
            FPopL;
        } else if constexpr (static_disabled_pop_l) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(dynamic_enabled_pop_r);
            FPopR;
        } else if (dynamic_enabled_pop_r) {
            FPopR;
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(dynamic_enabled_pop_l);
            FPopL;
        }
    } else {
        seq_cntr::Write(dst_cntr, &dst_cursor, dst_elem_cnt, provider, nullptr);
    }
    // NOLINTEND(bugprone-branch-clone)

#pragma pop_macro("FPopR")
#pragma pop_macro("FPopL")
#pragma pop_macro("FPushR")
#pragma pop_macro("FPushL")

#pragma pop_macro("TestCapability")
}

}  // namespace zeta::core
