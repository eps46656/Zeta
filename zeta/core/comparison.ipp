#pragma once

#include <zeta/core/comparison.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/define.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename Comparator>
template <comparison::IsOpType OpType, typename A, typename B>
constexpr decltype(auto)
comparison::MemberFuncComparatorTraitsAdapter<Comparator>::Compare(
    Comparator const& cmptr, OpType op, A const& a, B const& b) {
    return cmptr.Compare(op, a, b);
}

template <typename Comparator, comparison::IsOpType OpType, typename A,
          typename B>
    requires comparison::CanCompare<Comparator, A, B>
constexpr decltype(auto) comparison::Compare(Comparator const& cmptr, OpType op,
                                             A const& a, B const& b) {
    return ComparatorTraits<Comparator>::Compare(cmptr, op, a, b);
}

template <typename A, typename B>
constexpr comparison::Ordering
comparison::NativeOperatorComparatorTraitsAdapter<A, B>::Compare(
    auto const&, meta::AutoValueWrapper<Op::Order>, A const& a, B const& b) {
    if (a < b) { return Ordering::Less; }
    if (a > b) { return Ordering::Greater; }
    return Ordering::Equal;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorTraitsAdapter<A, B>::Compare(
    auto const&, meta::AutoValueWrapper<Op::Equal>, A const& a, B const& b) {
    return a == b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorTraitsAdapter<A, B>::Compare(
    auto const&, meta::AutoValueWrapper<Op::NotEqual>, A const& a, B const& b) {
    return a != b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorTraitsAdapter<A, B>::Compare(
    auto const&, meta::AutoValueWrapper<Op::Less>, A const& a, B const& b) {
    return a < b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorTraitsAdapter<A, B>::Compare(
    auto const&, meta::AutoValueWrapper<Op::LessEqual>, A const& a,
    B const& b) {
    return a <= b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorTraitsAdapter<A, B>::Compare(
    auto const&, meta::AutoValueWrapper<Op::Greater>, A const& a, B const& b) {
    return a > b;
}

template <typename A, typename B>
constexpr bool comparison::NativeOperatorComparatorTraitsAdapter<A, B>::Compare(
    auto const&, meta::AutoValueWrapper<Op::GreaterEqual>, A const& a,
    B const& b) {
    return a >= b;
}

template <typename A, typename B>
    requires(integral::IsIntegral<A> || meta::IsPointer<A> ||
             meta::IsArray<A>) &&
            (integral::IsIntegral<B> || meta::IsPointer<B> || meta::IsArray<B>)
struct comparison::ComparatorTraits<comparison::BasicComparator<A, B>>
    : public NativeOperatorComparatorTraitsAdapter<A, B> {};

template <typename OpType, typename A, typename B>
constexpr auto comparison::BasicCompare(OpType op, A const& a, B const& b) {
    return (Compare)(BasicComparator<A, B>{}, op, a, b);
}

template <>
struct comparison::ComparatorTraits<comparison::UniversalBasicComparator> {
    template <IsOpType OpType, typename A, typename B>
    static constexpr auto Compare(UniversalBasicComparator const&, OpType op,
                                  A const& a, B const& b) {
        return (BasicCompare)(op, a, b);
    }
};

template <typename CompareType, typename A, typename B>
constexpr auto comparison::TypeErasedBasicCompare(CompareType, void const* a,
                                                  void const* b) {
    return (BasicCompare)(CompareType{}, *static_cast<A const*>(a),
                          *static_cast<B const*>(b));
}

template <comparison::IsOpType OpType, typename A, typename B>
constexpr decltype(auto)
comparison::CppStdBasicComparator<OpType, A, B>::operator()(A const& a,
                                                            B const& b) const {
    return (BasicCompare)(OpType{}, a, b);
}

}  // namespace zeta::core

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_equal
constexpr bool operator==(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::meta::AutoValueWrapper<zeta::core::comparison::Op::Equal>{},
        a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_not_equal
constexpr bool operator!=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::meta::AutoValueWrapper<
            zeta::core::comparison::Op::NotEqual>{},
        a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_less
constexpr bool operator<(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::meta::AutoValueWrapper<zeta::core::comparison::Op::Less>{},
        a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_less_equal
constexpr bool operator<=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::meta::AutoValueWrapper<
            zeta::core::comparison::Op::LessEqual>{},
        a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_greater
constexpr bool operator>(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::meta::AutoValueWrapper<
            zeta::core::comparison::Op::Greater>{},
        a, b);
}

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_greater_equal
constexpr bool operator>=(A const& a, B const& b) {
    return zeta::core::comparison::BasicCompare(
        zeta::core::meta::AutoValueWrapper<
            zeta::core::comparison::Op::GreaterEqual>{},
        a, b);
}
