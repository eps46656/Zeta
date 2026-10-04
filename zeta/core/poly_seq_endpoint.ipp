#pragma once

#include <zeta/core/poly_seq_endpoint.hpp>

namespace zeta::core::poly_seq_endpoint {

template <meta::IsContainerElem Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> TargetAcceptor>
constexpr acceptor::Acceptor<Elem>::Acceptor(TargetAcceptor&& target_acceptor) {
    this->Set(target_acceptor);
}

template <meta::IsContainerElem Elem>
constexpr void acceptor::Acceptor<Elem>::Set(this Acceptor& self,
                                             Acceptor const& other_acceptor) {
    self = other_acceptor;
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> TargetAcceptor>
constexpr void acceptor::Acceptor<Elem>::Set(this Acceptor& self,
                                             TargetAcceptor&& target_acceptor) {
    self.target_acceptor = &target_acceptor;

    self.transfer = [](void* target_acceptor,
                       lifecycle::DataTransferSemantics src_transfer_semantics,
                       void* src, ptrdiff_t src_elem_stride,
                       size_t cnt) -> size_t {
        return seq_endpoint::acceptor::Transfer(
            *static_cast<meta::RemoveRef<TargetAcceptor>*>(target_acceptor),
            src_transfer_semantics, static_cast<Elem*>(src), src_elem_stride,
            cnt);
    };

    self.const_transfer =
        [](void* target_acceptor,
           lifecycle::DataTransferSemantics src_transfer_semantics,
           void const* src, ptrdiff_t src_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::acceptor::Transfer(
            *static_cast<meta::RemoveRef<TargetAcceptor>*>(target_acceptor),
            src_transfer_semantics, static_cast<Elem const*>(src),
            src_elem_stride, cnt);
    };
}

template <meta::IsContainerElem Elem>
constexpr size_t acceptor::Acceptor<Elem>::Transfer(
    this Acceptor& self, seq_endpoint::acceptor::Tag,
    lifecycle::DataTransferSemantics src_transfer_semantics, Elem* src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    return self.transfer(self.target_acceptor, src_transfer_semantics, src,
                         src_elem_stride, cnt);
}

template <meta::IsContainerElem Elem>
constexpr size_t acceptor::Acceptor<Elem>::Transfer(
    this Acceptor& self, seq_endpoint::acceptor::Tag,
    lifecycle::DataTransferSemantics src_transfer_semantics, Elem const* src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    return self.const_transfer(self.target_acceptor, src_transfer_semantics,
                               src, src_elem_stride, cnt);
}

// -----------------------------------------------------------------------------

template <meta::IsContainerElem Elem>
template <seq_endpoint::provider::IsProvider<Elem> TargetProvider>
constexpr provider::Provider<Elem>::Provider(TargetProvider&& target_provider) {
    this->Set(target_provider);
}

template <meta::IsContainerElem Elem>
constexpr void provider::Provider<Elem>::Set(this Provider& self,
                                             Provider const& other_provider) {
    self = other_provider;
}

template <meta::IsContainerElem Elem>
template <seq_endpoint::provider::IsProvider<Elem> TargetProvider>
constexpr void provider::Provider<Elem>::Set(this Provider& self,
                                             TargetProvider&& target_provider) {
    self.target_provider = &target_provider;

    self.transfer = [](void* target_provider,
                       lifecycle::DataLifeState dst_life_state, void* dst,
                       ptrdiff_t dst_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::provider::Transfer(
            *static_cast<meta::RemoveRef<TargetProvider>*>(target_provider),
            dst_life_state, static_cast<Elem*>(dst), dst_elem_stride, cnt);
    };
}

template <meta::IsContainerElem Elem>
constexpr size_t provider::Provider<Elem>::Transfer(
    this Provider& self, seq_endpoint::provider::Tag,
    lifecycle::DataLifeState dst_life_state, Elem* dst,
    ptrdiff_t dst_elem_stride, size_t cnt) {
    return self.transfer(self.target_provider, dst_life_state, dst,
                         dst_elem_stride, cnt);
}

}  // namespace zeta::core::poly_seq_endpoint
