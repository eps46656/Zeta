#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::elem_stream {

namespace provider {

struct Tag {};

template <typename Provider>
concept IsProvider =
    requires(Provider& provider, Tag tag, void* data, size_t elem_size,
             ptrdiff_t elem_stride, size_t cnt) {
        requires meta::IsSame<meta::RemoveCVRef<decltype(provider.IsEnd(tag))>,
                              bool>;

        requires meta::IsSame<meta::RemoveCVRef<decltype(provider.IsEnd(tag))>,
                              bool>;

        requires meta::IsSame<meta::RemoveCVRef<decltype(provider.Transfer(
                                  tag, data, elem_size, elem_stride, cnt))>,
                              size_t>;
    };

template <typename Provider>
constexpr size_t GetElemSize(Provider&& provider);

template <typename Provider>
constexpr bool IsEnd(Provider&& provider);

template <typename Provider>
constexpr size_t Transfer(Provider&& provider, void* dst, size_t dst_elem_size,
                          ptrdiff_t dst_elem_stride, size_t cnt);

struct EmptyProvider {
    static constexpr size_t GetElemSize(Tag);

    static constexpr bool IsEnd(Tag);

    static constexpr size_t Transfer(Tag, void* dst, size_t dst_elem_size,
                                     ptrdiff_t dst_elem_stride, size_t cnt);
};

using ArchetypeProvider = EmptyProvider;

}  // namespace provider

namespace acceptor {

struct Tag {};

template <typename Acceptor>
concept IsAcceptor =
    requires(Acceptor& acceptor, Tag tag, void const* data, size_t elem_size,
             ptrdiff_t elem_stride, size_t cnt) {
        requires meta::IsSame<
            meta::RemoveCVRef<decltype(acceptor.GetElemSize(tag))>, size_t>;

        requires meta::IsSame<meta::RemoveCVRef<decltype(acceptor.IsEnd(tag))>,
                              bool>;

        requires meta::IsSame<meta::RemoveCVRef<decltype(acceptor.Transfer(
                                  tag, data, elem_size, elem_stride, cnt))>,
                              size_t>;
    };

template <typename Acceptor>
constexpr size_t GetElemSize(Acceptor&& acceptor);

template <typename Acceptor>
constexpr bool IsEnd(Acceptor&& acceptor);

template <typename Acceptor>
constexpr size_t Transfer(Acceptor&& acceptor, void const* src,
                          size_t src_elem_size, ptrdiff_t src_elem_stride,
                          size_t cnt);

struct EmptyAcceptor {
    static constexpr bool IsEnd(Tag);

    static constexpr size_t GetElemSize(Tag);

    static constexpr size_t Transfer(Tag, void const* src, size_t src_elem_size,
                                     ptrdiff_t src_elem_stride, size_t cnt);
};

using ArchetypeAcceptor = EmptyAcceptor;

}  // namespace acceptor

}  // namespace zeta::core::elem_stream
