#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/multi_level_circular_array.ipp>
#include <zeta/core/poly_allocator.hpp>
#include <zeta/core/poly_allocator.ipp>
#include <zeta/core/poly_seq_cntr.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::multi_level_circular_array_utils {

template <core::meta::IsNonConstObjectType Elem>
using PolySeqCntr = core::poly_seq_cntr::Cntr<Elem>;

constexpr core::multi_level_circular_array::LayoutConfig<
    unsigned _BitInt(128),  // ActiveMap
    3,                      // seg_elem_slot_cnt
    4, 4, 5, 6, 7, 8        // branch_nums
    >
    default_layout_config;

using Config =
    core::meta::RemoveConstVolatileReference<decltype(default_layout_config)>;

static_assert(core::multi_level_circular_array::detail::IsLayoutConfig_<
              core::meta::RemoveConstVolatileReference<
                  decltype(default_layout_config)>>::value);

template <core::meta::IsNonConstObjectType Elem>
using MultiLevelCircularArray =
    core::multi_level_circular_array::Cntr<Elem, default_layout_config,
                                           core::poly_allocator::Allocator,
                                           core::poly_allocator::Allocator>;

template <core::meta::IsNonConstObjectType Elem>
struct Pack {
    std_allocator::Allocator node_alctr;
    std_allocator::Allocator seg_alctr;

    MultiLevelCircularArray<Elem> mlca;

    constexpr Pack() : mlca{ this->node_alctr, this->seg_alctr } {}
};

template <core::meta::IsNonConstObjectType Elem>
constexpr core::poly_seq_cntr::Cntr<Elem> Create();

template <core::meta::IsNonConstObjectType Elem>
constexpr void Destroy(void* mlca_) {
    MultiLevelCircularArray<Elem>* mlca{
        static_cast<MultiLevelCircularArray<Elem>*>(mlca_)
    };

    if (mlca == nullptr) { return; }

    Pack<Elem>* pack{ ZETA_Core_MemberToStruct(Pack<Elem>, mlca, mlca) };

    delete pack;
}

template <core::meta::IsNonConstObjectType Elem>
constexpr void Sanitize(void const* mlca_) {
    MultiLevelCircularArray<Elem> const* mlca{
        static_cast<MultiLevelCircularArray<Elem> const*>(mlca_)
    };

    if (mlca == nullptr) { return; }

    Pack<Elem>* pack{ ZETA_Core_MemberToStruct(Pack<Elem>, mlca, mlca) };

    core::debug_utils::mem_recording::MemRecorder node_mr;
    core::debug_utils::mem_recording::MemRecorder data_seg_mr;

    mlca->SanityCheck(node_mr, data_seg_mr);

    node_mr.InChargeOf(pack->node_alctr.mem_recorder);

    data_seg_mr.InChargeOf(pack->seg_alctr.mem_recorder);
}

template <core::meta::IsNonConstObjectType Elem>
constexpr core::poly_seq_cntr::Cntr<Elem> Create() {
    Pack<Elem>* pack{ new Pack<Elem>{} };

    auto* mlca{ &pack->mlca };

    seq_cntr_utils::AddSanitizeFunc(mlca, Sanitize<Elem>);

    seq_cntr_utils::AddDestroyFunc(mlca, Destroy<Elem>);

    return *mlca;
}

}  // namespace zeta::core_test::multi_level_circular_array_utils
