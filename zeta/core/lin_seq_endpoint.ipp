#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/lin_seq_endpoint.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <meta::IsContainerElem Elem>
constexpr meta::TypeWrapper<Elem>
lin_seq_endpoint::acceptor::Acceptor<Elem>::GetElemType(
    seq_endpoint::acceptor::Tag) {
    return {};
}

template <meta::IsContainerElem Elem>
constexpr bool lin_seq_endpoint::acceptor::Acceptor<Elem>::IsEnd(
    this Acceptor const& self, seq_endpoint::acceptor::Tag) {
    return self.elem_cnt == 0;
}

namespace lin_seq_endpoint::acceptor::detail {

template <meta::IsContainerElem Elem, typename SrcElem>
constexpr size_t Transfer_(
    Acceptor<Elem>& self, seq_endpoint::acceptor::Tag,
    lifecycle::DataTransferSemantics src_transfer_semantics, SrcElem* src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    size_t transfer_elem_cnt{ comparison_utils::BasicMin(self.elem_cnt, cnt) };

    utils::DisjointLinSeqTransfer(
        lifecycle::DeriveDataTransferOp(self.data_life_state,
                                        src_transfer_semantics),
        self.data, src, self.elem_stride, src_elem_stride, transfer_elem_cnt);

    self.data =
        utils::PtrInc(self.data, self.elem_stride *
                                     static_cast<ptrdiff_t>(transfer_elem_cnt));

    self.elem_cnt -= transfer_elem_cnt;

    return transfer_elem_cnt;
}

}  // namespace lin_seq_endpoint::acceptor::detail

template <meta::IsContainerElem Elem>
template <typename SrcElem>
constexpr size_t lin_seq_endpoint::acceptor::Acceptor<Elem>::Transfer(
    this Acceptor& self, seq_endpoint::acceptor::Tag,
    lifecycle::DataTransferSemantics src_transfer_semantics, SrcElem* src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    return detail::Transfer_(self, seq_endpoint::acceptor::Tag{},
                             src_transfer_semantics, src, src_elem_stride, cnt);
}

// -----------------------------------------------------------------------------

template <meta::IsContainerElem Elem>
constexpr meta::TypeWrapper<Elem>
lin_seq_endpoint::provider::Provider<Elem>::GetElemType(
    seq_endpoint::provider::Tag) {
    return {};
}

template <meta::IsContainerElem Elem>
constexpr bool lin_seq_endpoint::provider::Provider<Elem>::IsEnd(
    this Provider const& self, seq_endpoint::provider::Tag) {
    return self.elem_cnt == 0;
}

template <meta::IsContainerElem Elem>
template <typename DstElem>
constexpr size_t lin_seq_endpoint::provider::Provider<Elem>::Transfer(
    this Provider& self, seq_endpoint::provider::Tag,
    lifecycle::DataLifeState dst_life_state, DstElem* dst,
    ptrdiff_t dst_elem_stride, size_t cnt) {
    size_t transfer_elem_cnt{ comparison_utils::BasicMin(self.elem_cnt, cnt) };

    utils::DisjointLinSeqTransfer(
        lifecycle::DeriveDataTransferOp(dst_life_state,
                                        self.data_transfer_semantics),
        dst, self.data, dst_elem_stride, self.elem_stride, transfer_elem_cnt);

    self.data =
        utils::PtrInc(self.data, self.elem_stride *
                                     static_cast<ptrdiff_t>(transfer_elem_cnt));

    self.elem_cnt -= transfer_elem_cnt;

    return transfer_elem_cnt;
}

}  // namespace zeta::core
