#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::comparison {

struct Tag {};

struct Bits {
    using Value = unsigned;

    static constexpr Value less{ 0b001 };
    static constexpr Value equal{ 0b010 };
    static constexpr Value greater{ 0b100 };
};

enum struct Op : unsigned char {
    Order = 0,
    Equal = Bits::equal,
    NotEqual = Bits::less | Bits::greater,
    Less = Bits::less,
    LessEqual = Bits::less | Bits::equal,
    Greater = Bits::greater,
    GreaterEqual = Bits::greater | Bits::equal,
};

struct OpTags {
    using Order = meta::AutoValueWrapper<Op::Order>;
    using Equal = meta::AutoValueWrapper<Op::Equal>;
    using NotEqual = meta::AutoValueWrapper<Op::NotEqual>;
    using Less = meta::AutoValueWrapper<Op::Less>;
    using LessEqual = meta::AutoValueWrapper<Op::LessEqual>;
    using Greater = meta::AutoValueWrapper<Op::Greater>;
    using GreaterEqual = meta::AutoValueWrapper<Op::GreaterEqual>;
};

template <typename T>
concept IsOpTag = meta::IsAnySame<T,                    //
                                  OpTags::Order,        //
                                  OpTags::Equal,        //
                                  OpTags::NotEqual,     //
                                  OpTags::Less,         //
                                  OpTags::LessEqual,    //
                                  OpTags::Greater,      //
                                  OpTags::GreaterEqual  //
                                  >;

enum struct Ordering : unsigned char {
    Less = Bits::less,
    Equal = Bits::equal,
    Greater = Bits::greater,
};

template <IsOpTag OpTag>
constexpr auto DecayOrdering(OpTag, Ordering ordering);

template <typename Comparator, typename A, typename B>
concept CanCompare = requires(Comparator const& cmptr, Tag tag, A const& a,
                              B const& b) {
    requires meta::IsSame<
        meta::RemoveCVRef<decltype(cmptr.Compare(tag, OpTags::Order{}, a, b))>,
        Ordering>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(cmptr.Compare(tag, OpTags::Equal{}, a, b))>,
        bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(cmptr.Compare(
                              tag, OpTags::NotEqual{}, a, b))>,
                          bool>;

    requires meta::IsSame<
        meta::RemoveCVRef<decltype(cmptr.Compare(tag, OpTags::Less{}, a, b))>,
        bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(cmptr.Compare(
                              tag, OpTags::LessEqual{}, a, b))>,
                          bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(cmptr.Compare(
                              tag, OpTags::Greater{}, a, b))>,
                          bool>;

    requires meta::IsSame<meta::RemoveCVRef<decltype(cmptr.Compare(
                              tag, OpTags::GreaterEqual{}, a, b))>,
                          bool>;
};

template <typename Comparator, IsOpTag OpTag, typename A, typename B>
    requires comparison::CanCompare<Comparator, A, B>
constexpr decltype(auto) Compare(Comparator const& cmptr, OpTag, A const& a,
                                 B const& b);

template <typename A, typename B>
struct NativeOperatorComparatorAdapter {
    static constexpr Ordering Compare(Tag, OpTags::Order, A const& a,
                                      B const& b);

    static constexpr bool Compare(Tag, OpTags::Equal, A const& a, B const& b);

    static constexpr bool Compare(Tag, OpTags::NotEqual, A const& a,
                                  B const& b);

    static constexpr bool Compare(Tag, OpTags::Less, A const& a, B const& b);

    static constexpr bool Compare(Tag, OpTags::LessEqual, A const& a,
                                  B const& b);

    static constexpr bool Compare(Tag, OpTags::Greater, A const& a, B const& b);

    static constexpr bool Compare(Tag, OpTags::GreaterEqual, A const& a,
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
