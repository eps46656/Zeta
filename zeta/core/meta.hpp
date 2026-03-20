#pragma once

namespace zeta::core::meta {

template <typename T>
T Declval() {};

struct Monostate {};

struct TypeNone {};

struct TypeAny {};

namespace detail {

/*

            TypeNone    TypeAny     T
TypeNone    false       false       false
TypeAny     true        true        true
T           false       true        false/true

*/

template <typename X, typename... Ts>
struct IsAnyOf_;

template <typename X>
struct IsAnyOf_<X> {
    static constexpr bool value{ false };
};

template <typename... Ts>
struct IsAnyOf_<TypeNone, TypeNone, Ts...> {
    static constexpr bool value{ false };
};

template <typename... Ts>
struct IsAnyOf_<TypeNone, TypeAny, Ts...> {
    static constexpr bool value{ false };
};

template <typename T0, typename... Ts>
struct IsAnyOf_<TypeNone, T0, Ts...> {
    static constexpr bool value{ false };
};

template <typename... Ts>
struct IsAnyOf_<TypeAny, TypeNone, Ts...> {
    static constexpr bool value{ true };
};

template <typename... Ts>
struct IsAnyOf_<TypeAny, TypeAny, Ts...> {
    static constexpr bool value{ true };
};

template <typename T0, typename... Ts>
struct IsAnyOf_<TypeAny, T0, Ts...> {
    static constexpr bool value{ true };
};

template <typename X, typename... Ts>
struct IsAnyOf_<X, TypeNone, Ts...> {
    static constexpr bool value{ IsAnyOf_<X, Ts...>::value };
};

template <typename X, typename... Ts>
struct IsAnyOf_<X, TypeAny, Ts...> {
    static constexpr bool value{ true };
};

template <typename X, typename T0, typename... Ts>
struct IsAnyOf_<X, T0, Ts...> {
    static constexpr bool value{ IsAnyOf_<X, Ts...>::value };
};

template <typename X, typename... Ts>
struct IsAnyOf_<X, X, Ts...> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename X, typename... Ts>
constexpr bool IsAnyOf{ detail::IsAnyOf_<X, Ts...>::value };

namespace detail {

template <typename T>
struct RemoveConst_ {
    using type = T;
};

template <typename T>
struct RemoveConst_<T const> {
    using type = T;
};

}  // namespace detail

template <typename T>
using RemoveConst = typename detail::RemoveConst_<T>::type;

namespace detail {

template <typename T>
struct RemoveVolatile_ {
    using type = T;
};

template <typename T>
struct RemoveVolatile_<volatile T> {
    using type = T;
};

}  // namespace detail

template <typename T>
using RemoveVolatile = typename detail::RemoveVolatile_<T>::type;

namespace detail {

template <typename T>
struct RemovePointer_ {
    using type = T;
};

template <typename T>
struct RemovePointer_<T*> {
    using type = T;
};

}  // namespace detail

template <typename T>
using RemovePointer = typename detail::RemovePointer_<T>::type;

namespace detail {

template <typename T>
struct RemoveRef_ {
    using type = T;
};

template <typename T>
struct RemoveRef_<T&> {
    using type = T;
};

template <typename T>
struct RemoveRef_<T&&> {
    using type = T;
};

}  // namespace detail

template <typename T>
using RemoveRef = typename detail::RemoveRef_<T>::type;

namespace detail {

template <typename T>
struct RemoveArray_ {
    using type = T;
};

template <typename T>
struct RemoveArray_<T[]> {
    using type = T;
};

template <typename T, size_t N>
struct RemoveArray_<T[N]> {
    using type = T;
};

}  // namespace detail

template <typename T>
using RemoveArray = typename detail::RemoveArray_<T>::type;

namespace detail {

template <typename T>
struct RemoveAllArrays_ {
    using type = T;
};

template <typename T>
struct RemoveAllArrays_<T[]> {
    using type = typename RemoveAllArrays_<T>::type;
};

template <typename T, size_t N>
struct RemoveAllArrays_<T[N]> {
    using type = typename RemoveAllArrays_<T>::type;
};

}  // namespace detail

template <typename T>
using RemoveAllArrays = typename detail::RemoveAllArrays_<T>::type;

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
constexpr bool IsNullPointer{ IsAnyOf<T, decltype(nullptr)> };

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

namespace detail {

template <bool Cond, typename T1, typename T2>
struct Conditional_;

template <typename T1, typename T2>
struct Conditional_<true, T1, T2> {
    using type = T1;
};

template <typename T1, typename T2>
struct Conditional_<false, T1, T2> {
    using type = T2;
};

}  // namespace detail

template <bool Cond, typename T1, typename T2>
using Conditional = typename detail::Conditional_<Cond, T1, T2>::type;

namespace detail {

template <bool Cond, typename _ = void>
struct EnableIf_;

template <>
struct EnableIf_<true, void> {
    using type = void;
};

template <>
struct EnableIf_<false, void> {};

}  // namespace detail

template <bool Cond, typename _ = void>
using EnableIf = typename detail::EnableIf_<Cond, _>::type;

namespace detail {

template <typename, typename Func, typename... Args>
struct IsInvocable_ {
    static constexpr bool value{ false };
};

template <typename Func, typename... Args>
struct IsInvocable_<decltype((
                        Declval<Func>()(Forward<Args>(Declval<Args>())...), 0)),
                    Func, Args...> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename Func, typename... Args>
constexpr bool IsInvocable{ detail::IsInvocable_<int, Func, Args...>::value };

namespace detail {

template <auto FuncInst, typename FuncType>
struct IsInvocableAny_;

template <auto FuncInst, typename Ret, typename... Args>
struct IsInvocableAny_<FuncInst, Ret(Args...)> {
    static constexpr bool value{ IsInvocable<Ret(Args...), Args...> };
};

}  // namespace detail

template <auto FuncInst>
constexpr bool IsInvocableAny{
    detail::IsInvocableAny_<FuncInst, decltype(FuncInst)>::value
};

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

template <size_t N>
struct GetNthArg_ {
    template <typename Arg0, typename Arg1, typename Arg2, typename Arg3,
              typename Arg4, typename Arg5, typename Arg6, typename Arg7,
              typename Arg8, typename Arg9, typename Arg10, typename Arg11,
              typename Arg12, typename Arg13, typename Arg14, typename Arg15,
              typename... Args>
    static constexpr decltype(auto) Get(Arg0&& x0, Arg1&& x1, Arg2&& x2,
                                        Arg3&& x3, Arg4&& x4, Arg5&& x5,
                                        Arg6&& x6, Arg7&& x7, Arg8&& x8,
                                        Arg9&& x9, Arg10&& x10, Arg11&& x11,
                                        Arg12&& x12, Arg13&& x13, Arg14&& x14,
                                        Arg15&& x15, Args&&... args) {
#pragma push_macro("F")

#define F(i)                          \
    if constexpr (N == i) {           \
        return Forward<Arg##i>(x##i); \
    } else

        F(0)
        F(1)
        F(2)
        F(3)
        F(4)
        F(5)
        F(6)
        F(7)
        F(8)
        F(9)
        F(10)
        F(11)
        F(12)
        F(13)
        F(14)
        F(15) {  //
            return GetNthArg_<N - 16>::Get(Forward<Args>(args)...);
        }

#pragma pop_macro("F")
    }
};

}  // namespace detail

template <size_t N, typename... Args>
constexpr decltype(auto) GetNthArg(Args&&... args) {
    static_assert(N < sizeof...(Args));
    return detail::GetNthArg_<N>::Get(
        Forward<Args>(args)...,              //
        nullptr, nullptr, nullptr, nullptr,  // 0 ~ 3
        nullptr, nullptr, nullptr, nullptr,  // 4 ~ 7
        nullptr, nullptr, nullptr, nullptr,  // 8 ~ 11
        nullptr, nullptr, nullptr, nullptr   // 12 ~ 15
    );
}

namespace detail {

template <typename T>
struct GetNthTypeWrapper_ {
    using type = T;
};

}  // namespace detail

template <size_t N, typename... Ts>
using GetNthType = RemoveCVRef<decltype(GetNthArg<N>(
    detail::GetNthTypeWrapper_<Ts>{}...))>::type;

}  // namespace zeta::core::meta
