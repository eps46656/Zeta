#pragma once

#include <zeta/core/define.hpp>

namespace zeta::core::meta {

template <typename T>
T Declval() {};

struct NullTag {};

struct Monostate {};

/*

                    NeverMatchTag       AlwaysMatchTag      TB
NeverMatchTag       false                  X                false
AlwaysMatchTag          X               true                true
TA                  false               true                false/true

*/

struct NeverMatchedTag {};

struct AlwaysMatchedTag {};

template <typename TA, typename TB>
constexpr bool IsSame{ __is_same(TA, TB) };

// -----------------------------------------------------------------------------

namespace detail {

template <typename TA, typename TB>
struct IsMatched_ {
    static_assert(
        !((IsSame<TA, NeverMatchedTag> && IsSame<TB, AlwaysMatchedTag>) ||
          (IsSame<TA, AlwaysMatchedTag> && IsSame<TB, NeverMatchedTag>)));

    static constexpr bool value{ IsSame<TA, AlwaysMatchedTag> ||
                                 IsSame<TB, AlwaysMatchedTag> ||
                                 (!IsSame<TA, NeverMatchedTag> &&
                                  !IsSame<TB, NeverMatchedTag> &&
                                  IsSame<TA, TB>)};
};

}  // namespace detail

template <typename TA, typename TB>
concept IsMatched = detail::IsMatched_<TA, TB>::value;

// -----------------------------------------------------------------------------

namespace detail {

template <typename X, typename... Ts>
struct IsAnySame_;

template <typename X>
struct IsAnySame_<X> {
    static constexpr bool value{ false };
};

template <typename X, typename T0, typename... Ts>
struct IsAnySame_<X, T0, Ts...> {
    static constexpr bool value{ IsSame<X, T0> || IsAnySame_<X, Ts...>::value };
};

}  // namespace detail

template <typename X, typename... Ts>
concept IsAnySame = detail::IsAnySame_<X, Ts...>::value;

// -----------------------------------------------------------------------------

namespace detail {

template <typename X, typename... Ts>
struct IsAnyMatched_;

template <typename X>
struct IsAnyMatched_<X> {
    static constexpr bool value{ false };
};

template <typename X, typename T0, typename... Ts>
struct IsAnyMatched_<X, T0, Ts...> {
    static constexpr bool value{ IsMatched<X, T0> ||
                                 IsAnyMatched_<X, Ts...>::value };
};

}  // namespace detail

template <typename X, typename... Ts>
concept IsAnyMatched = detail::IsAnyMatched_<X, Ts...>::value;

// -----------------------------------------------------------------------------

namespace detail {

template <typename T>
struct RemoveReferenceFromType_ {
    using Type = T;
};

template <typename T>
struct RemoveReferenceFromType_<T&> {
    using Type = T;
};

template <typename T>
struct RemoveReferenceFromType_<T&&> {
    using Type = T;
};

}  // namespace detail

template <typename T>
using RemoveReferenceFromType =
    typename detail::RemoveReferenceFromType_<T>::Type;

// -----------------------------------------------------------------------------

namespace detail {

template <typename T>
struct RemoveConst_ {
    using Type = T;
};

template <typename T>
struct RemoveConst_<T const> {
    using Type = T;
};

}  // namespace detail

template <typename T>
using RemoveConst = typename detail::RemoveConst_<T>::Type;

// -----------------------------------------------------------------------------

namespace detail {

template <typename T>
struct RemoveVolatile_ {
    using Type = T;
};

template <typename T>
struct RemoveVolatile_<volatile T> {
    using Type = T;
};

}  // namespace detail

template <typename T>
using RemoveVolatile = typename detail::RemoveVolatile_<T>::Type;

// -----------------------------------------------------------------------------

namespace detail {

template <typename T>
struct RemoveArray_ {
    using Type = T;
};

template <typename T>
struct RemoveArray_<T[]> {
    using Type = T;
};

template <typename T, size_t N>
struct RemoveArray_<T[N]> {
    using Type = T;
};

}  // namespace detail

template <typename T>
using RemoveArray = typename detail::RemoveArray_<T>::Type;

// -----------------------------------------------------------------------------

namespace detail {

template <typename T>
struct RemoveAllArrays_ {
    using Type = T;
};

template <typename T>
struct RemoveAllArrays_<T[]> {
    using Type = typename RemoveAllArrays_<T>::Type;
};

template <typename T, size_t N>
struct RemoveAllArrays_<T[N]> {
    using Type = typename RemoveAllArrays_<T>::Type;
};

}  // namespace detail

// -----------------------------------------------------------------------------

template <typename T>
using RemoveAllArrays = typename detail::RemoveAllArrays_<T>::Type;

template <typename T>
using RemoveConstVolatile = RemoveConst<RemoveVolatile<T>>;

template <typename T>
using RemoveConstReference = RemoveConst<RemoveReferenceFromType<T>>;

template <typename T>
using RemoveVolatileReference = RemoveVolatile<RemoveReferenceFromType<T>>;

template <typename T>
using RemoveConstVolatileReference =
    RemoveConstVolatile<RemoveReferenceFromType<T>>;

// -----------------------------------------------------------------------------

namespace detail {

template <typename T>
struct RemovePointer_ {
    using Type = T;
};

template <typename T>
struct RemovePointer_<T*> {
    using Type = T;
};

}  // namespace detail

template <typename T>
using RemovePointer = typename detail::RemovePointer_<T>::Type;

// -----------------------------------------------------------------------------

template <typename T>
concept IsVoidType = __is_void(T);

template <typename T>
concept IsFloatingPointType = __is_floating_point(T);

template <typename T>
concept IsArithmeticType = __is_arithmetic(T);

template <typename T>
concept IsPointerType = __is_pointer(T);

template <typename T>
concept IsPointerToMember = __is_member_pointer(T);

template <typename T>
concept IsNullPtrType = IsSame<T, decltype(nullptr)>;

template <typename T>
concept IsEnumType = __is_enum(T);

template <typename T>
concept IsScalarType = __is_scalar(T);

template <typename T>
concept IsClassType = __is_class(T);

template <typename T>
concept IsArrayType = __is_array(T);

template <typename T>
concept IsUnionType = __is_union(T);

template <typename T>
concept IsCompoundType = __is_compound(T);

template <typename T>
concept IsObjectType = __is_object(T);

template <typename T>
concept IsFunctionType = __is_function(T);

template <typename T>
concept IsEmptyType = __is_empty(T);

template <typename T>
concept IsLeftReferenceType = __is_lvalue_reference(T);

template <typename T>
concept IsRightReferenceType = __is_rvalue_reference(T);

template <typename T>
concept IsReferenceType = __is_reference(T);

template <typename T>
concept IsNonReferenceType = !IsReferenceType<T>;

template <typename T>
concept IsConstType = __is_const(T);

template <typename T>
concept IsNonConstType = !IsConstType<T>;

template <typename T>
concept IsConstTypeWithoutReference = IsConstType<RemoveReferenceFromType<T>>;

template <typename T>
concept IsNonConstWithoutReference = IsNonConstType<RemoveReferenceFromType<T>>;

template <typename T>
concept IsVolatileType = __is_volatile(T);

template <typename T>
concept IsNonVolatileType = !IsVolatileType<T>;

template <typename T>
concept IsNonConstNonReferenceType = IsNonConstType<T> && IsNonReferenceType<T>;

template <typename T>
concept IsNonConstObjectType = IsObjectType<T> && !IsConstType<T>;

template <typename T>
concept IsConstObjectType = IsObjectType<T> && IsConstType<T>;

/*

type
    void type
    object type
        scalar type
            arithmetic type
                integral type
                    unsigned integral type
                    signed integral type
                floating point type
            pointer type
            pointer to member type
            null pointer type
            enumeration type
        compound type
            class type
            union type
            array type
    reference type
        lvalue reference type
        rvalue reference type
    function type

*/

template <typename Base, typename Derived>
concept IsBaseOf = __is_base_of(Base, Derived);

template <typename T>
concept IsTriviallyConstructible = __is_trivially_constructible(T);

template <typename T>
concept IsTriviallyDestructible = __is_trivially_destructible(T);

template <IsEnumType Enum>
using GetEnumUnderlyingType = __underlying_type(Enum);

template <IsEnumType Enum>
constexpr GetEnumUnderlyingType<Enum> EnumToUnderlying(Enum e) {
    return static_cast<GetEnumUnderlyingType<Enum>>(e);
}

namespace detail {

template <bool Cond, typename T1, typename T2>
struct Conditional_;

template <typename T1, typename T2>
struct Conditional_<true, T1, T2> {
    using Type = T1;
};

template <typename T1, typename T2>
struct Conditional_<false, T1, T2> {
    using Type = T2;
};

}  // namespace detail

template <bool Cond, typename T1, typename T2>
using Conditional = typename detail::Conditional_<Cond, T1, T2>::Type;

template <bool Cond, typename T>
using ConditionalAddConst = Conditional<Cond, T const, T>;

template <bool Cond, typename T>
using ConditionalAddVolatile = Conditional<Cond, T volatile, T>;

constexpr bool Implies(bool cond_p, bool cond_q) { return !cond_p || cond_q; }

template <typename T>
concept IsReferable = IsSame<T, RemoveReferenceFromType<T>>;

template <typename FromT, typename ToT>
concept IsConvertible = __is_convertible(FromT, ToT);

template <typename T>
constexpr RemoveReferenceFromType<T>&& Move(
    T&& t  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    return static_cast<RemoveReferenceFromType<T>&&>(t);
}

template <IsNonReferenceType Src, IsNonReferenceType Dst>
using CopyConst = ConditionalAddConst<IsConstType<Src>, RemoveConst<Dst>>;

template <IsNonReferenceType Src, IsNonReferenceType Dst>
using CopyVolatile =
    ConditionalAddVolatile<IsVolatileType<Src>, RemoveVolatile<Dst>>;

template <IsNonReferenceType Src, IsNonReferenceType Dst>
using CopyConstVolatile = CopyConst<Src, CopyVolatile<Src, Dst>>;

namespace detail {

template <typename Src, typename Dst>
struct CopyReferenceImpl_ {
    using Type = Dst;
};

template <typename Src, typename Dst>
struct CopyReferenceImpl_<Src&, Dst> {
    using Type = Dst&;
};

template <typename Src, typename Dst>
struct CopyReferenceImpl_<Src&&, Dst> {
    using Type = Dst&&;
};

}  // namespace detail

template <typename Src, typename Dst>
using CopyReference =
    typename detail::CopyReferenceImpl_<Src,
                                        RemoveReferenceFromType<Dst>>::Type;

template <typename Src, typename Dst>
using CopyConstVolatileReference =
    CopyReference<Src, CopyConstVolatile<RemoveReferenceFromType<Src>,
                                         RemoveReferenceFromType<Dst>>>;

template <typename T>
constexpr T&& Forward(RemoveReferenceFromType<T>& t) {
    return static_cast<T&&>(t);
}

template <typename Type_>
struct TypeWrapper {
    using AnUglyTypeAliasToDiscourageDirectAccess = Type_;
};

namespace detail {

template <typename T>
struct IsTypeWrapperImpl_ {
    static constexpr bool value{ false };
};

template <typename Type>
struct IsTypeWrapperImpl_<TypeWrapper<Type>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
concept IsTypeWrapper = detail::IsTypeWrapperImpl_<T>::value;

template <typename T, typename TargetType>
concept IsTypeWrapperT =
    IsTypeWrapper<T> &&
    meta::IsSame<typename T::AnUglyTypeAliasToDiscourageDirectAccess,
                 TargetType>;

template <IsTypeWrapper T>
    requires IsTypeWrapper<T>
using GetTypeWrapperType = typename T::AnUglyTypeAliasToDiscourageDirectAccess;

template <typename Type_>
struct ConvertibleTypeWrapper {
    using AnUglyNameOfTypeTodiscourageUserDirectlyAccess = Type_;

