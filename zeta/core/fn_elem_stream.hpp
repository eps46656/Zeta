#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/function_ref.hpp>

namespace zeta::core::fn_elem_stream {

struct Acceptor {
    size_t elem_size;
    function_ref::Ref<void(void const* src, size_t src_stride, size_t cnt)>
        transfer;

    Acceptor() = default;

    Acceptor(Acceptor const&) = default;

    Acceptor(Acceptor&&) = default;

    template <typename InnerAcceptor>
    Acceptor(InnerAcceptor&&);

    constexpr size_t GetElemSize() const;

    constexpr void Transfer(void const* src, size_t src_stride, size_t cnt);
};

struct Provider {
    size_t elem_size;
    function_ref::Ref<void(void* dst, size_t dst_stride, size_t cnt)> transfer;

    Provider() = default;

    Provider(Provider const&) = default;

    Provider(Provider&&) = default;

    template <typename InnerProvider>
    Provider(InnerProvider&&);

    constexpr size_t GetElemSize() const;

    constexpr void Transfer(void* dst, size_t dst_stride, size_t cnt);
};

}  // namespace zeta::core::fn_elem_stream

template <>
struct zeta::core::elem_stream::ProviderTraits<
    zeta::core::fn_elem_stream::Provider>
    : zeta::core::elem_stream::DefaultProviderTraits<
          zeta::core::fn_elem_stream::Provider> {};

template <>
struct zeta::core::elem_stream::AcceptorTraits<
    zeta::core::fn_elem_stream::Acceptor>
    : zeta::core::elem_stream::DefaultAcceptorTraits<
          zeta::core::fn_elem_stream::Acceptor> {};
