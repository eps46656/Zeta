#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/lin_seq_endpoint.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

template <lifecycle::IsStorageView ElemView>
constexpr meta::TypeWrapper<
    typename lin_seq_endpoint::acceptor::Acceptor<ElemView>::Elem>
lin_seq_endpoint::acceptor::Acceptor<ElemView>::GetElemType(
    seq_endpoint::acceptor::Tag) {
    return {};
}

template <lifecycle::IsStorageView ElemView>
constexpr bool lin_seq_endpoint::acceptor::Acceptor<ElemView>::IsEnd(
    this Acceptor const& self, seq_endpoint::acceptor::Tag) {
    return self.elem_cnt == 0;
}

template <lifecycle::IsStorageView ElemView>
template <lifecycle::IsSrcStorageView SrcStorageView>
constexpr size_t lin_seq_endpoint::acceptor::Acceptor<ElemView>::Transfer(
    this Acceptor& self, seq_endpoint::acceptor::Tag, SrcStorageView src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    size_t transfer_elem_cnt{ comparison_utils::BasicMin(self.elem_cnt, cnt) };

    utils::DisjointLinSeqTransfer(self.data, src, self.elem_stride,
                                  src_elem_stride, transfer_elem_cnt);

    self.data.ptr = utils::PtrInc(
        self.data.ptr,
        self.elem_stride * static_cast<ptrdiff_t>(transfer_elem_cnt));

    self.elem_cnt -= transfer_elem_cnt;

    return transfer_elem_cnt;
}

// -----------------------------------------------------------------------------

template <lifecycle::IsSrcStorageView ElemView>
constexpr meta::TypeWrapper<
    typename lin_seq_endpoint::provider::Provider<ElemView>::Elem>
lin_seq_endpoint::provider::Provider<ElemView>::GetElemType(
    seq_endpoint::provider::Tag) {
    return {};
}

template <lifecycle::IsSrcStorageView ElemView>
constexpr bool lin_seq_endpoint::provider::Provider<ElemView>::IsEnd(
    this Provider const& self, seq_endpoint::provider::Tag) {
    return self.elem_cnt == 0;
}

template <lifecycle::IsSrcStorageView ElemView>
template <lifecycle::IsDstStorageView DstStorageView>
constexpr size_t lin_seq_endpoint::provider::Provider<ElemView>::Transfer(
    this Provider& self, seq_endpoint::provider::Tag, DstStorageView dst,
    ptrdiff_t dst_elem_stride, size_t cnt) {
    size_t transfer_elem_cnt{ comparison_utils::BasicMin(self.elem_cnt, cnt) };

    utils::DisjointLinSeqTransfer(dst, self.data, dst_elem_stride,
                                  self.elem_stride, transfer_elem_cnt);

    self.data.ptr = utils::PtrInc(
        self.data.ptr,
        self.elem_stride * static_cast<ptrdiff_t>(transfer_elem_cnt));

    self.elem_cnt -= transfer_elem_cnt;

    return transfer_elem_cnt;
}

}  // namespace zeta::core
