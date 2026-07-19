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

struct NeverMatchTag {};

struct AlwaysMatchTag {};

template <typename TA, typename TB>
constexpr bool IsSame{ __is_same(TA, TB) };

namespace detail {

template <typename TA, typename TB>
struct IsMatched_ {
    ZETA_Core_StaticAssert(
        !((IsSame<TA, NeverMatchTag> && IsSame<TB, AlwaysMatchTag>) ||
          (IsSame<TA, AlwaysMatchTag> && IsSame<TB, NeverMatchTag>)));

    static constexpr bool value{ IsSame<TA, AlwaysMatchTag> ||
                                 IsSame<TB, AlwaysMatchTag> ||
                                 (!IsSame<TA, NeverMatchTag> &&
                                  !IsSame<TB, NeverMatchTag> &&
                                  IsSame<TA, TB>)};
};

}  // namespace detail

template <typename TA, typename TB>
constexpr bool IsMatched{ detail::IsMatched_<TA, TB>::value };

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
constexpr bool IsAnySame{ detail::IsAnySame_<X, Ts...>::value };

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
constexpr bool IsAnyMatched{ detail::IsAnyMatched_<X, Ts...>::value };

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
constexpr bool IsConst{ __is_const(T) };

template <typename T>
constexpr bool IsVolatile{ __is_volatile(T) };

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
constexpr bool IsVoid{ __is_void(T) };

template <typename T>
constexpr bool IsFloatingPoint{ __is_floating_point(T) };

template <typename T>
constexpr bool IsArithmetic{ __is_arithmetic(T) };

template <typename T>
constexpr bool IsPointer{ __is_pointer(T) };

template <typename T>
constexpr bool IsPointerToMember{ __is_member_pointer(T) };

template <typename T>
constexpr bool IsNullPointer{ IsSame<T, decltype(nullptr)> };

template <typename T>
constexpr bool IsEnum{ __is_enum(T) };

template <typename T>
constexpr bool IsScalar{ __is_scalar(T) };

template <typename T>
constexpr bool IsClass{ __is_class(T) };

template <typename T>
constexpr bool IsArray{ __is_array(T) };

template <typename T>
constexpr bool IsUnion{ __is_union(T) };

template <typename T>
constexpr bool IsCompound{ __is_compound(T) };

template <typename T>
constexpr bool IsObject{ __is_object(T) };

template <typename T>
constexpr bool IsLValueRef{ __is_lvalue_reference(T) };

template <typename T>
constexpr bool IsRValueRef{ __is_rvalue_reference(T) };

template <typename T>
constexpr bool IsRef{ __is_reference(T) };

template <typename T>
constexpr bool IsFunction{ __is_function(T) };

template <typename T>
constexpr bool IsEmpty{ __is_empty(T) };

template <typename Base, typename Derived>
constexpr bool IsBaseOf{ __is_base_of(Base, Derived) };

template <typename T>
constexpr bool IsTriviallyConstructible{ __is_trivially_constructible(T) };

template <typename T>
constexpr bool IsTriviallyDestructible{ __is_trivially_destructible(T) };

template <typename T>
constexpr bool IsComplete{ __is_complete_type(T) };

template <typename... T>
using VoidT = void;

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
constexpr bool IsConvertible{ __is_convertible(FromT, ToT) };

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

    constexpr TypeWrapper() = default;

    template <typename T, typename = EnableIf<IsConvertible<T, Type>>>
    constexpr TypeWrapper  // NOLINT(
                           // google-explicit-constructor,
                           // hicpp-explicit-conversions)
        (TypeWrapper<T> const&){};
};

template <typename T>
T& GetInstRef(T* inst) {
    return *inst;
}

template <typename T>
T& GetInstRef(T& inst) {
    return inst;
}

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
