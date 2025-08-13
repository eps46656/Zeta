#pragma once

#include <caching_array.h>
#include <zeta/core_test/cache_manager_utils.h>
#include <zeta/core_test/seq_cntr_utils.h>
#include <zeta/core_test/std_allocator.h>

#include <cstdlib>
#include <memory>

struct Zeta_CoreTest_CachingArrayPack {
    Zeta_CacheManager cm;
    Zeta_CachingArray ca;
};

// -----------------------------------------------------------------------------

Zeta_Core_SeqCntr Zeta_CoreTest_CachingArray_Create(Zeta_CacheManager cm,
                                                    size_t max_cache_cnt) {
    Zeta_CoreTest_CachingArrayPack* pack{
        new Zeta_CoreTest_CachingArrayPack{}
    };

    pack->cm = cm;

    pack->ca.cache_manager = cm;

    pack->ca.sd = ZETA_CacheManager_Open(cm, max_cache_cnt);

    Zeta_CachingArray_Init(&pack->ca);

    Zeta_CoreTest_SeqCntrUtils_AddSanitizeFunc(
        &Zeta_Core_CachingArray_seq_cntr_vtable,
        Zeta_CoreTest_CachingArray_Sanitize);

    Zeta_CoreTest_SeqCntrUtils_AddDestroyFunc(
        &Zeta_Core_CachingArray_seq_cntr_vtable,
        Zeta_CoreTest_CachingArray_Destroy);

    return { .vtable = &Zeta_Core_CachingArray_seq_cntr_vtable,
             .context = &pack->ca };
}

void Zeta_CoreTest_CachingArray_Destroy(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_CachingArray_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_CachingArrayPack* ca_pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_CachingArrayPack, ca, seq_cntr.context) };

    Zeta_CachingArray_Deinit(&ca_pack->ca);

    ZETA_CacheManager_Close(ca_pack->cm, ca_pack->ca.sd);

    delete ca_pack;
}

void Zeta_CoreTest_CachingArray_Sanitize(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_CachingArray_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_CachingArrayPack* ca_pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_CachingArrayPack, ca, seq_cntr.context) };

    Zeta_CoreTest_CacheManagerUtils_Sanitize(ca_pack->cm);
}
