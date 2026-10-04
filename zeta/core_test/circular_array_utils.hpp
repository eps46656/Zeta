#pragma once

#include <cstdlib>
#include <zeta/core/circular_array.hpp>
#include <zeta/core/circular_array.ipp>
#include <zeta/core/seq_cntr.hpp>
#include <zeta/core/seq_cntr.ipp>
#include <zeta/core_test/seq_cntr_utils.hpp>

namespace zeta::core_test::circular_array_utils {

template <core::meta::IsContainerElem Elem>
using CircularArray = core::circular_array::Cntr<Elem>;

template <typename Elem>
constexpr core::poly_seq_cntr::Cntr<Elem> Create(size_t stride,
                                                 size_t slot_cnt) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(sizeof(Elem) <= stride);
    ZETA_Core_DebugUtils_Diag_PromiseAssert(stride % alignof(Elem) == 0);

    auto* ca{ static_cast<CircularArray<Elem>*>(new CircularArray<Elem>{
        .data = static_cast<Elem*>(std::malloc(stride * slot_cnt)),
        .elem_stride = stride,
        .elem_cnt = 0,
        .slot_cnt = slot_cnt,
        .rot = 0,
    }) };

    seq_cntr_utils::AddSanitizeFunc(ca, [](void const*) {});

    seq_cntr_utils::AddDestroyFunc(ca, [](void* ca_) {
        CircularArray<Elem>* ca{ static_cast<CircularArray<Elem>*>(ca_) };

        if (ca == nullptr) { return; }

        delete ca;
    });

    static_assert(core::seq_cntr::IsSeqCntr<CircularArray<Elem>>);

    return *ca;
}

}  // namespace zeta::core_test::circular_array_utils
