#pragma once

#include <zeta/core/debug_utils/logging.ipp>
#include <zeta/core/debug_utils/revision.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core::debug_utils {

namespace revision::detail {

inline Stamp seed_{ 0xA5A5A5A5 };

constexpr Stamp Rotate_(Stamp stamp, unsigned long long salt) {
#if 64 <= ZETA_Core_ullong_width
    constexpr Stamp lcg_mul{ 0x1010101ULL };
    constexpr Stamp lcg_inc{ 0x2492492492492479ULL };
#elif 32 <= ZETA_Core_ullong_width
    constexpr Stamp lcg_mul{ 0x1010101ULL };
    constexpr Stamp lcg_inc{ 0x24924907ULL };
#else
#error "Unsupported architecture."
#endif

    stamp ^= salt;

    stamp += __builtin_readcyclecounter();

    stamp = stamp * lcg_mul + lcg_inc;

    stamp ^= salt;

    return stamp;
}

}  // namespace revision::detail

constexpr revision::Stamp revision::GetStamp() {
    detail::seed_ = detail::Rotate_(detail::seed_, 0x3F3F3F3F);

    Stamp ret{ detail::seed_ };

    detail::seed_ = detail::Rotate_(detail::seed_, 0xC1C1C1C1);

    return ret;
}

}  // namespace zeta::core::debug_utils
