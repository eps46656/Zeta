#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/error.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::integral_endec {

struct VariableOctetCntTag {};

enum struct Endianness : unsigned char {
    Little = 0,
    Big = 1,
};

template <typename T>
concept IsEndiannessLike =
    meta::IsAnySame<T, Endianness, meta::AutoValueWrapper<Endianness::Little>,
                    meta::AutoValueWrapper<Endianness::Big>>;

template <integral::IsUnsignedIntegral Integral>
struct CanonicalizeResult {
    bool out_of_range : 1;
    Integral value;
};

template <integral::IsUnsignedIntegral Integral>
constexpr CanonicalizeResult<Integral> CanonicalizeIntegral(Integral value,
                                                            Integral range_max);

struct EncodeResult {
    bool value_out_of_range : 1;
};

template <elem_stream::acceptor::IsAcceptor Acceptor,
          IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, integral::IsIntegral SrcIntegral>
    requires requires {
        requires 0 < DigitWidth;
        requires DigitWidth <= integral::WidthOf<DigitIntegral>;
    }
constexpr EncodeResult Encode(
    Acceptor&& acceptor, EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral> digit_integral,
    meta::ValueWrapper<size_t, DigitWidth> digit_width,
    DigitCntLike digit_cnt_like, SrcIntegral src_value);

template <integral::IsIntegral Integral>
struct DecodeResult {
    bool pad_zero : 1;
    bool digit_out_of_range : 1;
    bool value_out_of_range : 1;

    Integral value;
};

template <elem_stream::provider::IsProvider Provider,
          IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, integral::IsIntegral DstIntegral>
    requires requires {
        requires 0 < DigitWidth;
        requires DigitWidth <= integral::WidthOf<DigitIntegral>;
    }
constexpr DecodeResult<DstIntegral> Decode(
    Provider&& provider, EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral> digit_integral,
    meta::ValueWrapper<size_t, DigitWidth> digit_width,
    DigitCntLike digit_cnt_like, meta::TypeWrapper<DstIntegral> dst_integral);

}  // namespace zeta::core::integral_endec
