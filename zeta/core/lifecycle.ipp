#pragma once

#include <zeta/core/integral.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/tuple.hpp>

namespace zeta::core {

template <integral::IsIntegral Integral>
struct lifecycle::Traits<Integral> {
    static constexpr void Init(Integral&) {}

    template <integral::IsIntegral InitIntegral>
    static constexpr void Init(Integral& obj, InitIntegral& i) {
        obj = i;
    }

    static constexpr void Deinit(Integral&) {}
};

template <typename T>
template <typename... Args>
constexpr void lifecycle::MemberFuncTraitsAdapter<T>::Init(T& obj,
                                                           Args&&... args) {
    obj.Init(meta::Forward<Args>(args)...);
};

template <typename T>
constexpr void lifecycle::MemberFuncTraitsAdapter<T>::Deinit(T& obj) {
    obj.Deinit();
};

template <typename... Args>
constexpr auto lifecycle::MakeInitArgsTuple(Args&&... args) {
    return InitArgsTuple<Args...>{ .t{ meta::Forward<Args>(args)... } };
}

template <typename T>
constexpr void lifecycle::Init(T&, SkipInitTag) {}

template <typename T, typename... Args>
constexpr void lifecycle::Init(T& obj, Args&&... args) {
    Traits<T>::Init(obj, meta::Forward<Args>(args)...);
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
    Traits<T>::Deinit(obj);
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
