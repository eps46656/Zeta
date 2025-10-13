#pragma once

namespace zeta::core::value_wrapper {

template <auto V>
struct StaticValueWrapper {
    static constexpr auto value{ V };

    constexpr auto operator()() const;
};

template <typename T>
struct DynamicValueWrapper {
    T const& value;

    T const& operator()() const;
};

// -----------------------------------------------------------------------------

namespace detail {

template <typename T>
struct IsStaticValueWrapperImpl_ {
    static constexpr bool value{ false };
};

template <auto V>
struct IsStaticValueWrapperImpl_<StaticValueWrapper<V>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
constexpr bool IsStaticValueWrapper{
    detail::IsStaticValueWrapperImpl_<T>::value
};

// -----------------------------------------------------------------------------

namespace detail {

template <typename T>
struct IsDynamicValueWrapperImpl_ {
    static constexpr bool value{ false };
};

template <typename T>
struct IsDynamicValueWrapperImpl_<DynamicValueWrapper<T>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
constexpr bool IsDynamicValueWrapper{
    detail::IsDynamicValueWrapperImpl_<T>::value
};

// -----------------------------------------------------------------------------

template <bool V, typename TX, typename TY>
decltype(auto) Conditional(StaticValueWrapper<V> const& cond, TX const& x,
                           TY const& y);

template <typename TX, typename TY>
decltype(auto) Conditional(DynamicValueWrapper<bool> const& cond, TX const& x,
                           TY const& y);

// -----------------------------------------------------------------------------

template <bool V>
decltype(auto) Any(StaticValueWrapper<V> const& v);

decltype(auto) Any(DynamicValueWrapper<bool> const& v);

template <bool VX, bool VY>
decltype(auto) Any(StaticValueWrapper<VX> const& x,
                   StaticValueWrapper<VY> const& y);

template <bool VX>
decltype(auto) Any(StaticValueWrapper<VX> const& x,
                   DynamicValueWrapper<bool> const& y);

template <bool VY>
decltype(auto) Any(DynamicValueWrapper<bool> const& x,
                   StaticValueWrapper<VY> const& y);

decltype(auto) Any(DynamicValueWrapper<bool> const& x,
                   DynamicValueWrapper<bool> const& y);

template <typename TX, typename TY, typename TZ, typename... TS>
decltype(auto) Any(TX const& x, TY const& y, TZ const& z, TS const&... s);

// -----------------------------------------------------------------------------

template <bool V>
decltype(auto) All(StaticValueWrapper<V> const& v);

decltype(auto) All(DynamicValueWrapper<bool> const& v);

template <bool VX, bool VY>
decltype(auto) All(StaticValueWrapper<VX> const& x,
                   StaticValueWrapper<VY> const& y);

template <bool VX>
decltype(auto) All(StaticValueWrapper<VX> const& x,
                   DynamicValueWrapper<bool> const& y);

template <bool VY>
decltype(auto) All(DynamicValueWrapper<bool> const& x,
                   StaticValueWrapper<VY> const& y);

decltype(auto) All(DynamicValueWrapper<bool> const& x,
                   DynamicValueWrapper<bool> const& y);

template <typename TX, typename TY, typename TZ, typename... TS>
decltype(auto) All(TX const& x, TY const& y, TZ const& z, TS const&... s);

// -----------------------------------------------------------------------------

template <auto V>
decltype(auto) Merge(StaticValueWrapper<V> const& x);

template <typename TY>
decltype(auto) Merge(DynamicValueWrapper<TY> const& x);

template <auto VX, auto VY>
decltype(auto) Merge(StaticValueWrapper<VX> const& x,
                     StaticValueWrapper<VY> const& y);

template <auto VX, typename TY>
decltype(auto) Merge(StaticValueWrapper<VX> const& x,
                     DynamicValueWrapper<TY> const& y);

template <typename TX, auto VY>
decltype(auto) Merge(DynamicValueWrapper<TX> const& x,
                     StaticValueWrapper<VY> const& y);

template <typename TX, typename TY>
decltype(auto) Merge(DynamicValueWrapper<TX> const& x,
                     DynamicValueWrapper<TY> const& y);

template <typename TX, typename TY, typename TZ, typename... TS>
decltype(auto) Merge(TX const& x, TY const& y, TZ const& z, TS const&... s);

}  // namespace zeta::core::value_wrapper
