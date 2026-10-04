#pragma once

#include <zeta/core/circular_array.hpp>
#include <zeta/core/integral.hpp>

namespace zeta::core::seg_utils {

template <meta::IsContainerElem Elem>
constexpr void SegShoveL(circular_array::Cntr<Elem>& l_ca,
                         circular_array::Cntr<Elem>& r_ca, size_t shove_cnt);

template <meta::IsContainerElem Elem>
constexpr void SegShoveR(circular_array::Cntr<Elem>& l_ca,
                         circular_array::Cntr<Elem>& r_ca, size_t shove_cnt);

template <meta::IsContainerElem Elem,
          seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void SegInsertShoveL(circular_array::Cntr<Elem>& l_ca,
                               circular_array::Cntr<Elem>& r_ca, size_t rl_cnt,
                               size_t ins_cnt, size_t shove_cnt,
                               Provider& writer);

template <meta::IsContainerElem Elem,
          seq_endpoint::provider::IsProvider<Elem> Provider>
constexpr void SegInsertShoveR(circular_array::Cntr<Elem>& l_ca,
                               circular_array::Cntr<Elem>& r_ca, size_t lr_cnt,
                               size_t ins_cnt, size_t shove_cnt,
                               Provider& writer);

template <meta::IsContainerElem Elem,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void SegEraseShoveL(circular_array::Cntr<Elem>& l_ca,
                              circular_array::Cntr<Elem>& r_ca, size_t rl_cnt,
                              size_t ers_cnt, size_t shove_cnt,
                              Acceptor&& reader);

template <meta::IsContainerElem Elem,
          seq_endpoint::acceptor::IsAcceptor<Elem> Acceptor>
constexpr void SegEraseShoveR(circular_array::Cntr<Elem>& l_ca,
                              circular_array::Cntr<Elem>& r_ca, size_t lr_cnt,
                              size_t ers_cnt, size_t shove_cnt,
                              Acceptor&& reader);

template <meta::IsContainerElem Elem,
          seq_endpoint::provider::IsProvider<Elem> Provider,
          typename RLCntValueWrapper, typename RRCntValueWrapper,
          typename InsCntValueWrapper>
constexpr void AugSegShoveL(circular_array::Cntr<Elem>& l_ca,
                            circular_array::Cntr<Elem>& r_ca,
                            RLCntValueWrapper rl_cnt, RRCntValueWrapper rr_cnt,
                            InsCntValueWrapper ins_cnt, size_t shove_cnt,
                            Provider& writer);

}  // namespace zeta::core::seg_utils
