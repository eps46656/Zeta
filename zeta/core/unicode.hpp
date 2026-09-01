#pragma once

#include <zeta/core/integral.hpp>

namespace zeta::core::unicode {

using unichar_t = unsigned;

ZETA_Core_StaticAssert(integral::IsUnsignedIntegral<unichar_t>);
ZETA_Core_StaticAssert(21 <= integral::WidthOf<unichar_t>);

constexpr unichar_t surrogate_range_min{ 0xD800 };
constexpr unichar_t surrogate_range_max{ 0xDFFF };

constexpr unichar_t surrogate_h_range_min{ 0xD800 };
constexpr unichar_t surrogate_h_range_max{ 0xDBFF };

constexpr unichar_t surrogate_l_range_min{ 0xDC00 };
constexpr unichar_t surrogate_l_range_max{ 0xDFFF };

constexpr unichar_t codepoint_range_min{ 0x0000 };
constexpr unichar_t codepoint_range_max{ 0x10FFFF };

constexpr unichar_t null_codepoint{ integral::RangeMaxOf<unichar_t> };

}  // namespace zeta::core::unicode
