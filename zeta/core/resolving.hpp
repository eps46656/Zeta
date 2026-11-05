#pragma once

#include <zeta/core/lifecycle.hpp>

namespace zeta::core::resolving {

enum struct Intent : unsigned char {
    Null,
    Ref,
    RefAndTransfer,
    RefElseTransfer,
};

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
struct Packet {
    Intent intent;

    struct {
        bool resolved : 1;
        bool has_resolved_ref : 1;
        bool has_resolved_transfer : 1;
    } state;

    lifecycle::DynamicStorageView<RefObj> dst_ref;
    lifecycle::DynamicStorageView<TransferObj> dst_transfer;

    static constexpr Packet MakeNull();

    static constexpr Packet MakeRef();

    static constexpr Packet MakeRefAndTransfer(
        lifecycle::DynamicStorageView<TransferObj> const& dst_transfer_view);

    static constexpr Packet MakeRefElseTransfer(
        lifecycle::DynamicStorageView<TransferObj> const& dst_transfer_view);

    constexpr void AssertIsValid(this Packet const& self);

    constexpr void AssertResultIsNotResolved(this Packet const& self);
    constexpr void AssertResultIsResolved(this Packet const& self);
    constexpr void AssertResultHasRef(this Packet const& self);
    constexpr void AssertResultHasTransfer(this Packet const& self);

    constexpr void ClearResolved(this Packet& self);
    constexpr void SetResolved(this Packet& self, bool has_resolved_ref,
                               bool has_resolved_transfer);

    template <lifecycle::IsSrcStorageView SrcStorageView, bool can_resolve_ref,
              bool can_resolve_transfer>
    constexpr void ResolveFromView(
        this Packet& self, SrcStorageView src_view,
        meta::ValueWrapper<bool, can_resolve_ref>,
        meta::ValueWrapper<bool, can_resolve_transfer>);

    template <meta::IsNonConstObjectType Obj, bool can_resolve_ref,
              bool can_resolve_transfer>
    constexpr void ResolveFromView(
        this Packet& self, lifecycle::DynamicStorageView<Obj> src_view,
        meta::ValueWrapper<bool, can_resolve_ref>,
        meta::ValueWrapper<bool, can_resolve_transfer>);
};

}  // namespace zeta::core::resolving
