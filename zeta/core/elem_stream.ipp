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

template <typename Provider>
constexpr size_t
elem_stream::provider::MemberFuncProviderTraitsAdapter<Provider>::GetElemSize(
    Provider const& provider) {
    return provider.GetElemSize();
}

template <typename Provider>
constexpr bool
elem_stream::provider::MemberFuncProviderTraitsAdapter<Provider>::IsEnd(
    Provider const& provider) {
    return provider.IsEnd();
}

template <typename Provider>
constexpr size_t
elem_stream::provider::MemberFuncProviderTraitsAdapter<Provider>::Transfer(
    Provider& provider, void* dst, size_t dst_elem_size, size_t dst_elem_stride,
    size_t cnt) {
    return provider.Transfer(dst, dst_elem_size, dst_elem_stride, cnt);
}

constexpr size_t elem_stream::provider::EmptyProvider::GetElemSize() {
    return 0;
}

constexpr bool elem_stream::provider::EmptyProvider::IsEnd() { return true; }

constexpr size_t elem_stream::provider::EmptyProvider::Transfer(void*, size_t,
                                                                size_t,
                                                                size_t) {
    return 0;
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

template <typename Acceptor>
constexpr size_t
elem_stream::acceptor::MemberFuncAcceptorTraitsAdapter<Acceptor>::GetElemSize(
    Acceptor const& acceptor) {
    return acceptor.GetElemSize();
}

template <typename Acceptor>
constexpr bool
elem_stream::acceptor::MemberFuncAcceptorTraitsAdapter<Acceptor>::IsEnd(
    Acceptor const& acceptor) {
    return acceptor.IsEnd();
}

template <typename Acceptor>
constexpr size_t
elem_stream::acceptor::MemberFuncAcceptorTraitsAdapter<Acceptor>::Transfer(
    Acceptor& acceptor, void const* src, size_t src_elem_size,
    size_t src_elem_stride, size_t cnt) {
    return acceptor.Transfer(src, src_elem_size, src_elem_stride, cnt);
}

constexpr size_t elem_stream::acceptor::EmptyAcceptor::GetElemSize() {
    return 0;
}

constexpr bool elem_stream::acceptor::EmptyAcceptor::IsEnd() { return true; }

constexpr size_t elem_stream::acceptor::EmptyAcceptor::Transfer(void const*,
                                                                size_t, size_t,
                                                                size_t) {
    return 0;
}

}  // namespace zeta::core
