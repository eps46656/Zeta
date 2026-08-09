#pragma once

#include <cstdlib>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::circular_array_utils {

using SeqCntrRef = core::seq_cntr_ref::Ref;

namespace CircularArrayNS = core::circular_array;
using CircularArray = CircularArrayNS::Cntr;

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t capacity);

void Destroy(void* ca);

void Sanitize(void const* ca);

template <typename Elem>
SeqCntrRef Create(size_t stride, size_t slot_cnt) {
    ZETA_Core_DebugAssert(sizeof(Elem) <= stride);
    ZETA_Core_DebugAssert(stride % alignof(Elem) == 0);

    auto* ca{ static_cast<CircularArray*>(std::malloc(sizeof(CircularArray))) };

    ca->data = std::malloc(stride * slot_cnt);
    ca->elem_size = sizeof(Elem);
    ca->elem_stride = stride;
    ca->elem_cnt = 0;
    ca->slot_cnt = slot_cnt;
    ca->rot = 0;

    core::seq_cntr::CntrTraits<CircularArray>::Read(
        *ca, nullptr, 0, core::elem_stream::acceptor::EmptyAcceptor{});

    // SeqCntrRef seq_cntr_ref{ *ca };
    SeqCntrRef seq_cntr_ref;

    seq_cntr_utils::AddSanitizeFunc(ca, Sanitize);

    seq_cntr_utils::AddDestroyFunc(ca, Destroy);

    return seq_cntr_ref;
}

inline void Destroy(void* ca_) {
    CircularArray* ca{ static_cast<CircularArray*>(ca_) };

    if (ca == nullptr) { return; }

    // CircularArrayNS::Deinit(*ca);

    delete ca;
}

inline void Sanitize(void const*) {}

}  // namespace zeta::core_test::circular_array_utils
