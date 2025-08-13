#pragma once

#include <zeta/core/seg_vector.h>
#include <zeta/core_test/seq_cntr_utils.h>
#include <zeta/core_test/std_allocator.h>

#include <cstdlib>
#include <memory>

struct Zeta_CoreTest_SegVectorUtils_Pack {
    Zeta_CoreTest_StdAllocator seg_allocator;
    Zeta_CoreTest_StdAllocator data_allocator;

    Zeta_Core_SegVector seg_vector;
};

template <typename Elem>
Zeta_Core_SeqCntr Zeta_CoreTest_SegVectorUtils_Create(size_t stride,
                                                      size_t seg_capacity) {
    Zeta_CoreTest_SegVectorUtils_Pack* pack{
        new Zeta_CoreTest_SegVectorUtils_Pack{}
    };

    pack->seg_vector.stride = stride;
    pack->seg_vector.width = sizeof(Elem);
    pack->seg_vector.seg_capacity = seg_capacity;

    pack->seg_vector.seg_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->seg_allocator,
    };

    pack->seg_vector.data_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->data_allocator,
    };

    Zeta_Core_SegVector_Init(&pack->seg_vector);

    Zeta_CoreTest_SeqCntrUtils_AddSanitizeFunc(
        &Zeta_Core_DynamicVector_seq_cntr_vtableseg_vector_seq_cntr_vtable,
        Zeta_CoreTest_SegVectorUtils_Sanitize);

    Zeta_CoreTest_SeqCntrUtils_AddDestroyFunc(
        &Zeta_Core_DynamicVector_seq_cntr_vtableseg_vector_seq_cntr_vtable,
        Zeta_CoreTest_SegVectorUtils_Destroy);

    return { &Zeta_Core_DynamicVector_seq_cntr_vtableseg_vector_seq_cntr_vtable,
             &pack->seg_vector };
}

void Zeta_CoreTest_SegVectorUtils_Destroy(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(
        seq_cntr.vtable ==
        &Zeta_Core_DynamicVector_seq_cntr_vtableseg_vector_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_SegVectorUtils_Pack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_SegVectorUtils_Pack, seg_vector, seq_cntr.context) };

    Zeta_Core_SegVector_Deinit(&pack->seg_vector);

    delete pack;
}

void Zeta_CoreTest_SegVectorUtils_Sanitize(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(
        seq_cntr.vtable ==
        &Zeta_Core_DynamicVector_seq_cntr_vtableseg_vector_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_SegVectorUtils_Pack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_SegVectorUtils_Pack, seg_vector, seq_cntr.context) };

    Zeta_Core_MemRecorder* seg = Zeta_Core_MemRecorder_Create();
    Zeta_Core_MemRecorder* data = Zeta_Core_MemRecorder_Create();

    Zeta_Core_SegVector_Sanitize(
        const_cast<void*>(static_cast<void const*>(&pack->seg_vector)), seg,
        data);

    Zeta_Core_MemCheck_MatchRecords(pack->seg_allocator.mem_recorder, seg);
    Zeta_Core_MemCheck_MatchRecords(pack->data_allocator.mem_recorder, data);

    Zeta_Core_MemRecorder_Destroy(seg);
    Zeta_Core_MemRecorder_Destroy(data);
}
