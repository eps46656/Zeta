#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/hash.hpp>

namespace zeta::core::pair {

template <typename First, typename Second>
struct Pair {
    First first;
    Second second;
};

}  // namespace zeta::core::pair

namespace zeta::core {

template <typename First, typename Second>
struct hash::BasicHasher<pair::Pair<First, Second>> {
    static constexpr unsigned long long Hash(
        hash::Tag, pair::Pair<First, Second> const& value,
        unsigned long long salt);
};

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
struct comparison::BasicComparator<pair::Pair<AFirst, ASecond>,
                                   pair::Pair<BFirst, BSecond>> {
    template <IsOpTag OpTag>
    static constexpr auto Compare(comparison::Tag, OpTag,
                                  pair::Pair<AFirst, ASecond> const& a,
                                  pair::Pair<BFirst, BSecond> const& b);
};

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
struct comparison::EnableNativeOperatorByBasicComparison<
    pair::Pair<AFirst, ASecond>, pair::Pair<BFirst, BSecond>> {
    static constexpr bool enable_equal{ false };
    static constexpr bool enable_not_equal{ false };
    static constexpr bool enable_less{ true };
    static constexpr bool enable_less_equal{ true };
    static constexpr bool enable_greater{ true };
    static constexpr bool enable_greater_equal{ true };
};

}  // namespace zeta::core
