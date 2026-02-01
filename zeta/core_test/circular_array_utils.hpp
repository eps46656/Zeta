#pragma once

#include <cstdlib>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core/value_wrapper.hpp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::circular_array_utils {

using SeqCntrRef = core::seq_cntr::Ref<core::value_wrapper::FalseType>;

namespace CircularArrayNS = core::circular_array;
namespace CircularArrayOps = CircularArrayNS::ops;
using CircularArray = CircularArrayNS::Cntr;
using CircularArrayView = CircularArrayNS::SeqCntrView;

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t capacity);

void Destroy(void* ca);

void Sanitize(void const* ca);

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t capacity) {
    ZETA_Core_DebugAssert(sizeof(Elem) <= stride);
    ZETA_Core_DebugAssert(stride % alignof(Elem) == 0);

    auto* ca{ static_cast<CircularArray*>(std::malloc(sizeof(CircularArray))) };

    ca->data = std::malloc(stride * capacity);
    ca->width = sizeof(Elem);
    ca->stride = stride;
    ca->offset = 0;
    ca->size = 0;
    ca->capacity = capacity;

    SeqCntrRef seq_cntr_ref{ zeta::core::seq_cntr::MakeRef(
        reinterpret_cast<CircularArrayView*>(ca)) };

    seq_cntr_utils::AddSanitizeFunc(ca, Sanitize);

    seq_cntr_utils::AddDestroyFunc(ca, Destroy);

    return seq_cntr_ref;
}

inline void Destroy(void* ca_) {
    CircularArray* ca{ static_cast<CircularArray*>(ca_) };

    if (ca == nullptr) { return; }

    CircularArrayOps::Deinit(ca);

    delete ca;
}

inline void Sanitize(void const*) {}

}  // namespace zeta::core_test::circular_array_utils
