#pragma once

#include <zeta/core/integral.hpp>

namespace zeta::core::quad_cap_sys {

template <integral::IsUnsignedIntegralType Flag_, Flag_ empty_cap_flag_,
          Flag_ full_cap_flag_>
struct Config {
    static_assert((full_cap_flag_ & empty_cap_flag_) == empty_cap_flag_);

    using Flag = Flag_;

    static constexpr Flag empty_cap_flag{ empty_cap_flag_ };
    static constexpr Flag full_cap_flag{ full_cap_flag_ };
};

template <integral::IsUnsignedIntegralType Flag, Flag empty_cap_flag,
          Flag full_cap_flag>
constexpr bool CheckFlag(Config<Flag, empty_cap_flag, full_cap_flag>,
                         Flag cap_flag);

template <integral::IsUnsignedIntegralType Flag, Flag empty_cap_flag,
          Flag full_cap_flag>
constexpr bool CheckFlags(Config<Flag, empty_cap_flag, full_cap_flag>,
                          Flag enabled_cap_flag, Flag disabled_cap_flag);

template <integral::IsUnsignedIntegralType Flag, Flag empty_cap_flag,
          Flag full_cap_flag>
constexpr bool CheckQuadFlags(Config<Flag, empty_cap_flag, full_cap_flag>,
                              Flag static_enabled_cap_flag,
                              Flag static_disabled_cap_flag,
                              Flag dynamic_enabled_cap_flag,
                              Flag dynamic_disabled_cap_flag);

}  // namespace zeta::core::quad_cap_sys
