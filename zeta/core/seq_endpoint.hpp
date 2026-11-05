#pragma once

#include <zeta/core/lifecycle.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::seq_endpoint {

enum struct Kind : unsigned char {
    Acceptor = 0b01,
    Provider = 0b10,
};

namespace acceptor {

using Tag = meta::AutoValueWrapper<Kind::Acceptor>;

template <typename Acceptor, typename Elem>
concept IsAcceptor_ =
    requires(Acceptor& acceptor, Tag tag, ptrdiff_t elem_stride, size_t cnt) {
        requires meta::IsNonConstObjectType<Acceptor>;

        requires meta::IsNonConstObjectType<Elem>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(acceptor.IsEnd(tag))>,
            bool>;

#pragma push_macro("TransferCase")
#define TransferCase(storage_semantics_name)                                \
    requires meta::IsSame<                                                  \
        meta::RemoveConstVolatileReference<decltype(acceptor.Transfer(      \
            tag,                                                            \
            lifecycle::StorageView<                                         \
                lifecycle::StorageSemantics::storage_semantics_name, Elem>{ \
                nullptr },                                                  \
            elem_stride, cnt))>,                                            \
        size_t>

        TransferCase(DestructingObjToMem);
        TransferCase(LeftObj);
        TransferCase(ConstLeftObj);
        TransferCase(RightObj);
        TransferCase(ConstRightObj);

#pragma pop_macro("TransferCase")
    };

template <typename Acceptor, typename Elem>
concept IsAcceptor = IsAcceptor_<meta::RemoveReferenceFromType<Acceptor>, Elem>;

template <typename Acceptor, typename Elem>
concept IsAcceptorLike = IsAcceptor<meta::RemovePointer<Acceptor>, Elem>;

template <typename Acceptor>
constexpr decltype(auto) IsEnd(Acceptor&& acceptor);

template <typename Acceptor, lifecycle::IsSrcStorageView SrcStorageView>
constexpr decltype(auto) Transfer(Acceptor&& acceptor, SrcStorageView src,
                                  ptrdiff_t src_elem_stride, size_t cnt);

struct EmptyAcceptor {
    static constexpr bool IsEnd(Tag);

    template <lifecycle::IsSrcStorageView SrcStorageView>
    static constexpr size_t Transfer(Tag, SrcStorageView src,
                                     ptrdiff_t src_elem_stride, size_t cnt);
};

struct BasicAcceptor {
    static constexpr bool IsEnd(Tag);

    template <lifecycle::IsSrcStorageView SrcStorageView>
    static constexpr size_t Transfer(Tag, SrcStorageView src,
                                     ptrdiff_t src_elem_stride, size_t cnt);
};

template <typename Acceptor>
concept IsBasicAcceptor =
    meta::IsSame<meta::RemoveConstVolatileReference<Acceptor>, BasicAcceptor>;

using ArchetypeAcceptor = BasicAcceptor;

}  // namespace acceptor

namespace provider {

using Tag = meta::AutoValueWrapper<Kind::Provider>;

template <typename Provider, typename Elem>
concept IsProvider_ =
    requires(Provider& provider, Tag tag, ptrdiff_t elem_stride, size_t cnt) {
        requires meta::IsNonConstObjectType<Provider>;

        requires meta::IsObjectType<Elem>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(provider.IsEnd(tag))>,
            bool>;

#pragma push_macro("TransferCase")
#define TransferCase(storage_semantics_name)                                \
    requires meta::IsSame<                                                  \
        meta::RemoveConstVolatileReference<decltype(provider.Transfer(      \
            tag,                                                            \
            lifecycle::StorageView<                                         \
                lifecycle::StorageSemantics::storage_semantics_name, Elem>{ \
                nullptr },                                                  \
            elem_stride, cnt))>,                                            \
        size_t>

        TransferCase(ConstructingMemToObj);
        TransferCase(LeftObj);
        TransferCase(RightObj);

#pragma pop_macro("TransferCase")
    };

template <typename Provider, typename Elem>
concept IsProvider = IsProvider_<meta::RemoveReferenceFromType<Provider>, Elem>;

template <typename Provider, typename Elem>
concept IsProviderLike = IsProvider<meta::RemovePointer<Provider>, Elem>;

template <typename Provider>
constexpr decltype(auto) IsEnd(Provider&& provider);

template <typename Provider, lifecycle::IsDstStorageView DstStorageView>
constexpr decltype(auto) Transfer(Provider&& provider, DstStorageView dst,
                                  ptrdiff_t dst_elem_stride, size_t cnt);

struct EmptyProvider {
    static constexpr bool IsEnd(Tag);

    template <lifecycle::IsDstStorageView DstStorageView>
    static constexpr size_t Transfer(Tag, DstStorageView, ptrdiff_t, size_t);
};

struct BasicProvider {
    static constexpr bool IsEnd(Tag);

    template <lifecycle::IsDstStorageView DstStorageView>
    static constexpr size_t Transfer(Tag, DstStorageView dst,
                                     ptrdiff_t dst_elem_stride, size_t cnt);
};

template <typename Provider>
concept IsBasicProvider =
    meta::IsSame<meta::RemoveConstVolatileReference<Provider>, BasicProvider>;

using ArchetypeProvider = BasicProvider;

}  // namespace provider

}  // namespace zeta::core::seq_endpoint
