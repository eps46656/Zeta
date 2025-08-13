#pragma once

#include <zeta/core/staging_vector.h>
#include <zeta/core_test/seq_cntr_utils.h>
#include <zeta/core_test/std_allocator.h>

#include <cstdlib>
#include <memory>

struct Zeta_CoreTest_StagingVector_Pack {
    Zeta_CoreTest_StdAllocator seg_allocator;
    Zeta_CoreTest_StdAllocator data_allocator;

    Zeta_Core_StagingVector staging_vector;
};

template <typename Elem>
Zeta_Core_SeqCntr Zeta_CoreTest_StagingVector_Create(
    Zeta_Core_SeqCntr origin_seq_cntr, size_t stride, size_t seg_capacity) {
    ZETA_Core_DebugAssert(origin_seq_cntr.vtable != NULL);
    ZETA_Core_DebugAssert(0 < stride);
    ZETA_Core_DebugAssert(0 < seg_capacity);

    ZETA_Core_DebugAssert(sizeof(Elem) <=
                          ZETA_Core_SeqCntr_GetWidth(origin_seq_cntr));

    Zeta_CoreTest_StagingVector_Pack* pack{
        new Zeta_CoreTest_StagingVector_Pack{}
    };

    pack->staging_vector.stride = stride;
    pack->staging_vector.origin = origin_seq_cntr;
    pack->staging_vector.seg_capacity = seg_capacity;

    pack->staging_vector.seg_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->seg_allocator,
    };

    pack->staging_vector.data_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->data_allocator,
    };

    Zeta_Core_StagingVector_Init(&pack->staging_vector);

    Zeta_CoreTest_SeqCntrUtils_AddSanitizeFunc(
        &Zeta_Core_DynamicVector_seq_cntr_vtablestaging_vector_seq_cntr_vtable,
        Zeta_CoreTest_StagingVector_Sanitize);

    Zeta_CoreTest_SeqCntrUtils_AddDestroyFunc(
        &Zeta_Core_DynamicVector_seq_cntr_vtablestaging_vector_seq_cntr_vtable,
        Zeta_CoreTest_StagingVector_Destroy);

    return {
        &Zeta_Core_DynamicVector_seq_cntr_vtablestaging_vector_seq_cntr_vtable,
        &pack->staging_vector
    };
}

void Zeta_CoreTest_StagingVector_Destroy(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(
        seq_cntr.vtable ==
        &Zeta_Core_DynamicVector_seq_cntr_vtablestaging_vector_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_StagingVector_Pack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_StagingVector_Pack, staging_vector, seq_cntr.context) };

    Zeta_Core_StagingVector_Deinit(&pack->staging_vector);

    delete pack;
}

void Zeta_CoreTest_StagingVector_Sanitize(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(
        seq_cntr.vtable ==
        &Zeta_Core_DynamicVector_seq_cntr_vtablestaging_vector_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_StagingVector_Pack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_StagingVector_Pack, staging_vector, seq_cntr.context) };

    Zeta_Core_MemRecorder* seg = Zeta_Core_MemRecorder_Create();
    Zeta_Core_MemRecorder* data = Zeta_Core_MemRecorder_Create();

    Zeta_Core_StagingVector_Sanitize(
        const_cast<void*>(static_cast<void const*>(&pack->staging_vector)), seg,
        data);

    Zeta_Core_MemCheck_MatchRecords(pack->seg_allocator.mem_recorder, seg);
    Zeta_Core_MemCheck_MatchRecords(pack->data_allocator.mem_recorder, data);

    Zeta_Core_MemRecorder_Destroy(seg);
    Zeta_Core_MemRecorder_Destroy(data);
}
