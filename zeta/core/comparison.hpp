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

struct OpTags {
    using Order = meta::ValueWrapper<Bits::Value, 0>;

    using Equal = meta::ValueWrapper<Bits::Value, Bits::equal>;

    using NotEqual =
        meta::ValueWrapper<Bits::Value, Bits::less | Bits::greater>;

    using Less = meta::ValueWrapper<Bits::Value, Bits::less>;

    using LessEqual = meta::ValueWrapper<Bits::Value, Bits::less | Bits::equal>;

    using Greater = meta::ValueWrapper<Bits::Value, Bits::greater>;

    using GreaterEqual =
        meta::ValueWrapper<Bits::Value, Bits::greater | Bits::equal>;
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
    Less    = Bits::less,
    Equal   = Bits::equal,
    Greater = Bits::greater,
};

template <IsOpTag OpTag>
constexpr auto DecayOrdering(OpTag, Ordering ordering);

template <typename Comparator, typename A, typename B>
concept CanCompare_ =
    requires(Comparator const& cmptr, Tag tag, A const& a, B const& b) {
        requires meta::IsNonConstObjectType<Comparator>;

        requires meta::IsNonConstObjectType<A>;

        requires meta::IsNonConstObjectType<B>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(cmptr.Compare(
                tag, OpTags::Order{}, a, b))>,
            Ordering>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(cmptr.Compare(
                tag, OpTags::Equal{}, a, b))>,
            bool>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(cmptr.Compare(
                tag, OpTags::NotEqual{}, a, b))>,
            bool>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(cmptr.Compare(
                tag, OpTags::Less{}, a, b))>,
            bool>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(cmptr.Compare(
                tag, OpTags::LessEqual{}, a, b))>,
            bool>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(cmptr.Compare(
                tag, OpTags::Greater{}, a, b))>,
            bool>;

        requires meta::IsSame<
            meta::RemoveConstVolatileReference<decltype(cmptr.Compare(
                tag, OpTags::GreaterEqual{}, a, b))>,
            bool>;
    };

template <typename Comparator, typename A, typename B>
concept CanCompare = CanCompare_<meta::RemoveConstVolatileReference<Comparator>,
                                 meta::RemoveConstVolatileReference<A>,
                                 meta::RemoveConstVolatileReference<B>>;

template <typename Comparator, IsOpTag OpTag, typename A, typename B>
constexpr decltype(auto) Compare(Comparator const& cmptr, OpTag, A const& a,
                                 B const& b)
    requires comparison::CanCompare<Comparator, A, B>;

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
    requires(integral::IsIntegralType<A> || meta::IsPointerType<A> ||
             meta::IsArrayType<A>) &&
            (integral::IsIntegralType<B> || meta::IsPointerType<B> ||
             meta::IsArrayType<B>)
struct BasicComparator<A, B> : public NativeOperatorComparatorAdapter<A, B> {};

template <typename CompareType, typename A, typename B>
constexpr auto BasicCompare(CompareType, A const& a, B const& y);

template <typename A, typename B>
struct EnableNativeOperatorByBasicComparison {
    static constexpr bool equal_enable{ false };
    static constexpr bool not_equal_enable{ false };
    static constexpr bool less_enable{ false };
    static constexpr bool less_equal_enable{ false };
    static constexpr bool greater_enable{ false };
    static constexpr bool greater_equal_enable{ false };
};

struct UniversalBasicComparator {
    template <IsOpTag OpTag, typename A, typename B>
    static constexpr auto Compare(Tag, OpTag, A const& a, B const& b);
};

template <typename CompareType, typename A, typename B>
constexpr auto TypeErasedBasicCompare(CompareType, void const* a,
                                      void const* b);

template <IsOpTag OpTag, typename A, typename B>
struct CppStdBasicComparator {
    constexpr decltype(auto) operator()(A const& a, B const& b) const;
};

}  // namespace zeta::core::comparison

template <typename A, typename B>
constexpr bool operator==(A const& a, B const& b)
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveConstVolatileReference<A>,
        zeta::core::meta::RemoveConstVolatileReference<B>>::equal_enable;

template <typename A, typename B>
constexpr bool operator!=(A const& a, B const& b)
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveConstVolatileReference<A>,
        zeta::core::meta::RemoveConstVolatileReference<B>>::not_equal_enable;

template <typename A, typename B>
constexpr bool operator<(A const& a, B const& b)
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveConstVolatileReference<A>,
        zeta::core::meta::RemoveConstVolatileReference<B>>::less_enable;

template <typename A, typename B>
constexpr bool operator<=(A const& a, B const& b)
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveConstVolatileReference<A>,
        zeta::core::meta::RemoveConstVolatileReference<B>>::less_equal_enable;

template <typename A, typename B>
constexpr bool operator>(A const& a, B const& b)
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveConstVolatileReference<A>,
        zeta::core::meta::RemoveConstVolatileReference<B>>::greater_enable;

template <typename A, typename B>
constexpr bool operator>=(A const& a, B const& b)
    requires zeta::core::comparison::EnableNativeOperatorByBasicComparison<
        zeta::core::meta::RemoveConstVolatileReference<A>,
        zeta::core::meta::RemoveConstVolatileReference<B>>::greater_equal_enable
;
