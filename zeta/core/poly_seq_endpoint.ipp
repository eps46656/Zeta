#pragma once

#include <zeta/core/poly_seq_endpoint.hpp>

namespace zeta::core {

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> TargetAcceptor>
constexpr poly_seq_endpoint::acceptor::Acceptor<Elem>::Acceptor(
    TargetAcceptor&& target_acceptor) {
    this->Set(target_acceptor);
}

template <meta::IsObjectType Elem>
constexpr void poly_seq_endpoint::acceptor::Acceptor<Elem>::Set(
    this Acceptor& self, Acceptor const& other_acceptor) {
    self = other_acceptor;
}

template <meta::IsObjectType Elem>
template <seq_endpoint::acceptor::IsAcceptor<Elem> TargetAcceptor>
constexpr void poly_seq_endpoint::acceptor::Acceptor<Elem>::Set(
    this Acceptor& self, TargetAcceptor&& target_acceptor) {
    self.target_acceptor = &target_acceptor;

    self.transfer.destructing_obj_to_mem =
        [](void* target_acceptor, lifecycle::DestructingObjToMemView<Elem> src,
           ptrdiff_t src_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::acceptor::Transfer(
            *static_cast<meta::RemoveReferenceFromType<TargetAcceptor>*>(
                target_acceptor),
            src, src_elem_stride, cnt);
    };

    self.transfer.left_obj =
        [](void* target_acceptor, lifecycle::LeftObjView<Elem> src,
           ptrdiff_t src_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::acceptor::Transfer(
            *static_cast<meta::RemoveReferenceFromType<TargetAcceptor>*>(
                target_acceptor),
            src, src_elem_stride, cnt);
    };

    self.transfer.const_left_obj =
        [](void* target_acceptor, lifecycle::ConstLeftObjView<Elem> src,
           ptrdiff_t src_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::acceptor::Transfer(
            *static_cast<meta::RemoveReferenceFromType<TargetAcceptor>*>(
                target_acceptor),
            src, src_elem_stride, cnt);
    };

    self.transfer.right_obj =
        [](void* target_acceptor, lifecycle::RightObjView<Elem> src,
           ptrdiff_t src_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::acceptor::Transfer(
            *static_cast<meta::RemoveReferenceFromType<TargetAcceptor>*>(
                target_acceptor),
            src, src_elem_stride, cnt);
    };

    self.transfer.const_right_obj =
        [](void* target_acceptor, lifecycle::ConstRightObjView<Elem> src,
           ptrdiff_t src_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::acceptor::Transfer(
            *static_cast<meta::RemoveReferenceFromType<TargetAcceptor>*>(
                target_acceptor),
            src, src_elem_stride, cnt);
    };
}

template <meta::IsObjectType Elem>
template <lifecycle::IsSrcStorageView<Elem> SrcStorageView>
constexpr size_t poly_seq_endpoint::acceptor::Acceptor<Elem>::Transfer(
    this Acceptor& self, seq_endpoint::acceptor::Tag, SrcStorageView src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    if constexpr (SrcStorageView::semantics ==
                  lifecycle::StorageSemantics::DestructingObjToMem) {
        return self.transfer.destructing_obj_to_mem(self.target_acceptor, src,
                                                    src_elem_stride, cnt);
    } else if constexpr (SrcStorageView::semantics ==
                         lifecycle::StorageSemantics::LeftObj) {
        return self.transfer.left_obj(self.target_acceptor, src,
                                      src_elem_stride, cnt);
    } else if constexpr (SrcStorageView::semantics ==
                         lifecycle::StorageSemantics::ConstLeftObj) {
        return self.transfer.const_left_obj(self.target_acceptor, src,
                                            src_elem_stride, cnt);
    } else if constexpr (SrcStorageView::semantics ==
                         lifecycle::StorageSemantics::RightObj) {
        return self.transfer.right_obj(self.target_acceptor, src,
                                       src_elem_stride, cnt);
    } else if constexpr (SrcStorageView::semantics ==
                         lifecycle::StorageSemantics::ConstRightObj) {
        return self.transfer.const_right_obj(self.target_acceptor, src,
                                             src_elem_stride, cnt);
    } else {
        static_assert(false);
    }
}

// -----------------------------------------------------------------------------

template <meta::IsObjectType Elem>
template <seq_endpoint::provider::IsProvider<Elem> TargetProvider>
constexpr poly_seq_endpoint::provider::Provider<Elem>::Provider(
    TargetProvider&& target_provider) {
    this->Set(target_provider);
}

template <meta::IsObjectType Elem>
constexpr void poly_seq_endpoint::provider::Provider<Elem>::Set(
    this Provider& self, Provider const& other_provider) {
    self = other_provider;
}

template <meta::IsObjectType Elem>
template <seq_endpoint::provider::IsProvider<Elem> TargetProvider>
constexpr void poly_seq_endpoint::provider::Provider<Elem>::Set(
    this Provider& self, TargetProvider&& target_provider) {
    self.target_provider = &target_provider;

    self.transfer.constructing_mem_to_obj =
        [](void* target_provider, lifecycle::ConstructingMemToObjView<Elem> dst,
           ptrdiff_t dst_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::provider::Transfer(
            *static_cast<meta::RemoveReferenceFromType<TargetProvider>*>(
                target_provider),
            dst, dst_elem_stride, cnt);
    };

    self.transfer.left_obj =
        [](void* target_provider, lifecycle::LeftObjView<Elem> dst,
           ptrdiff_t dst_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::provider::Transfer(
            *static_cast<meta::RemoveReferenceFromType<TargetProvider>*>(
                target_provider),
            dst, dst_elem_stride, cnt);
    };

    self.transfer.right_obj =
        [](void* target_provider, lifecycle::RightObjView<Elem> dst,
           ptrdiff_t dst_elem_stride, size_t cnt) -> size_t {
        return seq_endpoint::provider::Transfer(
            *static_cast<meta::RemoveReferenceFromType<TargetProvider>*>(
                target_provider),
            dst, dst_elem_stride, cnt);
    };
}

template <meta::IsObjectType Elem>
template <lifecycle::IsDstStorageView<Elem> DstStorageView>
constexpr size_t poly_seq_endpoint::provider::Provider<Elem>::Transfer(
    this Provider& self, seq_endpoint::provider::Tag, DstStorageView dst,
    ptrdiff_t dst_elem_stride, size_t cnt) {
    if constexpr (DstStorageView::semantics ==
                  lifecycle::StorageSemantics::ConstructingMemToObj) {
        return self.transfer.constructing_mem_to_obj(self.target_provider, dst,
                                                     dst_elem_stride, cnt);
    } else if constexpr (DstStorageView::semantics ==
                         lifecycle::StorageSemantics::LeftObj) {
        return self.transfer.left_obj(self.target_provider, dst,
                                      dst_elem_stride, cnt);
    } else if constexpr (DstStorageView::semantics ==
                         lifecycle::StorageSemantics::RightObj) {
        return self.transfer.right_obj(self.target_provider, dst,
                                       dst_elem_stride, cnt);
    } else {
        static_assert(false);
    }
}

static_assert(seq_endpoint::acceptor::IsAcceptor<
              poly_seq_endpoint::acceptor::Acceptor<int>, int>);

static_assert(seq_endpoint::provider::IsProvider<
              poly_seq_endpoint::provider::Provider<int>, int>);

}  // namespace zeta::core
