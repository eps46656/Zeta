#pragma once

#include <zeta/core/poly_seq_endpoint.hpp>

namespace zeta::core::poly_seq_endpoint {

template <seq_endpoint::acceptor::IsAcceptor TargetAcceptor>
constexpr acceptor::Acceptor::Acceptor(TargetAcceptor&& target_acceptor) {
    this->Set(target_acceptor);
}

constexpr void acceptor::Acceptor::Set(this Acceptor& self,
                                       Acceptor const& other_acceptor) {
    self = other_acceptor;
}

template <seq_endpoint::acceptor::IsAcceptor TargetAcceptor>
constexpr void acceptor::Acceptor::Set(this Acceptor& self,
                                       TargetAcceptor&& target_acceptor) {
    self.target_acceptor = &target_acceptor;

    self.elem_size = seq_endpoint::acceptor::GetElemSize(target_acceptor);

    self.transfer = [](void* target_acceptor, void const* src,
                       size_t src_elem_size, ptrdiff_t src_elem_stride,
                       size_t cnt) -> size_t {
        return seq_endpoint::acceptor::Transfer(
            *static_cast<meta::RemoveRef<TargetAcceptor>*>(target_acceptor),
            src, src_elem_size, src_elem_stride, cnt);
    };
}

constexpr size_t acceptor::Acceptor::GetElemSize(this Acceptor const& self,
                                                 seq_endpoint::acceptor::Tag) {
    return self.elem_size;
}

constexpr size_t acceptor::Acceptor::Transfer(
    this Acceptor& self, seq_endpoint::acceptor::Tag, void const* src,
    size_t src_elem_size, ptrdiff_t src_elem_stride, size_t cnt) {
    return self.transfer(self.target_acceptor, src, src_elem_size,
                         src_elem_stride, cnt);
}

// -----------------------------------------------------------------------------

template <seq_endpoint::provider::IsProvider TargetProvider>
constexpr provider::Provider::Provider(TargetProvider&& target_provider) {
    this->Set(target_provider);
}

constexpr void provider::Provider::Set(this Provider& self,
                                       Provider const& other_provider) {
    self = other_provider;
}

template <seq_endpoint::provider::IsProvider TargetProvider>
constexpr void provider::Provider::Set(this Provider& self,
                                       TargetProvider&& target_provider) {
    self.target_provider = &target_provider;

    self.elem_size = seq_endpoint::provider::GetElemSize(target_provider);

    self.transfer = [](void* target_provider, void* dst, size_t dst_elem_size,
                       ptrdiff_t dst_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::provider::Transfer(
            *static_cast<meta::RemoveRef<TargetProvider>*>(target_provider),
            dst, dst_elem_size, dst_elem_stride, cnt);
    };
}

constexpr size_t provider::Provider::GetElemSize(this Provider const& self,
                                                 seq_endpoint::provider::Tag) {
    return self.elem_size;
}

constexpr size_t provider::Provider::Transfer(this Provider& self,
                                              seq_endpoint::provider::Tag,
                                              void* dst, size_t dst_elem_size,
                                              ptrdiff_t dst_elem_stride,
                                              size_t cnt) {
    return self.transfer(self.target_provider, dst, dst_elem_size,
                         dst_elem_stride, cnt);
}

// -----------------------------------------------------------------------------

template <seq_endpoint::provider::IsProvider TargetAcceptorProvider>
constexpr acceptor_provider::AcceptorProvider::AcceptorProvider(
    TargetAcceptorProvider&& target_acceptor_provider) {
    this->Set(target_acceptor_provider);
}

constexpr void acceptor_provider::AcceptorProvider::Set(
    this AcceptorProvider& self,
    AcceptorProvider const& other_acceptor_provider) {
    self = other_acceptor_provider;
}

template <seq_endpoint::provider::IsProvider TargetAcceptorProvider>
constexpr void acceptor_provider::AcceptorProvider::Set(
    this AcceptorProvider& self, TargetAcceptorProvider&& target_provider) {
    self.target_acceptor_provider = &target_provider;

    self.elem_size = seq_endpoint::provider::GetElemSize(target_provider);

    self.transfer = [](void* target_acceptor_provider, void* dst,
                       size_t dst_elem_size, ptrdiff_t dst_elem_stride,
                       size_t cnt) -> size_t {
        return seq_endpoint::provider::Transfer(
            *static_cast<meta::RemoveRef<TargetAcceptorProvider>*>(
                target_acceptor_provider),
            dst, dst_elem_size, dst_elem_stride, cnt);
    };
}

constexpr size_t acceptor_provider::AcceptorProvider::GetElemSize(
    this AcceptorProvider const& self, seq_endpoint::acceptor_provider::Tag) {
    return self.elem_size;
}

constexpr size_t acceptor_provider::AcceptorProvider::Transfer(
    this AcceptorProvider& self, seq_endpoint::acceptor_provider::Tag,
    void* dst, size_t dst_elem_size, ptrdiff_t dst_elem_stride, size_t cnt) {
    return self.transfer(self.target_acceptor_provider, dst, dst_elem_size,
                         dst_elem_stride, cnt);
}

}  // namespace zeta::core::poly_seq_endpoint
