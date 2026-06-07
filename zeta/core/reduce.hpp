#pragma once

namespace zeta::core::reduce {

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) LeftReduce(Operation const& opr, Value0&& value0,
                                    Values&&... values);

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) RightReduce(Operation const& opr, Value0&& value0,
                                     Values&&... values);

template <typename Operation, typename Value0, typename... Values>
constexpr decltype(auto) TreeReduce(Operation const& opr, Value0&& value0,
                                    Values&&... values);

}  // namespace zeta::core::reduce
