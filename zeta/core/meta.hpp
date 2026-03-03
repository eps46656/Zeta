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
struct GetNth_ {
    template <typename Arg0, typename Arg1, typename Arg2, typename Arg3,
              typename... Args>
    static constexpr decltype(auto) Get(Arg0&&, Arg1&&, Arg2&&, Arg3&&,
                                        Args&&... args) {
        return GetNth_<N - 4>::Get(Forward<Args>(args)...);
    }
};

template <>
struct GetNth_<0> {
    template <typename Arg0, typename... Args>
    static constexpr decltype(auto) Get(Arg0&& x, Args&&...) {
        return Forward<Arg0>(x);
    }
};

template <>
struct GetNth_<1> {
    template <typename Arg0, typename Arg1, typename... Args>
    static constexpr decltype(auto) Get(Arg0&&, Arg1&& x, Args&&...) {
        return Forward<Arg1>(x);
    }
};

template <>
struct GetNth_<2> {
    template <typename Arg0, typename Arg1, typename Arg2, typename... Args>
    static constexpr decltype(auto) Get(Arg0&&, Arg1&&, Arg2&& x, Args&&...) {
        return Forward<Arg2>(x);
    }
};

template <>
struct GetNth_<3> {
    template <typename Arg0, typename Arg1, typename Arg2, typename Arg3,
              typename... Args>
    static constexpr decltype(auto) Get(Arg0&&, Arg1&&, Arg2&&, Arg3&& x,
                                        Args&&...) {
        return Forward<Arg3>(x);
    }
};

}  // namespace detail

template <size_t N, typename... Args>
constexpr decltype(auto) GetNth(Args&&... args) {
    static_assert(N < 1 + sizeof...(Args));
    return detail::GetNth_<N>::Get(Forward<Args>(args)...);
}

}  // namespace zeta::core::meta
