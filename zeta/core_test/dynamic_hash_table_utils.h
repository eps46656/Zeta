#pragma once

#include <zeta/core/dynamic_hash_table.h>
#include <zeta/core/mem_check_utils.h>
#include <zeta/core_test/assoc_cntr_utils.h>
#include <zeta/core_test/std_allocator.h>

#include <cstdlib>

struct DynamicHashTablePack {
    Zeta_CoreTest_StdAllocator ght_table_node_allocator;
    Zeta_CoreTest_StdAllocator node_allocator;

    Zeta_Core_DynamicHashTable dht;
};

template <typename Elem>
Zeta_Core_AssocCntr DynamicHashTable_Create() {
    DynamicHashTablePack* pack{ new DynamicHashTablePack{} };

    pack->dht.width = sizeof(Elem);

    pack->dht.elem_hash_context = NULL;
    pack->dht.ElemHash = &ZetaHash<Elem>;

    pack->dht.elem_cmp_context = NULL;
    pack->dht.ElemCompare = &ZetaCompare<Elem, Elem>;

    pack->dht.ght.table_node_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->ght_table_node_allocator,
    };

    pack->dht.node_allocator = {
        .vtable = &Zeta_CoreTest_StdAllocator_allocator_vtable,
        .context = &pack->node_allocator,
    };

    Zeta_Core_DynamicHashTable_Init(&pack->dht);

    AssocCntrUtils_AddSanitizeFunc(
        &Zeta_Core_DynamicHashTable_assoc_cntr_vtable,
        DynamicHashTable_Sanitize);

    AssocCntrUtils_AddDestroyFunc(&Zeta_Core_DynamicHashTable_assoc_cntr_vtable,
                                  DynamicHashTable_Destroy);

    return { &Zeta_Core_DynamicHashTable_assoc_cntr_vtable, &pack->dht };
}

void DynamicHashTable_Destroy(Zeta_Core_AssocCntr assoc_cntr) {
    ZETA_Core_DebugAssert(assoc_cntr.vtable ==
                          &Zeta_Core_DynamicHashTable_assoc_cntr_vtable);
    if (assoc_cntr.context == NULL) { return; }

    DynamicHashTablePack* pack{ ZETA_Core_MemberToStruct(
        DynamicHashTablePack, dht, assoc_cntr.context) };

    Zeta_Core_DynamicHashTable_Deinit(&pack->dht);

    std::free(pack);
}

void DynamicHashTable_Sanitize(Zeta_Core_AssocCntr assoc_cntr) {
    ZETA_Core_DebugAssert(assoc_cntr.vtable ==
                          &Zeta_Core_DynamicHashTable_assoc_cntr_vtable);
    if (assoc_cntr.context == NULL) { return; }

#if ZETA_Core_EnableDebug
    DynamicHashTablePack* pack{ ZETA_Core_MemberToStruct(
        DynamicHashTablePack, dht, assoc_cntr.context) };

    Zeta_Core_MemRecorder* table_recorder = Zeta_Core_MemRecorder_Create();
    Zeta_Core_MemRecorder* node_recorder = Zeta_Core_MemRecorder_Create();

    Zeta_Core_DynamicHashTable_Sanitize(&pack->dht, table_recorder,
                                        node_recorder);

    Zeta_Core_MemCheck_MatchRecords(pack->ght_table_node_allocator.mem_recorder,
                                    table_recorder);
    Zeta_Core_MemCheck_MatchRecords(pack->node_allocator.mem_recorder,
                                    node_recorder);

    Zeta_Core_MemRecorder_Destroy(table_recorder);
    Zeta_Core_MemRecorder_Destroy(node_recorder);
#endif
}
