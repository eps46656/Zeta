#pragma once

#include <debug_hash_table.h>
#include <zeta/core/debugger.h>
#include <zeta/core_test/assoc_cntr_utils.h>

#include <cstdlib>

struct Zeta_CoreTest_DebugHashTablePack {
    Zeta_Core_DebugHashTable debug_ht;
};

template <typename Elem>
Zeta_Core_AssocCntr Zeta_CoreTest_DebugHashTable_Create() {
    Zeta_CoreTest_DebugHashTablePack* pack{
        new Zeta_CoreTest_DebugHashTablePack{}
    };

    pack->debug_ht.width = sizeof(Elem);

    pack->debug_ht.elem_hash_context = NULL;
    pack->debug_ht.ElemHash = &ZetaHash<Elem>;

    pack->debug_ht.elem_cmp_context = NULL;
    pack->debug_ht.ElemCompare = &ZetaCompare<Elem, Elem>;

    Zeta_Core_DebugHashTable_Init(&pack->debug_ht);

    AssocCntrUtils_AddSanitizeFunc(&Zeta_Core_DebugHashTable_assoc_cntr_vtable,
                                   Zeta_CoreTest_DebugHashTable_Sanitize);

    AssocCntrUtils_AddDestroyFunc(&Zeta_Core_DebugHashTable_assoc_cntr_vtable,
                                  Zeta_CoreTest_DebugHashTable_Destroy);

    return { &Zeta_Core_DebugHashTable_assoc_cntr_vtable, &pack->debug_ht };
}

void Zeta_CoreTest_DebugHashTable_Destroy(Zeta_Core_AssocCntr assoc_cntr) {
    ZETA_Core_DebugAssert(assoc_cntr.vtable ==
                          &Zeta_Core_DebugHashTable_assoc_cntr_vtable);
    if (assoc_cntr.context == NULL) { return; }

    Zeta_CoreTest_DebugHashTablePack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_DebugHashTablePack, debug_ht, assoc_cntr.context) };

    Zeta_Core_DebugHashTable_Deinit(&pack->debug_ht);

    std::free(pack);
}

void Zeta_CoreTest_DebugHashTable_Sanitize(Zeta_Core_AssocCntr assoc_cntr) {
    ZETA_Core_DebugAssert(assoc_cntr.vtable ==
                          &Zeta_Core_DebugHashTable_assoc_cntr_vtable);
    if (assoc_cntr.context == NULL) { return; }
}
