#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <comparison::IsOpTag OpTag>
constexpr auto comparison::DecayOrdering(OpTag, Ordering ordering) {
    if constexpr (meta::IsSame<OpTag, OpTags::Order>) {
        return ordering;
    } else {
        return (meta::ToUnderlying(OpTag::value) &
                meta::ToUnderlying(ordering)) != 0;
    }
}

template <typename Comparator, comparison::IsOpTag OpTag, typename A,
          typename B>
    requires comparison::CanCompare<Comparator, A, B>
constexpr decltype(auto) comparison::Compare(Comparator const& cmptr, OpTag,
                                             A const& a, B const& b) {
    return cmptr.Compare(Tag{}, OpTag{}, a, b);
}

template <typename A, typename B>
constexpr comparison::Ordering
comparison::NativeOperatorComparatorAdapter<A, B>::Compare(Tag, OpTags::Order,
                                                           A const& a,
                                                           B const& b) {
    return static_cast<Ordering>(
        (a < b ? Bits::less : (Bits::equal | Bits::greater)) &
        (a > b ? Bits::greater : (Bits::equal | Bits::less)));
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTags::Equal, A const& a, B const& b) {
    return a == b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTags::NotEqual, A const& a, B const& b) {
    return a != b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTags::Less, A const& a, B const& b) {
    return a < b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTags::LessEqual, A const& a, B const& b) {
    return a <= b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTags::Greater, A const& a, B const& b) {
    return a > b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTags::GreaterEqual, A const& a, B const& b) {
    return a >= b;
}

template <comparison::IsOpTag OpTag, typename A, typename B>
constexpr auto comparison::EmptyComparator::Compare(OpTag, A const&, B const&) {
    if constexpr (meta::IsSame<OpTag, OpTags::Order>) {
        return Ordering::Equal;
    } else {
        return (meta::ToUnderlying(OpTag::value) & Bits::equal) != 0;
    }
}

template <typename OpTag, typename A, typename B>
constexpr auto comparison::BasicCompare(OpTag, A const& a, B const& b) {
    return (Compare)(BasicComparator<A, B>{}, OpTag{}, a, b);
}

template <comparison::IsOpTag OpTag, typename A, typename B>
constexpr auto comparison::UniversalBasicComparator::Compare(Tag, OpTag,
                                                             A const& a,
                                                             B const& b) {
    return (BasicCompare)(OpTag{}, a, b);
}

template <typename CompareType, typename A, typename B>
constexpr auto comparison::TypeErasedBasicCompare(CompareType, void const* a,
                                                  void const* b) {
    return (BasicCompare)(CompareType{}, *static_cast<A const*>(a),
                          *static_cast<B const*>(b));
}

template <comparison::IsOpTag Op, typename A, typename B>
constexpr decltype(auto)
comparison::CppStdBasicComparator<Op, A, B>::operator()(A const& a,
                                                        B const& b) const {
    return (BasicCompare)(Op{}, a, b);
}

}  // namespace zeta::core

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_equal
constexpr bool operator==(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTags::Equal{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_not_equal
constexpr bool operator!=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTags::NotEqual{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_less
constexpr bool operator<(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTags::Less{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_less_equal
constexpr bool operator<=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTags::LessEqual{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_greater
constexpr bool operator>(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTags::Greater{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_greater_equal
constexpr bool operator>=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTags::GreaterEqual{}, a, b);
}
