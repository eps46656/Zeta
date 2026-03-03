#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator_ref.hpp>
#include <zeta/core/allocator_ref.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/mem_recorder.hpp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr_ref.ipp>
#include <zeta/core/staging_seg_vector.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::staging_seg_vector_utils {

using SeqCntrRef = core::seq_cntr_ref::Ref;

namespace StagingSegVectorNS = core::staging_seg_vector;
using StagingSegVector =
    StagingSegVectorNS::Cntr<SeqCntrRef, core::allocator_ref::Ref,
                             core::allocator_ref::Ref>;

struct Pack {
    std_allocator::Allocator seg_alctr;
    std_allocator::Allocator data_alctr;

    StagingSegVector sv;
};

template <typename Elem>
SeqCntrRef Create(SeqCntrRef origin_seq_cntr_ref, size_t stride,
                  size_t seg_capacity);

void Destroy(void* sv);

void Sanitize(void const* sv);

template <typename Elem>
SeqCntrRef Create(SeqCntrRef origin_seq_cntr_ref, size_t stride,
                  size_t seg_capacity) {
    ZETA_Core_DebugAssert(sizeof(Elem) <= origin_seq_cntr_ref.width);

    Pack* pack{ new Pack{} };

    auto* sv{ &pack->sv };

    sv->stride = stride;
    sv->origin = origin_seq_cntr_ref;
    sv->seg_capacity = seg_capacity;

    sv->seg_alctr = zeta::core::allocator_ref::ops::MakeRef(&pack->seg_alctr);

    sv->data_alctr = zeta::core::allocator_ref::ops::MakeRef(&pack->data_alctr);

    StagingSegVectorNS::ops::Init(sv);

    SeqCntrRef seq_cntr_ref{ zeta::core::seq_cntr_ref::ops::MakeRef(sv) };

    seq_cntr_utils::AddSanitizeFunc(sv, Sanitize);

    seq_cntr_utils::AddDestroyFunc(sv, Destroy);

    return seq_cntr_ref;
}

inline void Destroy(void* sv) {
    Pack* pack{ ZETA_Core_MemberToStruct(Pack, sv, sv) };

    StagingSegVectorNS::ops::Deinit(&pack->sv);

    delete pack;
}

inline void Sanitize(void const* sv) {
    Pack* pack{ ZETA_Core_MemberToStruct(Pack, sv, sv) };

    core::mem_recorder::MemRecorder seg;
    core::mem_recorder::MemRecorder data;

    StagingSegVectorNS::ops::Sanitize(&pack->sv, &seg, &data);

    core::mem_recorder::ops::MatchRecords(&pack->seg_alctr.mem_recorder, &seg);
    core::mem_recorder::ops::MatchRecords(&pack->data_alctr.mem_recorder,
                                          &data);
}

}  // namespace zeta::core_test::staging_seg_vector_utils
