#pragma once

#include <cstdlib>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::circular_array_utils {

using PolySeqCntr = core::poly_seq_cntr::Cntr;

namespace CircularArrayNS = core::circular_array;
using CircularArray = CircularArrayNS::Cntr;

template <typename Elem>
constexpr PolySeqCntr Create(size_t stride, size_t capacity);

constexpr void Destroy(void* ca);

constexpr void Sanitize(void const* ca);

template <typename Elem>
constexpr PolySeqCntr Create(size_t stride, size_t slot_cnt) {
    ZETA_Core_DebugAssert(sizeof(Elem) <= stride);
    ZETA_Core_DebugAssert(stride % alignof(Elem) == 0);

    auto* ca{ static_cast<CircularArray*>(std::malloc(sizeof(CircularArray))) };

    ca->data = std::malloc(stride * slot_cnt);
    ca->elem_size = sizeof(Elem);
    ca->elem_stride = stride;
    ca->elem_cnt = 0;
    ca->slot_cnt = slot_cnt;
    ca->rot = 0;

    seq_cntr_utils::AddSanitizeFunc(ca, Sanitize);

    seq_cntr_utils::AddDestroyFunc(ca, Destroy);

    ZETA_Core_StaticAssert(core::seq_cntr::IsSeqCntr<CircularArray>);

    return *ca;
}

constexpr void Destroy(void* ca_) {
    CircularArray* ca{ static_cast<CircularArray*>(ca_) };

    if (ca == nullptr) { return; }

    delete ca;
}

constexpr void Sanitize(void const*) {}

}  // namespace zeta::core_test::circular_array_utils
