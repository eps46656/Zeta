#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Comparator, comparison::IsOpTag OpTag, typename A,
          typename B>
    requires comparison::CanCompare<Comparator, A, B>
constexpr decltype(auto) comparison::Compare(Comparator const& cmptr, OpTag op,
                                             A const& a, B const& b) {
    return cmptr.Compare(Tag{}, op, a, b);
}

template <typename A, typename B>
constexpr comparison::Ordering
comparison::NativeOperatorComparatorAdapter<A, B>::Compare(Tag, OpTag::Order,
                                                           A const& a,
                                                           B const& b) {
    return static_cast<comparison::Ordering>(
        (a < b ? comparison::less_bit
               : (comparison::equal_bit | comparison::greater_bit)) &
        (a > b ? comparison::greater_bit
               : (comparison::equal_bit | comparison::less_bit)));
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTag::Equal, A const& a, B const& b) {
    return a == b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTag::NotEqual, A const& a, B const& b) {
    return a != b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTag::Less, A const& a, B const& b) {
    return a < b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTag::LessEqual, A const& a, B const& b) {
    return a <= b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTag::Greater, A const& a, B const& b) {
    return a > b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorAdapter<A, B>::Compare(
    Tag, OpTag::GreaterEqual, A const& a, B const& b) {
    return a >= b;
}

template <comparison::IsOpTag OpTag, typename A, typename B>
constexpr auto comparison::EmptyComparator::Compare(OpTag, A const&, B const&) {
    if constexpr (meta::IsSame<OpTag, comparison::OpTag::Order>) {
        return Ordering::Equal;
    } else {
        return (meta::ToUnderlying(OpTag::value) & equal_bit) != 0;
    }
}

template <typename OpTag, typename A, typename B>
constexpr auto comparison::BasicCompare(OpTag op, A const& a, B const& b) {
    return (Compare)(BasicComparator<A, B>{}, op, a, b);
}

template <comparison::IsOpTag OpTag, typename A, typename B>
constexpr auto comparison::UniversalBasicComparator::Compare(Tag, OpTag op,
                                                             A const& a,
                                                             B const& b) {
    return (BasicCompare)(op, a, b);
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
        zeta::core::comparison::OpTag::Equal{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_not_equal
constexpr bool operator!=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTag::NotEqual{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_less
constexpr bool operator<(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTag::Less{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_less_equal
constexpr bool operator<=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTag::LessEqual{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_greater
constexpr bool operator>(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTag::Greater{}, a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_greater_equal
constexpr bool operator>=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::comparison::OpTag::GreaterEqual{}, a, b);
}
