#pragma once

#include <zeta/core/dynamic_vector.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core_test/seq_cntr_utils.h>
#include <zeta/core_test/std_allocator.h>

#include <cstdlib>

struct Zeta_CoreTest_DynamicVectorPack {
    Zeta_CoreTest_StdAllocator data_allocator;
    Zeta_CoreTest_StdAllocator seg_allocator;

    Zeta_Core_DynamicVector dynamic_vec;
};

template <typename Val>
Zeta_Core_SeqCntr Zeta_CoreTest_DynamicVector_Create(size_t seg_capacity) {
    Zeta_CoreTest_DynamicVectorPack* dynamic_vec_pack{
        static_cast<Zeta_CoreTest_DynamicVectorPack*>(
            std::malloc(sizeof(Zeta_CoreTest_DynamicVectorPack)))
    };

    new (&dynamic_vec_pack->data_allocator) Zeta_CoreTest_StdAllocator{};
    new (&dynamic_vec_pack->seg_allocator) Zeta_CoreTest_StdAllocator{};

    dynamic_vec_pack->dynamic_vec.width = sizeof(Val);
    dynamic_vec_pack->dynamic_vec.seg_capacity = seg_capacity;

    dynamic_vec_pack->dynamic_vec.data_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->data_allocator,
    };

    dynamic_vec_pack->dynamic_vec.seg_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->seg_allocator,
    };

    Zeta_Core_DynamicVector_Init(&dynamic_vec_pack->dynamic_vec);

    Zeta_CoreTest_SeqCntrUtils_AddSanitizeFunc(
        &Zeta_Core_DynamicVector_seq_cntr_vtabledynamic_vector_seq_cntr_vtable,
        Zeta_CoreTest_DynamicVector_Sanitize);

    Zeta_CoreTest_SeqCntrUtils_AddDestroyFunc(
        &Zeta_Core_DynamicVector_seq_cntr_vtabledynamic_vector_seq_cntr_vtable,
        Zeta_CoreTest_DynamicVector_Destroy);

    return {
        &Zeta_Core_DynamicVector_seq_cntr_vtabledynamic_vector_seq_cntr_vtable,
        &dynamic_vec_pack->dynamic_vec
    };
}

void Zeta_CoreTest_DynamicVector_Destroy(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(
        seq_cntr.vtable ==
        &Zeta_Core_DynamicVector_seq_cntr_vtabledynamic_vector_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_DynamicVectorPack* dynamic_vec_pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_DynamicVectorPack, dynamic_vec, seq_cntr.context) };

    Zeta_Core_DynamicVector_Deinit(&dynamic_vec_pack->dynamic_vec);

    std::free(dynamic_vec_pack);
}

void Zeta_CoreTest_DynamicVector_Sanitize(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(
        seq_cntr.vtable ==
        &Zeta_Core_DynamicVector_seq_cntr_vtabledynamic_vector_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

#if ZETA_Core_EnableDebug
    Zeta_CoreTest_DynamicVectorPack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_DynamicVectorPack, dynamic_vec, seq_cntr.context) };

    Zeta_Core_MemRecorder* data = Zeta_Core_MemRecorder_Create();
    Zeta_Core_MemRecorder* seg = Zeta_Core_MemRecorder_Create();

    Zeta_Core_DynamicVector_Sanitize(
        const_cast<void*>(static_cast<void const*>(&pack->dynamic_vec)), data,
        seg);

    Zeta_Core_MemCheck_MatchRecords(pack->data_allocator.mem_recorder, data);
    Zeta_Core_MemCheck_MatchRecords(pack->seg_allocator.mem_recorder, seg);

    Zeta_Core_MemRecorder_Destroy(data);
    Zeta_Core_MemRecorder_Destroy(seg);
#endif
}
