#pragma once

#include <zeta/core/compare.hpp>
#include <zeta/core/compare.ipp>
#include <zeta/core/compare_utils.ipp>
#include <zeta/core/hash.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core {

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator==(Pair<AFirst, ASecond> const& a,
                                Pair<BFirst, BSecond> const& b) {
    return compare::BasicCompare(a, b) == 0;
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator!=(Pair<AFirst, ASecond> const& a,
                                Pair<BFirst, BSecond> const& b) {
    return compare::BasicCompare(a, b) != 0;
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator<(Pair<AFirst, ASecond> const& a,
                               Pair<BFirst, BSecond> const& b) {
    return compare::BasicCompare(a, b) < 0;
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator<=(Pair<AFirst, ASecond> const& a,
                                Pair<BFirst, BSecond> const& b) {
    return compare::BasicCompare(a, b) <= 0;
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator>(Pair<AFirst, ASecond> const& a,
                               Pair<BFirst, BSecond> const& b) {
    return compare::BasicCompare(a, b) > 0;
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool pair::operator>=(Pair<AFirst, ASecond> const& a,
                                Pair<BFirst, BSecond> const& b) {
    return compare::BasicCompare(a, b) >= 0;
}

template <typename First, typename Second>
constexpr unsigned long long
hash::BasicHasher<pair::Pair<First, Second>>::operator()(
    pair::Pair<First, Second> const& value, unsigned long long salt) const {
    unsigned long long h1{ hash::BasicHash(value.first, salt) };
    unsigned long long h2{ hash::BasicHash(value.second, h1) };
    return hash::BasicHash(h2, salt);
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr int compare::BasicComparator<pair::Pair<AFirst, ASecond>,
                                       pair::Pair<BFirst, BSecond>>::
operator()(pair::Pair<AFirst, ASecond> const& a,
           pair::Pair<BFirst, BSecond> const& b) const {
    return compare_utils::PairWiseLexCompare(
        a.first, b.first, compare::BasicCompare<AFirst, BFirst>,     //
        a.second, b.second, compare::BasicCompare<ASecond, BSecond>  //
    );
}

}  // namespace zeta::core
