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
    ValueType value;

    ValueType const& operator()() const;
};

using FalseType = StaticValueWrapper<bool, false>;
using TrueType = StaticValueWrapper<bool, true>;

namespace detail {

template <typename T>
struct IsStaticValueWrapperImpl_ {
    static constexpr bool value{ false };
};

template <typename ValueType, ValueType Value>
struct IsStaticValueWrapperImpl_<StaticValueWrapper<ValueType, Value>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
concept IsStaticValueWrapper = detail::IsStaticValueWrapperImpl_<T>::value;

namespace detail {

template <typename T, typename ValueType>
struct IsStaticValueWrapperWithImpl_ {
    static constexpr bool value{ false };
};

template <typename ValueType, ValueType Value>
struct IsStaticValueWrapperWithImpl_<StaticValueWrapper<ValueType, Value>,
                                     ValueType> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T, typename TargetValueType>
concept IsStaticValueWrapperWith =
    detail::IsStaticValueWrapperWithImpl_<T, TargetValueType>::value;

namespace detail {

template <typename T>
struct IsDynamicValueWrapper_ {
    static constexpr bool value{ false };
};

template <typename ValueType>
struct IsDynamicValueWrapper_<DynamicValueWrapper<ValueType>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
concept IsDynamicValueWrapper = detail::IsDynamicValueWrapper_<T>::value;

namespace detail {

template <typename T, typename ValueType>
struct IsDynamicValueWrapperWithImpl_ {
    static constexpr bool value{ false };
};

template <typename ValueType>
struct IsDynamicValueWrapperWithImpl_<DynamicValueWrapper<ValueType>,
                                      ValueType> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T, typename TargetValueType>
concept IsDynamicValueWrapperWith =
    detail::IsDynamicValueWrapperWithImpl_<T, TargetValueType>::value;

template <typename T>
concept IsValueWrapper = IsStaticValueWrapper<T> || IsDynamicValueWrapper<T>;

template <typename T, typename ValueType>
concept IsValueWrapperWith = IsStaticValueWrapperWith<T, ValueType> ||
                             IsDynamicValueWrapperWith<T, ValueType>;

template <bool Cond, typename TX, typename TY>
constexpr decltype(auto) Conditional(StaticValueWrapper<bool, Cond> const& cond,
                                     TX&& x, TY&& y);

template <typename TX, typename TY>
constexpr decltype(auto) Conditional(DynamicValueWrapper<bool> const& cond,
                                     TX&& x, TY&& y);

}  // namespace zeta::core::value_wrapper
