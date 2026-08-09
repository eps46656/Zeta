#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/function_ref.hpp>

namespace zeta::core::fn_elem_stream {

struct Acceptor {
    void* inner_acceptor;

    size_t elem_size;

    size_t (*transfer)(void* inner_acceptor, void const* src,
                       size_t src_elem_size, size_t src_elem_stride,
                       size_t cnt);

    constexpr Acceptor() = default;

    constexpr Acceptor(Acceptor const&) = default;

    constexpr Acceptor(Acceptor&&) = default;

    template <elem_stream::acceptor::IsAcceptor InnerAcceptor>
    constexpr Acceptor(InnerAcceptor&& inner_acceptor);

    constexpr size_t GetElemSize(this Acceptor const& acceptor);

    constexpr size_t Transfer(this Acceptor& acceptor, void const* src,
                              size_t src_elem_size, size_t src_elem_stride,
                              size_t cnt);
};

struct Provider {
    void* inner_provider;

    size_t elem_size;

    size_t (*transfer)(void* inner_provider, void* dst, size_t dst_elem_size,
                       size_t dst_elem_stride, size_t cnt);

    constexpr Provider() = default;

    constexpr Provider(Provider const&) = default;

    constexpr Provider(Provider&&) = default;

    template <elem_stream::provider::IsProvider InnerProvider>
    constexpr Provider(InnerProvider&& inner_provider);

    constexpr size_t GetElemSize(this Provider const& provider);

    constexpr size_t Transfer(this Provider& provider, void* dst,
                              size_t dst_elem_size, size_t dst_elem_stride,
                              size_t cnt);
};

}  // namespace zeta::core::fn_elem_stream

template <>
struct zeta::core::elem_stream::provider::ProviderTraits<
    zeta::core::fn_elem_stream::Provider>
    : MemberFuncProviderTraitsAdapter<fn_elem_stream::Provider> {};

template <>
struct zeta::core::elem_stream::acceptor::AcceptorTraits<
    zeta::core::fn_elem_stream::Acceptor>
    : MemberFuncAcceptorTraitsAdapter<fn_elem_stream::Acceptor> {};
