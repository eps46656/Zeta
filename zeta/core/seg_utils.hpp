#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core::seg_utils {

template <typename Writer>
void SegShoveL(circular_array::Cntr* l_ca, circular_array::Cntr* r_ca,
               size_t rl_cnt, size_t ins_cnt, size_t shove_cnt,
               Writer&& writer);

template <typename Writer>
void SegShoveR(circular_array::Cntr* l_ca, circular_array::Cntr* r_ca,
               size_t lr_cnt, size_t ins_cnt, size_t shove_cnt,
               Writer&& writer);

}  // namespace zeta::core::seg_utils
