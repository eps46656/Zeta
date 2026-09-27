#pragma once

#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::poly_seq_endpoint {

namespace acceptor {

struct Acceptor {
    void* target_acceptor;

    size_t elem_size;

    size_t (*transfer)(void* target_acceptor, void const* src,
                       size_t src_elem_size, ptrdiff_t src_elem_stride,
                       size_t cnt);

    constexpr Acceptor() = default;

    constexpr Acceptor(Acceptor const&) = default;

    template <seq_endpoint::acceptor::IsAcceptor TargetAcceptor>
    constexpr Acceptor(TargetAcceptor&& target_acceptor);

    constexpr Acceptor& operator=(Acceptor const&) = default;

    constexpr void Set(this Acceptor& self, Acceptor const& other_acceptor);

    template <seq_endpoint::acceptor::IsAcceptor TargetAcceptor>
    constexpr void Set(this Acceptor& self, TargetAcceptor&& target_acceptor);

    constexpr bool IsEnd(this Acceptor const& self,
                         seq_endpoint::acceptor::Tag);

    constexpr size_t GetElemSize(this Acceptor const& self,
                                 seq_endpoint::acceptor::Tag);

    constexpr size_t Transfer(this Acceptor& self, seq_endpoint::acceptor::Tag,
                              void const* src, size_t src_elem_size,
                              ptrdiff_t src_elem_stride, size_t cnt);
};

}  // namespace acceptor

namespace provider {

struct Provider {
    void* target_provider;

    size_t elem_size;

    size_t (*transfer)(void* target_provider, void* dst, size_t dst_elem_size,
                       ptrdiff_t dst_elem_stride, size_t cnt);

    constexpr Provider() = default;

    constexpr Provider(Provider const&) = default;

    template <seq_endpoint::provider::IsProvider TargetProvider>
    constexpr Provider(TargetProvider&& target_provider);

    constexpr Provider& operator=(Provider const&) = default;

    constexpr void Set(this Provider& self, Provider const& other_provider);

    template <seq_endpoint::provider::IsProvider TargetProvider>
    constexpr void Set(this Provider& self, TargetProvider&& target_provider);

    constexpr bool IsEnd(this Provider const& self,
                         seq_endpoint::provider::Tag);

    constexpr size_t GetElemSize(this Provider const& self,
                                 seq_endpoint::provider::Tag);

    constexpr size_t Transfer(this Provider& self, seq_endpoint::provider::Tag,
                              void* dst, size_t dst_elem_size,
                              ptrdiff_t dst_elem_stride, size_t cnt);
};

}  // namespace provider

namespace acceptor_provider {

struct AcceptorProvider {
    void* target_acceptor_provider;

    size_t elem_size;

    size_t (*transfer)(void* target_acceptor_provider, void* dst,
                       size_t dst_elem_size, ptrdiff_t dst_elem_stride,
                       size_t cnt);

    constexpr AcceptorProvider() = default;

    constexpr AcceptorProvider(AcceptorProvider const&) = default;

    template <seq_endpoint::provider::IsProvider TargetProvider>
    constexpr AcceptorProvider(TargetProvider&& target_acceptor_provider);

    constexpr AcceptorProvider& operator=(AcceptorProvider const&) = default;

    constexpr void Set(this AcceptorProvider& self,
                       AcceptorProvider const& other_acceptor_provider);

    template <seq_endpoint::provider::IsProvider TargetProvider>
    constexpr void Set(this AcceptorProvider& self,
                       TargetProvider&& target_acceptor_provider);

    constexpr bool IsEnd(this AcceptorProvider const& self,
                         seq_endpoint::acceptor_provider::Tag);

    constexpr size_t GetElemSize(this AcceptorProvider const& self,
                                 seq_endpoint::acceptor_provider::Tag);

    constexpr size_t Transfer(this AcceptorProvider& self,
                              seq_endpoint::acceptor_provider::Tag, void* dst,
                              size_t dst_elem_size, ptrdiff_t dst_elem_stride,
                              size_t cnt);
};

}  // namespace acceptor_provider

}  // namespace zeta::core::poly_seq_endpoint
