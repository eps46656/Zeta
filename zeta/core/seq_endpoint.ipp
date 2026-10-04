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

template <typename Acceptor, typename SrcElem>
constexpr decltype(auto) acceptor::Transfer(
    Acceptor&& inst, lifecycle::DataTransferSemantics src_transfer_semantics,
    SrcElem* src, ptrdiff_t src_elem_stride, size_t cnt) {
    return inst.Transfer(Tag{}, src_transfer_semantics, src, src_elem_stride,
                         cnt);
}

template <typename Acceptor, typename SrcElem>
constexpr decltype(auto) acceptor::Transfer(
    Acceptor&& inst, lifecycle::DataTransferSemantics src_transfer_semantics,
    SrcElem const* src, ptrdiff_t src_elem_stride, size_t cnt) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        src_transfer_semantics == lifecycle::DataTransferSemantics::Copy);

    return inst.Transfer(Tag{}, src_transfer_semantics, src, src_elem_stride,
                         cnt);
}

constexpr bool acceptor::BasicAcceptor::IsEnd(Tag) { return false; }

template <typename SrcElem>
constexpr size_t acceptor::BasicAcceptor::Transfer(
    Tag, lifecycle::DataTransferSemantics src_transfer_semantics, SrcElem* src,
    ptrdiff_t src_elem_stride, size_t cnt) {
    lin_seq_utils::Check(src, src_elem_stride, cnt);

    if (cnt == 0) { return 0; }

    switch (src_transfer_semantics) {
    case lifecycle::DataTransferSemantics::Copy:
    case lifecycle::DataTransferSemantics::Move: return cnt;
    case lifecycle::DataTransferSemantics::Reloc: break;
    }

    for (size_t i{ src_elem_stride == 0 ? 1 : cnt }; 0 < i--;
         src = utils::PtrInc(src, src_elem_stride)) {
        lifecycle::InvokeDestructor(*src);
    }

    return cnt;
}

// -----------------------------------------------------------------------------

template <typename Provider>
constexpr decltype(auto) provider::IsEnd(Provider&& inst) {
    return inst.IsEnd(Tag{});
}

template <typename Provider, typename DstElem>
constexpr decltype(auto) provider::Transfer(
    Provider&& inst, lifecycle::DataLifeState dst_life_state, DstElem* dst,
    ptrdiff_t dst_elem_stride, size_t cnt) {
    return inst.Transfer(Tag{}, dst_life_state, dst, dst_elem_stride, cnt);
}

constexpr bool provider::BasicProvider::IsEnd(Tag) { return false; }

template <typename DstElem>
constexpr size_t provider::BasicProvider::Transfer(
    Tag, lifecycle::DataLifeState dst_life_state, DstElem* dst,
    ptrdiff_t dst_elem_stride, size_t cnt) {
    lin_seq_utils::Check(dst, dst_elem_stride, cnt);

    if (cnt == 0) { return 0; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(dst_life_state ==
                                            lifecycle::DataLifeState::Mem);

    switch (dst_life_state) {
    case lifecycle::DataLifeState::Obj: return cnt;
    case lifecycle::DataLifeState::Mem: break;
    }

    for (size_t i{ dst_elem_stride == 0 ? 1 : cnt }; 0 < i--;
         dst = utils::PtrInc(dst, dst_elem_stride)) {
        lifecycle::InvokeDestructor(*dst);
    }

    return cnt;
}

}  // namespace zeta::core::seq_endpoint
