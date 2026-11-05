#pragma once

#include <zeta/core/quad_cap_sys.hpp>

namespace zeta::core {

template <integral::IsUnsignedIntegralType Flag, Flag empty_cap_flag,
          Flag full_cap_flag>
constexpr bool quad_cap_sys::CheckFlag(
    Config<Flag, empty_cap_flag, full_cap_flag>, Flag cap_flag) {
    if ((cap_flag & empty_cap_flag) != empty_cap_flag) { return false; }
    if ((cap_flag | full_cap_flag) != full_cap_flag) { return false; }

    return true;
}

template <integral::IsUnsignedIntegralType Flag, Flag empty_cap_flag,
          Flag full_cap_flag>
constexpr bool quad_cap_sys::CheckFlags(
    Config<Flag, empty_cap_flag, full_cap_flag> config, Flag enabled_cap_flag,
    Flag disabled_cap_flag) {
    if (!(CheckFlag)(config, enabled_cap_flag)) { return false; }
    if (!(CheckFlag)(config, disabled_cap_flag)) { return false; }

    if ((enabled_cap_flag & disabled_cap_flag) != empty_cap_flag) {
        return false;
    }

    return true;
}

template <integral::IsUnsignedIntegralType Flag, Flag empty_cap_flag,
          Flag full_cap_flag>
constexpr bool quad_cap_sys::CheckQuadFlags(
    Config<Flag, empty_cap_flag, full_cap_flag> config,
    Flag static_enabled_cap_flag, Flag static_disabled_cap_flag,
    Flag dynamic_enabled_cap_flag, Flag dynamic_disabled_cap_flag) {
    Flag cap_flags[]{
        static_enabled_cap_flag,
        static_disabled_cap_flag,
        dynamic_enabled_cap_flag,
        dynamic_disabled_cap_flag,
    };

    for (Flag cap_flag : cap_flags) {
        if (!(CheckFlag)(config, cap_flag)) { return false; }
    }

    Flag acc{ empty_cap_flag };

    for (Flag cap_flag : cap_flags) {
        if ((acc & cap_flag) != empty_cap_flag) { return false; }
        acc |= cap_flag;
    }

    if (acc != full_cap_flag) { return false; }

    return true;
}

}  // namespace zeta::core
