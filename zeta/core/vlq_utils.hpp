#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/error.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/serde_utils.hpp>

namespace zeta::core::vlq_utils {

template <integral::IsIntegral Integral, size_t UnitWidth>
    requires requires { requires 2 <= UnitWidth; }
constexpr size_t EstimateSerializedUnitCnt(
    Integral value, meta::ValueWrapper<size_t, UnitWidth> unit_width);

template <integral::IsIntegral Integral,
          serde_utils::IsEndiannessType EndiannessType,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          elem_stream::acceptor::IsAcceptor Acceptor>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
bool SerializeIntegral(Integral src_value, EndiannessType endianness,
                       meta::TypeWrapper<UnitIntegral> unit_integral,
                       meta::ValueWrapper<size_t, UnitWidth> unit_width,
                       bool allow_lossy, Acceptor&& acceptor,
                       error::Error* dst_error);

template <integral::IsIntegral Integral,
          serde_utils::IsEndiannessType EndiannessType,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          elem_stream::provider::IsProvider Provider>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
bool DeserializeIntegral(Integral& dst_value, EndiannessType endianness,
                         meta::TypeWrapper<UnitIntegral> unit_integral,
                         meta::ValueWrapper<size_t, UnitWidth> unit_width,
                         bool allow_lossy, Provider&& provider,
                         error::Error* dst_error);

}  // namespace zeta::core::vlq_utils
