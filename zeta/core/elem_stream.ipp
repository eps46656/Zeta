#pragma once

#include <zeta/core/elem_stream.hpp>

namespace zeta::core {

template <typename Provider>
constexpr size_t elem_stream::provider::GetElemSize(Provider&& provider) {
    return provider.GetElemSize(Tag{});
}

template <typename Provider>
constexpr bool elem_stream::provider::IsEnd(Provider&& provider) {
    return provider.IsEnd(Tag{});
}

template <typename Provider>
constexpr size_t elem_stream::provider::Transfer(Provider&& provider, void* dst,
                                                 size_t dst_elem_size,
                                                 ptrdiff_t dst_elem_stride,
                                                 size_t cnt) {
    return provider.Transfer(Tag{}, dst, dst_elem_size, dst_elem_stride, cnt);
}

constexpr size_t elem_stream::provider::EmptyProvider::GetElemSize(Tag) {
    return 0;
}

constexpr bool elem_stream::provider::EmptyProvider::IsEnd(Tag) { return true; }

constexpr size_t elem_stream::provider::EmptyProvider::Transfer(Tag, void*,
                                                                size_t,
                                                                ptrdiff_t,
                                                                size_t) {
    return 0;
}

template <typename Acceptor>
constexpr size_t elem_stream::acceptor::GetElemSize(Acceptor&& acceptor) {
    return acceptor.GetElemSize(Tag{});
}

template <typename Acceptor>
constexpr bool elem_stream::acceptor::IsEnd(Acceptor&& acceptor) {
    return acceptor.IsEnd(Tag{});
}

template <typename Acceptor>
constexpr size_t elem_stream::acceptor::Transfer(Acceptor&& acceptor,
                                                 void const* src,
                                                 size_t src_elem_size,
                                                 ptrdiff_t src_elem_stride,
                                                 size_t cnt) {
    return acceptor.Transfer(Tag{}, src, src_elem_size, src_elem_stride, cnt);
}

constexpr size_t elem_stream::acceptor::EmptyAcceptor::GetElemSize(Tag) {
    return 0;
}

constexpr bool elem_stream::acceptor::EmptyAcceptor::IsEnd(Tag) { return true; }

constexpr size_t elem_stream::acceptor::EmptyAcceptor::Transfer(
    Tag, void const*, size_t, ptrdiff_t, size_t) {
    return 0;
}

}  // namespace zeta::core
