#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/function_ref.hpp>

namespace zeta::core::elem_stream_ref {

namespace provider {

struct Provider {
    void* target_provider;

    size_t elem_size;

    size_t (*transfer)(void* target_provider, void* dst, size_t dst_elem_size,
                       size_t dst_elem_stride, size_t cnt);

    constexpr Provider() = default;

    constexpr Provider(Provider const&) = default;

    template <elem_stream::provider::IsProvider TargetProvider>
    constexpr Provider(TargetProvider&& target_provider);

    constexpr void Init(this Provider& provider,
                        Provider const& other_provider);

    template <elem_stream::provider::IsProvider TargetProvider>
    constexpr void Init(this Provider& provider,
                        TargetProvider&& target_provider);

    constexpr Provider& operator=(Provider const&) = default;

    constexpr void Set(this Provider& provider, Provider const& other_provider);

    template <elem_stream::provider::IsProvider TargetProvider>
    constexpr void Set(this Provider& provider,
                       TargetProvider&& target_provider);

    constexpr size_t GetElemSize(this Provider const& provider);

    constexpr size_t Transfer(this Provider& provider, void* dst,
                              size_t dst_elem_size, size_t dst_elem_stride,
                              size_t cnt);
};

}  // namespace provider

namespace acceptor {

struct Acceptor {
    void* target_acceptor;

    size_t elem_size;

    size_t (*transfer)(void* target_acceptor, void const* src,
                       size_t src_elem_size, size_t src_elem_stride,
                       size_t cnt);

    constexpr Acceptor() = default;

    constexpr Acceptor(Acceptor const&) = default;

    template <elem_stream::acceptor::IsAcceptor TargetAcceptor>
    constexpr Acceptor(TargetAcceptor&& target_acceptor);

    constexpr void Init(this Acceptor& acceptor,
                        Acceptor const& other_acceptor);

    template <elem_stream::acceptor::IsAcceptor TargetAcceptor>
    constexpr void Init(this Acceptor& acceptor,
                        TargetAcceptor&& target_acceptor);

    constexpr Acceptor& operator=(Acceptor const&) = default;

    constexpr void Set(this Acceptor& acceptor, Acceptor const& other_acceptor);

    template <elem_stream::acceptor::IsAcceptor TargetAcceptor>
    constexpr void Set(this Acceptor& acceptor,
                       TargetAcceptor&& target_acceptor);

    constexpr size_t GetElemSize(this Acceptor const& acceptor);

    constexpr size_t Transfer(this Acceptor& acceptor, void const* src,
                              size_t src_elem_size, size_t src_elem_stride,
                              size_t cnt);
};

}  // namespace acceptor

}  // namespace zeta::core::elem_stream_ref

template <>
struct zeta::core::elem_stream::provider::ProviderTraits<
    zeta::core::elem_stream_ref::provider::Provider>
    : MemberFuncProviderTraitsAdapter<elem_stream_ref::provider::Provider> {};

template <>
struct zeta::core::elem_stream::acceptor::AcceptorTraits<
    zeta::core::elem_stream_ref::acceptor::Acceptor>
    : MemberFuncAcceptorTraitsAdapter<elem_stream_ref::acceptor::Acceptor> {};
