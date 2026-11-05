#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/poly_allocator.hpp>
#include <zeta/core/poly_allocator.ipp>
#include <zeta/core/poly_seq_cntr.ipp>
#include <zeta/core/seg_vector.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::seg_vector_utils {

template <core::meta::IsNonConstObjectType Elem>
using PolySeqCntr = core::poly_seq_cntr::Cntr<Elem>;

template <core::meta::IsNonConstObjectType Elem>
using SegVector = core::seg_vector::Cntr<Elem, core::poly_allocator::Allocator,
                                         core::poly_allocator::Allocator>;

static_assert(core::seq_cntr::IsSeqCntr<SegVector<int>&>);
static_assert(core::seq_cntr::IsSeqCntr<SegVector<int> const&>);
static_assert(core::seq_cntr::IsSeqCntr<SegVector<int>&&>);
static_assert(core::seq_cntr::IsSeqCntr<SegVector<int> const&&>);

template <core::meta::IsNonConstObjectType Elem>
struct Pack {
    std_allocator::Allocator node_alctr;
    std_allocator::Allocator seg_alctr;

    SegVector<Elem> sv;

    constexpr Pack(size_t seg_elem_slot_cnt)
        : sv{
              seg_elem_slot_cnt,
              this->node_alctr,
              this->seg_alctr,
          } {}
};

template <core::meta::IsNonConstObjectType Elem>
constexpr PolySeqCntr<Elem> Create(size_t stride, size_t seg_elem_slot_cnt);

template <core::meta::IsNonConstObjectType Elem>
constexpr void Destroy(void* sv);

template <core::meta::IsNonConstObjectType Elem>
constexpr void Sanitize(void const* sv);

template <core::meta::IsNonConstObjectType Elem>
constexpr PolySeqCntr<Elem> Create(size_t seg_elem_slot_cnt) {
    Pack<Elem>* pack{ new Pack<Elem>{ seg_elem_slot_cnt } };

    auto* sv{ &pack->sv };

    PolySeqCntr<Elem> poly_seq_cntr{ *sv };

    seq_cntr_utils::AddSanitizeFunc(sv, Sanitize<Elem>);

    seq_cntr_utils::AddDestroyFunc(sv, Destroy<Elem>);

    return poly_seq_cntr;
}

template <core::meta::IsNonConstObjectType Elem>
constexpr void Destroy(void* sv_) {
    SegVector<Elem>* sv{ static_cast<SegVector<Elem>*>(sv_) };

    if (sv == nullptr) { return; }

    Pack<Elem>* pack{ ZETA_Core_MemberToStruct(Pack<Elem>, sv, sv) };

    delete pack;
}

template <core::meta::IsNonConstObjectType Elem>
constexpr void Sanitize(void const* sv_) {
    SegVector<Elem> const* sv{ static_cast<SegVector<Elem> const*>(sv_) };

    if (sv == nullptr) { return; }

    Pack<Elem>* pack{ ZETA_Core_MemberToStruct(Pack<Elem>, sv, sv) };

    core::debug_utils::mem_recording::MemRecorder seg_mr;
    core::debug_utils::mem_recording::MemRecorder data_mr;

    sv->SanityCheck(seg_mr, data_mr);

    seg_mr.InChargeOf(pack->node_alctr.mem_recorder);
    data_mr.InChargeOf(pack->seg_alctr.mem_recorder);
}

}  // namespace zeta::core_test::seg_vector_utils
