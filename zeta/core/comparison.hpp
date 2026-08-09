#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::comparison {

constexpr unsigned less_bit{ 0b001 };
constexpr unsigned equal_bit{ 0b010 };
constexpr unsigned greater_bit{ 0b100 };

enum struct Op : unsigned char {
    Order = 0b1000,
    Equal = equal_bit,
    NotEqual = less_bit | greater_bit,
    Less = less_bit,
    LessEqual = less_bit | equal_bit,
    Greater = greater_bit,
    GreaterEqual = greater_bit | equal_bit,
};

template <typename T>
concept IsOpType = meta::IsAnySame<
    T, Op, meta::AutoValueWrapper<Op::Order>, meta::AutoValueWrapper<Op::Equal>,
    meta::AutoValueWrapper<Op::NotEqual>, meta::AutoValueWrapper<Op::Less>,
    meta::AutoValueWrapper<Op::LessEqual>, meta::AutoValueWrapper<Op::Greater>,
    meta::AutoValueWrapper<Op::GreaterEqual>>;

enum struct Ordering : unsigned char {
    Less = less_bit,
    Equal = equal_bit,
    Greater = greater_bit,
};

template <typename Comparator>
struct ComparatorTraits;

template <typename Comparator, typename A, typename B>
concept CanCompare = requires(Comparator const& cmptr, A const& a, B const& b) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ComparatorTraits<Comparator>::Compare(
            cmptr, meta::AutoValueWrapper<Op::Order>{}, a, b))>,
        Ordering>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ComparatorTraits<Comparator>::Compare(
            cmptr, meta::AutoValueWrapper<Op::Equal>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ComparatorTraits<Comparator>::Compare(
            cmptr, meta::AutoValueWrapper<Op::NotEqual>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ComparatorTraits<Comparator>::Compare(
            cmptr, meta::AutoValueWrapper<Op::Less>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ComparatorTraits<Comparator>::Compare(
            cmptr, meta::AutoValueWrapper<Op::LessEqual>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ComparatorTraits<Comparator>::Compare(
            cmptr, meta::AutoValueWrapper<Op::Greater>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(ComparatorTraits<Comparator>::Compare(
            cmptr, meta::AutoValueWrapper<Op::GreaterEqual>{}, a, b))>,
        bool>;
};

template <typename Comparator, IsOpType OpType, typename A, typename B>
    requires comparison::CanCompare<Comparator, A, B>
constexpr decltype(auto) Compare(Comparator const& cmptr, OpType, A const& a,
                                 B const& b);

template <typename Comparator>
struct MemberFuncComparatorTraitsAdapter {
    template <IsOpType OpType, typename A, typename B>
    static constexpr decltype(auto) Compare(Comparator const& cmptr, OpType op,
                                            A const& a, B const& b);
};

template <typename A, typename B>
struct NativeOperatorComparatorTraitsAdapter {
    static constexpr Ordering Compare(auto const&,
                                      meta::AutoValueWrapper<Op::Order>,
                                      A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<Op::Equal>, A const& a,
                                  B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<Op::NotEqual>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&, meta::AutoValueWrapper<Op::Less>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<Op::LessEqual>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<Op::Greater>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<Op::GreaterEqual>,
                                  A const& a, B const& b);
};

template <typename A, typename B>
struct BasicComparator {};

template <typename CompareType, typename A, typename B>
constexpr auto BasicCompare(CompareType, A const& a, B const& y);

template <typename A, typename B>
struct EnableNativeOperatorByBasicComparison {
    static constexpr bool enable_equal{ false };
    static constexpr bool enable_not_equal{ false };
    static constexpr bool enable_less{ false };
    static constexpr bool enable_less_equal{ false };
    static constexpr bool enable_greater{ false };
    static constexpr bool enable_greater_equal{ false };
};

struct UniversalBasicComparator {};

struct EmptyComparator {
    template <IsOpType CompareType, typename A, typename B>
    static constexpr Ordering Compare(CompareType, A const&, B const&) {
        return Ordering::Equal;
    }
};

using ArchetComparator = EmptyComparator;

template <typename CompareType, typename A, typename B>
constexpr auto TypeErasedBasicCompare(CompareType, void const* a,
                                      void const* b);

template <IsOpType OpType, typename A, typename B>
struct CppStdBasicComparator {
    constexpr decltype(auto) operator()(A const& a, B const& b) const;
};

}  // namespace zeta::core::comparison

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_equal
constexpr bool operator==(A const& a, B const& b);

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_not_equal
constexpr bool operator!=(A const& a, B const& b);

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_less
constexpr bool operator<(A const& a, B const& b);

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_less_equal
constexpr bool operator<=(A const& a, B const& b);

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_greater
constexpr bool operator>(A const& a, B const& b);

template <typename A, typename B>
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveCVRef<A>,
        zeta::core::meta::RemoveCVRef<B>>::enable_greater_equal
constexpr bool operator>=(A const& a, B const& b);
