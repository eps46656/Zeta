#pragma once

#include <zeta/core/circular_array.hpp>

namespace zeta::core::seg_utils {

template <typename Writer>
void SegShoveL(CircularArray* l_ca, CircularArray* r_ca, size_t rl_cnt,
               size_t ins_cnt, size_t shove_cnt, Writer&& writer);

template <typename Writer>
void SegShoveR(CircularArray* l_ca, CircularArray* r_ca, size_t lr_cnt,
               size_t ins_cnt, size_t shove_cnt, Writer&& writer);

}  // namespace zeta::core::seg_utils
