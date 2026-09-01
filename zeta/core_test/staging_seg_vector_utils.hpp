#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/poly_allocator.hpp>
#include <zeta/core/poly_allocator.ipp>
#include <zeta/core/poly_seq_cntr.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/staging_seg_vector.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::staging_seg_vector_utils {

using PolySeqCntr = core::poly_seq_cntr::Cntr;

namespace StagingSegVectorNS = core::staging_seg_vector;
using StagingSegVector =
    StagingSegVectorNS::Cntr<PolySeqCntr, core::poly_allocator::Allocator,
                             core::poly_allocator::Allocator>;

struct Pack {
    std_allocator::Allocator seg_alctr;
    std_allocator::Allocator data_alctr;

    StagingSegVector sv;

    constexpr Pack(size_t elem_stride, size_t seg_capacity,
                   PolySeqCntr origin_seq_cntr_ref)
        : sv{
              elem_stride,          //
              seg_capacity,         //
              origin_seq_cntr_ref,  //
              this->seg_alctr,      //
              this->data_alctr,     //
          } {
        ZETA_Core_Debug_PrintCurPos;
    }

    constexpr Pack(size_t elem_stride, size_t seg_capacity,
                   PolySeqCntr origin_seq_cntr_ref, StagingSegVector& src_sv)
        : sv{
              elem_stride,          //
              seg_capacity,         //
              origin_seq_cntr_ref,  //
              this->seg_alctr,      //
              this->data_alctr,     //
              src_sv,               //
          } {
        ZETA_Core_Debug_PrintCurPos;
    }
};

constexpr void Destroy(void* sv);

constexpr void Sanitize(void const* sv);

constexpr PolySeqCntr Create(size_t elem_stride, size_t seg_capacity,
                             PolySeqCntr origin_seq_cntr_ref) {
    Pack* pack{ new Pack{ elem_stride, seg_capacity, origin_seq_cntr_ref } };

    auto* sv{ &pack->sv };

    ZETA_Core_Debug_PrintVar(sv);

    seq_cntr_utils::AddSanitizeFunc(sv, Sanitize);

    seq_cntr_utils::AddDestroyFunc(sv, Destroy);

    return *sv;
}

constexpr PolySeqCntr Create(size_t elem_stride, size_t seg_capacity,
                             PolySeqCntr origin_seq_cntr_ref,
                             StagingSegVector& src_sv) {
    Pack* pack{ new Pack{
        elem_stride,
        seg_capacity,
        origin_seq_cntr_ref,
        src_sv,
    } };

    auto* sv{ &pack->sv };

    ZETA_Core_Debug_PrintVar(sv);

    seq_cntr_utils::AddSanitizeFunc(sv, Sanitize);

    seq_cntr_utils::AddDestroyFunc(sv, Destroy);

    return *sv;
}

constexpr void Destroy(void* sv) {
    Pack* pack{ ZETA_Core_MemberToStruct(Pack, sv, sv) };

    delete pack;
}

constexpr void Sanitize(void const* sv) {
    Pack* pack{ ZETA_Core_MemberToStruct(Pack, sv, sv) };

    core::mem_recorder::MemRecorder seg;
    core::mem_recorder::MemRecorder data;

    pack->sv.Sanitize(&seg, &data);

    core::mem_recorder::MatchRecords(pack->seg_alctr.mem_recorder, seg);
    core::mem_recorder::MatchRecords(pack->data_alctr.mem_recorder, data);
}

}  // namespace zeta::core_test::staging_seg_vector_utils
