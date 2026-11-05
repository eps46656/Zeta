#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/define.hpp>
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

namespace zeta::core::fixed_point {

constexpr size_t total_width_max{ comparison_utils::BasicMin(
    ZETA_Core_bitint_max_width,
    static_cast<size_t>(integral::RangeMaxOf<long long> / 256)) };

constexpr size_t integral_width_max{ total_width_max };

constexpr size_t fraction_width_max{ total_width_max };

template <FixedPointTplParamList(, _)>
struct FixedPoint {
    static constexpr bool is_signed{ is_signed_ };
    static constexpr size_t integral_width{ integral_width_ };
    static constexpr size_t fraction_width{ fraction_width_ };

    static_assert(integral_width <= integral_width_max);

    static_assert(fraction_width <= fraction_width_max);

    static_assert(2 <= integral_width + fraction_width);
    static_assert(integral_width + fraction_width <= total_width_max);

    using Value =
        meta::Conditional<is_signed,
                          signed _BitInt(integral_width + fraction_width),
                          unsigned _BitInt(integral_width + fraction_width)>;

    Value value;

    static constexpr FixedPoint FromValue(Value value);

    template <integral::IsIntegralType Integral>
    static constexpr FixedPoint FromIntegral(Integral integral);

    template <integral::IsIntegralType Num, integral::IsIntegralType Denom>
    static constexpr FixedPoint FromFraction(Num num, Denom denom);

    constexpr FixedPoint() = default;

    template <integral::IsIntegralType Integral>
    constexpr FixedPoint(Integral const& integral);

    constexpr FixedPoint(FixedPoint const&) = default;

    constexpr FixedPoint(FixedPoint&&) = default;

    template <FixedPointTplParamList(Src, )>
    explicit constexpr FixedPoint(
        FixedPoint<FixedPointTplArgList(Src)> const& src);

    template <integral::IsIntegralType Integral>
    constexpr FixedPoint& operator=(Integral const& integral);

    constexpr FixedPoint& operator=(FixedPoint const&) = default;

    constexpr FixedPoint& operator=(FixedPoint&&) = default;

    template <FixedPointTplParamList(Src, )>
    constexpr FixedPoint& operator=(
        FixedPoint<FixedPointTplArgList(Src)> const& src);

    constexpr auto Floor() const;

    constexpr auto Ceil() const;
};

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr auto operator+(FixedPoint<FixedPointTplArgList(A)> const& a,
                         FixedPoint<FixedPointTplArgList(B)> const& b);

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr auto operator-(FixedPoint<FixedPointTplArgList(A)> const& a,
                         FixedPoint<FixedPointTplArgList(B)> const& b);

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr auto operator*(FixedPoint<FixedPointTplArgList(A)> const& a,
                         FixedPoint<FixedPointTplArgList(B)> const& b);

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
constexpr auto operator/(FixedPoint<FixedPointTplArgList(A)> const& a,
                         FixedPoint<FixedPointTplArgList(B)> const& b);

template <size_t fraction_width, integral::IsIntegralType Num,
          integral::IsIntegralType Denom>
constexpr auto FromFraction(meta::ValueWrapper<size_t, fraction_width>, Num num,
                            Denom denom);

template <integral::IsIntegralType Integral>
constexpr auto FromIntegral(Integral integral);

template <comparison::IsOpTag OpTag, FixedPointTplParamList(A, ),
          FixedPointTplParamList(B, )>
constexpr auto MathCompare(OpTag op,
                           FixedPoint<FixedPointTplArgList(A)> const& a,
                           FixedPoint<FixedPointTplArgList(B)> const& b);

}  // namespace zeta::core::fixed_point

namespace zeta::core {

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
struct comparison::BasicComparator<
    fixed_point::FixedPoint<FixedPointTplArgList(A)>,
    fixed_point::FixedPoint<FixedPointTplArgList(B)>> {
    template <IsOpTag OpTag>
    static constexpr auto Compare(
        comparison::Tag, OpTag,
        fixed_point::FixedPoint<FixedPointTplArgList(A)> const& a,
        fixed_point::FixedPoint<FixedPointTplArgList(B)> const& b);
};

template <FixedPointTplParamList(A, ), FixedPointTplParamList(B, )>
struct comparison::EnableNativeOperatorByBasicComparison<
    fixed_point::FixedPoint<FixedPointTplArgList(A)>,
    fixed_point::FixedPoint<FixedPointTplArgList(B)>> {
    static constexpr bool enable_equal{ true };
    static constexpr bool enable_not_equal{ true };
    static constexpr bool enable_less{ true };
    static constexpr bool enable_less_equal{ true };
    static constexpr bool enable_greater{ true };
    static constexpr bool enable_greater_equal{ true };
};

}  // namespace zeta::core

#pragma pop_macro("FixedPointTplArgList")
#pragma pop_macro("FixedPointTplParamList")
