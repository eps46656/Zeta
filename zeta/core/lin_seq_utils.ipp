#pragma once

#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/lin_seq_utils.hpp>

namespace zeta::core {

template <typename Elem>
constexpr void lin_seq_utils::Check(Elem* data, size_t elem_stride,
                                    size_t cnt) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_stride % alignof(Elem) == 0);

    if (cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(elem_stride == 0 ||
                                            sizeof(Elem) <= elem_stride);
}

template <typename Elem>
constexpr void lin_seq_utils::Check(Elem* data, ptrdiff_t elem_stride,
                                    size_t cnt) {
    if (elem_stride % static_cast<ptrdiff_t>(alignof(Elem)) != 0) {
        ZETA_Core_DebugUtils_Diag_LogVar(elem_stride);
        ZETA_Core_DebugUtils_Diag_LogVar(alignof(Elem));

        ZETA_Core_DebugUtils_Diag_PromiseAssert(
            elem_stride % static_cast<ptrdiff_t>(alignof(Elem)) == 0);
    }

    if (cnt == 0) { return; }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(data != nullptr);

    ZETA_Core_DebugUtils_Diag_PromiseAssert(
        elem_stride == 0 || sizeof(Elem) <= static_cast<size_t>(elem_stride) ||
        sizeof(Elem) <= static_cast<size_t>(-elem_stride));
}

}  // namespace zeta::core
