#pragma once

#include <zeta/core/lifecycle.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::seq_endpoint {

enum struct Type : unsigned char {
    Acceptor = 0b01,
    Provider = 0b10,
};

namespace acceptor {

using Tag = meta::AutoValueWrapper<Type::Acceptor>;

template <typename Acceptor, typename Elem>
concept IsAcceptor = requires(
    Acceptor& inst, Tag tag,
    lifecycle::DataTransferSemantics transfer_semantics, Elem* data,
    ptrdiff_t elem_stride, size_t cnt) {
    requires meta::IsContainerElem<Elem>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

    requires meta::IsSame<

        meta::RemoveCVRef<decltype(inst.Transfer(tag, transfer_semantics, data,
                                                 elem_stride, cnt))>,
        size_t>;
};

template <typename Acceptor>
constexpr decltype(auto) IsEnd(Acceptor&& inst);

template <typename Acceptor, typename SrcElem>
constexpr decltype(auto) Transfer(
    Acceptor&& inst, lifecycle::DataTransferSemantics src_transfer_semantics,
    SrcElem* src, ptrdiff_t src_elem_stride, size_t cnt);

template <typename Acceptor, typename SrcElem>
constexpr decltype(auto) Transfer(
    Acceptor&& inst, lifecycle::DataTransferSemantics src_transfer_semantics,
    SrcElem const* src, ptrdiff_t src_elem_stride, size_t cnt);

struct BasicAcceptor {
    static constexpr bool IsEnd(Tag);

    template <typename SrcElem>
    static constexpr size_t Transfer(
        Tag, lifecycle::DataTransferSemantics src_transfer_semantics,
        SrcElem* src, ptrdiff_t src_elem_stride, size_t cnt);
};

template <typename Acceptor>
concept IsBasicAcceptor =
    meta::IsSame<meta::RemoveCVRef<Acceptor>, BasicAcceptor>;

using ArchetypeAcceptor = BasicAcceptor;

}  // namespace acceptor

namespace provider {

using Tag = meta::AutoValueWrapper<Type::Provider>;

template <typename Provider, typename Elem>
concept IsProvider = requires(Provider& inst, Tag tag,
                              lifecycle::DataLifeState life_state, Elem* data,
                              ptrdiff_t elem_stride, size_t cnt) {
    requires meta::IsContainerElem<Elem>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.Transfer(
                              tag, life_state, data, elem_stride, cnt))>,
                          size_t>;
};

template <typename Provider>
constexpr decltype(auto) IsEnd(Provider&& inst);

template <typename Provider, typename DstElem>
constexpr decltype(auto) Transfer(Provider&& inst,
                                  lifecycle::DataLifeState dst_life_state,
                                  DstElem* dst, ptrdiff_t dst_elem_stride,
                                  size_t cnt);

struct BasicProvider {
    static constexpr bool IsEnd(Tag);

    template <typename DstElem>
    static constexpr size_t Transfer(Tag,
                                     lifecycle::DataLifeState dst_life_state,
                                     DstElem* dst, ptrdiff_t dst_elem_stride,
                                     size_t cnt);
};

template <typename Provider>
concept IsBasicProvider =
    meta::IsSame<meta::RemoveCVRef<Provider>, BasicProvider>;

using ArchetypeProvider = BasicProvider;

}  // namespace provider

}  // namespace zeta::core::seq_endpoint
