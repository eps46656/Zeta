#pragma once

#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::poly_seq_endpoint {

namespace acceptor {

template <meta::IsContainerElem Elem>
struct Acceptor {
    void* target_acceptor;

    size_t (*transfer)(void* target_acceptor,
                       lifecycle::DataTransferSemantics src_transfer_semantics,
                       void* src, ptrdiff_t src_elem_stride, size_t cnt);

    size_t (*const_transfer)(
        void* target_acceptor,
        lifecycle::DataTransferSemantics src_transfer_semantics,
        void const* src, ptrdiff_t src_elem_stride, size_t cnt);

    constexpr Acceptor() = default;

    constexpr Acceptor(Acceptor const&) = default;

    template <seq_endpoint::acceptor::IsAcceptor<Elem> TargetAcceptor>
    constexpr Acceptor(TargetAcceptor&& target_acceptor);

    constexpr Acceptor& operator=(Acceptor const&) = default;

    constexpr void Set(this Acceptor& self, Acceptor const& other_acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> TargetAcceptor>
    constexpr void Set(this Acceptor& self, TargetAcceptor&& target_acceptor);

    constexpr bool IsEnd(this Acceptor const& self,
                         seq_endpoint::acceptor::Tag);

    constexpr size_t Transfer(
        this Acceptor& self, seq_endpoint::acceptor::Tag,
        lifecycle::DataTransferSemantics src_transfer_semantics, Elem* src,
        ptrdiff_t src_elem_stride, size_t cnt);

    constexpr size_t Transfer(
        this Acceptor& self, seq_endpoint::acceptor::Tag,
        lifecycle::DataTransferSemantics src_transfer_semantics,
        Elem const* src, ptrdiff_t src_elem_stride, size_t cnt);
};

}  // namespace acceptor

namespace provider {

template <meta::IsContainerElem Elem>
struct Provider {
    void* target_provider;

    size_t (*transfer)(void* target_provider,
                       lifecycle::DataLifeState dst_life_state, void* dst,
                       ptrdiff_t dst_elem_stride, size_t cnt);

    constexpr Provider() = default;

    constexpr Provider(Provider const&) = default;

    template <seq_endpoint::provider::IsProvider<Elem> TargetProvider>
    constexpr Provider(TargetProvider&& target_provider);

    constexpr Provider& operator=(Provider const&) = default;

    constexpr void Set(this Provider& self, Provider const& other_provider);

    template <seq_endpoint::provider::IsProvider<Elem> TargetProvider>
    constexpr void Set(this Provider& self, TargetProvider&& target_provider);

    constexpr bool IsEnd(this Provider const& self,
                         seq_endpoint::provider::Tag);

    constexpr size_t Transfer(this Provider& self, seq_endpoint::provider::Tag,
                              lifecycle::DataLifeState dst_life_state,
                              Elem* dst, ptrdiff_t dst_elem_stride, size_t cnt);
};

}  // namespace provider

}  // namespace zeta::core::poly_seq_endpoint
