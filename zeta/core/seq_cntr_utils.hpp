#pragma once

#include <zeta/core/array.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::seq_cntr_utils {

namespace acceptor {

template <seq_cntr::IsSeqCntr Cntr>
struct Acceptor {
    using Elem = meta::GetTypeWrapperType<decltype(seq_cntr::GetElemType(
        meta::TypeWrapper<meta::RemoveReferenceFromType<Cntr>>{}))>;

    using Cursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<meta::RemoveReferenceFromType<Cntr>>{}))>;

    Cntr&& cntr;
    Cursor cursor;
    size_t cnt;

    constexpr bool IsEnd(this Acceptor const& self,
                         seq_endpoint::acceptor::Tag);

    template <lifecycle::IsSrcStorageView SrcStorageView>
    constexpr size_t Transfer(this Acceptor& self, seq_endpoint::acceptor::Tag,
                              SrcStorageView src, ptrdiff_t src_elem_stride,
                              size_t cnt);
};

}  // namespace acceptor

namespace provider {

template <seq_cntr::IsSeqCntr Cntr>
struct Provider {
    using Elem = meta::GetTypeWrapperType<decltype(seq_cntr::GetElemType(
        meta::TypeWrapper<Cntr>{}))>;

    using Cursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<Cntr>{}))>;

    Cntr cntr;
    Cursor cursor;
    size_t cnt;

    constexpr bool IsEnd(this Provider const& self,
                         seq_endpoint::provider::Tag);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr size_t Transfer(this Provider& self, seq_endpoint::provider::Tag,
                              DstStorageView dst, ptrdiff_t dst_elem_stride,
                              size_t cnt);
};

}  // namespace provider

template <seq_cntr::IsSeqCntr DstSeqCntr, seq_cntr::IsSeqCntr SrcSeqCntr>
constexpr void RangeTransfer(DstSeqCntr&& dst_cntr, SrcSeqCntr&& src_cntr,
                             size_t dst_beg, size_t src_beg, size_t cnt);

template <seq_cntr::IsSeqCntr DstSeqCntr, seq_cntr::IsSeqCntr SrcSeqCntr>
constexpr void Transfer(DstSeqCntr&& dst_cntr, SrcSeqCntr&& src_cntr);

enum struct CapabilityKind : unsigned char {
    StaticEnable   = 0b0001,
    StaticDisable  = 0b0010,
    DynamicEnable  = 0b0100,
    DynamicDisable = 0b1000,

    SE = StaticEnable,
    SD = StaticDisable,
    DE = DynamicEnable,
    DD = DynamicDisable,
};

template <size_t ChannelCnt>
struct CapabilityFlagConfigurator {
#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, num, is_const) \
    array::Array<CapabilityKind, ChannelCnt> const name;

    ZETA_Core_SeqCntr_Capability_XMacro(F, );

#pragma pop_macro("F")

    constexpr seq_cntr::capability::Flag BuildFlag(
        this CapabilityFlagConfigurator const& self, size_t channel_idx,
        CapabilityKind capability_kind);
};

}  // namespace zeta::core::seq_cntr_utils
