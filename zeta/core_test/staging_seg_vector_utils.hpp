#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/poly_allocator.hpp>
#include <zeta/core/poly_allocator.ipp>
#include <zeta/core/poly_seq_cntr.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/staging_seg_vector.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::staging_seg_vector_utils {

template <core::meta::IsNonConstObjectType Elem>
using PolySeqCntr = core::poly_seq_cntr::Cntr<Elem>;

template <core::meta::IsNonConstObjectType Elem>
using StagingSegVector =
    core::staging_seg_vector::Cntr<PolySeqCntr<Elem>,
                                   core::poly_allocator::Allocator,
                                   core::poly_allocator::Allocator>;

template <core::meta::IsNonConstObjectType Elem>
struct Pack {
    std_allocator::Allocator seg_alctr;
    std_allocator::Allocator data_alctr;

    StagingSegVector<Elem> sv;

    constexpr Pack(size_t seg_elem_slot_cnt,
                   PolySeqCntr<Elem> origin_seq_cntr_ref)
        : sv{
              seg_elem_slot_cnt,    //
              origin_seq_cntr_ref,  //
              this->seg_alctr,      //
              this->data_alctr,     //
          } {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
    }

    constexpr Pack(size_t seg_elem_slot_cnt,
                   PolySeqCntr<Elem> origin_seq_cntr_ref,
                   StagingSegVector<Elem>& src_sv)
        : sv{
              seg_elem_slot_cnt,    //
              origin_seq_cntr_ref,  //
              this->seg_alctr,      //
              this->data_alctr,     //
              src_sv,               //
          } {
        ZETA_Core_DebugUtils_Diag_LogCurPos();
    }
};

template <core::meta::IsNonConstObjectType Elem>
constexpr void Destroy(void* sv);

template <core::meta::IsNonConstObjectType Elem>
constexpr void Sanitize(void const* sv);

template <core::meta::IsNonConstObjectType Elem>
constexpr PolySeqCntr<Elem> Create(size_t seg_capacity,
                                   PolySeqCntr<Elem> origin_seq_cntr_ref) {
    Pack<Elem>* pack{ new Pack<Elem>{ seg_capacity, origin_seq_cntr_ref } };

    auto* sv{ &pack->sv };

    ZETA_Core_DebugUtils_Diag_LogVar(sv);

    seq_cntr_utils::AddSanitizeFunc(sv, Sanitize<Elem>);

    seq_cntr_utils::AddDestroyFunc(sv, Destroy<Elem>);

    return *sv;
}

template <core::meta::IsNonConstObjectType Elem>
constexpr PolySeqCntr<Elem> Create(size_t seg_capacity,
                                   PolySeqCntr<Elem> origin_seq_cntr_ref,
                                   StagingSegVector<Elem>& src_sv) {
    Pack<Elem>* pack{ new Pack<Elem>{
        seg_capacity,
        origin_seq_cntr_ref,
        src_sv,
    } };

    auto* sv{ &pack->sv };

    ZETA_Core_DebugUtils_Diag_LogVar(sv);

    seq_cntr_utils::AddSanitizeFunc(sv, Sanitize);

    seq_cntr_utils::AddDestroyFunc(sv, Destroy);

    return *sv;
}

template <core::meta::IsNonConstObjectType Elem>
constexpr void Destroy(void* sv) {
    Pack<Elem>* pack{ ZETA_Core_MemberToStruct(Pack<Elem>, sv, sv) };

    delete pack;
}

template <core::meta::IsNonConstObjectType Elem>
constexpr void Sanitize(void const* sv_) {
    StagingSegVector<Elem> const* sv{
        static_cast<StagingSegVector<Elem> const*>(sv_)
    };

    if (sv == nullptr) { return; }

    Pack<Elem>* pack{ ZETA_Core_MemberToStruct(Pack<Elem>, sv, sv) };

    core::debug_utils::mem_recording::MemRecorder seg_mr;
    core::debug_utils::mem_recording::MemRecorder data_mr;

    sv->SanityCheck(seg_mr, data_mr);

    seg_mr.InChargeOf(pack->seg_alctr.mem_recorder);
    data_mr.InChargeOf(pack->data_alctr.mem_recorder);
}

}  // namespace zeta::core_test::staging_seg_vector_utils
