#pragma once

#include <zeta/core/comparison.hpp>

namespace zeta::core::comparison_utils {

template <typename Comparator, typename Value0, typename... Values>
constexpr decltype(auto) Min(Comparator const& cmptr, Value0&& value0,
                             Values&&... values);

template <typename Value0, typename... Values>
constexpr decltype(auto) BasicMin(Value0&& value0, Values&&... values);

template <typename Comparator, typename Value0, typename... Values>
constexpr decltype(auto) Max(Comparator const& cmptr, Value0&& value0,
                             Values&&... values);

template <typename Value0, typename... Values>
constexpr decltype(auto) BasicMax(Value0&& value0, Values&&... values);

int MemLexCompare(void const* a, void const* b, size_t a_size, size_t b_size);

int MemSeqLexCompare(void const* a, void const* b, size_t a_elem_size,
                     size_t b_elem_size, ptrdiff_t a_elem_stride,
                     ptrdiff_t b_elem_stride, size_t a_elem_cnt, size_t b_cnt);

template <typename... Args>
    requires requires { requires sizeof...(Args) % 3 == 0; }
constexpr comparison::Ordering PairWiseLexCompare(Args&&... args);

template <comparison::IsOpTag OpTag, typename... Args>
    requires requires { requires sizeof...(Args) % 2 == 0; }
constexpr auto BasicPairWiseLexCompare(OpTag, Args&&... args);

template <typename Comparator, typename SeqA, typename SeqB>
int SeqWiseLexCompare(Comparator const& cmptr, SeqA&& a, SeqB&& b);

}  // namespace zeta::core::comparison_utils
