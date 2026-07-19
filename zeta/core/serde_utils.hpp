#pragma once

#include <zeta/core/error.hpp>
#include <zeta/core/meta.hpp>

namespace zeta::core::serde_utils {

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

template <typename UnsignedIntegral, typename OctetCntLike,
          typename EndiannessLike, typename Acceptor>
bool SerializeUnsignedIntegral(Acceptor&& acceptor, UnsignedIntegral src_value,
                               OctetCntLike octet_cnt_like,
                               EndiannessLike endianness_like, bool allow_lossy,
                               error::Error* dst_error);

template <typename UnsignedIntegral, typename OctetCntLike,
          typename EndiannessLike, typename Provider>
bool DeserializeUnsignedIntegral(Provider&& provider,
                                 UnsignedIntegral& dst_value,
                                 OctetCntLike octet_cnt_like,
                                 EndiannessLike endianness_like,
                                 bool allow_lossy, error::Error* dst_error);

template <typename SignedIntegral, typename OctetCntLike,
          typename EndiannessLike, typename Acceptor>
bool SerializeSignedIntegral(Acceptor&& acceptor, SignedIntegral src_value,
                             OctetCntLike octet_cnt_like,
                             EndiannessLike endianness_like, bool allow_lossy,
                             error::Error* dst_error);

template <typename SignedIntegral, typename OctetCntLike,
          typename EndiannessLike, typename Provider>
bool DeserializeSignedIntegral(Provider&& provider, SignedIntegral& dst_value,
                               OctetCntLike octet_cnt_like,
                               EndiannessLike endianness_like, bool allow_lossy,
                               error::Error* dst_error);

template <typename Integral, typename OctetCntLike, typename EndiannessLike,
          typename Acceptor>
bool SerializeIntegral(Acceptor&& acceptor, Integral src_value,
                       OctetCntLike octet_cnt_like,
                       EndiannessLike endianness_like, bool allow_lossy,
                       error::Error* dst_error);

template <typename Integral, typename OctetCntLike, typename EndiannessLike,
          typename Provider>
bool DeserializeIntegral(Provider&& provider, Integral& dst_value,
                         OctetCntLike octet_cnt_like,
                         EndiannessLike endianness_like, bool allow_lossy,
                         error::Error* dst_error);

}  // namespace zeta::core::serde_utils
