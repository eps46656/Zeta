#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::value_wrapper {

template <typename T, T V>
constexpr auto StaticValueWrapper<T, V>::operator()() const {
    return V;
}

template <typename T>
T const& DynamicValueWrapper<T>::operator()() const {
    return this->value;
}

template <bool V, typename TX, typename TY>
decltype(auto) Conditional(StaticValueWrapper<bool, V> const&, TX&& x, TY&& y) {
    if constexpr (V) {
        return meta::Forward<TX>(x);
    } else {
        return meta::Forward<TY>(y);
    }
}

template <typename TX, typename TY>
decltype(auto) Conditional(DynamicValueWrapper<bool> const& cond, TX&& x,
                           TY&& y) {
    return cond.value ? meta::Forward<TX>(x) : meta::Forward<TY>(y);
}

namespace detail {

struct Merge_ {
    template <typename TX, auto VX, typename TY, auto VY>
    decltype(auto) Merge(StaticValueWrapper<TX, VX> const& x,
                         StaticValueWrapper<TY, VY> const& y) {
        ZETA_Core_StaticAssert(x.value == y.value);
        return x;
    }

    template <typename TX, auto VX, typename TY>
    decltype(auto) Merge(StaticValueWrapper<TX, VX> const& x,
                         DynamicValueWrapper<TY> const& y) {
        ZETA_Core_DebugAssert(x.value == y.value);
        return x;
    }

    template <typename TX, typename TY, auto VY>
    decltype(auto) Merge(DynamicValueWrapper<TX> const& x,
                         StaticValueWrapper<TY, VY> const& y) {
        ZETA_Core_DebugAssert(x.value == y.value);
        return y;
    }

    template <typename TX, typename TY>
    decltype(auto) Merge(DynamicValueWrapper<TX> const& x,
                         DynamicValueWrapper<TY> const& y) {
        ZETA_Core_DebugAssert(x.value == y.value);
        return x;
    }
};

}  // namespace detail

template <typename T0, typename... Ts>
decltype(auto) Merge(T0 const& x0, Ts const&... xs) {
    return TreeReduce(detail::Merge_{}, meta::Forward<T0>(x0),
                      meta::Forward<Ts>(xs)...);
}

}  // namespace zeta::core::value_wrapper
