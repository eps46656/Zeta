#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/resolving.hpp>

namespace zeta::core {

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr resolving::Packet<RefObj, TransferObj>
resolving::Packet<RefObj, TransferObj>::MakeNull() {
    return {
        .intent{ Intent::Null },
        .state{
            .resolved{ false },
            .has_resolved_ref{ false },
            .has_resolved_transfer{ false },
        },
        .dst_ref{
            lifecycle::StorageSemantics::Null,
            nullptr,
        },
        .dst_transfer{
            lifecycle::StorageSemantics::Null,
            nullptr,
        },
    };
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr resolving::Packet<RefObj, TransferObj>
resolving::Packet<RefObj, TransferObj>::MakeRef() {
    return {
        .intent{ Intent::Ref },
        .state{
            .resolved{ false },
            .has_resolved_ref{ false },
            .has_resolved_transfer{ false },
        },
        .dst_ref{
            lifecycle::StorageSemantics::Null,
            nullptr,
        },
        .dst_transfer{
            lifecycle::StorageSemantics::Null,
            nullptr,
        },
    };
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr resolving::Packet<RefObj, TransferObj>
resolving::Packet<RefObj, TransferObj>::MakeRefAndTransfer(
    lifecycle::DynamicStorageView<TransferObj> const& dst_transfer) {
    return {
        .intent{ Intent::RefAndTransfer },
        .state{
            .resolved{ false },
            .has_resolved_ref{ false },
            .has_resolved_transfer{ false },
        },
        .dst_ref{
            lifecycle::StorageSemantics::Null,
            nullptr,
        },
        .dst_transfer{ dst_transfer },
    };
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr resolving::Packet<RefObj, TransferObj>
resolving::Packet<RefObj, TransferObj>::MakeRefElseTransfer(
    lifecycle::DynamicStorageView<TransferObj> const& dst_transfer) {
    return {
        .intent{ Intent::RefElseTransfer },
        .state{
            .resolved{ false },
            .has_resolved_ref{ false },
            .has_resolved_transfer{ false },
        },
        .dst_ref{
            lifecycle::StorageSemantics::Null,
            nullptr,
        },
        .dst_transfer{ dst_transfer },
    };
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr void resolving::Packet<RefObj, TransferObj>::AssertIsValid(
    this Packet const& self) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        self.intent == Intent::Null || self.intent == Intent::Ref ||
        self.intent == Intent::RefAndTransfer ||
        self.intent == Intent::RefElseTransfer);

    if (self.intent == Intent::RefAndTransfer ||
        self.intent == Intent::RefElseTransfer) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(!self.dst_transfer.IsNull());
    }

    if (!self.state.resolved) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(!self.state.has_resolved_ref);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !self.state.has_resolved_transfer);
        return;
    }

    switch (self.intent) {
    case Intent::Null:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(!self.state.has_resolved_ref);
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !self.state.has_resolved_transfer);
        break;
    case Intent::Ref:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !self.state.has_resolved_transfer);
        break;
    case Intent::RefAndTransfer: break;
    case Intent::RefElseTransfer:
        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            !self.state.has_resolved_ref || !self.state.has_resolved_transfer);
        break;
    default: ZETA_Core_DebugUtils_Diag_Unreachable();
    }

    if (self.state.has_resolved_ref) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(!self.dst_ref.IsNull());
    }

    if (self.state.has_resolved_transfer) {
        ZETA_Core_DebugUtils_Diag_PromiseAssert(!self.dst_transfer.IsNull());
    }
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr void
resolving::Packet<RefObj, TransferObj>::AssertResultIsNotResolved(
    this Packet const& self) {
    self.AssertIsValid();

    ZETA_Core_DebugUtils_Diag_PromiseAssert(!self.state.resolved);
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr void resolving::Packet<RefObj, TransferObj>::AssertResultIsResolved(
    this Packet const& self) {
    self.AssertIsValid();

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.state.resolved);
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr void resolving::Packet<RefObj, TransferObj>::AssertResultHasRef(
    this Packet const& self) {
    self.AssertIsValid();

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.state.has_resolved_ref);
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr void resolving::Packet<RefObj, TransferObj>::AssertResultHasTransfer(
    this Packet const& self) {
    self.AssertIsValid();

    ZETA_Core_DebugUtils_Diag_PromiseAssert(self.state.has_resolved_transfer);
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr void resolving::Packet<RefObj, TransferObj>::ClearResolved(
    this Packet& self) {
    self.AssertIsValid();

    self.state.resolved              = false;
    self.state.has_resolved_ref      = false;
    self.state.has_resolved_transfer = false;
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
constexpr void resolving::Packet<RefObj, TransferObj>::SetResolved(
    this Packet& self, bool has_resolved_ref, bool has_resolved_transfer) {
    self.AssertResultIsNotResolved();

    self.state.resolved              = true;
    self.state.has_resolved_ref      = has_resolved_ref;
    self.state.has_resolved_transfer = has_resolved_transfer;
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
template <lifecycle::IsSrcStorageView SrcStorageView, bool can_resolve_ref,
          bool can_resolve_transfer>
constexpr void resolving::Packet<RefObj, TransferObj>::ResolveFromView(
    this Packet& self, SrcStorageView src_view,
    meta::ValueWrapper<bool, can_resolve_ref>,
    meta::ValueWrapper<bool, can_resolve_transfer>) {
    self.AssertResultIsNotResolved();

    self.state.resolved = true;

    if constexpr (src_view.IsNull()) {
        return;
    } else {
        if (self.intent == Intent::Null) { return; }

        if constexpr (can_resolve_ref) {
            self.state.has_resolved_ref = true;
            self.dst_ref                = src_view;

            switch (self.intent) {
            case Intent::Ref:
            case Intent::RefElseTransfer: return;

            case Intent::RefAndTransfer: break;

            case Intent::Null:
            default: ZETA_Core_DebugUtils_Diag_Unreachable();
            }
        } else if (self.intent == Intent::Ref) {
            return;
        }

        if constexpr (can_resolve_transfer) {
            self.state.has_resolved_transfer = true;
            lifecycle::StorageTransfer(self.dst_transfer, src_view);
        }
    }
}

template <meta::IsObjectType RefObj, meta::IsObjectType TransferObj>
template <meta::IsNonConstObjectType Obj, bool can_resolve_ref,
          bool can_resolve_transfer>
constexpr void resolving::Packet<RefObj, TransferObj>::ResolveFromView(
    this Packet& self, lifecycle::DynamicStorageView<Obj> src_view,
    meta::ValueWrapper<bool, can_resolve_ref>,
    meta::ValueWrapper<bool, can_resolve_transfer>) {
    self.AssertResultIsNotResolved();

    self.state.resolved = true;

    if (src_view.IsNull() || self.intent == Intent::Null) { return; }

    if constexpr (can_resolve_ref) {
        self.state.has_resolved_ref = true;
        self.dst_ref                = src_view;

        switch (self.intent) {
        case Intent::Ref:
        case Intent::RefElseTransfer: return;

        case Intent::RefAndTransfer: break;

        case Intent::Null:
        default: ZETA_Core_DebugUtils_Diag_Unreachable();
        }
    } else if (self.intent == Intent::Ref) {
        return;
    }

    if constexpr (can_resolve_transfer) {
        self.state.has_resolved_transfer = true;
        lifecycle::StorageTransfer(self.dst_transfer, src_view);
    }
}

}  // namespace zeta::core
