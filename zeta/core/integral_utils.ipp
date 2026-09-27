#pragma once

#include <zeta/core/comparison.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename Integral>
constexpr Integral integral_utils::FromString(char const* str) {
    ZETA_Core_DebugUtils_Diag_PromiseAssert(*str != '\0');

    bool is_neg{ false };

    if (*str == '-') {
        is_neg = true;
        ++str;
    } else if (*str == '+') {
        ++str;
    }

    ZETA_Core_DebugUtils_Diag_PromiseAssert(*str != '\0');

    Integral ret{ 0 };

    while (*str == '\'') { ++str; }

    if (*str == '0') {
        switch (*(++str)) {
        case 'b':
            for (;;) {
                char c{ *(++str) };
                if (c == '\'') { continue; }
                if (c == '\0') { break; }

                ZETA_Core_DebugUtils_Diag_PromiseAssert('0' <= c && c <= '1');

                ret *= 2;
                ret += static_cast<Integral>(c - '0');
            }

            return is_neg ? -ret : ret;

        case 'o':
            for (;;) {
                char c{ *(++str) };
                if (c == '\'') { continue; }
                if (c == '\0') { break; }

                ZETA_Core_DebugUtils_Diag_PromiseAssert('0' <= c && c <= '7');

                ret *= 8;
                ret += static_cast<Integral>(c - '0');
            }

            return is_neg ? -ret : ret;

        case 'd':
            for (;;) {
                char c{ *(++str) };
                if (c == '\'') { continue; }
                if (c == '\0') { break; }

                ZETA_Core_DebugUtils_Diag_PromiseAssert('0' <= c && c <= '9');

                ret *= 10;
                ret += static_cast<Integral>(c - '0');
            }

            return is_neg ? -ret : ret;

        case 'x':
            for (;;) {
                char c{ *(++str) };
                if (c == '\'') { continue; }
                if (c == '\0') { break; }

                ZETA_Core_DebugUtils_Diag_PromiseAssert(
                    ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
                    ('A' <= c && c <= 'F'));

                ret *= 16;

                if ('0' <= c && c <= '9') {
                    ret += static_cast<Integral>(c - '0');
                } else if ('a' <= c && c <= 'f') {
                    ret += static_cast<Integral>(c - 'a' + 10);
                } else {
                    ret += static_cast<Integral>(c - 'A' + 10);
                }
            }

            return is_neg ? -ret : ret;
        }
    }

    for (;; ++str) {
        char c{ *str };
        if (c == '\'') { continue; }
        if (c == '\0') { break; }

        ZETA_Core_DebugUtils_Diag_PromiseAssert('0' <= c && c <= '9');

        ret *= 10;
        ret += static_cast<Integral>(c - '0');
    }

    return is_neg ? -ret : ret;
}

}  // namespace zeta::core
