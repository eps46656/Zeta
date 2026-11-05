#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils/diag.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/fixed_point.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

ZETA_Core_ClangdPreambleBarrier;

#pragma push_macro("FixedPointTplParamList")
#define FixedPointTplParamList(prefix, suffix)                             \
    bool prefix##is_signed##suffix, size_t prefix##integral_width##suffix, \
        size_t prefix##fraction_width##suffix

#pragma push_macro("FixedPointTplArgList")
#define FixedPointTplArgList(prefix) \
    prefix##is_signed, prefix##integral_width, prefix##fraction_width

namespace zeta::core {

template <FixedPointTplParamList(, )>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromValue(Value value) {
    FixedPoint ret;
    ret.value = value;
    return ret;
}

template <FixedPointTplParamList(, )>
template <integral::IsIntegralType Integral>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromIntegral(
    Integral integral) {
    return static_cast<FixedPoint<FixedPointTplArgList()>>(
        (FromIntegral)(integral));
}

template <FixedPointTplParamList(, )>
template <integral::IsIntegralType Num, integral::IsIntegralType Denom>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromFraction(Num num,
                                                              Denom denom) {
    return static_cast<FixedPoint<FixedPointTplArgList()>>(
        (FromFraction)(meta::AutoValueWrapper<fraction_width>{}, num, denom));
}

template <FixedPointTplParamList(, )>
template <integral::IsIntegralType Integral>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>::FixedPoint(
    Integral const& integral) {
    *this = (FromIntegral)(integral);
}

template <FixedPointTplParamList(, )>
template <FixedPointTplParamList(src_, )>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>::FixedPoint(
    FixedPoint<FixedPointTplArgList(src_)> const& src) {
    *this = src;
}

template <FixedPointTplParamList(, )>
template <integral::IsIntegralType Integral>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>&
fixed_point::FixedPoint<FixedPointTplArgList()>::operator=(
    Integral const& integral) {
    return *this = (FromIntegral)(integral);
}

template <FixedPointTplParamList(, )>
template <FixedPointTplParamList(src_, )>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>&
fixed_point::FixedPoint<FixedPointTplArgList()>::operator=(
    FixedPoint<FixedPointTplArgList(src_)> const& src) {
    constexpr size_t op_total_width{ comparison_utils::BasicMax(
        integral_width + fraction_width,
        src_integral_width + src_fraction_width) };

    using OpIntegral = unsigned _BitInt(op_total_width);

    bool is_neg{ src_is_signed && src.value < 0 };

    OpIntegral op_src_value{ static_cast<OpIntegral>(is_neg ? -src.value
                                                            : src.value) };

    if constexpr (src_fraction_width < fraction_width) {
        op_src_value <<= fraction_width - src_fraction_width;
    } else if constexpr (src_fraction_width > fraction_width) {
        constexpr size_t shift{ src_fraction_width - fraction_width };

        constexpr OpIntegral k{ static_cast<OpIntegral>(1) << shift };

        op_src_value += k / 2;

        if ((op_src_value & (k - 1)) == k / 2) {
            op_src_value &= ~static_cast<OpIntegral>(1);
        }

        op_src_value >>= shift;
    }

    op_src_value =
        !is_signed && is_neg
            ? 0
            : comparison_utils::BasicMin(
                  op_src_value,
                  static_cast<OpIntegral>(integral::RangeMaxOf<Value>));

    this->value = is_neg ? -static_cast<Value>(op_src_value)
                         : static_cast<Value>(op_src_value);

    return *this;
}

template <FixedPointTplParamList(, )>
constexpr auto fixed_point::FixedPoint<FixedPointTplArgList()>::Floor() const {
    using Value = FixedPoint<FixedPointTplArgList()>::Value;

    using RIntegral =
        meta::Conditional<is_signed, signed _BitInt(integral_width + 1),
                          unsigned _BitInt(integral_width)>;

    constexpr Value mod{ static_cast<Value>(1) << fraction_width };

    RIntegral k{ static_cast<RIntegral>(static_cast<Value>(this->value) /
                                        mod) };

    if constexpr (is_signed) {
        if (this->value < 0 && static_cast<Value>(this->value) % mod != 0) {
            --k;
        }
    }

    return k;
}

template <FixedPointTplParamList(, )>
constexpr auto fixed_point::FixedPoint<FixedPointTplArgList()>::Ceil() const {
    using Value = FixedPoint<FixedPointTplArgList()>::Value;

    using RIntegral =
        meta::Conditional<is_signed, signed _BitInt(integral_width + 1),
                          unsigned _BitInt(integral_width + 1)>;

    Value mod{ static_cast<Value>(1) << fraction_width };

    RIntegral k{ static_cast<RIntegral>(static_cast<Value>(this->value) /
                                        mod) };

    if (0 < this->value && static_cast<Value>(this->value) % mod != 0) { ++k; }

    return k;
}

template <FixedPointTplParamList(a_, ), FixedPointTplParamList(b_, )>
constexpr auto fixed_point::operator+(
    FixedPoint<FixedPointTplArgList(a_)> const& a,
    FixedPoint<FixedPointTplArgList(b_)> const& b) {
    /*

    unsigned a + unsigned b
    [0, 2^a) + [0, 2^b)
    <=
    [0, 2^a + 2^b)
    <=
    [0, 2^{max(a, b) + 1})
    <=
    [0, 2^w)        w >= max(a + 1, b + 1)

    unsigned a + signed b
    [0, 2^a) + (-2^{b - 1}, 2^{b - 1})
    <=
    [-2^{b - 1}, 2^{max(a, b - 1) + 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= b - 1      w - 1 >= max(a, b - 1) + 1
    w >= b              w >= max(a + 2, b + 1)

    signed a + unsigned b
    (-2^{a - 1}, 2^{a - 1}) + [0, 2^b)

    w >= max(a + 1, b + 2)

    signed a + signed b
    (-2^{a - 1}, 2^{a - 1}) + (-2^{b - 1}, 2^{b - 1})
    <=
    (-2^{max(a - 1, b - 1) + 1}, 2^{max(a - 1, b - 1) + 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= max(a - 1, b - 1) + 1
    w >= max(a + 1, b + 1)

    */

    constexpr bool r_is_signed{ a_is_signed || b_is_signed };

    constexpr size_t r_integral_width{ comparison_utils::BasicMax(
                                           a_integral_width + a_is_signed,
                                           b_integral_width + b_is_signed) -
                                       (a_is_signed && b_is_signed) + 1 };

    constexpr size_t r_fraction_width{ comparison_utils::BasicMax(
        a_fraction_width, b_fraction_width) };

    using RFixedPoint =
        FixedPoint<r_is_signed, r_integral_width, r_fraction_width>;

    return RFixedPoint::FromValue(
        static_cast<RFixedPoint::Value>(a.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::fraction_width - a_fraction_width)) +
        static_cast<RFixedPoint::Value>(b.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::fraction_width - b_fraction_width)));
}

