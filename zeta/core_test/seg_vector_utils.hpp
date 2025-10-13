#pragma once

#include <cstdlib>
#include <zeta/core/seg_vector.ipp>
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

    pack->seg_vector.stride = stride;
    pack->seg_vector.width = sizeof(Elem);
    pack->seg_vector.seg_capacity = seg_capacity;

    pack->seg_vector.seg_allocator =
        StdAllocator::GetAllocatorRef(&pack->seg_allocator);

    pack->seg_vector.data_allocator =
        StdAllocator::GetAllocatorRef(&pack->data_allocator);

    SegVector::Init(&pack->seg_vector);

    SeqCntrRef seq_cntr_ref{ SegVector::GetSeqCntrRef(&pack->seg_vector) };

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
