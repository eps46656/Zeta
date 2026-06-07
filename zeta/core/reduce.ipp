#pragma once

#include <zeta/core/meta.hpp>
#include <zeta/core/reduce.hpp>

namespace zeta::core {

namespace reduce::detail {

template <typename Operation, typename Value0>
constexpr decltype(auto) LeftReduce_(Operation const&, Value0&& value0) {
    return meta::Forward<Value0>(value0);
}

template <typename Operation, typename Value0, typename Value1,
          typename... Values>
constexpr decltype(auto) LeftReduce_(Operation const& opr, Value0&& value0,
                                     Value1&& value1, Values&&... values) {
    return (LeftReduce_)(opr(meta::Forward<Value0>(value0),
                             meta::Forward<Value1>(value1)),
                         meta::Forward<Values>(values)...);
}

}  // namespace reduce::detail

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) reduce::LeftReduce(Operation const& opr,
                                            Value0&& value0,
                                            Values&&... values) {
    return detail::LeftReduce_(opr, meta::Forward<Value0>(value0),
                               meta::Forward<Values>(values)...);
}

namespace reduce::detail {

template <typename Operation, typename Value0>
static constexpr decltype(auto) RightReduce_(Operation const&,
                                             Value0&& value0) {
    return meta::Forward<Value0>(value0);
}

template <typename Operation, typename Value0, typename Value1,
          typename... Values>
static constexpr decltype(auto) RightReduce_(Operation const& opr,
                                             Value0&& value0, Value1&& value1,
                                             Values&&... values) {
    return opr(meta::Forward<Value0>(value0),
               (RightReduce_)(opr, meta::Forward<Value1>(value1),
                              meta::Forward<Values>(values)...));
}

}  // namespace reduce::detail

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) reduce::RightReduce(Operation const& opr,
                                             Value0&& value0,
                                             Values&&... values) {
    return detail::RightReduce_(opr, meta::Forward<Value0>(value0),
                                meta::Forward<Values>(values)...);
}

namespace reduce::detail {

template <size_t Beg, size_t End, typename Operation, typename... Values>
constexpr decltype(auto) TreeReduce_(Operation const& opr, Values&&... values) {
    if constexpr (Beg + 1 == End) {
        return meta::GetNthArg<Beg>(meta::Forward<Values>(values)...);
    } else {
        constexpr size_t Mid{ (Beg + End) / 2 };

        return opr(
            (TreeReduce_<Beg, Mid>)(opr, meta::Forward<Values>(values)...),
            (TreeReduce_<Mid, End>)(opr, meta::Forward<Values>(values)...));
    }
}

}  // namespace reduce::detail

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) reduce::TreeReduce(Operation const& opr,
                                            Value0&& value0,
                                            Values&&... values) {
    return detail::TreeReduce_<0, 1 + sizeof...(values)>(
        opr, meta::Forward<Value0>(value0), meta::Forward<Values>(values)...);
}

}  // namespace zeta::core
