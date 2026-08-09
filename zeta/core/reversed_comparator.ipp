#pragma once

#include <zeta/core/reversed_comparator.hpp>

namespace zeta::core {

template <typename Comparator>
template <typename A, typename B>
int reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::comparison_type::ThreeWay, A const& a, B const& b) const {
    return -this->cmptr.Compare(comparison::comparison_type::ThreeWay{}, a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::comparison_type::Equal, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::comparison_type::Equal{}, a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::comparison_type::NotEqual, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::comparison_type::NotEqual{}, a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::comparison_type::Less, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::comparison_type::Greater{}, a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::comparison_type::LessEqual, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::comparison_type::GreaterEqual{}, a,
                               b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::comparison_type::Greater, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::comparison_type::Less{}, a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::comparison_type::GreaterEqual, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::comparison_type::LessEqual{}, a, b);
}

template <typename Comparator>
reversed_comparator::ReversedComparator<Comparator> const&
reversed_comparator::MakeReversedComparator(Comparator const& cmptr) {
    return { cmptr };
}

template <typename Comparator>
Comparator const& reversed_comparator::MakeReversedComparator(
    ReversedComparator<Comparator> const& revered_cmptr) {
    return revered_cmptr.cmptr;
}

}  // namespace zeta::core
