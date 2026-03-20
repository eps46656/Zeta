

#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/fixed_point.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/utils.ipp>

#pragma push_macro("FixedPointTplParamList")
#define FixedPointTplParamList(prefix)                          \
    typename prefix##SignedTag, typename prefix##IntegralWidth, \
        typename prefix##FractionWidth

#pragma push_macro("FixedPointTplArgList")
#define FixedPointTplArgList(prefix) \
    prefix##SignedTag, prefix##IntegralWidth, prefix##FractionWidth

namespace zeta::core {

template <FixedPointTplParamList()>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromValue(Value value) {
    FixedPoint ret;
    ret.value = value;
    return ret;
}

template <FixedPointTplParamList()>
template <typename Integral>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromIntegral(
    Integral integral) {
    return static_cast<fixed_point::FixedPoint<FixedPointTplArgList()>>(
        fixed_point::FromIntegral(integral));
}

template <FixedPointTplParamList()>
template <typename Num, typename Denom>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromFraction(Num num,
                                                              Denom denom) {
    return static_cast<fixed_point::FixedPoint<FixedPointTplArgList()>>(
        fixed_point::FromFraction<FractionWidth>(num, denom));
}

template <FixedPointTplParamList()>
template <typename Integral, typename>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>::FixedPoint(
    Integral const& integral) {
    *this = (FromIntegral)(integral);
}

template <FixedPointTplParamList()>
template <FixedPointTplParamList(Src)>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>::FixedPoint(
    FixedPoint<FixedPointTplArgList(Src)> const& src) {
    *this = src;
}

template <FixedPointTplParamList()>
template <typename Integral, typename>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>&
fixed_point::FixedPoint<FixedPointTplArgList()>::operator=(
    Integral const& integral) {
    return *this = (FromIntegral)(integral);
}

template <FixedPointTplParamList()>
template <FixedPointTplParamList(Src)>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>&
fixed_point::FixedPoint<FixedPointTplArgList()>::operator=(
    FixedPoint<FixedPointTplArgList(Src)> const& src) {
    constexpr size_t op_total_width{ utils::Max(
        IntegralWidth::value + FractionWidth::value,
        SrcIntegralWidth::value + SrcFractionWidth::value) };

    using OpIntegral = unsigned _BitInt(op_total_width);

    bool is_neg{ SrcSignedTag::value && src.value < 0 };

    OpIntegral op_src_value{ static_cast<OpIntegral>(is_neg ? -src.value
                                                            : src.value) };

    if constexpr (SrcFractionWidth::value < FractionWidth::value) {
        op_src_value <<= FractionWidth::value - SrcFractionWidth::value;
    } else if constexpr (SrcFractionWidth::value > FractionWidth::value) {
        constexpr size_t shift{ SrcFractionWidth::value -
                                FractionWidth::value };

        constexpr OpIntegral k{ static_cast<OpIntegral>(1) << shift };

        op_src_value += k / 2;

        if ((op_src_value & (k - 1)) == k / 2) {
            op_src_value &= ~static_cast<OpIntegral>(1);
        }

        op_src_value >>= shift;
    }

    op_src_value =
        !SignedTag::value && is_neg
            ? 0
            : utils::Min(op_src_value,
                         static_cast<OpIntegral>(integral::RangeMaxOf<Value>));

    this->value = is_neg ? -static_cast<Value>(op_src_value)
                         : static_cast<Value>(op_src_value);

    return *this;
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool fixed_point::operator==(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    return MathCompare(x, y) == 0;
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool fixed_point::operator!=(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    return MathCompare(x, y) != 0;
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool fixed_point::operator<(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    return MathCompare(x, y) < 0;
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool fixed_point::operator<=(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    return MathCompare(x, y) <= 0;
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool fixed_point::operator>(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    return MathCompare(x, y) > 0;
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool fixed_point::operator>=(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    return MathCompare(x, y) >= 0;
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr auto fixed_point::operator+(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    /*

    unsigned x + unsigned y
    [0, 2^x) + [0, 2^y)
    <=
    [0, 2^x + 2^y)
    <=
    [0, 2^{max(x, y) + 1})
    <=
    [0, 2^w)        w >= max(x + 1, y + 1)

    unsigned x + signed y
    [0, 2^x) + (-2^{y - 1}, 2^{y - 1})
    <=
    [-2^{y - 1}, 2^{max(x, y - 1) + 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= y - 1      w - 1 >= max(x, y - 1) + 1
    w >= y              w >= max(x + 2, y + 1)

    signed x + unsigned y
    (-2^{x - 1}, 2^{x - 1}) + [0, 2^y)

    w >= max(x + 1, y + 2)

    signed x + signed y
    (-2^{x - 1}, 2^{x - 1}) + (-2^{y - 1}, 2^{y - 1})
    <=
    (-2^{max(x - 1, y - 1) + 1}, 2^{max(x - 1, y - 1) + 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= max(x - 1, y - 1) + 1
    w >= max(x + 1, y + 1)

    */

    using RSignedTag =
        value_wrapper::StaticValueWrapper<bool, XSignedTag::value ||
                                                    YSignedTag::value>;

    using RIntegralWidth = value_wrapper::StaticValueWrapper<
        size_t, utils::Max(XIntegralWidth::value + XSignedTag::value,
                           YIntegralWidth::value + YSignedTag::value) -
                    (XSignedTag::value && YSignedTag::value) + 1>;

    using RFractionWidth = value_wrapper::StaticValueWrapper<
        size_t, utils::Max(XFractionWidth::value, YFractionWidth::value)>;

    using RFixedPoint = FixedPoint<RSignedTag, RIntegralWidth, RFractionWidth>;

    return RFixedPoint::FromValue(
        static_cast<RFixedPoint::Value>(x.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::FractionWidth::value - XFractionWidth::value)) +
        static_cast<RFixedPoint::Value>(y.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::FractionWidth::value - YFractionWidth::value)));
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr auto fixed_point::operator-(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    /*

    unsigned x - unsigned y
    [0, 2^x) - [0, 2^y)
    <=
    (-2^y, 2^x)
    <=
    (-2^{w - 1}, 2^{w - 1})         w >= max(x + 1, y + 1)

    unsigned x - signed y
    [0, 2^x) - (-2^{y - 1}, 2^{y - 1})
    <=
    (-2^{y - 1}, 2^x + 2^{y - 1})
    <=
    (-2^{y - 1}, 2^{max(x, y - 1) + 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= y - 1      w - 1 >= max(x, y - 1) + 1
    w >= y              w >= max(x + 2, y + 1)

    signed x - unsigned y
    (-2^{x - 1}, 2^{x - 1}) - [0, 2^y)
    <=
    (-2^{x - 1} - 2^y, 2^{x - 1})
    <=
    (-2^{max(x - 1, y) + 1}, 2^{x - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= max(x - 1, y) + 1      w - 1 >= x - 1
    w >= max(x + 1, y + 2)          w >= x

    signed x - signed y
    (-2^{x - 1}, 2^{x - 1}) - (-2^{y - 1}, 2^{y - 1})
    <=
    (-2^{x - 1} - 2^{y - 1}, 2^{x - 1} + 2^{y - 1})
    <=
    (-2^{max(x - 1, y - 1) + 1}, 2^{max(x - 1, y - 1) + 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= max(x - 1, y - 1) + 1
    w >= max(x + 1, y + 1)

    */

    using RSignedTag = value_wrapper::TrueType;

    using RIntegralWidth = value_wrapper::StaticValueWrapper<
        size_t, utils::Max(XIntegralWidth::value + XSignedTag::value,
                           YIntegralWidth::value + YSignedTag::value) -
                    (XSignedTag::value && YSignedTag::value) + 1>;

    using RFractionWidth = value_wrapper::StaticValueWrapper<
        size_t, utils::Max(XFractionWidth::value, YFractionWidth::value)>;

    using RFixedPoint = FixedPoint<RSignedTag, RIntegralWidth, RFractionWidth>;

    return RFixedPoint::FromValue(
        static_cast<RFixedPoint::Value>(x.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::FractionWidth::value - XFractionWidth::value)) -
        static_cast<RFixedPoint::Value>(y.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::FractionWidth::value - YFractionWidth::value)));
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr auto fixed_point::operator*(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    /*

    unsigned x * unsigned y
    [0, 2^x) * [0, 2^y)
    <=
    [0, 2^{x + y})
    <=
    [0, 2^{w})         w >= x + y

    unsigned x * signed y
    [0, 2^x) * (-2^{y - 1}, 2^{y - 1})
    <=
    (-2^{x + y - 1}, 2^{x + y - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= x + y - 1
    w >= x + y

    signed x * unsigned y
    (-2^{x - 1}, 2^{x - 1}) * [0, 2^y)

    w >= x + y

    signed x * signed y
    (-2^{x - 1}, 2^{x - 1}) * (-2^{y - 1}, 2^{y - 1})
    <=
    (-2^{x - 1 + y - 1}, 2^{x - 1 + y - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w - 1 >= x - 1 + y - 1
    w >= x + y - 1

    */

    using RSignedTag =
        value_wrapper::StaticValueWrapper<bool, XSignedTag::value ||
                                                    YSignedTag::value>;

    using RIntegralWidth = value_wrapper::StaticValueWrapper<
        size_t, XIntegralWidth::value + YIntegralWidth::value -
                    (XSignedTag::value && YSignedTag::value)>;

    using RFractionWidth =
        value_wrapper::StaticValueWrapper<size_t, XFractionWidth::value +
                                                      YFractionWidth::value>;

    using FixedPointRet =
        FixedPoint<RSignedTag, RIntegralWidth, RFractionWidth>;

    return FixedPointRet::FromValue(static_cast<FixedPointRet::Value>(x.value) *
                                    static_cast<FixedPointRet::Value>(y.value));
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr auto fixed_point::operator/(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    return (FromFraction<value_wrapper::StaticValueWrapper<
                size_t, utils::Max(XFractionWidth::value,
                                   YFractionWidth::value)>>)(x.value, y.value);
}

template <typename FractionWidth, typename Num, typename Denom>
constexpr auto fixed_point::FromFraction(Num num, Denom denom) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);

    /*

    unsigned num / unsigned denom
    [0, 2^x) / [1, 2^y)
    <=
    [0, 2^x)
    <=
    [0, 2^w)

    w >= x

    unsigned num / signed denom
    [0, 2^x) / (-2^{y - 1}, 2^{y - 1})
    <=
    (-2^x, 2^x)
    <=
    (-2^{w - 1}, 2^{w - 1})

    w >= x + 1

    signed num / unsigned denom
    (-2^{x - 1}, 2^{x - 1}) / [1, 2^y)
    <=
    (-2^{x - 1}, 2^{x - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w >= x + 1

    signed num / signed denom
    (-2^{x - 1}, 2^{x - 1}) / (-2^{y - 1}, 2^{y - 1})
    <=
    (-2^{x - 1}, 2^{x - 1})
    <=
    (-2^{w - 1}, 2^{w - 1})

    w >= x + 1

    */

    ZETA_Core_StaticAssert(integral::IsIntegral<Num>);
    ZETA_Core_StaticAssert(integral::IsIntegral<Denom>);

    ZETA_Core_DebugAssert(denom != 0);

    constexpr bool any_signed{ integral::IsSigned<Num> ||
                               integral::IsSigned<Denom> };

    using RSignedTag = value_wrapper::StaticValueWrapper<bool, any_signed>;

    using RIntegralWidth =
        value_wrapper::StaticValueWrapper<size_t,
                                          integral::WidthOf<Num> + any_signed>;

    using RFractionWidth = FractionWidth;

    using RFixedPoint = FixedPoint<RSignedTag, RIntegralWidth, RFractionWidth>;

    using OpIntegral =
        unsigned _BitInt(RIntegralWidth::value + RFractionWidth::value);

    constexpr Num num_0{ static_cast<Num>(0) };

    bool num_is_neg{ integral::IsSigned<Num> && num < num_0 };
    bool denom_is_neg{ integral::IsSigned<Denom> && denom < num_0 };
    bool is_neg{ num_is_neg != denom_is_neg };

    OpIntegral a{ static_cast<OpIntegral>(num_is_neg ? -num : num)
                  << FractionWidth::value };

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

template <typename Integral>
constexpr auto fixed_point::FromIntegral(Integral integral) {
    return (FromFraction<value_wrapper::StaticValueWrapper<
                size_t, 0>>)(integral, static_cast<Integral>(1));
}

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr int fixed_point::MathCompare(
    FixedPoint<FixedPointTplArgList(X)> const& x,
    FixedPoint<FixedPointTplArgList(Y)> const& y) {
    if constexpr (XSignedTag::value && !YSignedTag::value) {
        if (x.value < 0) { return -1; }
    }

    if constexpr (!XSignedTag::value && YSignedTag::value) {
        if (y.value < 0) { return 1; }
    }

    constexpr size_t op_fraction_width{ utils::Max(XFractionWidth::value,
                                                   YFractionWidth::value) };

    constexpr size_t op_integral_width{ 1 + utils::Max(XIntegralWidth::value,
                                                       YIntegralWidth::value) };

    constexpr size_t op_total_width{ op_integral_width + op_fraction_width };

    using UOpIntegral = unsigned _BitInt(op_total_width);
    using SOpIntegral = signed _BitInt(op_total_width);

    SOpIntegral a{ static_cast<SOpIntegral>(x.value)
                   << (op_fraction_width - XFractionWidth::value) };

    SOpIntegral b{ static_cast<SOpIntegral>(y.value)
                   << (op_fraction_width - YFractionWidth::value) };

    return (b < a) - (a < b);
}

template <FixedPointTplParamList()>
constexpr auto fixed_point::Floor(FixedPoint<FixedPointTplArgList()> const& x) {
    ZETA_Core_StaticAssert(IntegralWidth::value + FractionWidth::value <=
                           ZETA_Core_bitint_max_width);

    using Value = FixedPoint<FixedPointTplArgList()>::Value;

    using RIntegral =
        meta::Conditional<SignedTag::value,
                          signed _BitInt(IntegralWidth::value + 1),
                          unsigned _BitInt(IntegralWidth::value)>;

    constexpr Value mod{ static_cast<Value>(1) << FractionWidth::value };

    RIntegral k{ static_cast<RIntegral>(static_cast<Value>(x.value) / mod) };

    if constexpr (SignedTag::value) {
        if (x.value < 0 && static_cast<Value>(x.value) % mod != 0) { --k; }
    }

    return k;
}

template <FixedPointTplParamList()>
constexpr auto fixed_point::Ceil(FixedPoint<FixedPointTplArgList()> const& x) {
    using Value = FixedPoint<FixedPointTplArgList()>::Value;

    using RIntegral =
        meta::Conditional<SignedTag::value,
                          signed _BitInt(IntegralWidth::value + 1),
                          unsigned _BitInt(IntegralWidth::value + 1)>;

    Value mod{ static_cast<Value>(1) << FractionWidth::value };

    RIntegral k{ static_cast<RIntegral>(static_cast<Value>(x.value) / mod) };

    if (0 < x.value && static_cast<Value>(x.value) % mod != 0) { ++k; }

    return k;
}

}  // namespace zeta::core

#pragma pop_macro("FixedPointTplParamList")
#pragma pop_macro("FixedPointTplArgList")
