#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/type_traits.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::value_wrapper {

template <auto V>
constexpr auto StaticValueWrapper<V>::operator()() const {
    return V;
}

template <typename T>
T const& DynamicValueWrapper<T>::operator()() const {
    return this->value;
}

// -----------------------------------------------------------------------------

template <bool V, typename TX, typename TY>
decltype(auto) Conditional(StaticValueWrapper<V> const&, TX&& x, TY&& y) {
    if constexpr (V) {
        return x;
    } else {
        return y;
    }
}

template <typename TX, typename TY>
decltype(auto) Conditional(DynamicValueWrapper<bool> const& cond, TX&& x,
                           TY&& y) {
    return cond.value ? x : y;
}

// -----------------------------------------------------------------------------

namespace detail {

struct Merge_ {
    template <auto VX, auto VY>
    decltype(auto) Merge(StaticValueWrapper<VX> const& x,
                         StaticValueWrapper<VY> const& y) {
        ZETA_Core_StaticAssert(x.value == y.value);
        return x;
    }

    template <auto VX, typename TY>
    decltype(auto) Merge(StaticValueWrapper<VX> const& x,
                         DynamicValueWrapper<TY> const& y) {
        ZETA_Core_DebugAssert(x.value == y.value);
        return x;
    }

    template <typename TX, auto VY>
    decltype(auto) Merge(DynamicValueWrapper<TX> const& x,
                         StaticValueWrapper<VY> const& y) {
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
    return TreeReduce(detail::Merge_{}, Forward<T0>(x0), Forward<Ts>(xs)...);
}

}  // namespace zeta::core::value_wrapper
