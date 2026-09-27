#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::seq_endpoint {

enum struct Type {
    Acceptor = 0b01,
    Provider = 0b10,
    AcceptorProvider = 0b11,
};

namespace acceptor {

using Tag = meta::AutoValueWrapper<Type::Acceptor>;

template <typename Acceptor>
concept IsAcceptor = requires(Acceptor& inst, Tag tag, void const* data,
                              size_t elem_size, ptrdiff_t elem_stride,
                              size_t cnt) {
    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.GetElemSize(tag))>,
                          size_t>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.Transfer(
                              tag, data, elem_size, elem_stride, cnt))>,
                          size_t>;
};

template <typename Acceptor>
constexpr size_t GetElemSize(Acceptor&& inst);

template <typename Acceptor>
constexpr bool IsEnd(Acceptor&& inst);

template <typename Acceptor>
constexpr size_t Transfer(Acceptor&& inst, void const* src,
                          size_t src_elem_size, ptrdiff_t src_elem_stride,
                          size_t cnt);

struct EmptyAcceptor {
    static constexpr bool IsEnd(Tag);

    static constexpr size_t GetElemSize(Tag);

    static constexpr size_t Transfer(Tag, void const* src, size_t src_elem_size,
                                     ptrdiff_t src_elem_stride, size_t cnt);
};

template <typename Acceptor>
concept IsEmptyAcceptor =
    meta::IsSame<meta::RemoveCVRef<Acceptor>, EmptyAcceptor>;

using ArchetypeAcceptor = EmptyAcceptor;

}  // namespace acceptor

namespace provider {

using Tag = meta::AutoValueWrapper<Type::Provider>;

template <typename Provider>
concept IsProvider = requires(Provider& inst, Tag tag, void* data,
                              size_t elem_size, ptrdiff_t elem_stride,
                              size_t cnt) {
    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.Transfer(
                              tag, data, elem_size, elem_stride, cnt))>,
                          size_t>;
};

template <typename Provider>
constexpr size_t GetElemSize(Provider&& inst);

template <typename Provider>
constexpr bool IsEnd(Provider&& inst);

template <typename Provider>
constexpr size_t Transfer(Provider&& inst, void* dst, size_t dst_elem_size,
                          ptrdiff_t dst_elem_stride, size_t cnt);

struct EmptyProvider {
    static constexpr size_t GetElemSize(Tag);

    static constexpr bool IsEnd(Tag);

    static constexpr size_t Transfer(Tag, void* dst, size_t dst_elem_size,
                                     ptrdiff_t dst_elem_stride, size_t cnt);
};

template <typename Provider>
concept IsEmptyProvider =
    meta::IsSame<meta::RemoveCVRef<Provider>, EmptyProvider>;

using ArchetypeProvider = EmptyProvider;

}  // namespace provider

namespace acceptor_provider {

using Tag = meta::AutoValueWrapper<Type::AcceptorProvider>;

template <typename AcceptorProvider>
concept IsAcceptorProvider = requires(AcceptorProvider& inst, Tag tag,
                                      void* data, size_t elem_size,
                                      ptrdiff_t elem_stride, size_t cnt) {
    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.Transfer(
                              tag, data, elem_size, elem_stride, cnt))>,
                          size_t>;
};

template <typename AcceptorProvider>
constexpr size_t GetElemSize(AcceptorProvider&& inst);

template <typename AcceptorProvider>
constexpr bool IsEnd(AcceptorProvider&& inst);

template <typename AcceptorProvider>
constexpr size_t Transfer(AcceptorProvider&& inst, void* dst,
                          size_t dst_elem_size, ptrdiff_t dst_elem_stride,
                          size_t cnt);

struct EmptyAcceptorProvider {
    static constexpr size_t GetElemSize(Tag);

    static constexpr bool IsEnd(Tag);

    static constexpr size_t Transfer(Tag, void* dst, size_t dst_elem_size,
                                     ptrdiff_t dst_elem_stride, size_t cnt);
};

template <typename AcceptorProvider>
concept IsEmptyAcceptorProvider =
    meta::IsSame<meta::RemoveCVRef<AcceptorProvider>, EmptyAcceptorProvider>;

using ArchetypeAcceptorProvider = EmptyAcceptorProvider;

}  // namespace acceptor_provider

}  // namespace zeta::core::seq_endpoint
