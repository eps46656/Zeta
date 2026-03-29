#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/allocator_ref.hpp>
#include <zeta/core/allocator_ref.ipp>
#include <zeta/core/seg_vector.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/seq_cntr_ref.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::seg_vector_utils {

using SeqCntrRef = core::seq_cntr_ref::Ref;

namespace SegVectorNS = core::seg_vector;
using SegVector =
    SegVectorNS::Cntr<core::allocator_ref::Ref, core::allocator_ref::Ref>;

struct Pack {
    std_allocator::Allocator seg_alctr;
    std_allocator::Allocator data_alctr;

    SegVector sv;
};

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t seg_capacity);

void Destroy(void* sv);

void Sanitize(void const* sv);

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t seg_capacity) {
    Pack* pack{ new Pack{} };

    auto* sv{ &pack->sv };

    sv->seg_alctr = zeta::core::allocator_ref::MakeRef(pack->seg_alctr);

    sv->data_alctr = zeta::core::allocator_ref::MakeRef(pack->data_alctr);

    SegVectorNS::Init(*sv, sizeof(Elem), stride, seg_capacity,
                      zeta::core::lifecycle::SkipInitTag{},
                      zeta::core::lifecycle::SkipInitTag{});

    SeqCntrRef seq_cntr_ref{ zeta::core::seq_cntr_ref::MakeRef(sv) };

    seq_cntr_utils::AddSanitizeFunc(sv, Sanitize);

    seq_cntr_utils::AddDestroyFunc(sv, Destroy);

    return seq_cntr_ref;
}

inline void Destroy(void* sv_) {
    SegVector* sv{ static_cast<SegVector*>(sv_) };

    if (sv == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, sv, sv) };

    SegVectorNS::Deinit(pack->sv);

    delete pack;
}

inline void Sanitize(void const* sv_) {
    SegVector const* sv{ static_cast<SegVector const*>(sv_) };

    if (sv == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, sv, sv) };

    core::mem_recorder::MemRecorder seg;
    core::mem_recorder::MemRecorder data;

    SegVectorNS::Sanitize(*sv, &seg, &data);

    core::mem_recorder::MatchRecords(&pack->seg_alctr.mem_recorder, &seg);
    core::mem_recorder::MatchRecords(&pack->data_alctr.mem_recorder, &data);
}

}  // namespace zeta::core_test::seg_vector_utils
