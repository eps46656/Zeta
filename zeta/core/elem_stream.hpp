#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::elem_stream {

namespace provider {

template <typename Provider>
struct ProviderTraits;

template <typename Provider>
concept IsProvider = requires(Provider& provider, void* data, size_t elem_size,
                              size_t elem_stride, size_t cnt) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ProviderTraits<meta::RemoveCVRef<Provider>>::
                                       IsEnd(provider))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ProviderTraits<meta::RemoveCVRef<Provider>>::
                                       GetElemSize(provider))>,
        size_t>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(ProviderTraits<meta::RemoveCVRef<Provider>>::Transfer(
                provider, data, elem_size, elem_stride, cnt))>,
        size_t>;
};

template <typename Provider>
struct DefaultProviderTraits {
    static constexpr bool IsEnd(Provider const& provider) {
        return provider.IsEnd();
    }

    static constexpr size_t GetElemSize(Provider const& provider) {
        return provider.GetElemSize();
    }

    static constexpr size_t Transfer(Provider& provider, void* dst,
                                     size_t dst_elem_size,
                                     size_t dst_elem_stride, size_t cnt) {
        return provider.Transfer(dst, dst_elem_size, dst_elem_stride, cnt);
    }
};

template <typename Provider>
constexpr size_t GetElemSize(Provider&& provider);

template <typename Provider>
constexpr bool IsEnd(Provider&& provider);

template <typename Provider>
constexpr size_t Transfer(Provider&& provider, void* dst, size_t dst_elem_size,
                          size_t dst_elem_stride, size_t cnt);

struct EmptyProvider {
    constexpr bool IsEnd(this EmptyProvider const&) { return true; }

    constexpr size_t GetElemSize(this EmptyProvider const&) { return 0; }

    constexpr size_t Transfer(this EmptyProvider&, void*, size_t, size_t,
                              size_t) {
        return 0;
    }
};

template <>
struct ProviderTraits<EmptyProvider>
    : public DefaultProviderTraits<EmptyProvider> {};

using ArchetProvider = EmptyProvider;

}  // namespace provider

namespace acceptor {

template <typename Acceptor>
struct AcceptorTraits;

template <typename Acceptor>
concept IsAcceptor = requires(Acceptor& acceptor, void const* data,
                              size_t elem_size, size_t elem_stride,
                              size_t cnt) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(AcceptorTraits<meta::RemoveCVRef<Acceptor>>::
                                       GetElemSize(acceptor))>,
        size_t>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(AcceptorTraits<meta::RemoveCVRef<Acceptor>>::
                                       GetElemSize(acceptor))>,
        size_t>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(AcceptorTraits<meta::RemoveCVRef<Acceptor>>::Transfer(
                acceptor, data, elem_size, elem_stride, cnt))>,
        size_t>;
};

template <typename Acceptor>
struct DefaultAcceptorTraits {
    static constexpr size_t IsEnd(Acceptor const& acceptor) {
        return acceptor.IsEnd();
    }

    static constexpr size_t GetElemSize(Acceptor const& acceptor) {
        return acceptor.GetElemSize();
    }

    static constexpr size_t Transfer(Acceptor& acceptor, void const* src,
                                     size_t src_elem_size,
                                     size_t src_elem_stride, size_t cnt) {
        return acceptor.Transfer(src, src_elem_size, src_elem_stride, cnt);
    }
};

template <typename Acceptor>
constexpr size_t GetElemSize(Acceptor&& acceptor);

template <typename Acceptor>
constexpr bool IsEnd(Acceptor&& acceptor);

template <typename Acceptor>
constexpr size_t Transfer(Acceptor&& acceptor, void const* src,
                          size_t src_elem_size, size_t src_elem_stride,
                          size_t cnt);

struct EmptyAcceptor {
    constexpr bool IsEnd(this EmptyAcceptor const&) { return true; }

    constexpr size_t GetElemSize(this EmptyAcceptor const&) { return 0; }

    constexpr size_t Transfer(this EmptyAcceptor&, void const*, size_t, size_t,
                              size_t) {
        return 0;
    }
};

template <>
struct AcceptorTraits<EmptyAcceptor> : DefaultAcceptorTraits<EmptyAcceptor> {};

using ArchetAcceptor = EmptyAcceptor;

}  // namespace acceptor

}  // namespace zeta::core::elem_stream
