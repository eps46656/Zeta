#pragma once

#include <cstdlib>
#include <zeta/core/circular_array.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::circular_array_utils {

using SeqCntrRef = core::seq_cntr::SeqCntrRef;
using CircularArray = core::CircularArray;

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t capacity);

void Destroy(SeqCntrRef seq_cntr_ref);

void Sanitize(SeqCntrRef seq_cntr_ref);

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t capacity) {
    ZETA_Core_DebugAssert(sizeof(Elem) <= stride);
    ZETA_Core_DebugAssert(stride % alignof(Elem) == 0);

    auto ca{ static_cast<CircularArray*>(std::malloc(sizeof(CircularArray))) };

    ca->data = std::malloc(stride * capacity);
    ca->width = sizeof(Elem);
    ca->stride = stride;
    ca->offset = 0;
    ca->size = 0;
    ca->capacity = capacity;

    SeqCntrRef seq_cntr_ref{ CircularArray::GetSeqCntrRef(ca) };

    seq_cntr_utils::AddSanitizeFunc(ca, Sanitize);

    seq_cntr_utils::AddDestroyFunc(ca, Destroy);

    return seq_cntr_ref;
}

inline void Destroy(SeqCntrRef seq_cntr_ref) {
    if (seq_cntr_ref.inst == NULL) { return; }

    auto ca{ static_cast<CircularArray*>(seq_cntr_ref.inst) };

    CircularArray::Deinit(ca);

    delete ca;
}

inline void Sanitize(SeqCntrRef seq_cntr_ref) {
    if (seq_cntr_ref.inst == NULL) { return; }
}

}  // namespace zeta::core_test::circular_array_utils
