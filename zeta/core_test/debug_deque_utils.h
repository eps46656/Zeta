#pragma once

#include <zeta/core/debug_deque.h>
#include <zeta/core_test/seq_cntr_utils.h>

#include <cstdlib>

struct Zeta_CoreTest_DebugDeque_Pack {
    Zeta_Core_DebugDeque debug_deque;
};

template <typename Elem>
Zeta_Core_SeqCntr Zeta_CoreTest_DebugDeque_Create() {
    Zeta_CoreTest_DebugDeque_Pack* pack{ new Zeta_CoreTest_DebugDeque_Pack{} };

    pack->debug_deque.width = sizeof(Elem);

    Zeta_Core_DebugDeque_Init(&pack->debug_deque);

    Zeta_CoreTest_SeqCntrUtils_AddSanitizeFunc(
        &Zeta_Core_DebugDeque_seq_cntr_vtable,
        Zeta_CoreTest_DebugDeque_Sanitize);

    Zeta_CoreTest_SeqCntrUtils_AddDestroyFunc(
        &Zeta_Core_DebugDeque_seq_cntr_vtable,
        Zeta_CoreTest_DebugDeque_Destroy);

    return { &Zeta_Core_DebugDeque_seq_cntr_vtable, &pack->debug_deque };
}

void Zeta_CoreTest_DebugDeque_Destroy(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_DebugDeque_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_DebugDeque_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_CoreTest_DebugDeque_Pack* pack{ ZETA_Core_MemberToStruct(
        Zeta_CoreTest_DebugDeque_Pack, debug_deque, seq_cntr.context) };

    Zeta_Core_DebugDeque_Deinit(seq_cntr.context);

    delete pack;
}

void Zeta_CoreTest_DebugDeque_Sanitize(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_DebugDeque_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }
}
