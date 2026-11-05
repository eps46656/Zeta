#pragma once

#include <zeta/core/error.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_endec.hpp>
#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::vlq_utils {

template <integral::IsIntegralType Integral, size_t UnitWidth>
    requires requires { requires 2 <= UnitWidth; }
constexpr size_t EstimateEncodedUnitCnt(
    Integral value, meta::ValueWrapper<size_t, UnitWidth> unit_width);

template <integral_endec::IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegralType UnitIntegral, size_t UnitWidth,
          integral::IsIntegralType SrcIntegral,
          seq_endpoint::acceptor::IsAcceptor<UnitIntegral> Acceptor>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
constexpr void Encode(EndiannessLike endianness,
                      meta::TypeWrapper<UnitIntegral> unit_integral,
                      meta::ValueWrapper<size_t, UnitWidth> unit_width,
                      SrcIntegral src_value, Acceptor&& acceptor);

template <integral::IsIntegralType Integra>
struct DecodeResult {
    bool digit_out_of_range : 1;
    bool value_out_of_range : 1;
    Integra value;
};

template <integral_endec::IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegralType UnitIntegral, size_t UnitWidth,
          integral::IsIntegralType DstIntegral,
          seq_endpoint::provider::IsProvider<UnitIntegral> Provider>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
constexpr DecodeResult<DstIntegral> Decode(
    EndiannessLike endianness, meta::TypeWrapper<UnitIntegral> unit_integral,
    meta::ValueWrapper<size_t, UnitWidth> unit_width,
    meta::TypeWrapper<DstIntegral> dst_integral, Provider&& provider);

}  // namespace zeta::core::vlq_utils
