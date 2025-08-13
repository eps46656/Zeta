#pragma once

#include <zeta/core/circular_array.h>
#include <zeta/core_test/seq_cntr_utils.h>

#include <cstdlib>

template <typename Elem>
Zeta_Core_SeqCntr Zeta_CoreTest_CircularArray_Create(size_t stride,
                                                     size_t capacity) {
    ZETA_Core_DebugAssert(sizeof(Elem) <= stride);
    ZETA_Core_DebugAssert(stride % alignof(Elem) == 0);

    Zeta_Core_CircularArray* ca{ static_cast<Zeta_Core_CircularArray*>(
        std::malloc(sizeof(Zeta_Core_CircularArray))) };

    ca->data = std::malloc(stride * capacity);
    ca->width = sizeof(Elem);
    ca->stride = stride;
    ca->offset = 0;
    ca->size = 0;
    ca->capacity = capacity;

    Zeta_CoreTest_SeqCntrUtils_AddSanitizeFunc(
        &Zeta_Core_CircularArray_seq_cntr_vtable,
        Zeta_CoreTest_CircularArray_Sanitize);

    Zeta_CoreTest_SeqCntrUtils_AddDestroyFunc(
        &Zeta_Core_CircularArray_seq_cntr_vtable,
        Zeta_CoreTest_CircularArray_Destroy);

    return { &Zeta_Core_CircularArray_seq_cntr_vtable, ca };
}

void Zeta_CoreTest_CircularArray_Destroy(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_CircularArray_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }

    Zeta_Core_CircularArray* ca{ static_cast<Zeta_Core_CircularArray*>(
        seq_cntr.context) };

    Zeta_Core_CircularArray_Deinit(ca);

    delete ca;
}

void Zeta_CoreTest_CircularArray_Sanitize(Zeta_Core_SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &Zeta_Core_CircularArray_seq_cntr_vtable);
    if (seq_cntr.context == NULL) { return; }
}
