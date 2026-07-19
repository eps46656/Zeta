#pragma once

#include <zeta/core/reversed_comparator.hpp>

namespace zeta::core {

template <typename Comparator>
template <typename A, typename B>
int reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::ComparisonTypeEnum::ThreeWay, A const& a, B const& b) const {
    return -this->cmptr.Compare(comparison::ComparisonTypeEnum::ThreeWay{}, a,
                                b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::ComparisonTypeEnum::EqualTo, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::ComparisonTypeEnum::EqualTo{}, a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::ComparisonTypeEnum::NotEqualTo, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::ComparisonTypeEnum::NotEqualTo{}, a,
                               b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::ComparisonTypeEnum::Less, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::ComparisonTypeEnum::Greater{}, a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::ComparisonTypeEnum::LessEqual, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::ComparisonTypeEnum::GreaterEqual{},
                               a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::ComparisonTypeEnum::Greater, A const& a, B const& b) const {
    return this->cmptr.Compare(comparison::ComparisonTypeEnum::Less{}, a, b);
}

template <typename Comparator>
template <typename A, typename B>
bool reversed_comparator::ReversedComparator<Comparator>::Compare(
    comparison::ComparisonTypeEnum::GreaterEqual, A const& a,
    B const& b) const {
    return this->cmptr.Compare(comparison::ComparisonTypeEnum::LessEqual{}, a,
                               b);
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
