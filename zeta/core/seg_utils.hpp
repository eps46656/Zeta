#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core::seg_utils {

constexpr void SegShoveL(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
                         size_t shove_cnt);

constexpr void SegShoveR(circular_array::Cntr& l_ca, circular_array::Cntr& r_ca,
                         size_t shove_cnt);

template <typename Writer>
constexpr void SegInsertShoveL(circular_array::Cntr& l_ca,
                               circular_array::Cntr& r_ca, size_t rl_cnt,
                               size_t ins_cnt, size_t shove_cnt,
                               Writer& writer);

template <typename Writer>
constexpr void SegInsertShoveR(circular_array::Cntr& l_ca,
                               circular_array::Cntr& r_ca, size_t lr_cnt,
                               size_t ins_cnt, size_t shove_cnt,
                               Writer& writer);

template <typename Reader>
constexpr void SegEraseShoveL(circular_array::Cntr& l_ca,
                              circular_array::Cntr& r_ca, size_t rl_cnt,
                              size_t ers_cnt, size_t shove_cnt,
                              Reader&& reader);

template <typename Reader>
constexpr void SegEraseShoveR(circular_array::Cntr& l_ca,
                              circular_array::Cntr& r_ca, size_t lr_cnt,
                              size_t ers_cnt, size_t shove_cnt,
                              Reader&& reader);

template <typename Writer, typename RLCntValueWrapper,
          typename RRCntValueWrapper, typename InsCntValueWrapper>
constexpr void AugSegShoveL(circular_array::Cntr& l_ca,
                            circular_array::Cntr& r_ca,
                            RLCntValueWrapper rl_cnt, RRCntValueWrapper rr_cnt,
                            InsCntValueWrapper ins_cnt, size_t shove_cnt,
                            Writer& writer);

}  // namespace zeta::core::seg_utils
