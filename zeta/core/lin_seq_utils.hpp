#pragma once

#include <zeta/core/define.hpp>

namespace zeta::core::lin_seq_utils {

template <typename Elem>
constexpr void Check(Elem* data, size_t elem_stride, size_t cnt);

template <typename Elem>
constexpr void Check(Elem* data, ptrdiff_t elem_stride, size_t cnt);

}  // namespace zeta::core::lin_seq_utils
