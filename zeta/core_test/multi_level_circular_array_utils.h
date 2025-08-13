#pragma once

#include <zeta/core/mem_check_utils.h>
#include <zeta/core/multi_level_circular_array.h>
#include <zeta/core_test/seq_cntr_utils.h>
#include <zeta/core_test/std_allocator.h>

#include <cstdlib>

struct Zeta_CoreTest_MultiLevelCircularArrayPack {
    Zeta_CoreTest_StdAllocator node_allocator;
    Zeta_CoreTest_StdAllocator seg_allocator;

    Zeta_Core_MultiLevelCircularArray mlca;
};

template <typename Elem>
Zeta_Core_SeqCntr Zeta_CoreTest_MultiLevelCircularArray_Create(
    size_t stride, size_t seg_capacity) {
    Zeta_CoreTest_MultiLevelCircularArrayPack* pack{
        new Zeta_CoreTest_MultiLevelCircularArrayPack{}
    };

    pack->mlca.width = sizeof(Elem);
    pack->mlca.stride = stride;
    pack->mlca.seg_capacity = seg_capacity;

    pack->mlca.node_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->node_allocator,
    };

    pack->mlca.seg_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->seg_allocator,
    };

    Zeta_Core_MultiLevelCircularArray_Init(&pack->mlca);

    Zeta_CoreTest_SeqCntrUtils_AddSanitizeFunc(
        &Zeta_Core_MultiLevelCircularArray_seq_cntr_vtable,
        Zeta_CoreTest_MultiLevelCircularArray_Sanitize);

    Zeta_CoreTest_SeqCntrUtils_AddDestroyFunc(
        &Zeta_Core_MultiLevelCircularArray_seq_cntr_vtable,
        Zeta_CoreTest_MultiLevelCircularArray_Destroy);

    return { &Zeta_Core_MultiLevelCircularArray_seq_cntr_vtable, &pack->mlca };
}

void Zeta_CoreTest_MultiLevelCircularArray_Destroy(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_MultiLevelCircularArray_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_MultiLevelCircularArrayPack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_MultiLevelCircularArrayPack, mlca, seq_cntr.context) };

    Zeta_Core_MultiLevelCircularArray_Deinit(&pack->mlca);

    delete pack;
}

void Zeta_CoreTest_MultiLevelCircularArray_Sanitize(
    Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_MultiLevelCircularArray_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

#if ZETA_Core_EnableDebug
    Zeta_CoreTest_MultiLevelCircularArrayPack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_MultiLevelCircularArrayPack, mlca, seq_cntr.context) };

    Zeta_Core_MemRecorder* data = Zeta_Core_MemRecorder_Create();
    Zeta_Core_MemRecorder* seg = Zeta_Core_MemRecorder_Create();

    Zeta_Core_MultiLevelCircularArray_Sanitize(
        const_cast<void*>(static_cast<void const*>(&pack->mlca)), data, seg);

    Zeta_Core_MemCheck_MatchRecords(pack->node_allocator.mem_recorder, data);
    Zeta_Core_MemCheck_MatchRecords(pack->seg_allocator.mem_recorder, seg);

    Zeta_Core_MemRecorder_Destroy(data);
    Zeta_Core_MemRecorder_Destroy(seg);
#endif
}
