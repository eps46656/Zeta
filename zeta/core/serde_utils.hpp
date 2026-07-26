#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/error.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core::serde_utils {

struct VariableOctetCntTag {};

struct EndiannessEnum {
    using Value = unsigned char;

    struct Little {
        static constexpr Value value{ 0 };
    };

    struct Big {
        static constexpr Value value{ 1 };
    };
};

template <typename T>
constexpr bool IsEndiannessEnum{
    meta::IsAnySame<T, EndiannessEnum::Little, EndiannessEnum::Big>
};

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
pair::Pair<Integral, bool> CanonicalizeIntegral(Integral value,
                                                Integral range_max);

template <integral::IsIntegral Integral, typename EndiannessLike,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, elem_stream::acceptor::IsAcceptor Acceptor>
    requires requires {
        requires IsEndiannessEnum<EndiannessLike> ||
                     value_wrapper::IsValueWrapperWith<EndiannessLike,
                                                       EndiannessEnum::Value>;

        requires 0 < DigitWidth;

        requires DigitWidth <= integral::WidthOf<DigitIntegral>;

        requires value_wrapper::IsValueWrapperWith<DigitCntLike, size_t>;
    }
bool SerializeIntegral(
    Integral src_value, EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral> digit_integral,
    value_wrapper::StaticValueWrapper<size_t, DigitWidth> digit_width,
    DigitCntLike digit_cnt_like, bool allow_lossy, Acceptor&& acceptor,
    error::Error* dst_error);

template <integral::IsIntegral Integral, typename EndiannessLike,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, elem_stream::provider::IsProvider Provider>
    requires requires {
        requires IsEndiannessEnum<EndiannessLike> ||
                     value_wrapper::IsValueWrapperWith<EndiannessLike,
                                                       EndiannessEnum::Value>;

        requires 0 < DigitWidth;

        requires DigitWidth <= integral::WidthOf<DigitIntegral>;

        requires meta::IsSame<DigitCntLike, VariableOctetCntTag> ||
                     value_wrapper::IsValueWrapperWith<DigitCntLike, size_t>;
    }
bool DeserializeIntegral(
    Integral& dst_value, EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral> digit_integral,
    value_wrapper::StaticValueWrapper<size_t, DigitWidth> digit_width,
    DigitCntLike digit_cnt_like, bool allow_lossy, Provider&& provider,
    error::Error* dst_error);

}  // namespace zeta::core::serde_utils
