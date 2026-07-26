#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/pair.ipp>
#include <zeta/core/serde_utils.hpp>
#include <zeta/core/utils.hpp>
#include <zeta/core/value_wrapper.ipp>

namespace zeta::core {

template <integral::IsUnsignedIntegral Integral>
pair::Pair<Integral, bool> serde_utils::CanonicalizeIntegral(
    Integral value, Integral range_max) {
    bool no_lossy{ true };

    if (range_max < value) {
        value %= range_max + 1;
        no_lossy = false;
    }

    return { value, no_lossy };
}

template <integral::IsIntegral Integral, typename EndiannessLike,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, elem_stream::acceptor::IsAcceptor Acceptor>
    requires requires {
        requires serde_utils::IsEndiannessEnum<EndiannessLike> ||
                     value_wrapper::IsValueWrapperWith<
                         EndiannessLike, serde_utils::EndiannessEnum::Value>;

        requires 0 < DigitWidth;

        requires DigitWidth <= integral::WidthOf<DigitIntegral>;

        requires value_wrapper::IsValueWrapperWith<DigitCntLike, size_t>;
    }
bool serde_utils::SerializeIntegral(
    Integral src_value, EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral>,
    value_wrapper::StaticValueWrapper<size_t, DigitWidth>,
    DigitCntLike digit_cnt_like, bool allow_lossy, Acceptor&& acceptor,
    error::Error* dst_error) {
    constexpr bool is_signed{ integral::IsSignedIntegral<Integral> };

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

    EndiannessEnum::Value endianness_value{ [=]() {
        if constexpr (IsEndiannessEnum<EndiannessLike>) {
            return EndiannessLike::value;
        } else {
            return endianness_like();
        }
    }() };

    constexpr DigitIntegral digit_range_max{
        integral_math::PowerOf2Minus1<DigitIntegral>(DigitWidth)
    };

    ZETA_Core_DebugAssert(0 < digit_cnt_like());

    bool is_neg{ src_value < 0 };

    OpUnsignedIntegral op_src_value{ static_cast<OpUnsignedIntegral>(
        src_value) };

    constexpr size_t src_int_digit_cnt{ integral_math::CeilDiv(
        integral::WidthOf<Integral>, DigitWidth) };

    constexpr size_t op_un_int_digit_cnt{ integral_math::CeilDiv(
        integral::WidthOf<OpUnsignedIntegral>, DigitWidth) };

    DigitIntegral buffer[op_un_int_digit_cnt];

    size_t exc_digit_cnt;
    size_t eff_digit_cnt;

    if (digit_cnt_like() < src_int_digit_cnt) {
        exc_digit_cnt = 0;
        eff_digit_cnt = digit_cnt_like();
    } else {
        exc_digit_cnt = digit_cnt_like() - src_int_digit_cnt;
        eff_digit_cnt = src_int_digit_cnt;
    }

    bool no_lossy{ true };

    auto proc_highest_eff_digit{ [is_neg, eff_digit_cnt,
                                  &no_lossy](DigitIntegral highest_eff_digit) {
        ZETA_Core_Unused(is_neg);
        ZETA_Core_Unused(eff_digit_cnt);
        ZETA_Core_Unused(no_lossy);

        constexpr size_t k{ integral::WidthOf<Integral> % DigitWidth };

        if constexpr (is_signed) {
            if (k == 0 || eff_digit_cnt < src_int_digit_cnt) {
                ZETA_Core_Debug_PrintVar(is_neg);
                ZETA_Core_Debug_PrintVar(digit_range_max / 2);
                ZETA_Core_Debug_PrintVar(highest_eff_digit);

                no_lossy &= is_neg == digit_range_max / 2 < highest_eff_digit;
            } else if (is_neg) {
                ZETA_Core_Debug_PrintVar(digit_range_max -
                                         (digit_range_max >> (DigitWidth - k)));

                highest_eff_digit +=
                    digit_range_max - (digit_range_max >> (DigitWidth - k));
            }
        }

        return highest_eff_digit;
    } };

    switch (endianness_value) {
    case EndiannessEnum::Little::value: {
        ZETA_Core_Debug_PrintVar(eff_digit_cnt);

        for (size_t i{ 0 }; i < eff_digit_cnt; ++i) {
            ZETA_Core_Debug_PrintVar(digit_range_max);

            buffer[i] = static_cast<DigitIntegral>(
                op_src_value %
                (static_cast<OpUnsignedIntegral>(digit_range_max) + 1U));

            ZETA_Core_Debug_PrintVar(buffer[i]);

            op_src_value >>= DigitWidth;
        }

        buffer[eff_digit_cnt - 1] =
            proc_highest_eff_digit(buffer[eff_digit_cnt - 1]);

        ZETA_Core_Debug_PrintVar(buffer[eff_digit_cnt - 1]);

        elem_stream::acceptor::Transfer(acceptor, buffer, sizeof(DigitIntegral),
                                        sizeof(DigitIntegral), eff_digit_cnt);

        if (0 < exc_digit_cnt) {
            buffer[0] = is_neg ? digit_range_max : 0;

            elem_stream::acceptor::Transfer(
                acceptor, buffer, sizeof(DigitIntegral), 0, exc_digit_cnt);
        }

        break;
    }

    case EndiannessEnum::Big::value: {
        if (0 < exc_digit_cnt) {
            buffer[0] = is_neg ? digit_range_max : 0;
            elem_stream::acceptor::Transfer(
                acceptor, buffer, sizeof(DigitIntegral), 0, exc_digit_cnt);
        }

        for (size_t i{ eff_digit_cnt }; 0 < i--;) {
            buffer[i] = static_cast<DigitIntegral>(
                op_src_value %
                (static_cast<OpUnsignedIntegral>(digit_range_max) + 1U));

            op_src_value >>= DigitWidth;
        }

        buffer[0] = proc_highest_eff_digit(buffer[0]);

        elem_stream::acceptor::Transfer(acceptor, buffer, sizeof(DigitIntegral),
                                        sizeof(DigitIntegral), eff_digit_cnt);

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
            if constexpr (is_signed) {
                dst_error->message = ErrorMessage::
                    information_loss_when_serializing_signed_integral;
            } else {
                dst_error->message = ErrorMessage::
                    information_loss_when_serializing_unsigned_integral;
            }

            dst_error->analyzer = nullptr;
        }
    }

    return no_lossy;
}

template <integral::IsIntegral Integral, typename EndiannessLike,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, elem_stream::provider::IsProvider Provider>
    requires requires {
        requires serde_utils::IsEndiannessEnum<EndiannessLike> ||
                     value_wrapper::IsValueWrapperWith<
                         EndiannessLike, serde_utils::EndiannessEnum::Value>;

        requires 0 < DigitWidth;

        requires DigitWidth <= integral::WidthOf<DigitIntegral>;

        requires meta::IsSame<DigitCntLike, serde_utils::VariableOctetCntTag> ||
                     value_wrapper::IsValueWrapperWith<DigitCntLike, size_t>;
    }
bool serde_utils::DeserializeIntegral(
    Integral& dst_value, EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral>,
    value_wrapper::StaticValueWrapper<size_t, DigitWidth>,
    DigitCntLike digit_cnt_like, bool allow_lossy, Provider&& provider,
    error::Error* dst_error) {
    constexpr bool is_signed{ integral::IsSignedIntegral<Integral> };

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

    EndiannessEnum::Value endianness_value;

    if constexpr (IsEndiannessEnum<EndiannessLike>) {
        endianness_value = EndiannessLike::value;
    } else {
        endianness_value = endianness_like();
    }

    DigitIntegral digit_range_max{ integral_math::PowerOf2Minus1<DigitIntegral>(
        DigitWidth) };

    constexpr bool is_variable_digit_cnt{
        meta::IsSame<DigitCntLike, VariableOctetCntTag>
    };

    constexpr size_t src_int_digit_cnt{ integral_math::CeilDiv(
        integral::WidthOf<Integral>, DigitWidth) };

    constexpr size_t op_un_int_digit_cnt{ integral_math::CeilDiv(
        integral::WidthOf<OpUnsignedIntegral>, DigitWidth) };

    size_t digit_cnt;

    if constexpr (is_variable_digit_cnt) {
        digit_cnt = 0;
    } else {
        digit_cnt = digit_cnt_like();
    }

    constexpr size_t buffer_digit_cnt{ comparison_utils::BasicMax(
        src_int_digit_cnt, 32U) };

    DigitIntegral buffer_a[buffer_digit_cnt];
    DigitIntegral buffer_b[buffer_digit_cnt];

    size_t eff_digit_cnt;

    bool see0{ false };
    bool see1{ false };

    bool no_lossy{ true };

    auto proc_digits_from_buffer{ [digit_range_max, &no_lossy](
                                      DigitIntegral* buffer, size_t digit_cnt) {
        for (DigitIntegral* buffer_end{ buffer + digit_cnt };
             buffer < buffer_end; ++buffer) {
            auto [canon_digit, cur_no_lossy]{ (
                CanonicalizeIntegral)(*buffer, digit_range_max) };

            *buffer = canon_digit;
            no_lossy &= cur_no_lossy;
        }
    } };

    auto discard_digits_from_buffer{ [digit_range_max, &no_lossy, &see0, &see1](
                                         DigitIntegral* buffer,
                                         size_t digit_cnt) {
        for (DigitIntegral* buffer_end{ buffer + digit_cnt };
             buffer < buffer_end; ++buffer) {
            DigitIntegral d{ *buffer };

            if (digit_range_max < d) {
                no_lossy = false;
                ZETA_Core_Debug_PrintVar(no_lossy);
            }

            see0 |= d != digit_range_max;
            see1 |= d != 0;

            ZETA_Core_Debug_PrintVar(d);
            ZETA_Core_Debug_PrintVar(see0);
            ZETA_Core_Debug_PrintVar(see1);
        }
    } };

    auto discard_digits_from_provider{ [&provider, &proc_digits_from_buffer,
                                        &discard_digits_from_buffer](
                                           size_t res_digit_cnt) {
        DigitIntegral buffer[buffer_digit_cnt];

        for (; buffer_digit_cnt <= res_digit_cnt;
             res_digit_cnt -= buffer_digit_cnt) {
            elem_stream::provider::Transfer(
                provider, buffer, sizeof(DigitIntegral), sizeof(DigitIntegral),
                buffer_digit_cnt);

            proc_digits_from_buffer(buffer, buffer_digit_cnt);
            discard_digits_from_buffer(buffer, buffer_digit_cnt);
        }

        if (0 < res_digit_cnt) {
            elem_stream::provider::Transfer(
                provider, buffer, sizeof(DigitIntegral), sizeof(DigitIntegral),
                res_digit_cnt);

            proc_digits_from_buffer(buffer, res_digit_cnt);
            discard_digits_from_buffer(buffer, res_digit_cnt);
        }
    } };

    auto proc_highest_eff_digit{ [digit_range_max, &see0, &see1,
                                  &eff_digit_cnt](DigitIntegral d) {
        ZETA_Core_Unused(digit_range_max);
        ZETA_Core_Unused(see0);
        ZETA_Core_Unused(see1);

        if (eff_digit_cnt == src_int_digit_cnt) {
            // In the highest digit, there may be bits that are not contained in
            // the destination integral type. These bits must be checked to
            // determine whether information loss occurs.
            //
            // For unsigned integrals, such bits only exist when the width of
            // the destination integral type is not a multiple of DigitWidth.
            //
            // For signed integrals, the sign bit must also be preserved.
            // Therefore, there is always bits in the highest digit that must be
            // checked.
            //
            // k is the number of lower bits in the highest digit that are
            // removed before checking the remaining bits.

            constexpr size_t k{ (integral::WidthOf<Integral> + DigitWidth -
                                 is_signed) %
                                DigitWidth };

            if constexpr (is_signed || 0 < k) {
                d >>= k;

                see0 |= d != (digit_range_max >> k);
                see1 |= d != 0;

                ZETA_Core_Debug_PrintVar(d);
                ZETA_Core_Debug_PrintVar(see0);
                ZETA_Core_Debug_PrintVar(see1);
            }
        } else if constexpr (is_signed) {
            see0 |= d <= digit_range_max / 2;
            see1 |= digit_range_max / 2 < d;

            ZETA_Core_Debug_PrintVar(digit_range_max);
            ZETA_Core_Debug_PrintVar(d);
            ZETA_Core_Debug_PrintVar(see0);
            ZETA_Core_Debug_PrintVar(see1);
        }
    } };

    OpUnsignedIntegral op_dst_value{ 0 };

    ZETA_Core_Debug_PrintVar(endianness_value);

    switch (endianness_value) {
    case EndiannessEnum::Little::value: {
        DigitIntegral* buffer_h{ buffer_a };
        DigitIntegral* buffer_l{ buffer_b };

        if constexpr (is_variable_digit_cnt) {
            eff_digit_cnt = elem_stream::provider::Transfer(
                provider, buffer_l, sizeof(DigitIntegral),
                sizeof(DigitIntegral), src_int_digit_cnt);

            proc_digits_from_buffer(buffer_l, eff_digit_cnt);

            if (eff_digit_cnt == src_int_digit_cnt) {
                for (;;) {
                    size_t cur_act_digit_cnt{ elem_stream::provider::Transfer(
                        provider, buffer_h, sizeof(DigitIntegral),
                        sizeof(DigitIntegral), op_un_int_digit_cnt) };

                    if (cur_act_digit_cnt == 0) { break; }

                    ZETA_Core_Debug_PrintVar(cur_act_digit_cnt);

                    proc_digits_from_buffer(buffer_h, cur_act_digit_cnt);
                    discard_digits_from_buffer(buffer_h, cur_act_digit_cnt);
                }
            }
        } else {
            eff_digit_cnt =
                comparison_utils::BasicMin(src_int_digit_cnt, digit_cnt);

            elem_stream::provider::Transfer(
                provider, buffer_l, sizeof(DigitIntegral),
                sizeof(DigitIntegral), eff_digit_cnt);

            proc_digits_from_buffer(buffer_l, eff_digit_cnt);
            discard_digits_from_provider(digit_cnt - eff_digit_cnt);
        }

        if (eff_digit_cnt == 0) { break; }

        proc_highest_eff_digit(buffer_l[eff_digit_cnt - 1]);

        for (size_t i{ eff_digit_cnt }; 0 < i--;) {
            ZETA_Core_Debug_PrintVar(buffer_l[i]);

            op_dst_value <<= DigitWidth;
            op_dst_value += buffer_l[i];
        }

        break;
    }

    case EndiannessEnum::Big::value: {
        DigitIntegral* buffer_h{ buffer_a };
        DigitIntegral* buffer_l{ buffer_b };

        size_t eff_digit_cnt_h;
        size_t eff_digit_cnt_l;

        if constexpr (is_variable_digit_cnt) {
            bool exceeded{ false };

            size_t last_eff_digit_cnt;

            for (;; exceeded = true) {
                last_eff_digit_cnt = elem_stream::provider::Transfer(
                    provider, buffer_l, sizeof(DigitIntegral),
                    sizeof(DigitIntegral), buffer_digit_cnt);

                proc_digits_from_buffer(buffer_l, last_eff_digit_cnt);

                if (exceeded) {
                    discard_digits_from_buffer(buffer_h, last_eff_digit_cnt);
                }

                if (last_eff_digit_cnt < buffer_digit_cnt) { break; }

                utils::Swap(buffer_h, buffer_l);
            }

            /*

            byte order highest to lowest:

            if (exceeded){
                buffer_h[last_eff_digit_cnt ~ buffer_digit_cnt - 1]
            }

            buffer_l[0 ~ last_eff_digit_cnt - 1]

            */

            if (exceeded) {
                buffer_h += last_eff_digit_cnt;

                eff_digit_cnt_h = buffer_digit_cnt - last_eff_digit_cnt;
                eff_digit_cnt_l = last_eff_digit_cnt;
            } else {
                eff_digit_cnt_h = 0;
                eff_digit_cnt_l = last_eff_digit_cnt;
            }

            /*

            byte order highest to lowest:

            if (exceeded){
                exc_buffer[0 ~ eff_digit_cnt_h - 1]
            }

            buffer_l[0 ~ last_eff_digit_cnt - 1]

            */

            if (src_int_digit_cnt < eff_digit_cnt_h + eff_digit_cnt_l) {
                ZETA_Core_Debug_PrintVar(src_int_digit_cnt);
                ZETA_Core_Debug_PrintVar(eff_digit_cnt_h);
                ZETA_Core_Debug_PrintVar(eff_digit_cnt_l);

                size_t k{ eff_digit_cnt_h + eff_digit_cnt_l -
                          src_int_digit_cnt };

                size_t k_h{ comparison_utils::BasicMin(k, eff_digit_cnt_h) };
                size_t k_l{ k - k_h };

                discard_digits_from_buffer(buffer_h, k_h);
                discard_digits_from_buffer(buffer_l, k_l);

                buffer_h += k_h;
                buffer_l += k_l;

                eff_digit_cnt_h -= k_h;
                eff_digit_cnt_l -= k_l;
            }
        } else {
            if (src_int_digit_cnt < digit_cnt) {
                discard_digits_from_provider(digit_cnt - src_int_digit_cnt);

                elem_stream::provider::Transfer(
                    provider, buffer_l, sizeof(DigitIntegral),
                    sizeof(DigitIntegral), src_int_digit_cnt);

                eff_digit_cnt_h = 0;
                eff_digit_cnt_l = src_int_digit_cnt;
            } else {
                elem_stream::provider::Transfer(
                    provider, buffer_l, sizeof(DigitIntegral),
                    sizeof(DigitIntegral), digit_cnt_like());

                eff_digit_cnt_h = 0;
                eff_digit_cnt_l = digit_cnt_like();
            }
        }

        eff_digit_cnt = eff_digit_cnt_h + eff_digit_cnt_l;

        ZETA_Core_Debug_PrintVar(src_int_digit_cnt);
        ZETA_Core_Debug_PrintVar(eff_digit_cnt_h);
        ZETA_Core_Debug_PrintVar(eff_digit_cnt_l);

        if (eff_digit_cnt == 0) { break; }

        ZETA_Core_Debug_PrintVar(eff_digit_cnt_h);
        ZETA_Core_Debug_PrintVar(eff_digit_cnt_l);

        proc_highest_eff_digit(0 < eff_digit_cnt_h ? buffer_h[0] : buffer_l[0]);

        for (size_t i{ 0 }; i < eff_digit_cnt_h; ++i) {
            ZETA_Core_Debug_PrintVar(buffer_h[i]);

            op_dst_value <<= DigitWidth;
            op_dst_value += buffer_h[i];
        }

        for (size_t i{ 0 }; i < eff_digit_cnt_l; ++i) {
            ZETA_Core_Debug_PrintVar(buffer_l[i]);

            op_dst_value <<= DigitWidth;
            op_dst_value += buffer_l[i];
        }

        break;
    }
    }

    if constexpr (is_signed) {
        ZETA_Core_Debug_PrintVar(see0);
        ZETA_Core_Debug_PrintVar(see1);
        ZETA_Core_Debug_PrintVar(no_lossy);

        no_lossy &= !see0 || !see1;

        ZETA_Core_Debug_PrintVar(eff_digit_cnt);
        ZETA_Core_Debug_PrintVar(op_dst_value);

        if (eff_digit_cnt == 0) {
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
                comparison_utils::BasicMin(DigitWidth * eff_digit_cnt,
                                           integral::WidthOf<Integral>) -
                1)
        };

        ZETA_Core_Debug_PrintVar(special_value);

        if (op_dst_value < special_value) {
            ZETA_Core_Debug_PrintCurPos;
            dst_value = static_cast<Integral>(op_dst_value);
        } else if (special_value < op_dst_value) {
            ZETA_Core_Debug_PrintCurPos;
            dst_value = -static_cast<Integral>(-op_dst_value % special_value);
        } else {
            ZETA_Core_Debug_PrintCurPos;
            dst_value = -static_cast<Integral>(special_value - 1) -
                        static_cast<Integral>(1);
        }
    } else {
        ZETA_Core_Debug_PrintVar(eff_digit_cnt);
        ZETA_Core_Debug_PrintVar(op_dst_value);

        no_lossy &= !see1;

        dst_value = static_cast<Integral>(op_dst_value);
    }

    if (dst_error != nullptr) {
        if (allow_lossy || no_lossy) {
            dst_error->message = nullptr;
            dst_error->analyzer = nullptr;
        } else {
            if constexpr (is_signed) {
                dst_error->message = ErrorMessage::
                    information_loss_when_deserializing_signed_integral;
            } else {
                dst_error->message = ErrorMessage::
                    information_loss_when_deserializing_unsigned_integral;
            }

            dst_error->analyzer = nullptr;
        }
    }

    return no_lossy;
}

}  // namespace zeta::core
