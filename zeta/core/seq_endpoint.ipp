#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/lin_seq_utils.ipp>
#include <zeta/core/seq_endpoint.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core::seq_endpoint {

template <typename Acceptor>
constexpr decltype(auto) acceptor::IsEnd(Acceptor&& inst) {
    return inst.IsEnd(Tag{});
}

template <typename Acceptor, lifecycle::IsSrcStorageView SrcStorageView>
constexpr decltype(auto) acceptor::Transfer(Acceptor&& inst, SrcStorageView src,
                                            ptrdiff_t src_elem_stride,
                                            size_t cnt) {
    return inst.Transfer(Tag{}, src, src_elem_stride, cnt);
}

constexpr bool acceptor::EmptyAcceptor::IsEnd(Tag) { return false; }

template <lifecycle::IsSrcStorageView SrcStorageView>
constexpr size_t acceptor::EmptyAcceptor::Transfer(Tag, SrcStorageView src,
                                                   ptrdiff_t src_elem_stride,
                                                   size_t cnt) {
    lin_seq_utils::Check(src.ptr, src_elem_stride, cnt);
    return cnt;
}

constexpr bool acceptor::BasicAcceptor::IsEnd(Tag) { return false; }

template <lifecycle::IsSrcStorageView SrcStorageView>
constexpr size_t acceptor::BasicAcceptor::Transfer(Tag, SrcStorageView src,
                                                   ptrdiff_t src_elem_stride,
                                                   size_t cnt) {
    lin_seq_utils::Check(src.ptr, src_elem_stride, cnt);

    if constexpr (src.semantics ==
                  lifecycle::StorageSemantics::DestructingObjToMem) {
        if (0 < cnt) {
            for (size_t i{ src_elem_stride == 0 ? 1 : cnt }; 0 < i--;
                 src.ptr = utils::PtrInc(src.ptr, src_elem_stride)) {
                lifecycle::InvokeDestructor(*src.ptr);
            }
        }
    }

    return cnt;
}

// -----------------------------------------------------------------------------

template <typename Provider>
constexpr decltype(auto) provider::IsEnd(Provider&& inst) {
    return inst.IsEnd(Tag{});
}

template <typename Provider, lifecycle::IsDstStorageView DstStorageView>
constexpr decltype(auto) provider::Transfer(Provider&& inst, DstStorageView dst,
                                            ptrdiff_t dst_elem_stride,
                                            size_t cnt) {
    return inst.Transfer(Tag{}, dst, dst_elem_stride, cnt);
}

constexpr bool provider::EmptyProvider::IsEnd(Tag) { return false; }

template <lifecycle::IsDstStorageView DstStorageView>
constexpr size_t provider::EmptyProvider::Transfer(Tag, DstStorageView dst,
                                                   ptrdiff_t dst_elem_stride,
                                                   size_t cnt) {
    lin_seq_utils::Check(dst.ptr, dst_elem_stride, cnt);
    return cnt;
}

constexpr bool provider::BasicProvider::IsEnd(Tag) { return false; }

template <lifecycle::IsDstStorageView DstStorageView>
constexpr size_t provider::BasicProvider::Transfer(Tag, DstStorageView dst,
                                                   ptrdiff_t dst_elem_stride,
                                                   size_t cnt) {
    lin_seq_utils::Check(dst.ptr, dst_elem_stride, cnt);

    if constexpr (dst.semantics ==
                  lifecycle::StorageSemantics::ConstructingMemToObj) {
        if (0 < cnt) {
            for (size_t i{ dst_elem_stride == 0 ? 1 : cnt }; 0 < i--;
                 dst.ptr = utils::PtrInc(dst.ptr, dst_elem_stride)) {
                lifecycle::InvokeConstructor(dst.ptr);
            }
        }
    }

    return cnt;
}

}  // namespace zeta::core::seq_endpoint
