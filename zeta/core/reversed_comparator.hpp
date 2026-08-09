#pragma once

#include <zeta/core/comparison.hpp>

namespace zeta::core::reversed_comparator {

template <typename Comparator>
struct ReversedComparator {
    Comparator const& cmptr;

    template <typename A, typename B>
    int Compare(comparison::comparison_type::tag::ThreeWay, A const& a,
                B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::comparison_type::tag::Equal, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::comparison_type::tag::NotEqual, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::comparison_type::tag::Less, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::comparison_type::tag::LessEqual, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::meta::AutoValueWrapper<Op::Greater>, A const& a,
                 B const& b) const;

    template <typename A, typename B>
    bool Compare(comparison::meta::AutoValueWrapper<Op::Greater> Equal,
                 A const& a, B const& b) const;
};

template <typename Comparator>
ReversedComparator<Comparator> const& MakeReversedComparator(
    Comparator const& cmptr);

template <typename Comparator>
Comparator const& MakeReversedComparator(
    ReversedComparator<Comparator> const& revered_cmptr);

}  // namespace zeta::core::reversed_comparator
