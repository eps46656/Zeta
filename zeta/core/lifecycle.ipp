#pragma once

#include <zeta/core/lifecycle.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core {

template <typename T, typename En>
constexpr void lifecycle::Traits<T, En>::Init(T&) {}

template <typename T, typename En>
template <typename Arg>
constexpr void lifecycle::Traits<T, En>::Init(T& obj, Arg&& arg) {
    obj = meta::Forward<Arg>(arg);
}

template <typename T, typename En>
constexpr void lifecycle::Traits<T, En>::Deinit(T&) {}

template <typename... Args>
constexpr auto lifecycle::MakeInitArgsTuple(Args&&... args) {
    return InitArgsTuple<Args...>{ meta::Forward<Args>(args)... };
}

template <typename T>
constexpr void lifecycle::Init(T&, SkipInitTag) {}

template <typename T, typename... Args>
constexpr void lifecycle::Init(T& obj, Args&&... args) {
    Traits<T, void>::Init(obj, meta::Forward<Args>(args)...);
}

namespace lifecycle::detail {

template <typename T>
struct InitFromTuple_ {
    T& obj;

    template <typename... UnpackedArgs>
    constexpr void operator()(UnpackedArgs&&... args) {
        (Init)(this->obj, meta::Forward<UnpackedArgs>(args)...);
    }
};

}  // namespace lifecycle::detail

template <typename T, typename... Args>
constexpr void lifecycle::Init(T& obj, InitArgsTuple<Args...>& t) {
    tuple::Apply(detail::InitFromTuple_{ obj }, t.t);
}

template <typename T, typename... Args>
constexpr void lifecycle::Init(T& obj, InitArgsTuple<Args...> const& t) {
    tuple::Apply(detail::InitFromTuple_{ obj }, t.t);
}

template <typename T, typename... Args>
constexpr void lifecycle::Init(T& obj, InitArgsTuple<Args...>&& t) {
    tuple::Apply(detail::InitFromTuple_{ obj }, meta::Move(t.t));
}

template <typename T, typename... Args>
constexpr void lifecycle::Deinit(T& obj) {
    Traits<T, void>::Deinit(obj);
}

}  // namespace zeta::core

/*


when const ref tuple

Arg     Unpacked
var     const &
&       const &
&&      const &

when ref tuple

Arg     Unpacked
var     &
&       &
&&      &

when r ref tuple

Arg     Unpacked
var     &&
&       &
&&      &&

*/
