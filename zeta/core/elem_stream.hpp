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
                                       IsEnd(provider))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(ProviderTraits<meta::RemoveCVRef<Provider>>::Transfer(
                provider, data, elem_size, elem_stride, cnt))>,
        size_t>;
};

template <typename Provider>
constexpr size_t GetElemSize(Provider&& provider);

template <typename Provider>
constexpr bool IsEnd(Provider&& provider);

template <typename Provider>
constexpr size_t Transfer(Provider&& provider, void* dst, size_t dst_elem_size,
                          size_t dst_elem_stride, size_t cnt);

template <typename Provider>
struct MemberFuncProviderTraitsAdapter {
    static constexpr size_t GetElemSize(Provider const& provider);

    static constexpr bool IsEnd(Provider const& provider);

    static constexpr size_t Transfer(Provider& provider, void* dst,
                                     size_t dst_elem_size,
                                     size_t dst_elem_stride, size_t cnt);
};

struct EmptyProvider {
    static constexpr size_t GetElemSize();

    static constexpr bool IsEnd();

    static constexpr size_t Transfer(void* dst, size_t dst_elem_size,
                                     size_t dst_elem_stride, size_t cnt);
};

template <>
struct ProviderTraits<EmptyProvider>
    : public MemberFuncProviderTraitsAdapter<EmptyProvider> {};

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
                                       IsEnd(acceptor))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(AcceptorTraits<meta::RemoveCVRef<Acceptor>>::Transfer(
                acceptor, data, elem_size, elem_stride, cnt))>,
        size_t>;
};

template <typename Acceptor>
constexpr size_t GetElemSize(Acceptor&& acceptor);

template <typename Acceptor>
constexpr bool IsEnd(Acceptor&& acceptor);

template <typename Acceptor>
constexpr size_t Transfer(Acceptor&& acceptor, void const* src,
                          size_t src_elem_size, size_t src_elem_stride,
                          size_t cnt);

template <typename Acceptor>
struct MemberFuncAcceptorTraitsAdapter {
    static constexpr bool IsEnd(Acceptor const& acceptor);

    static constexpr size_t GetElemSize(Acceptor const& acceptor);

    static constexpr size_t Transfer(Acceptor& acceptor, void const* src,
                                     size_t src_elem_size,
                                     size_t src_elem_stride, size_t cnt);
};

struct EmptyAcceptor {
    static constexpr bool IsEnd();

    static constexpr size_t GetElemSize();

    static constexpr size_t Transfer(void const* src, size_t src_elem_size,
                                     size_t src_elem_stride, size_t cnt);
};

template <>
struct AcceptorTraits<EmptyAcceptor>
    : MemberFuncAcceptorTraitsAdapter<EmptyAcceptor> {};

using ArchetAcceptor = EmptyAcceptor;

}  // namespace acceptor

}  // namespace zeta::core::elem_stream
