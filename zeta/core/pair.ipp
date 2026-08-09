#pragma once

#include <zeta/core/comparison.ipp>
#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/hash.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core {

template <typename First, typename Second>
constexpr unsigned long long
hash::BasicHasher<pair::Pair<First, Second>>::operator()(
    pair::Pair<First, Second> const& value, unsigned long long salt) const {
    unsigned long long h1{ hash::BasicHash(value.first, salt) };
    unsigned long long h2{ hash::BasicHash(value.second, h1) };
    return hash::BasicHash(h2, salt);
}

template <typename AFirst, typename ASecond, typename BFirst, typename BSecond>
template <comparison::IsOpType OpType>
constexpr auto comparison::ComparatorTraits<comparison::BasicComparator<
    pair::Pair<AFirst, ASecond>, pair::Pair<BFirst, BSecond>>>::
    Compare(auto const&, OpType op, pair::Pair<AFirst, ASecond> const& a,
            pair::Pair<BFirst, BSecond> const& b) {
    return comparison_utils::BasicPairWiseLexCompare(op, a.first, b.first,
                                                     a.second, b.second);
}

}  // namespace zeta::core
