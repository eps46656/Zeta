#pragma once

#include <zeta/core/fn_elem_stream.hpp>

namespace zeta::core {

template <elem_stream::acceptor::IsAcceptor InnserAcceptor>
constexpr fn_elem_stream::Acceptor::Acceptor(InnserAcceptor&& inner_acceptor)
    : inner_acceptor{ &inner_acceptor },
      elem_size{ elem_stream::acceptor::GetElemSize(inner_acceptor) },
      transfer{ [](void* inner_acceptor, void const* src, size_t src_elem_size,
                   size_t src_elem_stride, size_t cnt) -> size_t {
          return elem_stream::acceptor::Transfer(
              *static_cast<meta::RemoveRef<InnserAcceptor>*>(inner_acceptor),
              src, src_elem_size, src_elem_stride, cnt);
      } } {}

constexpr size_t fn_elem_stream::Acceptor::GetElemSize(
    this Acceptor const& acceptor) {
    return acceptor.elem_size;
}

constexpr size_t fn_elem_stream::Acceptor::Transfer(this Acceptor& acceptor,
                                                    void const* src,
                                                    size_t src_elem_size,
                                                    size_t src_elem_stride,
                                                    size_t cnt) {
    return acceptor.transfer(acceptor.inner_acceptor, src, src_elem_size,
                             src_elem_stride, cnt);
}

template <elem_stream::provider::IsProvider InnserProvider>
constexpr fn_elem_stream::Provider::Provider(InnserProvider&& inner_provider)
    : inner_provider{ &inner_provider },
      elem_size{ elem_stream::provider::GetElemSize(inner_provider) },
      transfer{ [](void* inner_provider, void* dst, size_t dst_elem_size,
                   size_t dst_elem_stride, size_t cnt) -> size_t {
          return elem_stream::provider::Transfer(
              *static_cast<meta::RemoveRef<InnserProvider>*>(inner_provider),
              dst, dst_elem_size, dst_elem_stride, cnt);
      } } {}

constexpr size_t fn_elem_stream::Provider::GetElemSize(
    this Provider const& provider) {
    return provider.elem_size;
}

constexpr size_t fn_elem_stream::Provider::Transfer(this Provider& provider,
                                                    void* dst,
                                                    size_t dst_elem_size,
                                                    size_t dst_elem_stride,
                                                    size_t cnt) {
    return provider.transfer(provider.inner_provider, dst, dst_elem_size,
                             dst_elem_stride, cnt);
}

}  // namespace zeta::core
