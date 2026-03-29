#pragma once

#include <zeta/core/meta.hpp>

namespace zeta::core::value_wrapper {

template <typename ValueType_, ValueType_ Value>
struct StaticValueWrapper {
    using ValueType = ValueType_;
    static constexpr ValueType value{ Value };

    constexpr ValueType operator()() const;
};

template <typename ValueType_>
struct DynamicValueWrapper {
    using ValueType = ValueType_;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
    ValueType const& value;

    ValueType const& operator()() const;
};

using FalseType = StaticValueWrapper<bool, false>;
using TrueType = StaticValueWrapper<bool, true>;

namespace detail {

template <typename T, typename TargetValueType>
struct IsStaticValueWrapperImpl_ {
    static constexpr bool value{ false };
};

template <typename ValueType, ValueType Value, typename TargetValueType>
struct IsStaticValueWrapperImpl_<StaticValueWrapper<ValueType, Value>,
                                 TargetValueType> {
    static constexpr bool value{ meta::IsAnyOf<ValueType, TargetValueType> };
};

}  // namespace detail

template <typename T, typename TargetValueType>
constexpr bool IsStaticValueWrapper{
    detail::IsStaticValueWrapperImpl_<T, TargetValueType>::value
};

namespace detail {

template <typename T, typename TargetValueType>
struct IsDynamicValueWrapperImpl_ {
    static constexpr bool value{ false };
};

template <typename ValueType, typename TargetValueType>
struct IsDynamicValueWrapperImpl_<DynamicValueWrapper<ValueType>,
                                  TargetValueType> {
    static constexpr bool value{ meta::IsAnyOf<ValueType, TargetValueType> };
};

}  // namespace detail

template <typename T, typename TargetValueType>
constexpr bool IsDynamicValueWrapper{
    detail::IsDynamicValueWrapperImpl_<T, TargetValueType>::value
};

template <typename T, typename TargetValueType>
constexpr bool IsValueWrapper{ IsStaticValueWrapper<T, TargetValueType> ||
                               IsDynamicValueWrapper<T, TargetValueType> };

template <bool Cond, typename TX, typename TY>
constexpr decltype(auto) Conditional(StaticValueWrapper<bool, Cond> const& cond,
                                     TX&& x, TY&& y);

template <typename TX, typename TY>
constexpr decltype(auto) Conditional(DynamicValueWrapper<bool> const& cond,
                                     TX&& x, TY&& y);

}  // namespace zeta::core::value_wrapper