template <FixedPointTplParamList(a_, ), FixedPointTplParamList(b_, )>
constexpr auto fixed_point::operator-(
    FixedPoint<FixedPointTplArgList(a_)> const& a,
    FixedPoint<FixedPointTplArgList(b_)> const& b) {
    /*

    unsigned a - unsigned b
    [0, 2^a) - [0, 2^b)
    <=
    (-2^b, 2^a)
    <=
    (-2^{w - 1}, 2^{w - 1})         w >= max(a + 1, b + 1)

    unsigned a - signed b
    [0, 2^a) - (-2^{b - 1}, 2^{b - 1})
    <=
    (-2^{b - 1}, 2^a + 2^{b - 1})
    <=
    (-2^{b - 1}, 2^{max(a, b - 1) + 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= b - 1      w - 1 >= max(a, b - 1) + 1
    w >= b              w >= max(a + 2, b + 1)

    signed a - unsigned b
    (-2^{a - 1}, 2^{a - 1}) - [0, 2^b)
    <=
    (-2^{a - 1} - 2^b, 2^{a - 1})
    <=
    (-2^{max(a - 1, b) + 1}, 2^{a - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= max(a - 1, b) + 1      w - 1 >= a - 1
    w >= max(a + 1, b + 2)          w >= a

    signed a - signed b
    (-2^{a - 1}, 2^{a - 1}) - (-2^{b - 1}, 2^{b - 1})
    <=
    (-2^{a - 1} - 2^{b - 1}, 2^{a - 1} + 2^{b - 1})
    <=
    (-2^{max(a - 1, b - 1) + 1}, 2^{max(a - 1, b - 1) + 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= max(a - 1, b - 1) + 1
    w >= max(a + 1, b + 1)

    */

    constexpr bool r_is_signed{ true };

    constexpr size_t r_integral_width{ comparison_utils::BasicMax(
                                           a_integral_width + a_is_signed,
                                           b_integral_width + b_is_signed) -
                                       (a_is_signed && b_is_signed) + 1 };

    constexpr size_t r_fraction_width{ comparison_utils::BasicMax(
        a_fraction_width, b_fraction_width) };

    using RFixedPoint =
        FixedPoint<r_is_signed, r_integral_width, r_fraction_width>;

    return RFixedPoint::FromValue(
        static_cast<RFixedPoint::Value>(a.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::fraction_width - a_fraction_width)) -
        static_cast<RFixedPoint::Value>(b.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::fraction_width - b_fraction_width)));
}

