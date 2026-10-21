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
concept IsAcceptor = requires(Acceptor& inst, Tag tag, Elem* data,
                              ptrdiff_t elem_stride, size_t cnt) {
    requires meta::IsNonConstObj<Elem>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

#pragma push_macro("TransferCase")
#define TransferCase(storage_semantics_name)                                \
    requires meta::IsSame<                                                  \
        meta::RemoveCVRef<decltype(inst.Transfer(                           \
            tag,                                                            \
            lifecycle::StorageView<                                         \
                lifecycle::StorageSemantics::storage_semantics_name, Elem>{ \
                data },                                                     \
            elem_stride, cnt))>,                                            \
        size_t>

    TransferCase(DestructingObjToMem);
    TransferCase(LeftObj);
    TransferCase(ConstLeftObj);
    TransferCase(RightObj);
    TransferCase(ConstRightObj);

#pragma pop_macro("TransferCase")
};

template <typename Acceptor>
constexpr decltype(auto) IsEnd(Acceptor&& inst);

template <typename Acceptor, lifecycle::IsSrcStorageView SrcStorageView>
constexpr decltype(auto) Transfer(Acceptor&& inst, SrcStorageView src,
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
    meta::IsSame<meta::RemoveCVRef<Acceptor>, BasicAcceptor>;

using ArchetypeAcceptor = BasicAcceptor;

}  // namespace acceptor

namespace provider {

using Tag = meta::AutoValueWrapper<Kind::Provider>;

template <typename Provider, typename Elem>
concept IsProvider = requires(Provider& inst, Tag tag, Elem* data,
                              Elem* const_data, ptrdiff_t elem_stride,
                              size_t cnt) {
    requires meta::IsObj<Elem>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(inst.IsEnd(tag))>, bool>;

#pragma push_macro("TransferCase")
#define TransferCase(storage_semantics_name)                                \
    requires meta::IsSame<                                                  \
        meta::RemoveCVRef<decltype(inst.Transfer(                           \
            tag,                                                            \
            lifecycle::StorageView<                                         \
                lifecycle::StorageSemantics::storage_semantics_name, Elem>{ \
                data },                                                     \
            elem_stride, cnt))>,                                            \
        size_t>

    TransferCase(ConstructingMemToObj);
    TransferCase(LeftObj);
    TransferCase(RightObj);

#pragma pop_macro("TransferCase")
};

template <typename Provider>
constexpr decltype(auto) IsEnd(Provider&& inst);

template <typename Provider, lifecycle::IsDstStorageView DstStorageView>
constexpr decltype(auto) Transfer(Provider&& inst, DstStorageView dst,
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
    meta::IsSame<meta::RemoveCVRef<Provider>, BasicProvider>;

using ArchetypeProvider = BasicProvider;

}  // namespace provider

template <typename SeqEndpoint, typename Elem>
concept IsEndpoint = acceptor::IsAcceptor<SeqEndpoint, Elem> ||
                     provider::IsProvider<SeqEndpoint, Elem>;

}  // namespace zeta::core::seq_endpoint
