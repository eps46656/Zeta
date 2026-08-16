#pragma once

#include <zeta/core/array.ipp>
#include <zeta/core/ascii.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core {

namespace ascii::detail {

array::Array<CharEnum, 256> char_table{ []() {
    array::Array<CharEnum, 256> ret{ CharEnum::null };

#pragma push_macro("F")

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define F(name, code) ret[code] = CharEnum::name;

    ZETA_Core_ascii_Char_XMacro(F, )

#pragma pop_macro("F")

        return ret;
}() };

}  // namespace ascii::detail

constexpr ascii::CharEnum ascii::CodeToChar(unsigned char code) {
    if constexpr (detail::char_table.N <= integral::RangeMaxOf<unsigned char>) {
        if (detail::char_table.N <= code) { return CharEnum::null; }
    }

    return detail::char_table.elems[code];
}

}  // namespace zeta::core