template <FixedPointTplParamList(a_, ), FixedPointTplParamList(b_, )>
constexpr auto fixed_point::operator*(
    FixedPoint<FixedPointTplArgList(a_)> const& a,
    FixedPoint<FixedPointTplArgList(b_)> const& b) {
    /*

    unsigned a * unsigned b
    [0, 2^a) * [0, 2^b)
    <=
    [0, 2^{a + b})
    <=
    [0, 2^{w})         w >= a + b

    unsigned a * signed b
    [0, 2^a) * (-2^{b - 1}, 2^{b - 1})
    <=
    (-2^{a + b - 1}, 2^{a + b - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= a + b - 1
    w >= a + b

    signed a * unsigned b
    (-2^{a - 1}, 2^{a - 1}) * [0, 2^b)

    w >= a + b

    signed a * signed b
    (-2^{a - 1}, 2^{a - 1}) * (-2^{b - 1}, 2^{b - 1})
    <=
    (-2^{a - 1 + b - 1}, 2^{a - 1 + b - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= a - 1 + b - 1
    w >= a + b - 1

    */

    constexpr bool r_is_signed{ a_is_signed || b_is_signed };

    constexpr size_t r_integral_width{ a_integral_width + b_integral_width -
                                       (a_is_signed && b_is_signed) };

    constexpr size_t r_fraction_width = { a_fraction_width + b_fraction_width };

    using FixedPointRet =
        FixedPoint<r_is_signed, r_integral_width, r_fraction_width>;

    return FixedPointRet::FromValue(static_cast<FixedPointRet::Value>(a.value) *
                                    static_cast<FixedPointRet::Value>(b.value));
}

template <FixedPointTplParamList(a_, ), FixedPointTplParamList(b_, )>
constexpr auto fixed_point::operator/(
    FixedPoint<FixedPointTplArgList(a_)> const& a,
    FixedPoint<FixedPointTplArgList(b_)> const& b) {
    return (FromFraction)(meta::AutoValueWrapper<comparison_utils::BasicMax(
                              a_fraction_width, b_fraction_width)>{},
                          a.value, b.value);
}

template <size_t fraction_width, integral::IsIntegralType Num,
          integral::IsIntegralType Denom>
