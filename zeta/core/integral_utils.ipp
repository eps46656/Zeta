#pragma once

#include <zeta/core/comparison.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral_utils.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <typename Integral>
constexpr Integral integral_utils::FromString(char const* str) {
    ZETA_Core_DebugAssert(*str != '\0');

    bool is_neg{ false };

    if (*str == '-') {
        is_neg = true;
        ++str;
    } else if (*str == '+') {
        ++str;
    }

    ZETA_Core_DebugAssert(*str != '\0');

    Integral ret{ 0 };

    while (*str == '\'') { ++str; }

    if (*str == '0') {
        switch (*(++str)) {
        case 'b':
            for (;;) {
                char c{ *(++str) };
                if (c == '\'') { continue; }
                if (c == '\0') { break; }

                ZETA_Core_DebugAssert('0' <= c && c <= '1');

                ret *= 2;
                ret += static_cast<Integral>(c - '0');
            }

            return is_neg ? -ret : ret;

        case 'o':
            for (;;) {
                char c{ *(++str) };
                if (c == '\'') { continue; }
                if (c == '\0') { break; }

                ZETA_Core_DebugAssert('0' <= c && c <= '7');

                ret *= 8;
                ret += static_cast<Integral>(c - '0');
            }

            return is_neg ? -ret : ret;

        case 'd':
            for (;;) {
                char c{ *(++str) };
                if (c == '\'') { continue; }
                if (c == '\0') { break; }

                ZETA_Core_DebugAssert('0' <= c && c <= '9');

                ret *= 10;
                ret += static_cast<Integral>(c - '0');
            }

            return is_neg ? -ret : ret;

        case 'x':
            for (;;) {
                char c{ *(++str) };
                if (c == '\'') { continue; }
                if (c == '\0') { break; }

                ZETA_Core_DebugAssert(('0' <= c && c <= '9') ||
                                      ('a' <= c && c <= 'f') ||
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

        ZETA_Core_DebugAssert('0' <= c && c <= '9');

        ret *= 10;
        ret += static_cast<Integral>(c - '0');
    }

    return is_neg ? -ret : ret;
}

template <integral::IsIntegral IntegralDst, integral::IsIntegral IntegralA,
          integral::IsIntegral IntegralB>
constexpr IntegralDst integral_utils::OverflowAdd(IntegralA a, IntegralB b,
                                                  bool& no_lossy) {
    IntegralDst dst;

    no_lossy &= !__builtin_add_overflow(a, b, &dst);

    return dst;
}

template <integral::IsIntegral IntegralDst, integral::IsIntegral IntegralA,
          integral::IsIntegral IntegralB>
constexpr IntegralDst integral_utils::OverflowSub(IntegralA a, IntegralB b,
                                                  bool& no_lossy) {
    IntegralDst dst;

    no_lossy &= !__builtin_sub_overflow(a, b, &dst);

    return dst;
}

template <integral::IsIntegral IntegralDst, integral::IsIntegral IntegralA,
          integral::IsIntegral IntegralB>
constexpr IntegralDst integral_utils::OverflowMul(IntegralA a, IntegralB b,
                                                  bool& no_lossy) {
    IntegralDst dst;

    no_lossy &= !__builtin_mul_overflow(a, b, &dst);

    return dst;
}

template <integral::IsIntegral IntegralA, integral::IsIntegral IntegralB>
constexpr comparison::Ordering integral_utils::MathCompare(IntegralA a,
                                                           IntegralB b) {
    constexpr bool a_is_signed{ integral::IsSignedIntegral<IntegralA> };
    constexpr bool b_is_signed{ integral::IsSignedIntegral<IntegralB> };

    if constexpr (a_is_signed == b_is_signed) {
        return comparison::BasicCompare(
            meta::AutoValueWrapper<comparison::Op::Order>{}, a, b);
    }

    if constexpr (a_is_signed && !b_is_signed) {
        return a < 0 ? comparison::Ordering::Less
                     : comparison::BasicCompare(
                           meta::AutoValueWrapper<comparison::Op::Order>{},
                           static_cast<integral::MakeUnsignedOf<IntegralA>>(a),
                           b);
    }

    if constexpr (!a_is_signed && b_is_signed) {
        return b < 0 ? comparison::Ordering::Greater
                     : comparison::BasicCompare(
                           meta::AutoValueWrapper<comparison::Op::Order>{}, a,
                           static_cast<integral::MakeUnsignedOf<IntegralB>>(b));
    }

    ZETA_Core_Unreachable();
}

template <integral::IsIntegral DstIntegral, integral::IsIntegral SrcIntegral>
constexpr DstIntegral integral_utils::LossyDetectingCast(SrcIntegral const& a,
                                                         bool& no_lossy) {
    DstIntegral dst{ static_cast<DstIntegral>(a) };

    no_lossy &= (MathCompare)(integral::RangeMinOf<DstIntegral>, a) <= 0 &&
                (MathCompare)(a, integral::RangeMaxOf<DstIntegral>) <= 0;

    return dst;
}

template <integral::IsIntegral Integral>
constexpr Integral integral_utils::LossyDetectingAdd(Integral const& a,
                                                     Integral const& b,
                                                     bool& no_lossy) {
    Integral dst;

    no_lossy &= !__builtin_add_overflow(a, b, &dst);

    return dst;
}

template <integral::IsIntegral Integral>
constexpr Integral integral_utils::LossyDetectingSub(Integral const& a,
                                                     Integral const& b,
                                                     bool& no_lossy) {
    Integral dst;

    no_lossy &= !__builtin_sub_overflow(a, b, &dst);

    return dst;
}

template <integral::IsIntegral Integral>
constexpr Integral integral_utils::LossyDetectingMul(Integral const& a,
                                                     Integral const& b,
                                                     bool& no_lossy) {
    Integral dst;

    no_lossy &= !__builtin_mul_overflow(a, b, &dst);

    return dst;
}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<
    Integral>::LossyDetectingIntegral()
    : value{ 0 }, no_lossy{ true } {}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<
    Integral>::LossyDetectingIntegral(Integral const& value)
    : value{ value }, no_lossy{ true } {}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<
    Integral>::LossyDetectingIntegral(Integral const& value, bool no_lossy)
    : value{ value }, no_lossy{ no_lossy } {}

template <integral::IsIntegral Integral>
template <integral::IsIntegral OtherIntegral>
constexpr integral_utils::LossyDetectingIntegral<Integral>::
    LossyDetectingIntegral(LossyDetectingIntegral<OtherIntegral> const& other) {
    this->no_lossy = false;

    this->value = (LossyDetectingCast<Integral, OtherIntegral>)(other.value,
                                                                this->no_lossy);
}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<Integral>
integral_utils::operator+(LossyDetectingIntegral<Integral> const& a) {
    return a;
}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<Integral>
integral_utils::operator-(LossyDetectingIntegral<Integral> const& a) {
    return { -a.value, a.no_lossy };
}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<Integral>
integral_utils::operator+(LossyDetectingIntegral<Integral> const& a,
                          LossyDetectingIntegral<Integral> const& b) {
    LossyDetectingIntegral<Integral> dst{ .no_lossy =
                                              a.no_lossy && b.no_lossy };

    dst.value = (LossyDetectingAdd)(a.value, b.value, dst.no_lossy);

    return dst;
}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<Integral>
integral_utils::operator-(LossyDetectingIntegral<Integral> const& a,
                          LossyDetectingIntegral<Integral> const& b) {
    LossyDetectingIntegral<Integral> dst{ .no_lossy =
                                              a.no_lossy && b.no_lossy };

    dst.value = (LossyDetectingSub)(a.value, b.value, dst.no_lossy);

    return dst;
}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<Integral>
integral_utils::operator*(LossyDetectingIntegral<Integral> const& a,
                          LossyDetectingIntegral<Integral> const& b) {
    LossyDetectingIntegral<Integral> dst{ .no_lossy =
                                              a.no_lossy && b.no_lossy };

    dst.value = (LossyDetectingMul)(a.value, b.value, dst.no_lossy);

    return dst;
}

template <integral::IsIntegral Integral>
constexpr integral_utils::LossyDetectingIntegral<Integral>
integral_utils::operator/(LossyDetectingIntegral<Integral> const& a,
                          LossyDetectingIntegral<Integral> const& b) {
    return { .value = a.value / b.value,
             .no_lossy = b.value != 0 && a.no_lossy && b.no_lossy };
}

}  // namespace zeta::core
