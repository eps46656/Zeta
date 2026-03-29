#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core::seg_utils {

void SegShoveL(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
               size_t shove_cnt);

void SegShoveR(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
               size_t shove_cnt);

template <typename Writer>
void SegInsertShoveL(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
                     size_t rl_cnt, size_t ins_cnt, size_t shove_cnt,
                     Writer&& writer);

template <typename Writer>
void SegInsertShoveR(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
                     size_t lr_cnt, size_t ins_cnt, size_t shove_cnt,
                     Writer&& writer);

void SegEraseShoveL(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
                    size_t rl_cnt, size_t ers_cnt, size_t shove_cnt);

void SegEraseShoveR(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
                    size_t lr_cnt, size_t ers_cnt, size_t shove_cnt);

template <typename Writer, typename RLCntValueWrapper,
          typename RRCntValueWrapper, typename InsCntValueWrapper>
void AugSegShoveL(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
                  RLCntValueWrapper rl_cnt, RRCntValueWrapper rr_cnt,
                  InsCntValueWrapper ins_cnt, size_t shove_cnt,
                  Writer&& writer);

}  // namespace zeta::core::seg_utils
