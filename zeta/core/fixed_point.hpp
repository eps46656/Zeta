#pragma once

#include <zeta/core/compare_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/value_wrapper.hpp>

#pragma push_macro("FixedPointTplDeclParamList")
#define FixedPointTplDeclParamList \
    typename SignedTag_, typename IntegralWidth_, typename FractionWidth_

#pragma push_macro("FixedPointTplParamList")
#define FixedPointTplParamList(prefix)                          \
    typename prefix##SignedTag, typename prefix##IntegralWidth, \
        typename prefix##FractionWidth

#pragma push_macro("FixedPointTplArgList")
#define FixedPointTplArgList(prefix) \
    prefix##SignedTag, prefix##IntegralWidth, prefix##FractionWidth

namespace zeta::core::fixed_point {

constexpr size_t max_total_width{ compare_utils::BasicMin(
    ZETA_Core_bitint_max_width,
    static_cast<size_t>(integral::RangeMaxOf<long long> / 256)) };

constexpr size_t max_integral_width{ max_total_width };

constexpr size_t max_fraction_width{ max_total_width };

template <FixedPointTplDeclParamList>
struct FixedPoint {
    using SignedTag = SignedTag_;
    using IntegralWidth = IntegralWidth_;
    using FractionWidth = FractionWidth_;

    ZETA_Core_StaticAssert(meta::IsAnyOf<SignedTag, value_wrapper::FalseType,
                                         value_wrapper::TrueType>);

    ZETA_Core_StaticAssert(
        value_wrapper::IsStaticValueWrapper<IntegralWidth, size_t>);
    ZETA_Core_StaticAssert(IntegralWidth::value <= max_integral_width);

    ZETA_Core_StaticAssert(
        value_wrapper::IsStaticValueWrapper<FractionWidth, size_t>);
    ZETA_Core_StaticAssert(FractionWidth::value <= max_fraction_width);

    ZETA_Core_StaticAssert(2 <= IntegralWidth::value + FractionWidth::value);
    ZETA_Core_StaticAssert(IntegralWidth::value + FractionWidth::value <=
                           max_total_width);

    using Value = meta::Conditional<
        SignedTag::value,
        signed _BitInt(IntegralWidth::value + FractionWidth::value),
        unsigned _BitInt(IntegralWidth::value + FractionWidth::value)>;

    Value value;

    static constexpr FixedPoint FromValue(Value value);

    template <typename Integral>
    static constexpr FixedPoint FromIntegral(Integral integral);

    template <typename Num, typename Denom>
    static constexpr FixedPoint FromFraction(Num num, Denom denom);

    constexpr FixedPoint() = default;

    template <typename Integral,
              typename = meta::EnableIf<integral::IsIntegral<Integral>>>
    constexpr FixedPoint(Integral const& integral);

    constexpr FixedPoint(FixedPoint const&) = default;

    constexpr FixedPoint(FixedPoint&&) = default;

    template <FixedPointTplParamList(Src)>
    explicit constexpr FixedPoint(
        FixedPoint<FixedPointTplArgList(Src)> const& src);

    template <typename Integral,
              typename = meta::EnableIf<integral::IsIntegral<Integral>>>
    constexpr FixedPoint& operator=(Integral const& integral);

    constexpr FixedPoint& operator=(FixedPoint const&) = default;

    constexpr FixedPoint& operator=(FixedPoint&&) = default;

    template <FixedPointTplParamList(Src)>
    constexpr FixedPoint& operator=(
        FixedPoint<FixedPointTplArgList(Src)> const& src);
};

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool operator==(FixedPoint<FixedPointTplArgList(X)> const& x,
                          FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool operator!=(FixedPoint<FixedPointTplArgList(X)> const& x,
                          FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool operator<(FixedPoint<FixedPointTplArgList(X)> const& x,
                         FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool operator<=(FixedPoint<FixedPointTplArgList(X)> const& x,
                          FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool operator>(FixedPoint<FixedPointTplArgList(X)> const& x,
                         FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr bool operator>=(FixedPoint<FixedPointTplArgList(X)> const& x,
                          FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr auto operator+(FixedPoint<FixedPointTplArgList(X)> const& x,
                         FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr auto operator-(FixedPoint<FixedPointTplArgList(X)> const& x,
                         FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr auto operator*(FixedPoint<FixedPointTplArgList(X)> const& x,
                         FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr auto operator/(FixedPoint<FixedPointTplArgList(X)> const& x,
                         FixedPoint<FixedPointTplArgList(Y)> const& y);

template <typename FractionWidth, typename Num, typename Denom>
constexpr auto FromFraction(Num num, Denom denom);

template <typename Integral>
constexpr auto FromIntegral(Integral integral);

template <FixedPointTplParamList(X), FixedPointTplParamList(Y)>
constexpr int MathCompare(FixedPoint<FixedPointTplArgList(X)> const& x,
                          FixedPoint<FixedPointTplArgList(Y)> const& y);

template <FixedPointTplParamList()>
constexpr auto Floor(FixedPoint<FixedPointTplArgList()> const& x);

template <FixedPointTplParamList()>
constexpr auto Ceil(FixedPoint<FixedPointTplArgList()> const& x);

}  // namespace zeta::core::fixed_point

#pragma pop_macro("FixedPointTplArgList")
#pragma pop_macro("FixedPointTplParamList")
#pragma pop_macro("FixedPointTplDeclParamList")
