#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/error.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>

namespace zeta::core::serde_utils {

struct VariableOctetCntTag {};

enum struct EndiannessEnum : unsigned char {
    Little = 0,
    Big = 1,
};

template <typename T>
concept IsEndiannessType =
    meta::IsAnySame<T, EndiannessEnum,
                    meta::AutoValueWrapper<EndiannessEnum::Little>,
                    meta::AutoValueWrapper<EndiannessEnum::Big>>;

struct ErrorMessage {
    static constexpr unsigned char
        information_loss_when_serializing_unsigned_integral[]{
            "Information loss when serializing unsigned integral"
        };

    static constexpr unsigned char
        information_loss_when_deserializing_unsigned_integral[]{
            "Information loss when deserializing unsigned integral"
        };

    static constexpr unsigned char
        information_loss_when_serializing_signed_integral[]{
            "Information loss when serializing signed integral"
        };

    static constexpr unsigned char
        information_loss_when_deserializing_signed_integral[]{
            "Information loss when deserializing signed integral"
        };
};

template <integral::IsUnsignedIntegral Integral>
constexpr pair::Pair<Integral, bool> CanonicalizeIntegral(Integral value,
                                                          Integral range_max);

template <integral::IsIntegral Integral, IsEndiannessType EndiannessType,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, elem_stream::acceptor::IsAcceptor Acceptor>
    requires requires {
        requires 0 < DigitWidth;
        requires DigitWidth <= integral::WidthOf<DigitIntegral>;
    }
constexpr bool SerializeIntegral(
    Integral src_value, EndiannessType endianness,
    meta::TypeWrapper<DigitIntegral> digit_integral,
    meta::ValueWrapper<size_t, DigitWidth> digit_width,
    DigitCntLike digit_cnt_like, bool allow_lossy, Acceptor&& acceptor,
    error::Error* dst_error);

template <integral::IsIntegral Integral, IsEndiannessType EndiannessType,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, elem_stream::provider::IsProvider Provider>
    requires requires {
        requires 0 < DigitWidth;
        requires DigitWidth <= integral::WidthOf<DigitIntegral>;
    }
constexpr bool DeserializeIntegral(
    Integral& dst_value, EndiannessType endianness_like,
    meta::TypeWrapper<DigitIntegral> digit_integral,
    meta::ValueWrapper<size_t, DigitWidth> digit_width,
    DigitCntLike digit_cnt_like, bool allow_lossy, Provider&& provider,
    error::Error* dst_error);

}  // namespace zeta::core::serde_utils
