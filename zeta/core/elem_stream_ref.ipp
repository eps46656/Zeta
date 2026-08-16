#pragma once

#include <zeta/core/elem_stream_ref.hpp>

namespace zeta::core {

template <elem_stream::provider::IsProvider TargetProvider>
constexpr elem_stream_ref::provider::Provider::Provider(
    TargetProvider&& target_provider) {
    this->Set(target_provider);
}

constexpr void elem_stream_ref::provider::Provider::Init(
    this Provider& provider, Provider const& other_provider) {
    provider = other_provider;
}

template <elem_stream::provider::IsProvider TargetProvider>
constexpr void elem_stream_ref::provider::Provider::Init(
    this Provider& provider, TargetProvider&& target_provider) {
    provider.Set(target_provider);
}

constexpr void elem_stream_ref::provider::Provider::Set(
    this Provider& provider, Provider const& other_provider) {
    provider = other_provider;
}

template <elem_stream::provider::IsProvider TargetProvider>
constexpr void elem_stream_ref::provider::Provider::Set(
    this Provider& provider, TargetProvider&& target_provider) {
    provider.target_provider = &target_provider;

    provider.elem_size = elem_stream::provider::GetElemSize(target_provider);

    provider.transfer = [](void* target_provider, void* dst,
                           size_t dst_elem_size, size_t dst_elem_stride,
                           size_t cnt) -> size_t {
        return elem_stream::provider::Transfer(
            *static_cast<meta::RemoveRef<TargetProvider>*>(target_provider),
            dst, dst_elem_size, dst_elem_stride, cnt);
    };
}

constexpr size_t elem_stream_ref::provider::Provider::GetElemSize(
    this Provider const& provider) {
    return provider.elem_size;
}

constexpr size_t elem_stream_ref::provider::Provider::Transfer(
    this Provider& provider, void* dst, size_t dst_elem_size,
    size_t dst_elem_stride, size_t cnt) {
    return provider.transfer(provider.target_provider, dst, dst_elem_size,
                             dst_elem_stride, cnt);
}

template <elem_stream::acceptor::IsAcceptor TargetAcceptor>
constexpr elem_stream_ref::acceptor::Acceptor::Acceptor(
    TargetAcceptor&& target_acceptor) {
    this->Set(target_acceptor);
}

constexpr void elem_stream_ref::acceptor::Acceptor::Init(
    this Acceptor& acceptor, Acceptor const& other_acceptor) {
    acceptor = other_acceptor;
}

template <elem_stream::acceptor::IsAcceptor TargetAcceptor>
constexpr void elem_stream_ref::acceptor::Acceptor::Init(
    this Acceptor& acceptor, TargetAcceptor&& target_acceptor) {
    acceptor.Set(target_acceptor);
}

constexpr void elem_stream_ref::acceptor::Acceptor::Set(
    this Acceptor& acceptor, Acceptor const& other_acceptor) {
    acceptor = other_acceptor;
}

template <elem_stream::acceptor::IsAcceptor TargetAcceptor>
constexpr void elem_stream_ref::acceptor::Acceptor::Set(
    this Acceptor& acceptor, TargetAcceptor&& target_acceptor) {
    acceptor.target_acceptor = &target_acceptor;

    acceptor.elem_size = elem_stream::acceptor::GetElemSize(target_acceptor);

    acceptor.transfer = [](void* target_acceptor, void const* src,
                           size_t src_elem_size, size_t src_elem_stride,
                           size_t cnt) -> size_t {
        return elem_stream::acceptor::Transfer(
            *static_cast<meta::RemoveRef<TargetAcceptor>*>(target_acceptor),
            src, src_elem_size, src_elem_stride, cnt);
    };
}

constexpr size_t elem_stream_ref::acceptor::Acceptor::GetElemSize(
    this Acceptor const& acceptor) {
    return acceptor.elem_size;
}

constexpr size_t elem_stream_ref::acceptor::Acceptor::Transfer(
    this Acceptor& acceptor, void const* src, size_t src_elem_size,
    size_t src_elem_stride, size_t cnt) {
    return acceptor.transfer(acceptor.target_acceptor, src, src_elem_size,
                             src_elem_stride, cnt);
}

}  // namespace zeta::core
