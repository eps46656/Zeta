#pragma once

#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::seq_cntr_utils {

namespace acceptor {

template <seq_cntr::IsSeqCntr Cntr>
struct Acceptor {
    using Elem = meta::GetTypeWrapperType<decltype(seq_cntr::GetElemType(
        meta::TypeWrapper<Cntr>{}))>;

    using Cursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<Cntr>{}))>;

    Cntr&& cntr;
    Cursor cursor;
    size_t cnt;

    constexpr Acceptor(Cntr&& cntr, Cursor const& cursor, size_t elem_cnt);

    constexpr Acceptor(Acceptor const&) = default;

    constexpr bool IsEnd(this Acceptor const& self, seq_cntr::Tag);

    constexpr size_t Transfer(
        this Acceptor& self, seq_cntr::Tag,
        lifecycle::DataTransferSemantics src_transfer_semantics, Elem* src,
        ptrdiff_t src_elem_stride, size_t cnt);
};

}  // namespace acceptor

namespace provider {

template <seq_cntr::IsSeqCntr Cntr>
struct Provider {
    using Elem = meta::GetTypeWrapperType<decltype(seq_cntr::GetElemType(
        meta::TypeWrapper<Cntr>{}))>;

    using Cursor = meta::GetTypeWrapperType<decltype(seq_cntr::GetCursorType(
        meta::TypeWrapper<Cntr>{}))>;

    Cntr&& cntr;
    Cursor cursor;
    size_t cnt;

    constexpr Provider(Cntr&& cntr, Cursor const& cursor, size_t cnt);

    constexpr Provider(Provider const&) = default;

    constexpr bool IsEnd(this Provider const& self, seq_cntr::Tag);

    constexpr size_t Transfer(this Provider& self, seq_cntr::Tag,
                              lifecycle::DataLifeState dst_life_state,
                              Elem* dst, ptrdiff_t dst_elem_stride, size_t cnt);
};

}  // namespace provider

template <seq_cntr::IsSeqCntr DstSeqCntr, seq_cntr::IsSeqCntr SrcSeqCntr>
constexpr void RangeTransfer(DstSeqCntr&& dst_cntr, SrcSeqCntr&& src_cntr,
                             size_t dst_beg, size_t src_beg, size_t cnt);

template <seq_cntr::IsSeqCntr DstSeqCntr, seq_cntr::IsSeqCntr SrcSeqCntr>
constexpr void Transfer(DstSeqCntr&& dst_cntr, SrcSeqCntr&& src_cntr);

}  // namespace zeta::core::seq_cntr_utils
