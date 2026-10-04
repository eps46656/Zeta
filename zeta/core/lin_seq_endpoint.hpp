#pragma once

#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::lin_seq_endpoint {

namespace acceptor {

template <meta::IsContainerElem Elem>
struct Acceptor {
    lifecycle::DataLifeState data_life_state;
    Elem* data;
    ptrdiff_t elem_stride;
    size_t elem_cnt;

    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_endpoint::acceptor::Tag);

    constexpr bool IsEnd(this Acceptor const& self,
                         seq_endpoint::acceptor::Tag);

    template <typename SrcElem>
    constexpr size_t Transfer(
        this Acceptor& self, seq_endpoint::acceptor::Tag,
        lifecycle::DataTransferSemantics src_transfer_semantics, SrcElem* src,
        ptrdiff_t src_elem_stride, size_t cnt);
};

}  // namespace acceptor

namespace provider {

template <meta::IsContainerElem Elem>
struct Provider {
    lifecycle::DataTransferSemantics data_transfer_semantics;
    Elem const* data;
    ptrdiff_t elem_stride;
    size_t elem_cnt;

    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_endpoint::provider::Tag);

    constexpr bool IsEnd(this Provider const& self,
                         seq_endpoint::provider::Tag);

    template <typename DstElem>
    constexpr size_t Transfer(this Provider& self, seq_endpoint::provider::Tag,
                              lifecycle::DataLifeState dst_life_state,
                              DstElem* dst, ptrdiff_t dst_elem_stride,
                              size_t cnt);
};

}  // namespace provider

}  // namespace zeta::core::lin_seq_endpoint
