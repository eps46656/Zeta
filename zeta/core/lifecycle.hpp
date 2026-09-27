#pragma once

#include <zeta/core/meta.hpp>
#include <zeta/core/static_seq.hpp>
#include <zeta/core/tuple.ipp>

#define ZETA_Core_Lifecycle_PackConstructArgs(...) \
    ::zeta::core::lifecycle::detail::PackConstructArgs(__VA_ARGS__)

#define ZETA_Core_Lifecycle_UnpackConstructArg(Target, Arg, arg)            \
    ::zeta::core::lifecycle::detail::UnpackConstructArg<Target, 1>::Unpack( \
        ::zeta::core::meta::Forward<Arg>(arg))

#define ZETA_Core_Lifecycle_UnpackConstructArgs(Target, Args, args) \
    ::zeta::core::lifecycle::detail::                               \
        UnpackConstructArg<Target, sizeof...(Args)>::Unpack(        \
            ::zeta::core::meta::Forward<Args>(args))...

namespace zeta::core::lifecycle {

struct DirectConstructTag {};

namespace detail {

template <typename... Args>
struct ConstructArgsTuple : public tuple::Tuple<Args...> {};

template <typename Target, typename... Args>
struct TargetedConstructArgsTuple : public tuple::Tuple<Args...> {
    using TargetType = Target;

    template <size_t... ArgIdxes>
    constexpr Target Call(static_seq::StaticSeq<size_t, ArgIdxes...>) const {
        return { this->template ForwardAccess<ArgIdxes>()... };
    }

    constexpr operator Target() const {
        return this->Call(
            static_seq::MakeLinearStaticIntegralSeq<size_t, 0, 1,
                                                    sizeof...(Args)>{});
    }
};

template <typename... Args>
constexpr decltype(auto) PackConstructArgsFromConstructArgTuple(
    ConstructArgsTuple<Args&&...> const& construct_args_tuple) {
    return construct_args_tuple;
}

template <typename... Args>
constexpr decltype(auto) PackConstructArgs(Args&&... args)
    requires(!requires {
        (PackConstructArgsFromConstructArgTuple)(meta::Forward<Args>(args)...);
    })
{
    return ConstructArgsTuple<Args&&...>{ { meta::Forward<Args>(args)... } };
}

template <typename... Args>
constexpr decltype(auto) PackConstructArgs(
    ConstructArgsTuple<Args&&...> const& construct_args_tuple) {
    return (PackConstructArgsFromConstructArgTuple)(construct_args_tuple);
}

template <typename Target, size_t UpperArgsCnt>
struct UnpackConstructArg {
    template <typename... Args, size_t... ArgIdxes>
    static constexpr decltype(auto) UnpackFromConstructArgsTuple(
        ConstructArgsTuple<Args&&...> const& construct_args_tuple,
        static_seq::StaticSeq<size_t, ArgIdxes...>) {
        if constexpr (UpperArgsCnt == 1) {
            return TargetedConstructArgsTuple<Target, Args&&...>{
                { construct_args_tuple.template ForwardAccess<ArgIdxes>()... }
            };
        } else {
            return meta::Forward<ConstructArgsTuple<Args&&...>>(
                construct_args_tuple);
        }
    }

    template <typename... Args, size_t... ArgIdxes>
    static constexpr decltype(auto) UnpackFromConstructArgsTuple(
        ConstructArgsTuple<Args&&...> const& construct_args_tuple) {
        return (UnpackFromConstructArgsTuple)(construct_args_tuple,
                                              static_seq::
                                                  MakeLinearStaticIntegralSeq<
                                                      size_t, 0, 1,
                                                      sizeof...(Args)>{});
    }

    template <typename Arg>
    static constexpr decltype(auto) Unpack(Arg&& arg)
        requires(!requires {
            (UnpackFromConstructArgsTuple)(meta::Forward<Arg>(arg));
        })
    {
        return meta::Forward<Arg>(arg);
    }

    template <typename... Args>
    static constexpr decltype(auto) Unpack(
        ConstructArgsTuple<Args&&...> const& construct_args_tuple) {
        return (UnpackFromConstructArgsTuple)(construct_args_tuple);
    }
};

}  // namespace detail

namespace example {

struct Point {
    int x;
    int y;

    constexpr Point(int xy) : x{ xy }, y{ xy } {}
    constexpr Point(int x, int y) : x{ x }, y{ y } {}
};

struct C {
    Point point_a;
    Point point_b;

    template <typename PointAConstructArg, typename PointBConstructArg>
    constexpr C(PointAConstructArg&& point_a_construct_arg,
                PointBConstructArg&& point_b_construct_arg)
        : point_a{ ZETA_Core_Lifecycle_UnpackConstructArg(
              Point, PointAConstructArg, point_a_construct_arg) },
          point_b{ ZETA_Core_Lifecycle_UnpackConstructArg(
              Point, PointBConstructArg, point_b_construct_arg) } {}
};

constexpr void F() {
    C c1{ 1, 2 };
    C c2{ 1, ZETA_Core_Lifecycle_PackConstructArgs(2, 3) };
    C c3{ ZETA_Core_Lifecycle_PackConstructArgs(1, 2), 3 };
    C c4{ ZETA_Core_Lifecycle_PackConstructArgs(1, 2),
          ZETA_Core_Lifecycle_PackConstructArgs(
              ZETA_Core_Lifecycle_PackConstructArgs(3, 4)) };

    ZETA_Core_Unused(c1);
    ZETA_Core_Unused(c2);
    ZETA_Core_Unused(c3);
    ZETA_Core_Unused(c4);
}

}  // namespace example

}  // namespace zeta::core::lifecycle
