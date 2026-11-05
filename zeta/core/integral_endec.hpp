#pragma once

#include <zeta/core/error.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/seq_endpoint.hpp>

namespace zeta::core::integral_endec {

struct VariableOctetCntTag {};

enum struct Endianness : unsigned char {
    Little = 0,
    Big    = 1,
};

template <typename T>
concept IsEndiannessLike =
    meta::IsAnySame<T, Endianness, meta::AutoValueWrapper<Endianness::Little>,
                    meta::AutoValueWrapper<Endianness::Big>>;

template <integral::IsUnsignedIntegralType Integral>
struct CanonicalizeResult {
    bool out_of_range : 1;
    Integral value;
};

template <integral::IsUnsignedIntegralType Integral>
constexpr CanonicalizeResult<Integral> CanonicalizeIntegral(Integral value,
                                                            Integral range_max);

struct EncodeResult {
    bool value_out_of_range : 1;
};

template <integral::IsUnsignedIntegralType DigitIntegral,
          IsEndiannessLike EndiannessLike, size_t DigitWidth,
          typename DigitCntLike, integral::IsIntegralType SrcIntegral,
          seq_endpoint::acceptor::IsAcceptor<DigitIntegral> Acceptor>
    requires requires {
        requires 0 < DigitWidth;
        requires DigitWidth <= integral::WidthOf<DigitIntegral>;

        requires meta::IsSame<DigitCntLike, size_t> ||
                     meta::IsValueWrapperT<DigitCntLike, size_t>;
    }
constexpr EncodeResult Encode(
    EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral> digit_integral,
    meta::ValueWrapper<size_t, DigitWidth> digit_width,
    DigitCntLike digit_cnt_like, SrcIntegral src_value, Acceptor&& acceptor);

template <integral::IsIntegralType Integral>
struct DecodeResult {
    bool pad_zero : 1;
    bool digit_out_of_range : 1;
    bool value_out_of_range : 1;

    Integral value;
};

template <IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegralType DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, integral::IsIntegralType DstIntegral,
          seq_endpoint::provider::IsProvider<DigitIntegral> Provider>
    requires requires {
        requires 0 < DigitWidth;
        requires DigitWidth <= integral::WidthOf<DigitIntegral>;

        requires meta::IsSame<DigitCntLike, size_t> ||
                     meta::IsValueWrapperT<DigitCntLike, size_t> ||
                     meta::IsSame<DigitCntLike, VariableOctetCntTag>;
    }
constexpr DecodeResult<DstIntegral> Decode(
    EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral> digit_integral,
    meta::ValueWrapper<size_t, DigitWidth> digit_width,
    DigitCntLike digit_cnt_like, meta::TypeWrapper<DstIntegral> dst_integral,
    Provider&& provider);

}  // namespace zeta::core::integral_endec
