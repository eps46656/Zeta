#pragma once

#include <zeta/core/meta.hpp>
#include <zeta/core/reduce.hpp>
#include <zeta/core/tuple.ipp>

namespace zeta::core {

namespace reduce::detail {

template <size_t Beg, typename Operation, typename AccValue, typename... Values>
constexpr decltype(auto) LeftReduce_(Operation const& opr, AccValue&& acc_val,
                                     tuple::Tuple<Values&&...>& t) {
    if constexpr (Beg == sizeof...(Values)) {
        return meta::Forward<AccValue>(acc_val);
    } else {
        return (LeftReduce_<Beg + 1>)(opr,
                                      opr(meta::Forward<AccValue>(acc_val),
                                          t.template ForwardAccess<Beg>()),
                                      t);
    }
}

}  // namespace reduce::detail

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) reduce::LeftReduce(Operation const& opr,
                                            Value0&& value0,
                                            Values&&... values) {
    tuple::Tuple<Value0&&, Values&&...> t{ meta::Forward<Value0>(value0),
                                           meta::Forward<Values>(values)... };

    return detail::LeftReduce_<1>(opr, t.template ForwardAccess<0>(), t);
}

namespace reduce::detail {

template <size_t End, typename Operation, typename AccValue, typename... Values>
constexpr decltype(auto) RightReduce_(Operation const& opr, AccValue&& acc_val,
                                      tuple::Tuple<Values&&...>& t) {
    if constexpr (End == 0) {
        return meta::Forward<AccValue>(acc_val);
    } else {
        return (RightReduce_<End - 1>)(opr,
                                       opr(t.template ForwardAccess<End - 1>(),
                                           meta::Forward<AccValue>(acc_val)),
                                       t);
    }
}

}  // namespace reduce::detail

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) reduce::RightReduce(Operation const& opr,
                                             Value0&& value0,
                                             Values&&... values) {
    tuple::Tuple<Value0&&, Values&&...> t{ meta::Forward<Value0>(value0),
                                           meta::Forward<Values>(values)... };

    return detail::RightReduce_<sizeof...(values)>(
        opr, t.template ForwardAccess<sizeof...(values)>(), t);
}

namespace reduce::detail {

template <size_t Beg, size_t End, typename Operation, typename... Values>
constexpr decltype(auto) TreeReduce_(Operation const& opr,
                                     tuple::Tuple<Values...>& t) {
    if constexpr (Beg + 1 == End) {
        return t.template ForwardAccess<Beg>();
    } else {
        constexpr size_t Mid{ (Beg + End) / 2 };

        return opr((TreeReduce_<Beg, Mid>)(opr, t),
                   (TreeReduce_<Mid, End>)(opr, t));
    }
}

}  // namespace reduce::detail

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) reduce::TreeReduce(Operation const& opr,
                                            Value0&& value0,
                                            Values&&... values) {
    tuple::Tuple<Value0&&, Values&&...> t{ meta::Forward<Value0>(value0),
                                           meta::Forward<Values>(values)... };

    return detail::TreeReduce_<0, 1 + sizeof...(values)>(opr, t);
}

}  // namespace zeta::core
