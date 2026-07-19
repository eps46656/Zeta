#pragma once

#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/hash.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core {

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator==(Pair<AFirst, ASecond> const& a,
                                Pair<BFirst, BSecond> const& b) {
    return comparison::BasicCompare(comparison::ComparisonTypeEnum::EqualTo{},
                                    a, b);
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator!=(Pair<AFirst, ASecond> const& a,
                                Pair<BFirst, BSecond> const& b) {
    return comparison::BasicCompare(
        comparison::ComparisonTypeEnum::NotEqualTo{}, a, b);
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator<(Pair<AFirst, ASecond> const& a,
                               Pair<BFirst, BSecond> const& b) {
    return comparison::BasicCompare(comparison::ComparisonTypeEnum::Less{}, a,
                                    b);
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator<=(Pair<AFirst, ASecond> const& a,
                                Pair<BFirst, BSecond> const& b) {
    return comparison::BasicCompare(comparison::ComparisonTypeEnum::LessEqual{},
                                    a, b);
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator>(Pair<AFirst, ASecond> const& a,
                               Pair<BFirst, BSecond> const& b) {
    return comparison::BasicCompare(comparison::ComparisonTypeEnum::Greater{},
                                    a, b);
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator>=(Pair<AFirst, ASecond> const& a,
                                Pair<BFirst, BSecond> const& b) {
    return comparison::BasicCompare(
        comparison::ComparisonTypeEnum::GreaterEqual{}, a, b);
}

template <typename First, typename Second>
constexpr unsigned long long
hash::BasicHasher<pair::Pair<First, Second>>::operator()(
    pair::Pair<First, Second> const& value, unsigned long long salt) const {
    unsigned long long h1{ hash::BasicHash(value.first, salt) };
    unsigned long long h2{ hash::BasicHash(value.second, h1) };
    return hash::BasicHash(h2, salt);
}

template <typename ComparisonType, typename AFirst, typename ASecond,
          typename BFirst, typename BSecond>
constexpr int comparison::BasicComparator<
    ComparisonType, pair::Pair<AFirst, ASecond>, pair::Pair<BFirst, BSecond>>::
operator()(ComparisonType, pair::Pair<AFirst, ASecond> const& a,
           pair::Pair<BFirst, BSecond> const& b) const {
    ZETA_Core_StaticAssert(comparison::IsComparisonType<ComparisonType>);

    return comparison_utils::PairWiseLexCompare(
        ComparisonType{}, a.first, b.first,
        comparison::BasicCompare<AFirst, BFirst>,                       //
        a.second, b.second, comparison::BasicCompare<ASecond, BSecond>  //
    );
}

}  // namespace zeta::core
