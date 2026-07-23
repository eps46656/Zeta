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

template <integral::IsIntegral Integral, typename OctetCntLike,
          typename EndiannessLike, elem_stream::IsAcceptor Acceptor>
bool serde_utils::SerializeIntegral2(Acceptor&& acceptor, Integral src_value,
                                     OctetCntLike octet_cnt_like,
                                     EndiannessLike endianness_like,
                                     bool allow_lossy,
                                     error::Error* dst_error) {
    ZETA_Core_StaticAssert(
        value_wrapper::IsValueWrapperWith<OctetCntLike, unsigned long long>);

    ZETA_Core_DebugAssert(0 < octet_cnt_like());

    ZETA_Core_StaticAssert(
        IsEndiannessEnum<EndiannessLike> ||
        value_wrapper::IsValueWrapperWith<EndiannessLike,
                                          EndiannessEnum::Value>);

    constexpr bool signedness{ integral::IsSignedIntegral<Integral> };

    using OpUnsignedIntegral = decltype([]() {
        if constexpr (integral::WidthOf<Integral> <=
                      integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned>{};
        } else if constexpr (integral::WidthOf<Integral> <=
                             integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned long>{};
        } else if constexpr (integral::WidthOf<Integral> <=
                             integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned long long>{};
        } else {
            return meta::TypeWrapper<integral::MakeUnsignedOf<Integral>>{};
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

    constexpr unsigned long long src_int_octet_cnt{ integral_math::CeilDiv(
        integral::WidthOf<Integral>, 8ULL) };

    constexpr unsigned long long op_un_int_octet_cnt{ integral_math::CeilDiv(
        integral::WidthOf<OpUnsignedIntegral>, 8ULL) };

    unsigned char buffer[op_un_int_octet_cnt];

    unsigned long long exc_size;
    unsigned long long eff_size;

    if (octet_cnt_like() < src_int_octet_cnt) {
        exc_size = 0;
        eff_size = octet_cnt_like();
    } else {
        exc_size = octet_cnt_like() - src_int_octet_cnt;
        eff_size = src_int_octet_cnt;
    }

    bool no_lossy{ true };

    auto proc_highest_eff_octet{ [is_neg, eff_size,
                                  &no_lossy](unsigned char highest_eff_octet) {
        constexpr unsigned long long k{ integral::WidthOf<OpUnsignedIntegral> %
                                        8 };

        if (k == 0 || eff_size < src_int_octet_cnt) {
            if (is_neg) {
                no_lossy &= 128 <= highest_eff_octet;
            } else if constexpr (signedness) {
                no_lossy &= highest_eff_octet < 128;
            }
        } else if (is_neg) {
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

template <integral::IsIntegral Integral, typename OctetCntLike,
          typename EndiannessLike, elem_stream::IsProvider Provider>
bool serde_utils::DeserializeIntegral2(Provider&& provider, Integral& dst_value,
                                       OctetCntLike octet_cnt_like,
                                       EndiannessLike endianness_like,
                                       bool allow_lossy,
                                       error::Error* dst_error) {
    ZETA_Core_StaticAssert(
        meta::IsSame<OctetCntLike, VariableOctetCntTag> ||
        value_wrapper::IsValueWrapperWith<OctetCntLike, unsigned long long>);

    constexpr bool signedness{ integral::IsSignedIntegral<Integral> };

    using OpUnsignedIntegral = decltype([]() {
        if constexpr (integral::WidthOf<Integral> <=
                      integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned>{};
        } else if constexpr (integral::WidthOf<Integral> <=
                             integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned long>{};
        } else if constexpr (integral::WidthOf<Integral> <=
                             integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned long long>{};
        } else {
            return meta::TypeWrapper<integral::MakeUnsignedOf<Integral>>{};
        }
    }())::Type;

    constexpr bool is_variable_octet_cnt{
        meta::IsSame<OctetCntLike, VariableOctetCntTag>
    };

    EndiannessEnum::Value endianness_value{ [=]() {
        if constexpr (IsEndiannessEnum<EndiannessLike>) {
            return EndiannessLike::value;
        } else {
            return endianness_like.value();
        }
    }() };

    constexpr size_t src_int_octet_cnt{ integral_math::CeilDiv(
        integral::WidthOf<Integral>, 8ULL) };

    constexpr size_t op_un_int_octet_cnt{ integral_math::CeilDiv(
        integral::WidthOf<OpUnsignedIntegral>, 8ULL) };

    constexpr size_t buffer_octet_cnt{ comparison_utils::BasicMax(
        src_int_octet_cnt, 32U) };

    unsigned char buffer_a[buffer_octet_cnt];
    unsigned char buffer_b[buffer_octet_cnt];

    size_t eff_octet_cnt;

    bool see0{ false };
    bool see1{ false };

    bool no_lossy{ true };

    auto discard_octets_from_buffer{
        [&no_lossy, &see0, &see1](unsigned char* buffer, size_t octet_cnt) {
            for (unsigned char* buffer_end{ buffer + octet_cnt };
                 buffer < buffer_end; ++buffer) {
                unsigned char c{ *buffer };

                if (255 < c) { no_lossy = false; }

                see0 |= c != 0;
                see1 |= c != 255;
            }
        }
    };

    auto discard_octets_from_provider{ [&provider](size_t res_octet_cnt) {
        unsigned char buffer[buffer_octet_cnt];

        for (; buffer_octet_cnt <= res_octet_cnt;
             res_octet_cnt -= buffer_octet_cnt) {
            provider(buffer, 1, buffer_octet_cnt);
            discard_octets_from_buffer(buffer, buffer_octet_cnt);
        }

        if (0 < res_octet_cnt) {
            provider(buffer, 1, res_octet_cnt);
            discard_octets_from_buffer(buffer, res_octet_cnt);
        }
    } };

    auto inspect_highest_eff_octet{ [&see0, &see1,
                                     &eff_octet_cnt](unsigned char m_octet) {
        if (eff_octet_cnt == src_int_octet_cnt) {
            // In the highest octet, there may be bits that are not contained in
            // the destination integral type. These bits must be checked to
            // determine whether information loss occurs.
            //
            // For unsigned integrals, such bits only exist when the width of
            // the destination integral type is not a multiple of 8.
            //
            // For signed integrals, the sign bit must also be preserved.
            // Therefore, there is always bits in the highest octet that must be
            // checked.
            //
            // k is the number of lower bits in the highest octet that are
            // removed before checking the remaining bits.

            constexpr size_t k{ (integral::WidthOf<Integral> + 8 - signedness) %
                                8 };

            if constexpr (signedness || 0 < k) {
                m_octet >>= k;

                see0 |= m_octet != ((1U << (8 - k)) - 1U);
                see1 |= m_octet != 0;
            }
        } else if constexpr (signedness) {
            see0 |= m_octet < 128;
            see1 |= 128 <= m_octet;
        }
    } };

    OpUnsignedIntegral op_dst_value{ 0 };

    switch (endianness_value) {
    case EndiannessEnum::Little::value: {
        unsigned char* buffer_h{ buffer_a };
        unsigned char* buffer_l{ buffer_b };

        if constexpr (is_variable_octet_cnt) {
            eff_octet_cnt = provider(buffer_l, 1, src_int_octet_cnt);

            for (size_t i{ 0 }; i < eff_octet_cnt; ++i) {
                if (255 < buffer_l[i]) {
                    buffer_l[i] %= 256;
                    no_lossy = false;
                }
            }

            if (eff_octet_cnt == src_int_octet_cnt) {
                for (;;) {
                    size_t cur_actual_octet_cnt{ provider(
                        buffer_h, 1, op_un_int_octet_cnt) };

                    if (cur_actual_octet_cnt == 0) { break; }

                    discard_octets_from_buffer(buffer_h, cur_actual_octet_cnt);
                }
            }
        } else {
            eff_octet_cnt =
                comparison_utils::BasicMin(src_int_octet_cnt, octet_cnt_like());

            provider(buffer_l, 1, eff_octet_cnt);

            discard_octets_from_provider(octet_cnt_like() - eff_octet_cnt);
        }

        if (eff_octet_cnt == 0) { break; }

        inspect_highest_eff_octet(buffer_l[eff_octet_cnt - 1]);

        for (size_t i{ eff_octet_cnt - 1 }; 0 < i--;) {
            op_dst_value *= 256;
            op_dst_value += buffer_l[i];
        }

        break;
    }

    case EndiannessEnum::Big::value: {
        unsigned char* buffer_h{ buffer_a };
        unsigned char* buffer_l{ buffer_b };

        size_t eff_octet_cnt_h;
        size_t eff_octet_cnt_l;

        if constexpr (is_variable_octet_cnt) {
            bool exceeded{ false };

            size_t last_eff_octet_cnt;

            for (;; exceeded = true) {
                last_eff_octet_cnt = provider(buffer_l, 1, buffer_octet_cnt);

                for (size_t i{ 0 }; i < last_eff_octet_cnt; ++i) {
                    if (255 < buffer_l[i]) {
                        buffer_l[i] %= 256;
                        no_lossy = false;
                    }
                }

                if (exceeded) {
                    discard_octets_from_buffer(buffer_h, last_eff_octet_cnt);
                }

                if (last_eff_octet_cnt < buffer_octet_cnt) { break; }

                utils::Swap(buffer_l, buffer_h);
            }

            /*

            byte order highest to lowest:

            if (exceeded){
                buffer_h[last_eff_octet_cnt ~ buffer_octet_cnt - 1]
            }

            buffer_l[0 ~ last_eff_octet_cnt - 1]

            */

            if (exceeded) {
                buffer_h += last_eff_octet_cnt;

                eff_octet_cnt_h = buffer_octet_cnt - last_eff_octet_cnt;
                eff_octet_cnt_l = last_eff_octet_cnt;
            } else {
                eff_octet_cnt_h = 0;
                eff_octet_cnt_l = last_eff_octet_cnt;
            }

            /*

            byte order highest to lowest:

            if (exceeded){
                exc_buffer[0 ~ eff_octet_cnt_h - 1]
            }

            buffer_l[0 ~ last_eff_octet_cnt - 1]

            */

            if (src_int_octet_cnt < eff_octet_cnt_h + eff_octet_cnt_l) {
                size_t k{ eff_octet_cnt_h + eff_octet_cnt_l -
                          src_int_octet_cnt };

                size_t k_h{ comparison_utils::BasicMin(k, eff_octet_cnt_h) };
                size_t k_l{ k - k_h };

                discard_octets_from_buffer(buffer_h, k_h);

                discard_octets_from_buffer(buffer_l, k_l);

                buffer_h += k_h;
                buffer_l += k_l;

                eff_octet_cnt_h -= k_h;
                eff_octet_cnt_l -= k_l;
            }
        } else {
            if (src_int_octet_cnt < octet_cnt_like()) {
                discard_octets_from_provider(octet_cnt_like() -
                                             src_int_octet_cnt);

                provider(buffer_l, 1, src_int_octet_cnt);

                eff_octet_cnt_h = 0;
                eff_octet_cnt_l = src_int_octet_cnt;
            } else {
                provider(buffer_l, 1, octet_cnt_like());

                eff_octet_cnt_h = 0;
                eff_octet_cnt_l = octet_cnt_like();
            }
        }

        eff_octet_cnt = eff_octet_cnt_h + eff_octet_cnt_l;

        if (eff_octet_cnt == 0) { break; }

        inspect_highest_eff_octet(0 < eff_octet_cnt_h ? buffer_h[0]
                                                      : buffer_l[0]);

        for (size_t i{ 0 }; i < eff_octet_cnt_h; ++i) {
            op_dst_value *= 256;
            op_dst_value += buffer_h[i];
        }

        for (size_t i{ 0 }; i < eff_octet_cnt_l; ++i) {
            op_dst_value *= 256;
            op_dst_value += buffer_l[i];
        }

        break;
    }
    }

    if constexpr (signedness) {
        no_lossy &= (see0 == 0) || (see1 == 0);

        if (eff_octet_cnt == 0) {
            dst_value = static_cast<Integral>(0);
            return no_lossy;
        }

        if constexpr (integral::WidthOf<Integral> <
                      integral::WidthOf<OpUnsignedIntegral>) {
            op_dst_value %= integral_math::PowerOf2<OpUnsignedIntegral>(
                integral::WidthOf<Integral>);
        }

        OpUnsignedIntegral special_value{
            integral_math::PowerOf2<OpUnsignedIntegral>(
                comparison_utils::BasicMin(eff_octet_cnt * 8,
                                           integral::WidthOf<Integral>) -
                1)
        };

        if (op_dst_value < special_value) {
            dst_value = static_cast<Integral>(op_dst_value);
        } else if (special_value < op_dst_value) {
            dst_value = -static_cast<Integral>(-op_dst_value % special_value);
        } else {
            dst_value = -integral_math::PowerOf2Minus1<Integral>(
                            integral::WidthOf<Integral> - 1) -
                        static_cast<Integral>(1);
        }
    } else {
        no_lossy &= see0 == 0;

        dst_value = static_cast<Integral>(op_dst_value);
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
