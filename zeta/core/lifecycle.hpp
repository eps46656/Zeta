#pragma once

#include <zeta/core/meta.hpp>
#include <zeta/core/static_seq.hpp>
#include <zeta/core/tuple.ipp>

#define ZETA_Core_Lifecycle_PackInitArgs(...) \
    ::zeta::core::lifecycle::detail::PackInitArgs(__VA_ARGS__)

#define ZETA_Core_Lifecycle_UnpackInitArg(Target, Arg, arg)            \
    ::zeta::core::lifecycle::detail::UnpackInitArg<Target, 1>::Unpack( \
        ::zeta::core::meta::Forward<Arg>(arg))

#define ZETA_Core_Lifecycle_UnpackInitArgs(Target, Args, args)                \
    ::zeta::core::lifecycle::detail::UnpackInitArg<Target, sizeof...(Args)>:: \
        Unpack(::zeta::core::meta::Forward<Args>(args))...

namespace zeta::core::lifecycle::detail {

template <typename... Args>
struct InitArgsTuple : public tuple::Tuple<Args...> {};

template <typename Target, typename... Args>
struct TargetedInitArgsTuple : public tuple::Tuple<Args...> {
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
constexpr decltype(auto) PackInitArgsFromInitArgTuple(
    InitArgsTuple<Args&&...> const& init_args_tuple) {
    return init_args_tuple;
}

template <typename... Args>
constexpr decltype(auto) PackInitArgs(Args&&... args)
    requires(!requires {
        (PackInitArgsFromInitArgTuple)(meta::Forward<Args>(args)...);
    })
{
    return InitArgsTuple<Args&&...>{ { meta::Forward<Args>(args)... } };
}

template <typename... Args>
constexpr decltype(auto) PackInitArgs(
    InitArgsTuple<Args&&...> const& init_args_tuple) {
    return (PackInitArgsFromInitArgTuple)(init_args_tuple);
}

template <typename Target, size_t UpperArgsCnt>
struct UnpackInitArg {
    template <typename... Args, size_t... ArgIdxes>
    static constexpr decltype(auto) UnpackFromInitArgsTuple(
        InitArgsTuple<Args&&...> const& init_args_tuple,
        static_seq::StaticSeq<size_t, ArgIdxes...>) {
        if constexpr (UpperArgsCnt == 1) {
            return TargetedInitArgsTuple<Target, Args&&...>{
                { init_args_tuple.template ForwardAccess<ArgIdxes>()... }
            };
        } else {
            return meta::Forward<InitArgsTuple<Args&&...>>(init_args_tuple);
        }
    }

    template <typename... Args, size_t... ArgIdxes>
    static constexpr decltype(auto) UnpackFromInitArgsTuple(
        InitArgsTuple<Args&&...> const& init_args_tuple) {
        return (
            UnpackFromInitArgsTuple)(init_args_tuple,
                                     static_seq::MakeLinearStaticIntegralSeq<
                                         size_t, 0, 1, sizeof...(Args)>{});
    }

    template <typename Arg>
    static constexpr decltype(auto) Unpack(Arg&& arg)
        requires(!requires {
            (UnpackFromInitArgsTuple)(meta::Forward<Arg>(arg));
        })
    {
        return meta::Forward<Arg>(arg);
    }

    template <typename... Args>
    static constexpr decltype(auto) Unpack(
        InitArgsTuple<Args&&...> const& init_args_tuple) {
        return (UnpackFromInitArgsTuple)(init_args_tuple);
    }
};

}  // namespace zeta::core::lifecycle::detail

namespace zeta::core::lifecycle::excample {

struct Point {
    int x;
    int y;

    constexpr Point(int xy) : x{ xy }, y{ xy } {}
    constexpr Point(int x, int y) : x{ x }, y{ y } {}
};

struct C {
    Point point_a;
    Point point_b;

    template <typename PointAInitArg, typename PointBInitArg>
    constexpr C(PointAInitArg&& point_a_init_arg,
                PointBInitArg&& point_b_init_arg)
        : point_a{ ZETA_Core_Lifecycle_UnpackInitArg(Point, PointAInitArg,
                                                     point_a_init_arg) },
          point_b{ ZETA_Core_Lifecycle_UnpackInitArg(Point, PointBInitArg,
                                                     point_b_init_arg) } {}
};

constexpr void F() {
    C c1{ 1, 2 };
    C c2{ 1, ZETA_Core_Lifecycle_PackInitArgs(2, 3) };
    C c3{ ZETA_Core_Lifecycle_PackInitArgs(1, 2), 3 };
    C c4{ ZETA_Core_Lifecycle_PackInitArgs(1, 2),
          ZETA_Core_Lifecycle_PackInitArgs(
              ZETA_Core_Lifecycle_PackInitArgs(3, 4)) };

    ZETA_Core_Unused(c1);
    ZETA_Core_Unused(c2);
    ZETA_Core_Unused(c3);
    ZETA_Core_Unused(c4);
}

}  // namespace zeta::core::lifecycle::excample
