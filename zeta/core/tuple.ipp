#pragma once

#include <zeta/core/tuple.hpp>

namespace zeta::core {

namespace tuple::detail {

template <size_t TargetIdx, typename Elem>
constexpr ElemWrapper_<TargetIdx, Elem>& AccessElemWrapper_(
    ElemWrapper_<TargetIdx, Elem>& elem_wrapper) {
    return elem_wrapper;
}

template <size_t TargetIdx, typename Elem>
constexpr ElemWrapper_<TargetIdx, Elem> const& AccessElemWrapper_(
    ElemWrapper_<TargetIdx, Elem> const& elem_wrapper) {
    return elem_wrapper;
}

template <size_t... Idxes, typename... Elems>
template <size_t... ArgIdxes, typename... Args>
constexpr Tuple_<static_seq::StaticSeq<size_t, Idxes...>, Elems...>::Tuple_(
    static_seq::StaticSeq<size_t, ArgIdxes...>, Args&&... args)
    : meta::RemoveRef<decltype(AccessElemWrapper_<ArgIdxes>(*this))>{
          meta::Forward<Args>(args)
      }... {}

template <size_t... Idxes, typename... Elems>
template <size_t Idx>
constexpr decltype(auto)
Tuple_<static_seq::StaticSeq<size_t, Idxes...>, Elems...>::ForwardAccess(
    this Tuple_ const& self) {
    auto& elem_wrapper{ (AccessElemWrapper_<Idx>)(self) };

    return meta::Forward<
        typename meta::RemoveRef<decltype(elem_wrapper)>::Elem>(
        elem_wrapper.elem);
}

template <size_t... Idxes, typename... Elems>
template <size_t Idx>
constexpr decltype(auto) Tuple_<static_seq::StaticSeq<size_t, Idxes...>,
                                Elems...>::Access(this auto&& self) {
    if constexpr (meta::IsRValueRef<decltype(self)>) {
        return (AccessElemWrapper_<Idx>)(self).elem;
    } else {
        return meta::Move((AccessElemWrapper_<Idx>)(self).elem);
    }
}

}  // namespace tuple::detail

template <typename... Elems>
template <typename... Args>
constexpr tuple::Tuple<Elems...>::Tuple(Args&&... args)
    : tuple::detail::Tuple_<static_seq::MakeLinearStaticIntegralSeq<
                                size_t, 0, 1, sizeof...(Elems)>,
                            Elems...>{ static_seq::MakeLinearStaticIntegralSeq<
                                           size_t, 0, 1, sizeof...(Args)>{},
                                       meta::Forward<Args>(args)... } {}

namespace tuple::detail {

template <typename IdxSeq>
struct Apply_;

template <size_t... Idxes>
struct Apply_<static_seq::StaticSeq<size_t, Idxes...>> {
    template <typename Func, typename TupleType>
    static constexpr decltype(auto) Call(Func&& func, TupleType&& t) {
        return meta::Forward<Func>(func)(
            meta::Forward<TupleType>(t).template Access<Idxes>()...);
    }
};

}  // namespace tuple::detail

template <typename Func, typename... Elems>
constexpr decltype(auto) tuple::Apply(Func&& func, Tuple<Elems...>& t) {
    return detail::Apply_<static_seq::MakeLinearStaticIntegralSeq<
        size_t, 0, 1,
        Tuple<Elems...>::ElemsCnt>>::Call(meta::Forward<Func>(func), t);
}

template <typename Func, typename... Elems>
constexpr decltype(auto) tuple::Apply(Func&& func, Tuple<Elems...> const& t) {
    return detail::Apply_<static_seq::MakeLinearStaticIntegralSeq<
        size_t, 0, 1,
        Tuple<Elems...>::ElemsCnt>>::Call(meta::Forward<Func>(func), t);
}

template <typename Func, typename... Elems>
constexpr decltype(auto) tuple::Apply(Func&& func, Tuple<Elems...>&& t) {
    return detail::Apply_<static_seq::MakeLinearStaticIntegralSeq<
        size_t, 0, 1, Tuple<Elems...>::ElemsCnt>>::
        Call(meta::Forward<Func>(func), meta::Move(t));
}

}  // namespace zeta::core
