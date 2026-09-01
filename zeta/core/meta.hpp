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

namespace detail {

template <typename TA, typename TB>
struct IsMatched_ {
    ZETA_Core_StaticAssert(
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

namespace detail {

template <typename T>
struct RemoveRef_ {
    using Type = T;
};

template <typename T>
struct RemoveRef_<T&> {
    using Type = T;
};

template <typename T>
struct RemoveRef_<T&&> {
    using Type = T;
};

}  // namespace detail

template <typename T>
using RemoveRef = typename detail::RemoveRef_<T>::Type;

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

template <typename T>
using RemoveAllArrays = typename detail::RemoveAllArrays_<T>::Type;

template <typename T>
using RemoveCV = RemoveConst<RemoveVolatile<T>>;

template <typename T>
using RemoveCRef = RemoveConst<RemoveRef<T>>;

template <typename T>
using RemoveVRef = RemoveVolatile<RemoveRef<T>>;

template <typename T>
using RemoveCVRef = RemoveConst<RemoveVolatile<RemoveRef<T>>>;

template <typename T>
concept IsConst = __is_const(T);

template <typename T>
concept IsVolatile = __is_volatile(T);

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

template <typename T>
concept IsVoid = __is_void(T);

template <typename T>
concept IsFloatingPoint = __is_floating_point(T);

template <typename T>
concept IsArithmetic = __is_arithmetic(T);

template <typename T>
concept IsPointer = __is_pointer(T);

template <typename T>
concept IsPointerToMember = __is_member_pointer(T);

template <typename T>
concept IsNullPointer = IsSame<T, decltype(nullptr)>;

template <typename T>
concept IsEnum = __is_enum(T);

template <typename T>
concept IsScalar = __is_scalar(T);

template <typename T>
concept IsClass = __is_class(T);

template <typename T>
concept IsArray = __is_array(T);

template <typename T>
concept IsUnion = __is_union(T);

template <typename T>
concept IsCompound = __is_compound(T);

template <typename T>
concept IsObject = __is_object(T);

template <typename T>
concept IsLValueRef = __is_lvalue_reference(T);

template <typename T>
concept IsRValueRef = __is_rvalue_reference(T);

template <typename T>
concept IsRef = __is_reference(T);

template <typename T>
concept IsFunction = __is_function(T);

template <typename T>
concept IsEmpty = __is_empty(T);

template <typename Base, typename Derived>
concept IsBaseOf = __is_base_of(Base, Derived);

template <typename T>
concept IsTriviallyConstructible = __is_trivially_constructible(T);

template <typename T>
concept IsTriviallyDestructible = __is_trivially_destructible(T);

template <typename T>
concept IsComplete = __is_complete_type(T);

template <typename... T>
using VoidT = void;

template <IsEnum Enum>
using UnderlyingType = __underlying_type(Enum);

template <IsEnum Enum>
constexpr UnderlyingType<Enum> ToUnderlying(Enum e) {
    return static_cast<UnderlyingType<Enum>>(e);
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

namespace detail {

template <bool Cond, typename _ = void>
struct EnableIf_;

template <>
struct EnableIf_<true, void> {
    using Type = void;
};

template <>
struct EnableIf_<false, void> {};

}  // namespace detail

template <bool Cond, typename _ = void>
using EnableIf = typename detail::EnableIf_<Cond, _>::Type;

template <typename FromT, typename ToT>
concept IsConvertible = __is_convertible(FromT, ToT);

template <typename T>
constexpr RemoveRef<T>&& Move(
    T&& t  // NOLINT(cppcoreguidelines-missing-std-forward)
) {
    return static_cast<RemoveRef<T>&&>(t);
}

template <typename T>
constexpr T&& Forward(RemoveRef<T>& t) {
    return static_cast<T&&>(t);
}

namespace detail {

template <typename, typename Func, typename... Args>
struct IsInvocable_ {
    static constexpr bool value{ false };
};

template <typename Func, typename... Args>
struct IsInvocable_<VoidT<decltype(Declval<Func>()(Declval<Args>()...))>, Func,
                    Args...> {
    static constexpr bool value{ true };
};

template <typename, typename Ret, typename Func, typename... Args>
struct IsInvocableR_ {
    static constexpr bool value{ false };
};

template <typename Ret, typename Func, typename... Args>
struct IsInvocableR_<
    EnableIf<IsConvertible<decltype(Declval<Func>()(Declval<Args>()...)), Ret>>,
    Ret, Func, Args...> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename Func, typename... Args>
constexpr bool IsInvocable{ detail::IsInvocable_<void, Func, Args...>::value };

template <typename Ret, typename Func, typename... Args>
constexpr bool IsInvocableR{
    detail::IsInvocableR_<void, Ret, Func, Args...>::value
};

template <typename Type_>
struct TypeWrapper {
    using Type = Type_;
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
    IsTypeWrapper<T> && meta::IsSame<typename T::Type, TargetType>;

template <IsTypeWrapper T>
using GetTypeWrapperType = typename T::Type;

template <typename Type_>
struct ConvertibleTypeWrapper {
    using Type = Type_;

    constexpr ConvertibleTypeWrapper() = default;

    template <typename OtherType>
        requires IsConvertible<OtherType, Type>
    constexpr ConvertibleTypeWrapper  // NOLINT(
                                      // google-explicit-constructor,
                                      // hicpp-explicit-conversions)
        (ConvertibleTypeWrapper<OtherType> const&){};
};

template <typename Type_, Type_ Value_>
struct ValueWrapper {
    using Type = Type_;

    static constexpr Type value{ Value_ };

    constexpr ValueWrapper() = default;

    template <typename OtherType, OtherType OtherValue>
        requires IsConvertible<OtherType, Type> &&
                 (static_cast<Type>(OtherValue) == value)
    constexpr ValueWrapper  // NOLINT(
                            // google-explicit-constructor,
                            // hicpp-explicit-conversions)
        (ValueWrapper<OtherType, OtherValue> const&){};
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
    IsValueWrapper<T> && meta::IsSame<typename T::Type, TargetType>;

template <typename T, typename TargetType, TargetType Value>
concept IsValueWrapperTV =
    IsValueWrapperT<T, TargetType> && (T::Value == Value);

template <IsValueWrapper T>
using GetValueWrapperType = typename T::Type;

template <IsValueWrapper T>
constexpr GetValueWrapperType<T> GetValueWrapperValue{ T::value };

template <auto Value>
using AutoValueWrapper = ValueWrapper<RemoveCVRef<decltype(Value)>, Value>;

template <typename T>
T& GetInstRef(T* inst) {
    return *inst;
}

template <typename T>
T& GetInstRef(T& inst) {
    return inst;
}

template <typename T>
using GetInstRefType = RemoveRef<decltype(GetInstRef(Declval<T>()))>;

template <typename T>
T* GetInstPtr(T* inst) {
    return inst;
}

template <typename T>
T* GetInstPtr(T& inst) {
    return &inst;
}

template <typename T>
T* GetInstPtr(T&& inst) = delete;

}  // namespace zeta::core::meta
