#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/error.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_endec.hpp>

namespace zeta::core::vlq_utils {

template <integral::IsIntegral Integral, size_t UnitWidth>
    requires requires { requires 2 <= UnitWidth; }
constexpr size_t EstimateEncodedUnitCnt(
    Integral value, meta::ValueWrapper<size_t, UnitWidth> unit_width);

template <elem_stream::acceptor::IsAcceptor Acceptor,
          integral_endec::IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          integral::IsIntegral SrcIntegral>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
constexpr void Encode(Acceptor&& acceptor, EndiannessLike endianness,
                      meta::TypeWrapper<UnitIntegral> unit_integral,
                      meta::ValueWrapper<size_t, UnitWidth> unit_width,
                      SrcIntegral src_value);

template <integral::IsIntegral Integra>
struct DecodeResult {
    bool digit_out_of_range : 1;
    bool value_out_of_range : 1;
    Integra value;
};

template <elem_stream::provider::IsProvider Provider,
          integral_endec::IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          integral::IsIntegral DstIntegral>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
constexpr DecodeResult<DstIntegral> Decode(
    Provider&& provider, EndiannessLike endianness,
    meta::TypeWrapper<UnitIntegral> unit_integral,
    meta::ValueWrapper<size_t, UnitWidth> unit_width,
    meta::TypeWrapper<DstIntegral> dst_integral);

}  // namespace zeta::core::vlq_utils
