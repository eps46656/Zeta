#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/pair.ipp>
#include <zeta/core/serde_utils.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.hpp>

namespace zeta::core {

template <typename UnsignedIntegral, typename OctetCntLike,
          typename EndiannessLike, typename Acceptor>
bool serde_utils::SerializeUnsignedIntegral(Acceptor&& acceptor,
                                            UnsignedIntegral src_value,
                                            OctetCntLike octet_cnt_like,
                                            EndiannessLike endianness_like,
                                            bool allow_lossy,
                                            error::Error* dst_error) {
    ZETA_Core_StaticAssert(integral::IsUnsignedIntegral<UnsignedIntegral>);

    ZETA_Core_StaticAssert(
        value_wrapper::IsValueWrapperWith<OctetCntLike, unsigned long long>);

    ZETA_Core_DebugAssert(0 < octet_cnt_like());

    ZETA_Core_StaticAssert(
        IsEndiannessEnum<EndiannessLike> ||
        value_wrapper::IsValueWrapperWith<EndiannessLike,
                                          EndiannessEnum::Value>);

    using OpUnsignedIntegral = decltype([]() {
        if constexpr (integral::WidthOf<UnsignedIntegral> <=
                      integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned>{};
        } else if constexpr (integral::WidthOf<UnsignedIntegral> <=
                             integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned long>{};
        } else if constexpr (integral::WidthOf<UnsignedIntegral> <=
                             integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned long long>{};
        } else {
            return meta::TypeWrapper<UnsignedIntegral>{};
        }
    }())::Type;

    OpUnsignedIntegral op_src_value{ src_value };

    EndiannessEnum::Value endianness_value{ [=]() {
        if constexpr (IsEndiannessEnum<EndiannessLike>) {
            return EndiannessLike::value;
        } else {
            return endianness_like.value();
        }
    }() };

    constexpr unsigned long long op_unsigned_integral_octet_cnt{
        integral_math::CeilDiv(integral::WidthOf<OpUnsignedIntegral>, 8ULL)
    };

    unsigned char buffer[op_unsigned_integral_octet_cnt];

    switch (endianness_value) {
    case EndiannessEnum::Little::value: {
        unsigned long long m{ comparison_utils::BasicMin(
            op_unsigned_integral_octet_cnt, octet_cnt_like()) };

        for (unsigned long long i{ 0 }; i < m; ++i) {
            buffer[i] = static_cast<unsigned char>(op_src_value) % 256;
            op_src_value /= 256;
        }

        acceptor(buffer, 1, m);

        if (op_unsigned_integral_octet_cnt < octet_cnt_like()) {
            buffer[0] = 0;
            acceptor(buffer, 0,
                     octet_cnt_like() - op_unsigned_integral_octet_cnt);
        }

        break;
    }

    case EndiannessEnum::Big::value: {
        unsigned long long m{ comparison_utils::BasicMin(
            op_unsigned_integral_octet_cnt, octet_cnt_like()) };

        if (op_unsigned_integral_octet_cnt < octet_cnt_like()) {
            buffer[0] = 0;
            acceptor(buffer, 0,
                     octet_cnt_like() - op_unsigned_integral_octet_cnt);
        }

        for (unsigned long long i{ m }; 0 < i--;) {
            buffer[i] = static_cast<unsigned char>(op_src_value) % 256;
            op_src_value /= 256;
        }

        acceptor(buffer, 1, m);

        break;
    }

    default: ZETA_Core_Unreachable();
    }

    bool no_lossy{ op_src_value == 0 };

    if (dst_error != nullptr) {
        if (allow_lossy || no_lossy) {
            dst_error->message = nullptr;
            dst_error->analyzer = nullptr;
        } else {
            dst_error->message = ErrorMessage::
                information_loss_when_serializing_unsigned_integral;
            dst_error->analyzer = nullptr;
        }
    }

    return no_lossy;
}

template <typename UnsignedIntegral, typename OctetCntLike,
          typename EndiannessLike, typename Provider>
bool serde_utils::DeserializeUnsignedIntegral(Provider&& provider,
                                              UnsignedIntegral& dst_value,
                                              OctetCntLike octet_cnt_like,
                                              EndiannessLike endianness_like,
                                              bool allow_lossy,
                                              error::Error* dst_error) {
    ZETA_Core_StaticAssert(integral::IsUnsignedIntegral<UnsignedIntegral>);

    ZETA_Core_StaticAssert(
        value_wrapper::IsValueWrapperWith<OctetCntLike, unsigned long long>);

    ZETA_Core_DebugAssert(0 < octet_cnt_like());

    ZETA_Core_StaticAssert(
        IsEndiannessEnum<EndiannessLike> ||
        value_wrapper::IsValueWrapperWith<EndiannessLike,
                                          EndiannessEnum::Value>);

    using OpUnsignedIntegral = decltype([]() {
        if constexpr (integral::WidthOf<UnsignedIntegral> <=
                      integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned>{};
        } else if constexpr (integral::WidthOf<UnsignedIntegral> <=
                             integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned long>{};
        } else if constexpr (integral::WidthOf<UnsignedIntegral> <=
                             integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned long long>{};
        } else {
            return meta::TypeWrapper<UnsignedIntegral>{};
        }
    }())::Type;

    OpUnsignedIntegral op_dst_value;

    EndiannessEnum::Value endianness_value{ [=]() {
        if constexpr (IsEndiannessEnum<EndiannessLike>) {
            return EndiannessLike::value;
        } else {
            return endianness_like.value();
        }
    }() };

    constexpr unsigned long long unsigned_integral_octet_cnt{
        integral_math::CeilDiv(integral::WidthOf<UnsignedIntegral>, 8ULL)
    };

    constexpr unsigned long long op_unsigned_integral_octet_cnt{
        integral_math::CeilDiv(integral::WidthOf<OpUnsignedIntegral>, 8ULL)
    };

    bool no_lossy{ true };

    unsigned char buffer[op_unsigned_integral_octet_cnt];

    unsigned long long exc_size;
    unsigned long long eff_size;

    if (unsigned_integral_octet_cnt < octet_cnt_like()) {
        eff_size = unsigned_integral_octet_cnt;
        exc_size = octet_cnt_like() - unsigned_integral_octet_cnt;
    } else {
        eff_size = octet_cnt_like();
        exc_size = 0;
    }

    auto discard_exc_octets{ [&provider, &buffer, &no_lossy, exc_size]() {
        unsigned long long k{ exc_size };

        for (; op_unsigned_integral_octet_cnt <= k;
             k -= op_unsigned_integral_octet_cnt) {
            provider(buffer, 1, op_unsigned_integral_octet_cnt);

            for (unsigned long long i{ 0 }; i < op_unsigned_integral_octet_cnt;
                 ++i) {
                no_lossy &= buffer[i] == 0;
            }
        }

        if (0 < k) {
            provider(buffer, 1, k);

            for (unsigned long long i{ 0 }; i < k; ++i) {
                no_lossy &= buffer[i] == 0;
            }
        }
    } };

    auto pull_eff_octets{ [&provider, &buffer, &no_lossy, eff_size]() {
        provider(buffer, 1, eff_size);

        for (unsigned long long i{ 0 }; i < eff_size; ++i) {
            if (255 < buffer[i]) {
                buffer[i] %= 256;
                no_lossy = false;
            }
        }
    } };

    switch (endianness_value) {
    case EndiannessEnum::Little::value: {
        pull_eff_octets();

        no_lossy &= buffer[eff_size - 1] <=
                    integral::RangeMaxOf<UnsignedIntegral> >>
                    (8 * (eff_size - 1));

        op_dst_value = buffer[eff_size - 1];

        for (unsigned long long i{ eff_size - 1 }; 0 < i--;) {
            op_dst_value *= 256;
            op_dst_value += buffer[i];
        }

        discard_exc_octets();

        break;
    }

    case EndiannessEnum::Big::value: {
        discard_exc_octets();

        pull_eff_octets();

        no_lossy &= buffer[0] <= integral::RangeMaxOf<UnsignedIntegral> >>
                    (8 * (eff_size - 1));

        op_dst_value = buffer[0];

        for (unsigned long long i{ 1 }; i < octet_cnt_like(); ++i) {
            op_dst_value *= 256;
            op_dst_value += buffer[i];
        }

        break;
    }

    default: ZETA_Core_Unreachable();
    }

    no_lossy &= op_dst_value <= integral::RangeMaxOf<UnsignedIntegral>;

    dst_value = static_cast<UnsignedIntegral>(op_dst_value);

    if (dst_error != nullptr) {
        if (allow_lossy || no_lossy) {
            dst_error->message = nullptr;
            dst_error->analyzer = nullptr;
        } else {
            dst_error->message = ErrorMessage::
                information_loss_when_serializing_unsigned_integral;
            dst_error->analyzer = nullptr;
        }
    }

    return no_lossy;
}

template <typename SignedIntegral, typename OctetCntLike,
          typename EndiannessLike, typename Acceptor>
bool serde_utils::SerializeSignedIntegral(
    Acceptor&& acceptor, SignedIntegral src_value, OctetCntLike octet_cnt_like,
    EndiannessLike endianness_like, bool allow_lossy, error::Error* dst_error) {
    ZETA_Core_StaticAssert(integral::IsSignedIntegral<SignedIntegral>);

    ZETA_Core_StaticAssert(
        value_wrapper::IsValueWrapperWith<OctetCntLike, unsigned long long>);

    ZETA_Core_DebugAssert(0 < octet_cnt_like());

    ZETA_Core_StaticAssert(
        IsEndiannessEnum<EndiannessLike> ||
        value_wrapper::IsValueWrapperWith<EndiannessLike,
                                          EndiannessEnum::Value>);

    using OpUnsignedIntegral = decltype([]() {
        if constexpr (integral::WidthOf<SignedIntegral> <=
                      integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned>{};
        } else if constexpr (integral::WidthOf<SignedIntegral> <=
                             integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned long>{};
        } else if constexpr (integral::WidthOf<SignedIntegral> <=
                             integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned long long>{};
        } else {
            return meta::TypeWrapper<
                integral::MakeUnsignedOf<SignedIntegral>>{};
        }
    }())::Type;

    bool is_neg{ src_value < 0 };

    OpUnsignedIntegral op_src_value{ static_cast<OpUnsignedIntegral>(
        src_value) };

    EndiannessEnum::Value endianness_value{ [=]() {
        if constexpr (IsEndiannessEnum<EndiannessLike>) {
            return EndiannessLike::value;
        } else {
            return endianness_like.value();
        }
    }() };

    constexpr unsigned long long signed_integral_octet_cnt{
        integral_math::CeilDiv(integral::WidthOf<SignedIntegral>, 8ULL)
    };

    constexpr unsigned long long op_unsigned_integral_octet_cnt{
        integral_math::CeilDiv(integral::WidthOf<OpUnsignedIntegral>, 8ULL)
    };

    unsigned char buffer[op_unsigned_integral_octet_cnt];

    unsigned long long exc_size;
    unsigned long long eff_size;

    if (octet_cnt_like() < signed_integral_octet_cnt) {
        exc_size = 0;
        eff_size = octet_cnt_like();
    } else {
        exc_size = octet_cnt_like() - signed_integral_octet_cnt;
        eff_size = signed_integral_octet_cnt;
    }

    bool no_lossy{ true };

    auto proc_highest_eff_octet{ [is_neg, eff_size,
                                  &no_lossy](unsigned char highest_eff_octet) {
        if (integral::WidthOf<OpUnsignedIntegral> % 8 == 0 ||
            eff_size < signed_integral_octet_cnt) {
            if (is_neg) {
                no_lossy &= 128 <= highest_eff_octet;
            } else {
                no_lossy &= highest_eff_octet < 128;
            }
        } else if (is_neg) {
            constexpr unsigned long long k{
                integral::WidthOf<OpUnsignedIntegral> % 8
            };

            highest_eff_octet += ((1U << (8 - k)) - 1U) << k;
        }

        return highest_eff_octet;
    } };

    switch (endianness_value) {
    case EndiannessEnum::Little::value: {
        for (unsigned long long i{ 0 }; i < eff_size; ++i) {
            buffer[i] = static_cast<unsigned char>(op_src_value) % 256;
            op_src_value /= 256;
        }

        buffer[eff_size - 1] = proc_highest_eff_octet(buffer[eff_size - 1]);

        acceptor(buffer, 1, eff_size);

        if (0 < exc_size) {
            buffer[0] = is_neg ? 255 : 0;
            acceptor(buffer, 0, exc_size);
        }

        break;
    }

    case EndiannessEnum::Big::value: {
        if (0 < exc_size) {
            buffer[0] = is_neg ? 255 : 0;
            acceptor(buffer, 0, exc_size);
        }

        for (unsigned long long i{ eff_size - 1 }; 0 < i--;) {
            buffer[i] = static_cast<unsigned char>(op_src_value) % 256;
            op_src_value /= 256;
        }

        buffer[0] = proc_highest_eff_octet(buffer[0]);

        acceptor(buffer, 1, eff_size);

        break;
    }

    default: ZETA_Core_Unreachable();
    }

    if (is_neg) {
        no_lossy &= integral_math::IsPowerOf2(op_src_value + 1U);
    } else {
        no_lossy &= op_src_value == 0;
    }

    if (dst_error != nullptr) {
        if (allow_lossy || no_lossy) {
            dst_error->message = nullptr;
            dst_error->analyzer = nullptr;
        } else {
            dst_error->message =
                ErrorMessage::information_loss_when_serializing_signed_integral;
            dst_error->analyzer = nullptr;
        }
    }

    return no_lossy;
}

template <typename SignedIntegral, typename OctetCntLike,
          typename EndiannessLike, typename Provider>
bool serde_utils::DeserializeSignedIntegral(
    Provider&& provider, SignedIntegral& dst_value, OctetCntLike octet_cnt_like,
    EndiannessLike endianness_like, bool allow_lossy, error::Error* dst_error) {
    ZETA_Core_StaticAssert(integral::IsSignedIntegral<SignedIntegral>);

    ZETA_Core_StaticAssert(
        value_wrapper::IsValueWrapperWith<OctetCntLike, unsigned long long>);

    ZETA_Core_DebugAssert(0 < octet_cnt_like());

    ZETA_Core_StaticAssert(
        IsEndiannessEnum<EndiannessLike> ||
        value_wrapper::IsValueWrapperWith<EndiannessLike,
                                          EndiannessEnum::Value>);

    using OpUnsignedIntegral = decltype([]() {
        if constexpr (integral::WidthOf<SignedIntegral> <=
                      integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned>{};
        } else if constexpr (integral::WidthOf<SignedIntegral> <=
                             integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned long>{};
        } else if constexpr (integral::WidthOf<SignedIntegral> <=
                             integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned long long>{};
        } else {
            return meta::TypeWrapper<
                integral::MakeUnsignedOf<SignedIntegral>>{};
        }
    }())::Type;

    OpUnsignedIntegral op_dst_value;

    EndiannessEnum::Value endianness_value{ [=]() {
        if constexpr (IsEndiannessEnum<EndiannessLike>) {
            return EndiannessLike::value;
        } else {
            return endianness_like.value();
        }
    }() };

    constexpr unsigned long long signed_integral_octet_cnt{
        integral_math::CeilDiv(integral::WidthOf<SignedIntegral>, 8ULL)
    };

    constexpr unsigned long long op_unsigned_integral_octet_cnt{
        integral_math::CeilDiv(integral::WidthOf<OpUnsignedIntegral>, 8ULL)
    };

    unsigned char buffer[op_unsigned_integral_octet_cnt];

    unsigned long long exc_size;
    unsigned long long eff_size;

    if (octet_cnt_like() < signed_integral_octet_cnt) {
        exc_size = 0;
        eff_size = octet_cnt_like();
    } else {
        exc_size = octet_cnt_like() - signed_integral_octet_cnt;
        eff_size = signed_integral_octet_cnt;
    }

    bool no_lossy{ true };

    bool see0{ false };
    bool see1{ false };

    auto discard_exc_octets{ [&provider, &buffer, &no_lossy, exc_size, &see0,
                              &see1]() {
        unsigned long long k{ exc_size };

        for (; op_unsigned_integral_octet_cnt <= k;
             k -= op_unsigned_integral_octet_cnt) {
            provider(buffer, 1, op_unsigned_integral_octet_cnt);

            for (unsigned long long i{ 0 }; i < op_unsigned_integral_octet_cnt;
                 ++i) {
                if (255 < buffer[i]) {
                    buffer[i] %= 256;
                    no_lossy = false;
                }

                see0 |= buffer[i] != 255;
                see1 |= buffer[i] != 0;
            }
        }

        if (0 < k) {
            ZETA_Core_Debug_PrintVar(k);

            provider(buffer, 1, k);

            for (unsigned long long i{ 0 }; i < k; ++i) {
                if (255 < buffer[i]) {
                    buffer[i] %= 256;
                    no_lossy = false;
                }

                see0 |= buffer[i] != 255;
                see1 |= buffer[i] != 0;
            }
        }
    } };

    auto inspect_highest_eff_octet{ [&see0, &see1,
                                     eff_size](unsigned char m_octet) {
        if (eff_size < signed_integral_octet_cnt) {
            see0 |= m_octet < 128;
            see1 |= 128 <= m_octet;
        } else {
            constexpr unsigned long long k{
                (integral::WidthOf<SignedIntegral> + 7) % 8
            };

            m_octet >>= k;

            see0 |= m_octet != ((1U << (8 - k)) - 1U);
            see1 |= m_octet != 0;
        }
    } };

    switch (endianness_value) {
    case EndiannessEnum::Little::value: {
        provider(buffer, 1, eff_size);

        op_dst_value = buffer[eff_size - 1];

        for (unsigned long long i{ eff_size - 1 }; 0 < i--;) {
            unsigned char octet{ buffer[i] };

            if (255 < octet) {
                octet %= 256;
                no_lossy = false;
            }

            op_dst_value *= 256;
            op_dst_value += buffer[i];
        }

        inspect_highest_eff_octet(buffer[eff_size - 1]);

        discard_exc_octets();

        break;
    }

    case EndiannessEnum::Big::value: {
        discard_exc_octets();

        provider(buffer, 1, eff_size);

        op_dst_value = buffer[0];

        for (unsigned long long i{ 1 }; i < eff_size; ++i) {
            unsigned char octet{ buffer[i] };

            if (255 < octet) {
                octet %= 256;
                no_lossy = false;
            }

            op_dst_value *= 256;
            op_dst_value += buffer[i];
        }

        inspect_highest_eff_octet(buffer[0]);

        break;
    }

    default: ZETA_Core_Unreachable();
    }

    no_lossy &= (see0 == 0) || (see1 == 0);

    if constexpr (integral::WidthOf<SignedIntegral> <
                  integral::WidthOf<OpUnsignedIntegral>) {
        op_dst_value %= integral_math::PowerOf2<OpUnsignedIntegral>(
            integral::WidthOf<SignedIntegral>);
    }

    OpUnsignedIntegral special_value{
        integral_math::PowerOf2<OpUnsignedIntegral>(
            comparison_utils::BasicMin(eff_size * 8,
                                       integral::WidthOf<SignedIntegral>) -
            1)
    };

    if (op_dst_value < special_value) {
        dst_value = static_cast<SignedIntegral>(op_dst_value);
    } else if (special_value < op_dst_value) {
        dst_value = -static_cast<SignedIntegral>(-op_dst_value % special_value);
    } else {
        dst_value = -integral_math::PowerOf2Minus1<SignedIntegral>(
                        integral::WidthOf<SignedIntegral> - 1) -
                    static_cast<SignedIntegral>(1);
    }

    if (dst_error != nullptr) {
        if (allow_lossy || no_lossy) {
            dst_error->message = nullptr;
            dst_error->analyzer = nullptr;
        } else {
            dst_error->message = ErrorMessage::
                information_loss_when_deserializing_signed_integral;
            dst_error->analyzer = nullptr;
        }
    }

    return no_lossy;
}

template <typename Integral, typename OctetCntLike, typename EndiannessLike,
          typename Acceptor>
bool serde_utils::SerializeIntegral(Acceptor&& acceptor, Integral src_value,
                                    OctetCntLike octet_cnt_like,
                                    EndiannessLike endianness_like,
                                    bool allow_lossy, error::Error* dst_error) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    if constexpr (integral::IsSignedIntegral<Integral>) {
        return (SerializeSignedIntegral)(acceptor, src_value, octet_cnt_like,
                                         endianness_like, allow_lossy,
                                         dst_error);
    } else {
        return (SerializeUnsignedIntegral)(acceptor, src_value, octet_cnt_like,
                                           endianness_like, allow_lossy,
                                           dst_error);
    }
}

template <typename Integral, typename OctetCntLike, typename EndiannessLike,
          typename Provider>
bool serde_utils::DeserializeIntegral(Provider&& provider, Integral& dst_value,
                                      OctetCntLike octet_cnt_like,
                                      EndiannessLike endianness_like,
                                      bool allow_lossy,
                                      error::Error* dst_error) {
    ZETA_Core_StaticAssert(integral::IsIntegral<Integral>);

    if constexpr (integral::IsSignedIntegral<Integral>) {
        return (DeserializeSignedIntegral)(provider, dst_value, octet_cnt_like,
                                           endianness_like, allow_lossy,
                                           dst_error);
    } else {
        return (DeserializeUnsignedIntegral)(provider, dst_value,
                                             octet_cnt_like, endianness_like,
                                             allow_lossy, dst_error);
    }
}

}  // namespace zeta::core
