#pragma once

#include <zeta/core/comparison.hpp>

namespace zeta::core::reversed_comparator {

template <typename Comparator>
struct ReversedComparator {
    Comparator const& cmptr;

    template <typename A, typename B>
    int Compare(comparison::ComparisonTypeEnum::ThreeWay, A const& a,
                B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::ComparisonTypeEnum::EqualTo, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::ComparisonTypeEnum::NotEqualTo, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::ComparisonTypeEnum::Less, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::ComparisonTypeEnum::LessEqual, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::ComparisonTypeEnum::Greater, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::ComparisonTypeEnum::GreaterEqual, A const& a,
                 B const& b) const;
};

template <typename Comparator>
ReversedComparator<Comparator> const& MakeReversedComparator(
    Comparator const& cmptr);

template <typename Comparator>
Comparator const& MakeReversedComparator(
    ReversedComparator<Comparator> const& revered_cmptr);

}  // namespace zeta::core::reversed_comparator
