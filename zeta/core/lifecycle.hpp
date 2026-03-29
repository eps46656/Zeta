#pragma once

#include <zeta/core/tuple.hpp>

namespace zeta::core::lifecycle {

template <typename T, typename = void>
struct Traits {
    static constexpr void Init(T&);

    template <typename Arg>
    static constexpr void Init(T& obj, Arg&& arg);

    static constexpr void Deinit(T&);
};

template <typename... Ts>
struct InitArgsTuple {
    tuple::Tuple<Ts...> t;
};

template <typename... Args>
constexpr auto MakeInitArgsTuple(Args&&... args);

struct SkipInitTag {};

template <typename T>
constexpr void Init(T&, SkipInitTag);

template <typename T, typename... Args>
constexpr void Init(T& obj, Args&&... args);

template <typename T, typename... Args>
constexpr void Init(T& obj, InitArgsTuple<Args...>& t);

template <typename T, typename... Args>
constexpr void Init(T& obj, InitArgsTuple<Args...> const& t);

template <typename T, typename... Args>
constexpr void Init(T& obj, InitArgsTuple<Args...>&& t);

template <typename T, typename... Args>
constexpr void Deinit(T& obj);

}  // namespace zeta::core::lifecycle
