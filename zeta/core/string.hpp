#pragma once

#include <zeta/core/integral.hpp>

namespace zeta::core::string {

struct EncodingEnum {
    using Value = unsigned char;

    struct ASCII {
        static constexpr Value value{ 0 };
    };

    struct UTF8 {
        static constexpr Value value{ 1 };
    };

    struct UTF16 {
        static constexpr Value value{ 2 };
    };

    struct UTF32 {
        static constexpr Value value{ 3 };
    };
};

}  // namespace zeta::core::string
