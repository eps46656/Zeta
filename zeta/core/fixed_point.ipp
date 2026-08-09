

#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/fixed_point.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

#pragma push_macro("FixedPointTplParamList")
#define FixedPointTplParamList(prefix, suffix)                       \
    meta::IsValueWrapperT<bool> prefix##SignedTag##suffix,           \
        meta::IsValueWrapperT<size_t> prefix##IntegralWidth##suffix, \
        meta::IsValueWrapperT<size_t> prefix##FractionWidth##suffix

#pragma push_macro("FixedPointTplArgList")
#define FixedPointTplArgList(prefix) \
    prefix##SignedTag, prefix##IntegralWidth, prefix##FractionWidth

namespace zeta::core {

template <FixedPointTplParamList(, )>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromValue(Value value) {
    FixedPoint ret;
    ret.value = value;
    return ret;
}

template <FixedPointTplParamList(, )>
template <integral::IsIntegral Integral>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromIntegral(
    Integral integral) {
    return static_cast<fixed_point::FixedPoint<FixedPointTplArgList()>>(
        fixed_point::FromIntegral(integral));
}

template <FixedPointTplParamList(, )>
template <integral::IsIntegral Num, integral::IsIntegral Denom>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>
fixed_point::FixedPoint<FixedPointTplArgList()>::FromFraction(Num num,
                                                              Denom denom) {
    return static_cast<fixed_point::FixedPoint<FixedPointTplArgList()>>(
        fixed_point::FromFraction(FractionWidth{}, num, denom));
}

template <FixedPointTplParamList(, )>
template <integral::IsIntegral Integral>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>::FixedPoint(
    Integral const& integral) {
    *this = (FromIntegral)(integral);
}

template <FixedPointTplParamList(, )>
template <FixedPointTplParamList(Src, )>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>::FixedPoint(
    FixedPoint<FixedPointTplArgList(Src)> const& src) {
    *this = src;
}

template <FixedPointTplParamList(, )>
template <integral::IsIntegral Integral>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>&
fixed_point::FixedPoint<FixedPointTplArgList()>::operator=(
    Integral const& integral) {
    return *this = (FromIntegral)(integral);
}

template <FixedPointTplParamList(, )>
template <FixedPointTplParamList(Src, )>
constexpr fixed_point::FixedPoint<FixedPointTplArgList()>&
fixed_point::FixedPoint<FixedPointTplArgList()>::operator=(
    FixedPoint<FixedPointTplArgList(Src)> const& src) {
    constexpr size_t op_total_width{ comparison_utils::BasicMax(
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
        meta::Conditional<SignedTag::value,
                          signed _BitInt(IntegralWidth::value + 1),
                          unsigned _BitInt(IntegralWidth::value)>;

    constexpr Value mod{ static_cast<Value>(1) << FractionWidth::value };

    RIntegral k{ static_cast<RIntegral>(static_cast<Value>(this->value) /
                                        mod) };

    if constexpr (SignedTag::value) {
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
        meta::Conditional<SignedTag::value,
                          signed _BitInt(IntegralWidth::value + 1),
                          unsigned _BitInt(IntegralWidth::value + 1)>;

    Value mod{ static_cast<Value>(1) << FractionWidth::value };

    RIntegral k{ static_cast<RIntegral>(static_cast<Value>(this->value) /
                                        mod) };

    if (0 < this->value && static_cast<Value>(this->value) % mod != 0) { ++k; }

    return k;
}

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr auto fixed_point::operator+(
    FixedPoint<FixedPointTplArgList(A)> const& a,
    FixedPoint<FixedPointTplArgList(B)> const& b) {
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

    using RSignedTag =
        meta::AutoValueWrapper<ASignedTag::value || BSignedTag::value>;

    using RIntegralWidth =
        meta::AutoValueWrapper<comparison_utils::BasicMax(
                                   AIntegralWidth::value + ASignedTag::value,
                                   BIntegralWidth::value + BSignedTag::value) -
                               (ASignedTag::value && BSignedTag::value) + 1>;

    using RFractionWidth = meta::AutoValueWrapper<comparison_utils::BasicMax(
        AFractionWidth::value, BFractionWidth::value)>;

    using RFixedPoint = FixedPoint<RSignedTag, RIntegralWidth, RFractionWidth>;

    return RFixedPoint::FromValue(
        static_cast<RFixedPoint::Value>(a.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::FractionWidth::value - AFractionWidth::value)) +
        static_cast<RFixedPoint::Value>(b.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::FractionWidth::value - BFractionWidth::value)));
}

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr auto fixed_point::operator-(
    FixedPoint<FixedPointTplArgList(A)> const& a,
    FixedPoint<FixedPointTplArgList(B)> const& b) {
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

    using RSignedTag = meta::AutoValueWrapper<true>;

    using RIntegralWidth =
        meta::AutoValueWrapper<comparison_utils::BasicMax(
                                   AIntegralWidth::value + ASignedTag::value,
                                   BIntegralWidth::value + BSignedTag::value) -
                               (ASignedTag::value && BSignedTag::value) + 1>;

    using RFractionWidth = meta::AutoValueWrapper<comparison_utils::BasicMax(
        AFractionWidth::value, BFractionWidth::value)>;

    using RFixedPoint = FixedPoint<RSignedTag, RIntegralWidth, RFractionWidth>;

    return RFixedPoint::FromValue(
        static_cast<RFixedPoint::Value>(a.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::FractionWidth::value - AFractionWidth::value)) -
        static_cast<RFixedPoint::Value>(b.value) *
            (static_cast<RFixedPoint::Value>(1)
             << (RFixedPoint::FractionWidth::value - BFractionWidth::value)));
}

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr auto fixed_point::operator*(
    FixedPoint<FixedPointTplArgList(A)> const& a,
    FixedPoint<FixedPointTplArgList(B)> const& b) {
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

    using RSignedTag =
        meta::AutoValueWrapper<ASignedTag::value || BSignedTag::value>;

    using RIntegralWidth =
        meta::AutoValueWrapper<AIntegralWidth::value + BIntegralWidth::value -
                               (ASignedTag::value && BSignedTag::value)>;

    using RFractionWidth =
        meta::AutoValueWrapper<AFractionWidth::value + BFractionWidth::value>;

    using FixedPointRet =
        FixedPoint<RSignedTag, RIntegralWidth, RFractionWidth>;

    return FixedPointRet::FromValue(static_cast<FixedPointRet::Value>(a.value) *
                                    static_cast<FixedPointRet::Value>(b.value));
}

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr auto fixed_point::operator/(
    FixedPoint<FixedPointTplArgList(A)> const& a,
    FixedPoint<FixedPointTplArgList(B)> const& b) {
    return (FromFraction)(meta::AutoValueWrapper<comparison_utils::BasicMax(
                              AFractionWidth::value, BFractionWidth::value)>{},
                          a.value, b.value);
}

template <size_t FractionWidth, integral::IsIntegral Num,
          integral::IsIntegral Denom>
constexpr auto fixed_point::FromFraction(
    meta::ValueWrapper<size_t, FractionWidth>, Num num, Denom denom) {
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

    ZETA_Core_DebugAssert(denom != 0);

    constexpr bool any_signed{ integral::IsSignedIntegral<Num> ||
                               integral::IsSignedIntegral<Denom> };

    using RSignedTag = meta::AutoValueWrapper<any_signed>;

    using RIntegralWidth =
        meta::AutoValueWrapper<integral::WidthOf<Num> + any_signed>;

    using RFractionWidth = meta::ValueWrapper<size_t, FractionWidth>;

    using RFixedPoint = FixedPoint<RSignedTag, RIntegralWidth, RFractionWidth>;

    using OpIntegral =
        unsigned _BitInt(RIntegralWidth::value + RFractionWidth::value);

    constexpr Num num_0{ static_cast<Num>(0) };

    bool num_is_neg{ integral::IsSignedIntegral<Num> && num < num_0 };
    bool denom_is_neg{ integral::IsSignedIntegral<Denom> && denom < num_0 };
    bool is_neg{ num_is_neg != denom_is_neg };

    OpIntegral a{ static_cast<OpIntegral>(num_is_neg ? -num : num)
                  << FractionWidth };

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

template <integral::IsIntegral Integral>
constexpr auto fixed_point::FromIntegral(Integral integral) {
    return (FromFraction)(meta::ValueWrapper<size_t, 0>{}, integral,
                          static_cast<Integral>(1));
}

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr comparison::Ordering fixed_point::MathCompare(
    FixedPoint<FixedPointTplArgList(A)> const& a,
    FixedPoint<FixedPointTplArgList(B)> const& b) {
    if constexpr (ASignedTag::value && !BSignedTag::value) {
        if (a.value < 0) { return comparison::Ordering::Less; }
    }

    if constexpr (!ASignedTag::value && BSignedTag::value) {
        if (b.value < 0) { return comparison::Ordering::Greater; }
    }

    constexpr size_t op_fraction_width{ comparison_utils::BasicMax(
        AFractionWidth::value, BFractionWidth::value) };

    constexpr size_t op_integral_width{ 1 + comparison_utils::BasicMax(
                                                AIntegralWidth::value,
                                                BIntegralWidth::value) };

    constexpr size_t op_total_width{ op_integral_width + op_fraction_width };

    using SOpIntegral = signed _BitInt(op_total_width);

    SOpIntegral x{ static_cast<SOpIntegral>(a.value)
                   << (op_fraction_width - AFractionWidth::value) };

    SOpIntegral y{ static_cast<SOpIntegral>(b.value)
                   << (op_fraction_width - BFractionWidth::value) };

    return comparison::BasicCompare(
        meta::AutoValueWrapper<comparison::Op::Order>{}, x, y);
}

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
template <comparison::IsOpType OpType>
constexpr auto comparison::ComparatorTraits<comparison::BasicComparator<
    fixed_point::FixedPoint<FixedPointTplArgList(A)>,
    fixed_point::FixedPoint<FixedPointTplArgList(B)>>>::
    Compare(comparison::BasicComparator<
                fixed_point::FixedPoint<FixedPointTplArgList(A)>,
                fixed_point::FixedPoint<FixedPointTplArgList(B)>> const&,
            OpType, fixed_point::FixedPoint<FixedPointTplArgList(A)> const& a,
            fixed_point::FixedPoint<FixedPointTplArgList(B)> const& b) {
    comparison::Ordering ord{ fixed_point::MathCompare(a, b) };

    if constexpr (meta::IsSame<OpType,
                               meta::AutoValueWrapper<comparison::Op::Order>>) {
        return ord;
    } else {
        return (meta::ToUnderlying(ord) & meta::ToUnderlying(OpType::value)) !=
               0;
    }
}

}  // namespace zeta::core

#pragma pop_macro("FixedPointTplParamList")
#pragma pop_macro("FixedPointTplArgList")
