#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/error.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::vlq_utils {

template <integral::IsIntegral Integral, size_t UnitWidth>
    requires requires { requires 2 <= UnitWidth; }
constexpr size_t EstimateSerializedUnitCnt(
    Integral value,
    value_wrapper::StaticValueWrapper<size_t, UnitWidth> unit_width);

template <integral::IsIntegral Integral, typename EndiannessLike,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          typename Acceptor>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
bool SerializeIntegral(
    Integral src_value, EndiannessLike endianness_like,
    meta::TypeWrapper<UnitIntegral> unit_integral,
    value_wrapper::StaticValueWrapper<size_t, UnitWidth> unit_width,
    bool allow_lossy, Acceptor&& acceptor, error::Error* dst_error);

template <integral::IsIntegral Integral, typename EndiannessLike,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          typename Provider>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
bool DeserializeIntegral(
    Integral& dst_value, EndiannessLike endianness_like,
    meta::TypeWrapper<UnitIntegral> unit_integral,
    value_wrapper::StaticValueWrapper<size_t, UnitWidth> unit_width,
    bool allow_lossy, Provider&& provider, error::Error* dst_error);

}  // namespace zeta::core::vlq_utils
