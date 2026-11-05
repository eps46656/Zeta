#pragma once

#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::lin_seq_endpoint {

namespace acceptor {

template <lifecycle::IsDstStorageView ElemView>
struct Acceptor {
    using Elem = typename ElemView::Obj;

    ElemView data;
    ptrdiff_t elem_stride;
    size_t elem_cnt;

    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_endpoint::acceptor::Tag);

    constexpr bool IsEnd(this Acceptor const& self,
                         seq_endpoint::acceptor::Tag);

    template <lifecycle::IsSrcStorageView SrcStorageView>
    constexpr size_t Transfer(this Acceptor& self, seq_endpoint::acceptor::Tag,
                              SrcStorageView src, ptrdiff_t src_elem_stride,
                              size_t cnt);
};

}  // namespace acceptor

namespace provider {

template <lifecycle::IsSrcStorageView ElemView>
struct Provider {
    using Elem = typename ElemView::Obj;

    ElemView data;
    ptrdiff_t elem_stride;
    size_t elem_cnt;

    static constexpr meta::TypeWrapper<Elem> GetElemType(
        seq_endpoint::provider::Tag);

    constexpr bool IsEnd(this Provider const& self,
                         seq_endpoint::provider::Tag);

    template <lifecycle::IsDstStorageView DstStorageView>
    constexpr size_t Transfer(this Provider& self, seq_endpoint::provider::Tag,
                              DstStorageView dst, ptrdiff_t dst_elem_stride,
                              size_t cnt);
};

}  // namespace provider

}  // namespace zeta::core::lin_seq_endpoint
