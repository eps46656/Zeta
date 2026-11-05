#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/lin_seq_endpoint.ipp>
#include <zeta/core/seq_cntr_utils.hpp>

namespace zeta::core {

template <seq_cntr::IsSeqCntr Cntr>
constexpr bool seq_cntr_utils::acceptor::Acceptor<Cntr>::IsEnd(
    this Acceptor const& self, seq_endpoint::acceptor::Tag) {
    return self.cnt == 0;
}

template <seq_cntr::IsSeqCntr Cntr>
template <lifecycle::IsSrcStorageView SrcStorageView>
constexpr size_t seq_cntr_utils::acceptor::Acceptor<Cntr>::Transfer(
    this Acceptor& self, seq_endpoint::acceptor::Tag, SrcStorageView src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    size_t transfer_cnt{ comparison_utils::BasicMin(self.cnt, cnt) };

    seq_cntr::Write(meta::Forward<Cntr>(self.cntr), &self.cursor, transfer_cnt,
                    lin_seq_endpoint::provider::Provider<Elem>{
                        .data{ src },
                        .elem_stride{ src_elem_stride },
                        .elem_cnt{ transfer_cnt },
                    },
                    &self.cursor);

    self.cnt -= transfer_cnt;

    return transfer_cnt;
}

template <seq_cntr::IsSeqCntr Cntr>
constexpr bool seq_cntr_utils::provider::Provider<Cntr>::IsEnd(
    this Provider const& self, seq_endpoint::provider::Tag) {
    return self.cnt == 0;
}

template <seq_cntr::IsSeqCntr Cntr>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr size_t seq_cntr_utils::provider::Provider<Cntr>::Transfer(
    this Provider& self, seq_endpoint::provider::Tag, DstStorageView dst,
    ptrdiff_t dst_elem_stride, size_t cnt) {
    size_t transfer_cnt{ comparison_utils::BasicMin(self.cnt, cnt) };

    seq_cntr::Read(meta::Forward<Cntr>(self.cntr), &self.cursor, transfer_cnt,
                   lin_seq_endpoint::acceptor::Acceptor<DstStorageView>{
                       .data{ dst },
                       .elem_stride{ dst_elem_stride },
                       .elem_cnt{ transfer_cnt },
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

    seq_cntr::Refer(dst_cntr, dst_beg, &dst_cursor, nullptr);
    seq_cntr::Refer(src_cntr, src_beg, &src_cursor, nullptr);

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
       << meta::EnumToUnderlying(seq_cntr::capability::Kind::cap_name))) != 0)

    size_t dst_elem_cnt{ seq_cntr::GetElemCnt(dst_cntr) };
    size_t src_elem_cnt{ seq_cntr::GetElemCnt(src_cntr) };

    using DstCursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<DstCntr&&>{}))>;

    using SrcCursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<SrcCntr&&>{}))>;

    DstCursor dst_cursor;
    SrcCursor src_cursor;

    seq_cntr::PeekL(src_cntr, &src_cursor, nullptr);

    provider::Provider<SrcCntr> provider{ meta::Forward<SrcCntr>(src_cntr),
                                          src_cursor, src_elem_cnt };

    constexpr seq_cntr::capability::Flag se_capability_flag{
        seq_cntr::GetStaticEnabledCapabilityFlag(meta::TypeWrapper<DstCntr&&>())
    };

    constexpr seq_cntr::capability::Flag sd_capability_flag{
        seq_cntr::GetStaticDisabledCapabilityFlag(
            meta::TypeWrapper<DstCntr&&>())
    };

    seq_cntr::capability::Flag de_capability_flag{
        seq_cntr::GetDynamicEnabledCapabilityFlag(dst_cntr)
    };

    constexpr bool se_push_l{ TestCapability(se_capability_flag, PushL) };
    constexpr bool sd_push_l{ TestCapability(sd_capability_flag, PushL) };

    bool de_push_l{ TestCapability(de_capability_flag, PushL) };

    constexpr bool se_push_r{ TestCapability(se_capability_flag, PushR) };
    constexpr bool sd_push_r{ TestCapability(sd_capability_flag, PushR) };

    bool de_push_r{ TestCapability(de_capability_flag, PushR) };

    constexpr bool se_pop_l{ TestCapability(se_capability_flag, PopL) };
    constexpr bool sd_pop_l{ TestCapability(sd_capability_flag, PopL) };

    bool de_pop_l{ TestCapability(de_capability_flag, PopL) };

    constexpr bool se_pop_r{ TestCapability(se_capability_flag, PopR) };
    constexpr bool sd_pop_r{ TestCapability(sd_capability_flag, PopR) };

    bool de_pop_r{ TestCapability(de_capability_flag, PopR) };
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
    seq_cntr::PeekL(dst_cntr, &dst_cursor, nullptr);                          \
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
    seq_cntr::PeekL(dst_cntr, &dst_cursor, nullptr);                         \
    seq_cntr::Write(dst_cntr, &dst_cursor, dst_elem_cnt, provider, nullptr); \
    seq_cntr::PopR(dst_cntr, dst_elem_cnt - src_elem_cnt,                    \
                   seq_endpoint::acceptor::BasicAcceptor{}, nullptr);

    // NOLINTBEGIN(bugprone-branch-clone)
    if (dst_elem_cnt < src_elem_cnt) {
        if constexpr (se_push_r) {
            FPushR;
        } else if constexpr (se_push_l) {
            FPushL;
        } else if constexpr (sd_push_r) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(de_push_l);
            FPushL;
        } else if constexpr (sd_push_l) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(de_push_r);
            FPushR;
        } else if (de_push_r) {
            FPushR;
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(de_push_l);
            FPushL;
        }
    } else if (src_elem_cnt < dst_elem_cnt) {
        if constexpr (se_pop_r) {
            FPopR;
        } else if constexpr (se_pop_l) {
            FPopL;
        } else if constexpr (sd_pop_r) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(de_pop_l);
            FPopL;
        } else if constexpr (sd_pop_l) {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(de_pop_r);
            FPopR;
        } else if (de_pop_r) {
            FPopR;
        } else {
            ZETA_Core_DebugUtils_Diag_PromiseAssert(de_pop_l);
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

template <size_t ChannelCnt>
constexpr seq_cntr::capability::Flag
seq_cntr_utils::CapabilityFlagConfigurator<ChannelCnt>::BuildFlag(
    this CapabilityFlagConfigurator const& self, size_t channel_idx,
    CapabilityKind capability_kind) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(channel_idx < ChannelCnt);

    auto capability_kind_underlying{ meta::EnumToUnderlying(capability_kind) };

    return seq_cntr::capability::FlagBuilder{
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const)                               \
    .name{ (meta::EnumToUnderlying(self.name[channel_idx]) & \
            capability_kind_underlying) != 0 },

        ZETA_Core_SeqCntr_CapabilityWithoutAlwaysNever_XMacro(F, )

#pragma pop_macro("F")
    }();
}

}  // namespace zeta::core
