#pragma once

#include <cstdlib>
#include <zeta/core/circular_array.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::circular_array_utils {

using CircularArray = core::CircularArray;

template <typename Elem>
core::SeqCntr Create(size_t stride, size_t capacity);

void Destroy(core::SeqCntr seq_cntr);

void Sanitize(core::SeqCntr seq_cntr);

template <typename Elem>
core::SeqCntr Create(size_t stride, size_t capacity) {
    ZETA_Core_DebugAssert(sizeof(Elem) <= stride);
    ZETA_Core_DebugAssert(stride % alignof(Elem) == 0);

    auto ca{ static_cast<CircularArray*>(std::malloc(sizeof(CircularArray))) };

    ca->data = std::malloc(stride * capacity);
    ca->width = sizeof(Elem);
    ca->stride = stride;
    ca->offset = 0;
    ca->size = 0;
    ca->capacity = capacity;

    seq_cntr_utils::AddSanitizeFunc(&core::SeqCntr::MakeVTable<CircularArray>(),
                                    Sanitize);

    seq_cntr_utils::AddDestroyFunc(&core::SeqCntr::MakeVTable<CircularArray>(),
                                   Destroy);

    return CircularArray::ToSeqCntr(ca);
}

inline void Destroy(core::SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &core::SeqCntr::MakeVTable<CircularArray>());
    if (seq_cntr.inst == NULL) { return; }

    auto ca{ static_cast<CircularArray*>(seq_cntr.inst) };

    CircularArray::Deinit(ca);

    delete ca;
}

inline void Sanitize(core::SeqCntr seq_cntr) {
    ZETA_Core_DebugAssert(seq_cntr.vtable ==
                          &core::SeqCntr::MakeVTable<CircularArray>());
    if (seq_cntr.inst == NULL) { return; }
}

}  // namespace zeta::core_test::circular_array_utils
