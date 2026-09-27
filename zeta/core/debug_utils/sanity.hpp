#pragma once

#if !defined(ZETA_Core_DebugUtils_Sanity_Enable)
#error "ZETA_Core_DebugUtils_Sanity_Enable must be defined"
#endif

#include <zeta/core/debug_utils/logging.hpp>

#if ZETA_Core_DebugUtils_Sanity_Enable

#define ZETA_Core_DebugUtils_Sanity_ImmLogSkipMemMatching() \
    ZETA_Core_DebugUtils_Logging_ImmLogMsg(                 \
        ZETA_Core_DebugUtils_Logging_WarningColorCode       \
        "WARNING" ZETA_Core_DebugUtils_Logging_ValColorCode \
        ": skip memory responsibility check.");

namespace zeta::core::debug_utils::sanity {

enum struct SanityCheckScope : unsigned char {
    Basic = 1,
    Complete = 2,
};

using SanityCheckFunc = void (*)(void const* obj, SanityCheckScope scope);

constexpr size_t GetCurSanityCheckId();

constexpr bool RegisterSanityCheckFunc(void const* obj,
                                       SanityCheckFunc sanity_check_func);

constexpr bool UnregisterSanityCheckFunc(void const* obj);

constexpr void SanityCheck(void const* obj, SanityCheckScope scope);

constexpr void ExpandFinishedSanityCheckScope(void const* obj,
                                              SanityCheckScope scope);

constexpr void DummySanityCheckFunc(void const*, SanityCheckScope);

}  // namespace zeta::core::debug_utils::sanity

#endif
