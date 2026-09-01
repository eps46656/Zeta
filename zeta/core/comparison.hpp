#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::comparison {

struct Tag {};

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

struct OpTag {
    using Order = meta::AutoValueWrapper<Op::Order>;
    using Equal = meta::AutoValueWrapper<Op::Equal>;
    using NotEqual = meta::AutoValueWrapper<Op::NotEqual>;
    using Less = meta::AutoValueWrapper<Op::Less>;
    using LessEqual = meta::AutoValueWrapper<Op::LessEqual>;
    using Greater = meta::AutoValueWrapper<Op::Greater>;
    using GreaterEqual = meta::AutoValueWrapper<Op::GreaterEqual>;
};

template <typename T>
concept IsOpTag = meta::IsAnySame<T,                   //
                                  OpTag::Order,        //
                                  OpTag::Equal,        //
                                  OpTag::NotEqual,     //
                                  OpTag::Less,         //
                                  OpTag::LessEqual,    //
                                  OpTag::Greater,      //
                                  OpTag::GreaterEqual  //
                                  >;

enum struct Ordering : unsigned char {
    Less = less_bit,
    Equal = equal_bit,
    Greater = greater_bit,
};

template <typename Comparator, typename A, typename B>
concept CanCompare = requires(Comparator const& cmptr, Tag tag, A const& a,
                              B const& b) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(cmptr.Compare(tag, OpTag::Order{}, a, b))>,
        Ordering>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(cmptr.Compare(tag, OpTag::Equal{}, a, b))>,
        bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(cmptr.Compare(
                              tag, OpTag::NotEqual{}, a, b))>,
                          bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(cmptr.Compare(tag, OpTag::Less{}, a, b))>,
        bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(cmptr.Compare(
                              tag, OpTag::LessEqual{}, a, b))>,
                          bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(cmptr.Compare(tag, OpTag::Greater{}, a, b))>,
        bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(cmptr.Compare(
                              tag, OpTag::GreaterEqual{}, a, b))>,
                          bool>;
};

template <typename Comparator, IsOpTag OpTag, typename A, typename B>
    requires comparison::CanCompare<Comparator, A, B>
constexpr decltype(auto) Compare(Comparator const& cmptr, OpTag, A const& a,
                                 B const& b);

template <typename A, typename B>
struct NativeOperatorComparatorAdapter {
    static constexpr Ordering Compare(Tag, OpTag::Order, A const& a,
                                      B const& b);

    static constexpr bool Compare(Tag, OpTag::Equal, A const& a, B const& b);

    static constexpr bool Compare(Tag, OpTag::NotEqual, A const& a, B const& b);

    static constexpr bool Compare(Tag, OpTag::Less, A const& a, B const& b);

    static constexpr bool Compare(Tag, OpTag::LessEqual, A const& a,
                                  B const& b);

    static constexpr bool Compare(Tag, OpTag::Greater, A const& a, B const& b);

    static constexpr bool Compare(Tag, OpTag::GreaterEqual, A const& a,
                                  B const& b);
};

struct EmptyComparator {
    template <IsOpTag OpTag, typename A, typename B>
    static constexpr auto Compare(OpTag, A const&, B const&);
};

using ArchetypeComparator = EmptyComparator;

template <typename A, typename B>
struct BasicComparator;

template <typename A, typename B>
    requires(integral::IsIntegral<A> || meta::IsPointer<A> ||
             meta::IsArray<A>) &&
            (integral::IsIntegral<B> || meta::IsPointer<B> || meta::IsArray<B>)
struct BasicComparator<A, B> : public NativeOperatorComparatorAdapter<A, B> {};

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

struct UniversalBasicComparator {
    template <IsOpTag OpTag, typename A, typename B>
    static constexpr auto Compare(Tag, OpTag, A const& a, B const& b);
};

template <typename CompareType, typename A, typename B>
constexpr auto TypeErasedBasicCompare(CompareType, void const* a,
                                      void const* b);

template <IsOpTag Op, typename A, typename B>
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
