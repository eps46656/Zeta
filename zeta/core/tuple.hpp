#pragma once

#include <zeta/core/static_seq.hpp>

namespace zeta::core::tuple {

namespace detail {

template <size_t Idx, typename Elem_>
struct ElemWrapper_ {
    using Elem = Elem_;
    Elem elem;
};

template <typename StaticSeq, typename... Elems>
struct Tuple_ {
    ZETA_Core_StaticAssert(false);
};

template <size_t... Idxes, typename... Elems>
struct Tuple_<static_seq::StaticSeq<size_t, Idxes...>, Elems...>
    : public ElemWrapper_<Idxes, Elems>... {
    static constexpr size_t ElemsCnt{ sizeof...(Elems) };

    constexpr Tuple_() = default;

    constexpr Tuple_(Tuple_ const& t) = default;

    constexpr Tuple_(Tuple_&& t) = default;

    template <size_t... ArgIdxes, typename... Args>
    constexpr Tuple_(static_seq::StaticSeq<size_t, ArgIdxes...>,
                     Args&&... args);

    constexpr bool operator==(Tuple_ const& t) const = default;

    constexpr bool operator!=(Tuple_ const& t) const = default;

    template <size_t Idx>
    constexpr decltype(auto) ForwardAccess();

    template <size_t Idx>
    constexpr auto& Access() &;

    template <size_t Idx>
    constexpr auto const& Access() const&;

    template <size_t Idx>
    constexpr auto&& Access() &&;
};

}  // namespace detail

template <typename... Elems>
struct Tuple : public detail::Tuple_<static_seq::MakeLinearStaticIntegralSeq<
                                         size_t, 0, 1, sizeof...(Elems)>,
                                     Elems...> {
    constexpr Tuple() = default;

    constexpr Tuple(Tuple const& t) = default;

    constexpr Tuple(Tuple&& t) = default;

    template <typename... Args>
    constexpr Tuple(Args&&... args);

    constexpr bool operator==(Tuple const& t) const = default;

    constexpr bool operator!=(Tuple const& t) const = default;
};

namespace detail {

template <typename T>
struct IsTuple_ {
    static constexpr bool value{ false };
};

template <typename... Elems>
struct IsTuple_<Tuple<Elems...>> {
    static constexpr bool value{ true };
};

}  // namespace detail

template <typename T>
static constexpr bool IsTuple{ detail::IsTuple_<T>::value };

template <typename Func, typename... Elems>
constexpr decltype(auto) Apply(Func&& func, Tuple<Elems...>& t);

template <typename Func, typename... Elems>
constexpr decltype(auto) Apply(Func&& func, Tuple<Elems...> const& t);

template <typename Func, typename... Elems>
constexpr decltype(auto) Apply(Func&& func, Tuple<Elems...>&& t);

}  // namespace zeta::core::tuple
