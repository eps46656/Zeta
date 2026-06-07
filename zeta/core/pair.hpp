#pragma once

#include <zeta/core/compare.hpp>
#include <zeta/core/hash.hpp>

namespace zeta::core::pair {

template <typename First, typename Second>
struct Pair {
    First first;
    Second second;
};

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool operator==(Pair<AFirst, ASecond> const& a,
                          Pair<BFirst, BSecond> const& b);

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool operator!=(Pair<AFirst, ASecond> const& a,
                          Pair<BFirst, BSecond> const& b);

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool operator<(Pair<AFirst, ASecond> const& a,
                         Pair<BFirst, BSecond> const& b);

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool operator<=(Pair<AFirst, ASecond> const& a,
                          Pair<BFirst, BSecond> const& b);

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool operator>(Pair<AFirst, ASecond> const& a,
                         Pair<BFirst, BSecond> const& b);

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
constexpr bool operator>=(Pair<AFirst, ASecond> const& a,
                          Pair<BFirst, BSecond> const& b);

}  // namespace zeta::core::pair

namespace zeta::core {

template <typename First, typename Second>
struct hash::BasicHasher<pair::Pair<First, Second>> {
    constexpr unsigned long long operator()(
        pair::Pair<First, Second> const& value, unsigned long long salt) const;
};

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
struct compare::BasicComparator<pair::Pair<AFirst, ASecond>,
                                pair::Pair<BFirst, BSecond>> {
    constexpr int operator()(pair::Pair<AFirst, ASecond> const& a,
                             pair::Pair<BFirst, BSecond> const& b) const;
};

}  // namespace zeta::core
