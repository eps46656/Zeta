#pragma once

#include <cstdlib>
#include <zeta/core/allocator.hpp>
#include <zeta/core/allocator.ipp>
#include <zeta/core/seg_vector.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>
#include <zeta/core_test/std_allocator.hpp>

namespace zeta::core_test::seg_vector_utils {

using SeqCntrRef = core::seq_cntr::SeqCntrRef;
using SegVector = core::SegVector<core::allocator::AllocatorRef,
                                  core::allocator::AllocatorRef>;

struct Pack {
    StdAllocator seg_allocator;
    StdAllocator data_allocator;

    SegVector seg_vector;
};

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t seg_capacity);

void Destroy(SeqCntrRef seq_cntr);

void Sanitize(SeqCntrRef seq_cntr);

// -----------------------------------------------------------------------------

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t seg_capacity) {
    Pack* pack{ new Pack{} };

    auto sv{ &pack->seg_vector };

    sv->stride = stride;
    sv->width = sizeof(Elem);
    sv->seg_capacity = seg_capacity;

    sv->seg_allocator =
        zeta::core::allocator::MakeAllocatorRef(&pack->seg_allocator);

    sv->data_allocator =
        zeta::core::allocator::MakeAllocatorRef(&pack->data_allocator);

    SegVector::Init(&pack->seg_vector);

    SeqCntrRef seq_cntr_ref{ zeta::core::seq_cntr::MakeSeqCntrRef(sv) };

    seq_cntr_utils::AddSanitizeFunc(seq_cntr_ref.inst, Sanitize);

    seq_cntr_utils::AddDestroyFunc(seq_cntr_ref.inst, Destroy);

    return seq_cntr_ref;
}

inline void Destroy(SeqCntrRef seq_cntr) {
    if (seq_cntr.inst == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, seg_vector, seq_cntr.inst) };

    SegVector::Deinit(&pack->seg_vector);

    delete pack;
}

inline void Sanitize(SeqCntrRef seq_cntr) {
    if (seq_cntr.inst == nullptr) { return; }

    Pack* pack{ ZETA_Core_MemberToStruct(Pack, seg_vector, seq_cntr.inst) };

    core::MemRecorder* seg{ core::MemRecorder::Create() };
    core::MemRecorder* data{ core::MemRecorder::Create() };

    SegVector::Sanitize(
        const_cast<void*>(static_cast<void const*>(&pack->seg_vector)), seg,
        data);

    core::MemRecorder::MatchRecords(pack->seg_allocator.mem_recorder, seg);
    core::MemRecorder::MatchRecords(pack->data_allocator.mem_recorder, data);

    core::MemRecorder::Destroy(seg);
    core::MemRecorder::Destroy(data);
}

}  // namespace zeta::core_test::seg_vector_utils
