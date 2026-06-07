#pragma once

#include <zeta/core/compare.hpp>

namespace zeta::core::reversed_comparator {

template <typename Comparator>
struct ReversedComparator {
    Comparator const& cmptr;

    template <typename A, typename B>
    int Compare(compare::compare_type::ThreeWay, A const& a, B const& b) const;

    template <typename A, typename B>
    bool Compare(compare::compare_type::EqualTo, A const& a, B const& b) const;

    template <typename A, typename B>
    bool Compare(compare::compare_type::NotEqualTo, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(compare::compare_type::Less, A const& a, B const& b) const;

    template <typename A, typename B>
    bool Compare(compare::compare_type::LessEqual, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(compare::compare_type::Greater, A const& a, B const& b) const;

    template <typename A, typename B>
    bool Compare(compare::compare_type::GreaterEqual, A const& a,
                 B const& b) const;
};

template <typename Comparator>
ReversedComparator<Comparator> const& MakeReversedComparator(
    Comparator const& cmptr);

template <typename Comparator>
Comparator const& MakeReversedComparator(
    ReversedComparator<Comparator> const& revered_cmptr);

}  // namespace zeta::core::reversed_comparator