    constexpr ConvertibleTypeWrapper() = default;

    template <typename OtherType>
        requires IsConvertible<OtherType,
                               AnUglyNameOfTypeTodiscourageUserDirectlyAccess>
    constexpr ConvertibleTypeWrapper  // NOLINT(
                                      // google-explicit-constructor,
                                      // hicpp-explicit-conversions)
        (ConvertibleTypeWrapper<OtherType> const&){};
};

template <typename Type_, Type_ Value_>
struct ValueWrapper {
    using AnUglyTypeAliasToDiscourageDirectAccess = Type_;

    static constexpr AnUglyTypeAliasToDiscourageDirectAccess
        an_ugly_value_alias_to_discourage_direct_access{ Value_ };

    constexpr ValueWrapper() = default;

    constexpr ValueWrapper(ValueWrapper const&) = default;
};

namespace detail {

template <typename T>
struct IsValueWrapperImpl_ {
    static constexpr bool value{ false };
};

template <typename Type, Type Value>
struct IsValueWrapperImpl_<ValueWrapper<Type, Value>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
concept IsValueWrapper = detail::IsValueWrapperImpl_<T>::value;

template <typename T, typename TargetType>
concept IsValueWrapperT =
    IsValueWrapper<T> &&
    meta::IsSame<typename T::AnUglyTypeAliasToDiscourageDirectAccess,
                 TargetType>;

template <typename T, typename TargetType, TargetType Value>
concept IsValueWrapperTV =
    IsValueWrapperT<T, TargetType> &&
    (T::an_ugly_value_alias_to_discourage_direct_access == Value);

template <IsValueWrapper T>
    requires IsValueWrapper<T>
using GetValueWrapperType = typename T::AnUglyTypeAliasToDiscourageDirectAccess;

template <IsValueWrapper T>
    requires IsValueWrapper<T>
constexpr GetValueWrapperType<T> GetValueWrapperValue{
    T::an_ugly_value_alias_to_discourage_direct_access
};

template <typename T, typename TargetType>
concept IsValueOrValueWrapper =
    meta::IsSame<T, TargetType> || IsValueWrapperT<T, TargetType>;

template <auto Value>
using AutoValueWrapper =
    ValueWrapper<RemoveConstVolatileReference<decltype(Value)>, Value>;

template <typename T>
constexpr T& GetInstRefFromLike(T* inst) {
    return *inst;
}

template <typename T>
constexpr T& GetInstRefFromLike(T& inst) {
    return inst;
}

template <typename T>
constexpr T&& GetInstRefFromLike(T&& inst) {
    return (Move)(inst);
}

template <typename T>
constexpr T* GetInstPtrFromLike(T* inst) {
    return inst;
}

template <typename T>
constexpr T* GetInstPtrFromLike(T& inst) {
    return &inst;
}

template <typename T>
constexpr T* GetInstPtrFromLike(T&& inst) = delete;

template <typename T>
using GetInstTypeFromLike =
    RemoveReferenceFromType<decltype((GetInstRefFromLike)((Declval<T&>)()))>;

// This function is called to make a consteval expression fail to compile.
// Do not add constexpr to this function.
inline void ConstantEvaluationFailure() {}

}  // namespace zeta::core::meta
