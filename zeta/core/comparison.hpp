#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::comparison {

constexpr unsigned less_bit{ 0b001 };
constexpr unsigned equal_bit{ 0b010 };
constexpr unsigned greater_bit{ 0b100 };

enum struct OpEnum : unsigned char {
    Order = 0b1000,
    Equal = equal_bit,
    NotEqual = less_bit | greater_bit,
    Less = less_bit,
    LessEqual = less_bit | equal_bit,
    Greater = greater_bit,
    GreaterEqual = greater_bit | equal_bit,
};

template <typename T>
concept IsOpType =
    meta::IsAnySame<T, OpEnum, meta::AutoValueWrapper<OpEnum::Order>,
                    meta::AutoValueWrapper<OpEnum::Equal>,
                    meta::AutoValueWrapper<OpEnum::NotEqual>,
                    meta::AutoValueWrapper<OpEnum::Less>,
                    meta::AutoValueWrapper<OpEnum::LessEqual>,
                    meta::AutoValueWrapper<OpEnum::Greater>,
                    meta::AutoValueWrapper<OpEnum::GreaterEqual>>;

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
        meta::RemoveCVRef<
            decltype(ComparatorTraits<meta::RemoveCVRef<Comparator>>::Compare(
                cmptr, meta::AutoValueWrapper<OpEnum::Order>{}, a, b))>,
        Ordering>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(ComparatorTraits<meta::RemoveCVRef<Comparator>>::Compare(
                cmptr, meta::AutoValueWrapper<OpEnum::Equal>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(ComparatorTraits<meta::RemoveCVRef<Comparator>>::Compare(
                cmptr, meta::AutoValueWrapper<OpEnum::NotEqual>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(ComparatorTraits<meta::RemoveCVRef<Comparator>>::Compare(
                cmptr, meta::AutoValueWrapper<OpEnum::Less>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(ComparatorTraits<meta::RemoveCVRef<Comparator>>::Compare(
                cmptr, meta::AutoValueWrapper<OpEnum::LessEqual>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(ComparatorTraits<meta::RemoveCVRef<Comparator>>::Compare(
                cmptr, meta::AutoValueWrapper<OpEnum::Greater>{}, a, b))>,
        bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<
            decltype(ComparatorTraits<meta::RemoveCVRef<Comparator>>::Compare(
                cmptr, meta::AutoValueWrapper<OpEnum::GreaterEqual>{}, a, b))>,
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
                                      meta::AutoValueWrapper<OpEnum::Order>,
                                      A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<OpEnum::Equal>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<OpEnum::NotEqual>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<OpEnum::Less>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<OpEnum::LessEqual>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<OpEnum::Greater>,
                                  A const& a, B const& b);

    static constexpr bool Compare(auto const&,
                                  meta::AutoValueWrapper<OpEnum::GreaterEqual>,
                                  A const& a, B const& b);
};

struct EmptyComparator {
    template <IsOpType OpType, typename A, typename B>
    static constexpr auto Compare(OpType, A const&, B const&);
};

using ArchetComparator = EmptyComparator;

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

template <typename CompareType, typename A, typename B>
constexpr auto TypeErasedBasicCompare(CompareType, void const* a,
                                      void const* b);

template <meta::IsValueWrapperT<OpEnum> Op, typename A, typename B>
struct CppStdBasicComparator {
    constexpr decltype(auto) operator()(A const& a, B const& b) const;
};

}  // namespace zeta::core::comparison

namespace zeta::core {

template <>
struct comparison::ComparatorTraits<comparison::EmptyComparator>
    : public comparison::MemberFuncComparatorTraitsAdapter<
          comparison::EmptyComparator> {};

}  // namespace zeta::core

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
