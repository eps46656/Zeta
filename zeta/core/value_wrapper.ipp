#pragma once

#include <zeta/core/debug_utils.hpp>
#include <zeta/core/define.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core {

template <typename ValueType, ValueType Value>
constexpr ValueType
value_wrapper::StaticValueWrapper<ValueType, Value>::operator()() const {
    return Value;
}

template <typename ValueType>
ValueType const& value_wrapper::DynamicValueWrapper<ValueType>::operator()()
    const {
    return this->value;
}

template <bool Cond, typename TX, typename TY>
constexpr decltype(auto) value_wrapper::Conditional(
    StaticValueWrapper<bool, Cond> const&, TX&& x, TY&& y) {
    if constexpr (Cond) {
        return meta::Forward<TX>(x);
    } else {
        return meta::Forward<TY>(y);
    }
}

template <typename TX, typename TY>
constexpr decltype(auto) value_wrapper::Conditional(
    DynamicValueWrapper<bool> const& cond, TX&& x, TY&& y) {
    return cond.value ? meta::Forward<TX>(x) : meta::Forward<TY>(y);
}

}  // namespace zeta::core
