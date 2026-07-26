#pragma once

#include <zeta/core/elem_stream.hpp>

namespace zeta::core {

template <typename Provider>
constexpr size_t elem_stream::provider::GetElemSize(Provider&& provider) {
    return ProviderTraits<meta::RemoveCVRef<Provider>>::GetElemSize(provider);
}

template <typename Provider>
constexpr bool elem_stream::provider::IsEnd(Provider&& provider) {
    return ProviderTraits<meta::RemoveCVRef<Provider>>::IsEnd(provider);
}

template <typename Provider>
constexpr size_t elem_stream::provider::Transfer(Provider&& provider, void* dst,
                                                 size_t dst_elem_size,
                                                 size_t dst_elem_stride,
                                                 size_t cnt) {
    return ProviderTraits<meta::RemoveCVRef<Provider>>::Transfer(
        provider, dst, dst_elem_size, dst_elem_stride, cnt);
}

template <typename Acceptor>
constexpr size_t elem_stream::acceptor::GetElemSize(Acceptor&& acceptor) {
    return AcceptorTraits<meta::RemoveCVRef<Acceptor>>::GetElemSize(acceptor);
}

template <typename Acceptor>
constexpr bool elem_stream::acceptor::IsEnd(Acceptor&& acceptor) {
    return AcceptorTraits<meta::RemoveCVRef<Acceptor>>::IsEnd(acceptor);
}

template <typename Acceptor>
constexpr size_t elem_stream::acceptor::Transfer(Acceptor&& acceptor,
                                                 void const* src,
                                                 size_t src_elem_size,
                                                 size_t src_elem_stride,
                                                 size_t cnt) {
    return AcceptorTraits<meta::RemoveCVRef<Acceptor>>::Transfer(
        acceptor, src, src_elem_size, src_elem_stride, cnt);
}

}  // namespace zeta::core
