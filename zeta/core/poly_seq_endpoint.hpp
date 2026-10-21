#pragma once

#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::poly_seq_endpoint {

namespace acceptor {

template <meta::IsObj Elem>
struct Acceptor {
    void* target_acceptor;

    struct {
        size_t (*destructing_obj_to_mem)(
            void* target_acceptor, lifecycle::DestructingObjToMemView<Elem> src,
            ptrdiff_t src_elem_stride, size_t cnt);

        size_t (*left_obj)(void* target_acceptor,
                           lifecycle::LeftObjView<Elem> src,
                           ptrdiff_t src_elem_stride, size_t cnt);

        size_t (*const_left_obj)(void* target_acceptor,
                                 lifecycle::ConstLeftObjView<Elem> src,
                                 ptrdiff_t src_elem_stride, size_t cnt);

        size_t (*right_obj)(void* target_acceptor,
                            lifecycle::RightObjView<Elem> src,
                            ptrdiff_t src_elem_stride, size_t cnt);

        size_t (*const_right_obj)(void* target_acceptor,
                                  lifecycle::ConstRightObjView<Elem> src,
                                  ptrdiff_t src_elem_stride, size_t cnt);
    } transfer;

    constexpr Acceptor() = default;

    constexpr Acceptor(Acceptor const&) = default;

    template <seq_endpoint::acceptor::IsAcceptor<Elem> TargetAcceptor>
    constexpr Acceptor(TargetAcceptor&& target_acceptor);

    constexpr Acceptor& operator=(Acceptor const&) = default;

    constexpr void Set(this Acceptor& self, Acceptor const& other_acceptor);

    template <seq_endpoint::acceptor::IsAcceptor<Elem> TargetAcceptor>
    constexpr void Set(this Acceptor& self, TargetAcceptor&& target_acceptor);

    constexpr bool IsEnd(this Acceptor const& self,
                         seq_endpoint::acceptor::Tag);

    template <lifecycle::IsSrcStorageView<Elem> SrcStorageView>
    constexpr size_t Transfer(this Acceptor& self, seq_endpoint::acceptor::Tag,
                              SrcStorageView src, ptrdiff_t src_elem_stride,
                              size_t cnt);
};

}  // namespace acceptor

namespace provider {

template <meta::IsObj Elem>
struct Provider {
    void* target_provider;

    struct {
        size_t (*constructing_mem_to_obj)(
            void* target_provider,
            lifecycle::ConstructingMemToObjView<Elem> dst,
            ptrdiff_t dst_elem_stride, size_t cnt);

        size_t (*left_obj)(void* target_provider,
                           lifecycle::LeftObjView<Elem> dst,
                           ptrdiff_t dst_elem_stride, size_t cnt);

        size_t (*right_obj)(void* target_provider,
                            lifecycle::RightObjView<Elem> dst,
                            ptrdiff_t dst_elem_stride, size_t cnt);
    } transfer;

    constexpr Provider() = default;

    constexpr Provider(Provider const&) = default;

    template <seq_endpoint::provider::IsProvider<Elem> TargetProvider>
    constexpr Provider(TargetProvider&& target_provider);

    constexpr Provider& operator=(Provider const&) = default;

    constexpr void Set(this Provider& self, Provider const& other_provider);

    template <seq_endpoint::provider::IsProvider<Elem> TargetProvider>
    constexpr void Set(this Provider& self, TargetProvider&& target_provider);

    constexpr bool IsEnd(this Provider const& self,
                         seq_endpoint::provider::Tag);

    template <lifecycle::IsDstStorageView<Elem> DstStorageView>
    constexpr size_t Transfer(this Provider& self, seq_endpoint::provider::Tag,
                              DstStorageView dst, ptrdiff_t dst_elem_stride,
                              size_t cnt);
};

}  // namespace provider

}  // namespace zeta::core::poly_seq_endpoint