constexpr auto fixed_point::FromFraction(
    meta::ValueWrapper<size_t, fraction_width>, Num num, Denom denom) {
    /*

    unsigned num / unsigned denom
    [0, 2^a) / [1, 2^b)
    <=
    [0, 2^a)
    <=
    [0, 2^w)

    w >= a

    unsigned num / signed denom
    [0, 2^a) / (-2^{b - 1}, 2^{b - 1})
    <=
    (-2^a, 2^a)
    <=
    (-2^{w - 1}, 2^{w - 1})

    w >= a + 1

    signed num / unsigned denom
    (-2^{a - 1}, 2^{a - 1}) / [1, 2^b)
    <=
    (-2^{a - 1}, 2^{a - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w >= a + 1

    signed num / signed denom
    (-2^{a - 1}, 2^{a - 1}) / (-2^{b - 1}, 2^{b - 1})
    <=
    (-2^{a - 1}, 2^{a - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w >= a + 1

    */

    ZETA_Core_DebugUtils_Diag_PromiseAssert(denom != 0);

    constexpr bool any_signed{ integral::IsSignedIntegralType<Num> ||
                               integral::IsSignedIntegralType<Denom> };

    constexpr bool r_is_signed{ any_signed };

    constexpr size_t r_integral_width{ integral::WidthOf<Num> + any_signed };

    constexpr size_t r_fraction_width{ fraction_width };

    using RFixedPoint =
        FixedPoint<r_is_signed, r_integral_width, r_fraction_width>;

    using OpIntegral = unsigned _BitInt(r_integral_width + r_fraction_width);

    constexpr Num num_0{ static_cast<Num>(0) };

    bool num_is_neg{ integral::IsSignedIntegralType<Num> && num < num_0 };
    bool denom_is_neg{ integral::IsSignedIntegralType<Denom> && denom < num_0 };
    bool is_neg{ num_is_neg != denom_is_neg };

    OpIntegral a{ static_cast<OpIntegral>(num_is_neg ? -num : num)
                  << fraction_width };

    OpIntegral b{ static_cast<OpIntegral>(denom_is_neg ? -denom : denom) };

    OpIntegral q{ a / b };
    OpIntegral r{ a % b };

    if (r != 0) {
        if (b / 2 < r) {
            ++q;
        } else if (b / 2 == r && b % 2 == 0) {
            q += q % 2;
        }
    }

    return RFixedPoint::FromValue(is_neg ? -static_cast<RFixedPoint::Value>(q)
                                         : static_cast<RFixedPoint::Value>(q));
}

template <integral::IsIntegralType Integral>
constexpr auto fixed_point::FromIntegral(Integral integral) {
    return (FromFraction)(meta::ValueWrapper<size_t, 0>{}, integral,
                          static_cast<Integral>(1));
}

template <comparison::IsOpTag OpTag, FixedPointTplParamList(a_, ),
          FixedPointTplParamList(b_, )>
constexpr auto fixed_point::MathCompare(
    OpTag op, FixedPoint<FixedPointTplArgList(a_)> const& a,
    FixedPoint<FixedPointTplArgList(b_)> const& b) {
    if constexpr (a_is_signed && !b_is_signed) {
        if (a.value < 0) {
            return (meta::EnumToUnderlying(op) & comparison::Bits::less) != 0;
        }
    }

    if constexpr (!a_is_signed && b_is_signed) {
        if (b.value < 0) {
            return (meta::EnumToUnderlying(op) & comparison::Bits::greater) !=
                   0;
        }
    }

    constexpr size_t op_fraction_width{ comparison_utils::BasicMax(
        a_fraction_width, b_fraction_width) };

    constexpr size_t op_integral_width{
        1 + comparison_utils::BasicMax(a_integral_width, b_integral_width)
    };

    constexpr size_t op_total_width{ op_integral_width + op_fraction_width };

    using SOpIntegral = signed _BitInt(op_total_width);

    SOpIntegral x{ static_cast<SOpIntegral>(a.value)
                   << (op_fraction_width - a_fraction_width) };

    SOpIntegral y{ static_cast<SOpIntegral>(b.value)
                   << (op_fraction_width - b_fraction_width) };

    return comparison::BasicCompare(op, x, y);
}

template <FixedPointTplParamList(a_, ), FixedPointTplParamList(b_, )>
template <comparison::IsOpTag OpTag>
constexpr auto
comparison::BasicComparator<fixed_point::FixedPoint<FixedPointTplArgList(a_)>,
                            fixed_point::FixedPoint<FixedPointTplArgList(b_)>>::
    Compare(comparison::Tag, OpTag op,
            fixed_point::FixedPoint<FixedPointTplArgList(a_)> const& a,
            fixed_point::FixedPoint<FixedPointTplArgList(b_)> const& b) {
    return fixed_point::MathCompare(op, a, b);
}

}  // namespace zeta::core

#pragma pop_macro("FixedPointTplParamList")
#pragma pop_macro("FixedPointTplArgList")
