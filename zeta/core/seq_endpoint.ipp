#pragma once

#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::seq_endpoint {

template <typename Acceptor>
constexpr size_t acceptor::GetElemSize(Acceptor&& inst) {
    return inst.GetElemSize(Tag{});
}

template <typename Acceptor>
constexpr bool acceptor::IsEnd(Acceptor&& inst) {
    return inst.IsEnd(Tag{});
}

template <typename Acceptor>
constexpr size_t acceptor::Transfer(Acceptor&& inst, void const* src,
                                    size_t src_elem_size,
                                    ptrdiff_t src_elem_stride, size_t cnt) {
    return inst.Transfer(Tag{}, src, src_elem_size, src_elem_stride, cnt);
}

constexpr size_t acceptor::EmptyAcceptor::GetElemSize(Tag) { return 0; }

constexpr bool acceptor::EmptyAcceptor::IsEnd(Tag) { return true; }

constexpr size_t acceptor::EmptyAcceptor::Transfer(Tag, void const*, size_t,
                                                   ptrdiff_t, size_t) {
    return 0;
}

// -----------------------------------------------------------------------------

template <typename Provider>
constexpr size_t provider::GetElemSize(Provider&& inst) {
    return inst.GetElemSize(Tag{});
}

template <typename Provider>
constexpr bool provider::IsEnd(Provider&& inst) {
    return inst.IsEnd(Tag{});
}

template <typename Provider>
constexpr size_t provider::Transfer(Provider&& inst, void* dst,
                                    size_t dst_elem_size,
                                    ptrdiff_t dst_elem_stride, size_t cnt) {
    return inst.Transfer(Tag{}, dst, dst_elem_size, dst_elem_stride, cnt);
}

constexpr size_t provider::EmptyProvider::GetElemSize(Tag) { return 0; }

constexpr bool provider::EmptyProvider::IsEnd(Tag) { return true; }

constexpr size_t provider::EmptyProvider::Transfer(Tag, void*, size_t,
                                                   ptrdiff_t, size_t) {
    return 0;
}

// -----------------------------------------------------------------------------

template <typename Provider>
constexpr size_t acceptor_provider::GetElemSize(Provider&& inst) {
    return inst.GetElemSize(Tag{});
}

template <typename Provider>
constexpr bool acceptor_provider::IsEnd(Provider&& inst) {
    return inst.IsEnd(Tag{});
}

template <typename Provider>
constexpr size_t acceptor_provider::Transfer(Provider&& inst, void* dst,
                                             size_t dst_elem_size,
                                             ptrdiff_t dst_elem_stride,
                                             size_t cnt) {
    return inst.Transfer(Tag{}, dst, dst_elem_size, dst_elem_stride, cnt);
}

constexpr size_t acceptor_provider::EmptyAcceptorProvider::GetElemSize(Tag) {
    return 0;
}

constexpr bool acceptor_provider::EmptyAcceptorProvider::IsEnd(Tag) {
    return true;
}

constexpr size_t acceptor_provider::EmptyAcceptorProvider::Transfer(Tag, void*,
                                                                    size_t,
                                                                    ptrdiff_t,
                                                                    size_t) {
    return 0;
}

}  // namespace zeta::core::seq_endpoint
