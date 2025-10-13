#pragma once

namespace zeta::core {

template <typename T>
T Declval();

// -----------------------------------------------------------------------------

namespace detail {

template <typename X, typename... Ts>
struct IsAnyOf_;

template <typename X>
struct IsAnyOf_<X> {
    static constexpr bool value{ false };
};

template <typename X, typename... Ts>
struct IsAnyOf_<X, X, Ts...> {
    static constexpr bool value{ true };
};

template <typename X, typename T0, typename... Ts>
struct IsAnyOf_<X, T0, Ts...> {
    static constexpr bool value{ IsAnyOf_<X, Ts...>::value };
};

}  // namespace detail

template <typename X, typename... Ts>
constexpr bool IsAnyOf{ detail::IsAnyOf_<X, Ts...>::value };

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

template <typename T>
using RemoveCV = RemoveConst<RemoveVolatile<T>>;

template <typename T>
using RemoveCRef = RemoveConst<RemoveRef<T>>;

template <typename T>
using RemoveVRef = RemoveVolatile<RemoveRef<T>>;

template <typename T>
using RemoveCVRef = RemoveConst<RemoveVolatile<RemoveRef<T>>>;

// -----------------------------------------------------------------------------

template <typename T>
constexpr bool IsConst{ IsAnyOf<T, RemoveConst<T> const> };

template <typename T>
constexpr bool IsVolatile{ IsAnyOf<T, RemoveVolatile<T> volatile> };

template <typename T>
constexpr bool IsPointer{ IsAnyOf<T, RemovePointer<T>*> };

template <typename T>
constexpr bool IsLValueRef{ IsAnyOf<T, RemoveRef<T>&> };

template <typename T>
constexpr bool IsRValueRef{ IsAnyOf<T, RemoveRef<T>&&> };

template <typename T>
constexpr bool IsRef{ IsLValueRef<T> || IsRValueRef<T> };

template <typename T>
constexpr bool IsArray{ !IsAnyOf<T, RemoveArray<T>> };

template <typename T>
constexpr bool IsEmpty{ __is_empty(T) };

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

namespace detail {

template <typename Base, typename Derived>
struct IsBaseOf_ {
    using BareBase = RemoveCVRef<Base>;
    using BareDerived = RemoveCVRef<Derived>;

    static constexpr bool test(Base*) { return true; }
    static constexpr bool test(...) { return false; }

    static constexpr bool value{ !IsAnyOf<void, BareBase> &&
                                 test(static_cast<BareDerived*>(nullptr)) };
};

}  // namespace detail

template <typename Base, typename Derived>
constexpr bool IsBaseOf{ detail::IsBaseOf_<Base, Derived>::value };

// -----------------------------------------------------------------------------

namespace detail {

template <bool Cond, typename T, typename _ = void>
struct EnableIf_;

template <typename T>
struct EnableIf_<true, T, void> {
    using type = T;
};

template <typename T>
struct EnableIf_<false, T, void> {};

}  // namespace detail

template <bool Cond, typename T, typename _ = void>
using EnableIf = typename detail::EnableIf_<Cond, T, _>::type;

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

}  // namespace zeta::core
