#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/integral_endec.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/pair.ipp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

template <integral::IsUnsignedIntegral Integral>
constexpr integral_endec::CanonicalizeResult<Integral>
integral_endec::CanonicalizeIntegral(Integral value, Integral range_max) {
    ZETA_Core_DebugAssert(0 < range_max);

    bool out_of_range{ false };

    if (range_max < value) {
        value %= range_max + 1;
        out_of_range = true;
    } else if constexpr (integral::IsSignedIntegral<Integral>) {
        if (value < -range_max) {
            value %= range_max + 1;

            value += range_max;
            ++value;

            out_of_range = true;
        } else if (value < 0) {
            value += range_max;
            ++value;

            out_of_range = true;
        }
    }

    return {
        .out_of_range = out_of_range,
        .value = value,
    };
}

template <elem_stream::acceptor::IsAcceptor Acceptor,
          integral_endec::IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, integral::IsIntegral SrcIntegral>
    requires requires {
        requires 0 < DigitWidth;
        requires DigitWidth <= integral::WidthOf<DigitIntegral>;
    }
constexpr integral_endec::EncodeResult integral_endec::Encode(
    Acceptor&& acceptor, EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral>, meta::ValueWrapper<size_t, DigitWidth>,
    DigitCntLike digit_cnt_like, SrcIntegral src_value) {
    constexpr bool is_signed{ integral::IsSignedIntegral<SrcIntegral> };

    using OpUnsignedIntegral = decltype([]() {
        if constexpr (integral::WidthOf<SrcIntegral> <=
                      integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned _BitInt(
                integral_math::CeilDiv(integral::WidthOf<unsigned>,
                                       DigitWidth) *
                DigitWidth)>{};
        } else if constexpr (integral::WidthOf<SrcIntegral> <=
                             integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned _BitInt(
                integral_math::CeilDiv(integral::WidthOf<unsigned long>,
                                       DigitWidth) *
                DigitWidth)>{};
        } else if constexpr (integral::WidthOf<SrcIntegral> <=
                             integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned _BitInt(
                integral_math::CeilDiv(integral::WidthOf<unsigned long long>,
                                       DigitWidth) *
                DigitWidth)>{};
        } else {
            return meta::TypeWrapper<unsigned _BitInt(
                integral_math::CeilDiv(integral::WidthOf<SrcIntegral>,
                                       DigitWidth) *
                DigitWidth)>{};
        }
    }())::Type;

    Endianness endianness_value{ [=]() -> Endianness {
        if constexpr (meta::IsValueWrapperT<EndiannessLike, Endianness>) {
            return EndiannessLike::value;
        } else if constexpr (meta::IsSame<EndiannessLike, Endianness>) {
            return endianness_like;
        } else {
            ZETA_Core_StaticAssert(false);
        }
    }() };

    size_t digit_cnt{ [=]() -> size_t {
        if constexpr (meta::IsValueWrapperT<DigitCntLike, size_t>) {
            ZETA_Core_StaticAssert(0 < DigitCntLike::value);
            return DigitCntLike::value;
        } else if constexpr (meta::IsSame<DigitCntLike, size_t>) {
            ZETA_Core_DebugAssert(0 < digit_cnt_like);
            return digit_cnt_like;
        } else {
            ZETA_Core_StaticAssert(false);
        }
    }() };

    constexpr DigitIntegral digit_range_max{
        integral_math::PowerOf2Minus1<DigitIntegral>(DigitWidth)
    };

    bool is_neg{ src_value < 0 };

    OpUnsignedIntegral op_src_value{ static_cast<OpUnsignedIntegral>(
        src_value) };

    constexpr size_t op_un_int_digit_cnt{ integral_math::CeilDiv(
        integral::WidthOf<OpUnsignedIntegral>, DigitWidth) };

    DigitIntegral buffer[op_un_int_digit_cnt];

    size_t exc_digit_cnt;
    size_t eff_digit_cnt;

    if (digit_cnt < op_un_int_digit_cnt) {
        exc_digit_cnt = 0;
        eff_digit_cnt = digit_cnt;
    } else {
        exc_digit_cnt = digit_cnt - op_un_int_digit_cnt;
        eff_digit_cnt = op_un_int_digit_cnt;
    }

    bool value_out_of_range{ false };

    auto proc_highest_eff_digit{ [is_neg, eff_digit_cnt, &value_out_of_range](
                                     DigitIntegral highest_eff_digit) -> void {
        ZETA_Core_Unused(is_neg);
        ZETA_Core_Unused(eff_digit_cnt);
        ZETA_Core_Unused(value_out_of_range);

        if constexpr (is_signed) {
            if (eff_digit_cnt < op_un_int_digit_cnt) {
                value_out_of_range |=
                    is_neg != digit_range_max / 2 < highest_eff_digit;
            }
        }
    } };

    switch (endianness_value) {
    case Endianness::Little: {
        for (size_t i{ 0 }; i < eff_digit_cnt; ++i) {
            buffer[i] = static_cast<DigitIntegral>(
                op_src_value %
                (static_cast<OpUnsignedIntegral>(digit_range_max) + 1U));

            op_src_value >>= DigitWidth;
        }

        proc_highest_eff_digit(buffer[eff_digit_cnt - 1]);

        elem_stream::acceptor::Transfer(acceptor, buffer, sizeof(DigitIntegral),
                                        sizeof(DigitIntegral), eff_digit_cnt);

        if (0 < exc_digit_cnt) {
            buffer[0] = is_neg ? digit_range_max : 0;

            elem_stream::acceptor::Transfer(
                acceptor, buffer, sizeof(DigitIntegral), 0, exc_digit_cnt);
        }

        break;
    }

    case Endianness::Big: {
        if (0 < exc_digit_cnt) {
            buffer[0] = is_neg ? digit_range_max : 0;

            elem_stream::acceptor::Transfer(
                acceptor, buffer, sizeof(DigitIntegral), 0, exc_digit_cnt);
        }

        for (size_t i{ eff_digit_cnt }; 0 < i--;) {
            buffer[i] = static_cast<DigitIntegral>(
                op_src_value %
                (static_cast<OpUnsignedIntegral>(digit_range_max) + 1U));

            ZETA_Core_Debug_PrintVar(buffer[i]);

            op_src_value >>= DigitWidth;

            ZETA_Core_Debug_PrintVar(op_src_value);
        }

        proc_highest_eff_digit(buffer[0]);

        elem_stream::acceptor::Transfer(acceptor, buffer, sizeof(DigitIntegral),
                                        sizeof(DigitIntegral), eff_digit_cnt);

        break;
    }

    default: ZETA_Core_Unreachable();
    }

    if (is_neg) {
        value_out_of_range |= !integral_math::IsPowerOf2(op_src_value + 1U);
    } else {
        value_out_of_range |= op_src_value != 0;
    }

    return {
        .value_out_of_range = value_out_of_range,
    };
}

template <elem_stream::provider::IsProvider Provider,
          integral_endec::IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegral DigitIntegral, size_t DigitWidth,
          typename DigitCntLike, integral::IsIntegral DstIntegral>
    requires requires {
        requires 0 < DigitWidth;
        requires DigitWidth <= integral::WidthOf<DigitIntegral>;
    }
constexpr integral_endec::DecodeResult<DstIntegral> integral_endec::Decode(
    Provider&& provider, EndiannessLike endianness_like,
    meta::TypeWrapper<DigitIntegral>, meta::ValueWrapper<size_t, DigitWidth>,
    DigitCntLike digit_cnt_like, meta::TypeWrapper<DstIntegral>) {
    constexpr bool is_signed{ integral::IsSignedIntegral<DstIntegral> };

    using OpUnsignedIntegral = decltype([]() {
        if constexpr (integral::WidthOf<DstIntegral> <=
                      integral::WidthOf<unsigned>) {
            return meta::TypeWrapper<unsigned>{};
        } else if constexpr (integral::WidthOf<DstIntegral> <=
                             integral::WidthOf<unsigned long>) {
            return meta::TypeWrapper<unsigned long>{};
        } else if constexpr (integral::WidthOf<DstIntegral> <=
                             integral::WidthOf<unsigned long long>) {
            return meta::TypeWrapper<unsigned long long>{};
        } else {
            return meta::TypeWrapper<integral::MakeUnsignedOf<DstIntegral>>{};
        }
    }())::Type;

    Endianness endianness_value{ [=]() -> Endianness {
        if constexpr (meta::IsValueWrapperT<EndiannessLike, Endianness>) {
            return EndiannessLike::value;
        } else if constexpr (meta::IsSame<EndiannessLike, Endianness>) {
            return endianness_like;
        } else {
            ZETA_Core_StaticAssert(false);
        }
    }() };

    DigitIntegral digit_range_max{ integral_math::PowerOf2Minus1<DigitIntegral>(
        DigitWidth) };

    constexpr size_t dst_int_digit_cnt{ integral_math::CeilDiv(
        integral::WidthOf<DstIntegral>, DigitWidth) };

    constexpr size_t op_un_int_digit_cnt{ integral_math::CeilDiv(
        integral::WidthOf<OpUnsignedIntegral>, DigitWidth) };

    size_t digit_cnt{ [=]() -> size_t {
        if constexpr (meta::IsSame<DigitCntLike,
                                   integral_endec::VariableOctetCntTag>) {
            return 0;
        } else if constexpr (meta::IsValueWrapperT<DigitCntLike, size_t>) {
            ZETA_Core_StaticAssert(0 < DigitCntLike::value);
            return DigitCntLike::value;
        } else if constexpr (meta::IsSame<DigitCntLike, size_t>) {
            ZETA_Core_DebugAssert(0 < digit_cnt_like);
            return digit_cnt_like;
        } else {
            ZETA_Core_StaticAssert(false);
        }
    }() };

    constexpr size_t buffer_digit_cnt{ comparison_utils::BasicMax(
        dst_int_digit_cnt, 32U) };

    DigitIntegral buffer_a[buffer_digit_cnt];
    DigitIntegral buffer_b[buffer_digit_cnt];

    size_t eff_digit_cnt;

    bool see0{ false };
    bool see1{ false };

    bool pad_zero{ false };

    bool digit_out_of_range{ false };

    bool value_out_of_range{ false };

    DstIntegral dst_value;

    auto transfer_digits{ [&provider, &pad_zero](DigitIntegral* dst,
                                                 size_t elem_cnt) -> void {
        size_t transfer_elem_cnt{ elem_stream::provider::Transfer(
            provider, dst, sizeof(DigitIntegral), sizeof(DigitIntegral),
            elem_cnt) };

        ZETA_Core_DebugAssert(transfer_elem_cnt <= elem_cnt);

        if (transfer_elem_cnt == elem_cnt) { return; }

        pad_zero = true;

        DigitIntegral* dst_end{ dst + elem_cnt };
        dst += transfer_elem_cnt;

        for (; dst != dst_end; ++dst) { *dst = 0; }
    } };

    auto proc_digits_from_buffer{ [digit_range_max, &digit_out_of_range](
                                      DigitIntegral* buffer,
                                      size_t digit_cnt) -> void {
        for (DigitIntegral* buffer_end{ buffer + digit_cnt };
             buffer < buffer_end; ++buffer) {
            auto [cur_digit_out_of_range, canon_digit]{ (
                CanonicalizeIntegral)(*buffer, digit_range_max) };

            digit_out_of_range |= cur_digit_out_of_range;
            *buffer = canon_digit;
        }
    } };

    auto discard_digits_from_buffer{ [digit_range_max, &digit_out_of_range,
                                      &see0, &see1](DigitIntegral* buffer,
                                                    size_t digit_cnt) -> void {
        for (DigitIntegral* buffer_end{ buffer + digit_cnt };
             buffer < buffer_end; ++buffer) {
            auto [cur_digit_out_of_range, canon_digit]{ (
                CanonicalizeIntegral)(*buffer, digit_range_max) };

            digit_out_of_range |= cur_digit_out_of_range;

            see0 |= canon_digit != digit_range_max;
            see1 |= canon_digit != 0;
        }
    } };

    auto discard_digits_from_provider{
        [&transfer_digits, &proc_digits_from_buffer,
         &discard_digits_from_buffer](size_t res_digit_cnt) -> void {
            DigitIntegral buffer[buffer_digit_cnt];

            for (; buffer_digit_cnt <= res_digit_cnt;
                 res_digit_cnt -= buffer_digit_cnt) {
                transfer_digits(buffer, buffer_digit_cnt);
                proc_digits_from_buffer(buffer, buffer_digit_cnt);
                discard_digits_from_buffer(buffer, buffer_digit_cnt);
            }

            if (0 < res_digit_cnt) {
                transfer_digits(buffer, res_digit_cnt);
                proc_digits_from_buffer(buffer, res_digit_cnt);
                discard_digits_from_buffer(buffer, res_digit_cnt);
            }
        }
    };

    auto proc_highest_eff_digit{ [digit_range_max, &see0, &see1,
                                  &eff_digit_cnt](DigitIntegral d) -> void {
        ZETA_Core_Unused(digit_range_max);
        ZETA_Core_Unused(see0);
        ZETA_Core_Unused(see1);

        if (eff_digit_cnt == dst_int_digit_cnt) {
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

            constexpr size_t k{ (integral::WidthOf<DstIntegral> + DigitWidth -
                                 is_signed) %
                                DigitWidth };

            if constexpr (is_signed || 0 < k) {
                d >>= k;

                see0 |= d != (digit_range_max >> k);
                see1 |= d != 0;
            }
        } else if constexpr (is_signed) {
            see0 |= d <= digit_range_max / 2;
            see1 |= digit_range_max / 2 < d;
        }
    } };

    OpUnsignedIntegral op_dst_value{ 0 };

    switch (endianness_value) {
    case Endianness::Little: {
        DigitIntegral* buffer_h{ buffer_a };
        DigitIntegral* buffer_l{ buffer_b };

        if (digit_cnt == 0) {
            eff_digit_cnt = elem_stream::provider::Transfer(
                provider, buffer_l, sizeof(DigitIntegral),
                sizeof(DigitIntegral), dst_int_digit_cnt);

            proc_digits_from_buffer(buffer_l, eff_digit_cnt);

            if (eff_digit_cnt == dst_int_digit_cnt) {
                for (;;) {
                    size_t cur_act_digit_cnt{ elem_stream::provider::Transfer(
                        provider, buffer_h, sizeof(DigitIntegral),
                        sizeof(DigitIntegral), op_un_int_digit_cnt) };

                    if (cur_act_digit_cnt == 0) { break; }

                    proc_digits_from_buffer(buffer_h, cur_act_digit_cnt);
                    discard_digits_from_buffer(buffer_h, cur_act_digit_cnt);
                }
            }
        } else {
            eff_digit_cnt =
                comparison_utils::BasicMin(dst_int_digit_cnt, digit_cnt);

            transfer_digits(buffer_l, eff_digit_cnt);

            proc_digits_from_buffer(buffer_l, eff_digit_cnt);
            discard_digits_from_provider(digit_cnt - eff_digit_cnt);
        }

        if (eff_digit_cnt == 0) { break; }

        proc_highest_eff_digit(buffer_l[eff_digit_cnt - 1]);

        for (size_t i{ eff_digit_cnt }; 0 < i--;) {
            op_dst_value <<= DigitWidth;
            op_dst_value += buffer_l[i];
        }

        break;
    }

    case Endianness::Big: {
        DigitIntegral* buffer_h{ buffer_a };
        DigitIntegral* buffer_l{ buffer_b };

        size_t eff_digit_cnt_h;
        size_t eff_digit_cnt_l;

        if (digit_cnt == 0) {
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

            if (dst_int_digit_cnt < eff_digit_cnt_h + eff_digit_cnt_l) {
                size_t k{ eff_digit_cnt_h + eff_digit_cnt_l -
                          dst_int_digit_cnt };

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
            if (dst_int_digit_cnt < digit_cnt) {
                discard_digits_from_provider(digit_cnt - dst_int_digit_cnt);

                transfer_digits(buffer_l, dst_int_digit_cnt);

                eff_digit_cnt_h = 0;
                eff_digit_cnt_l = dst_int_digit_cnt;
            } else {
                elem_stream::provider::Transfer(
                    provider, buffer_l, sizeof(DigitIntegral),
                    sizeof(DigitIntegral), digit_cnt);

                eff_digit_cnt_h = 0;
                eff_digit_cnt_l = digit_cnt;
            }
        }

        eff_digit_cnt = eff_digit_cnt_h + eff_digit_cnt_l;

        if (eff_digit_cnt == 0) { break; }

        proc_highest_eff_digit(0 < eff_digit_cnt_h ? buffer_h[0] : buffer_l[0]);

        for (size_t i{ 0 }; i < eff_digit_cnt_h; ++i) {
            op_dst_value <<= DigitWidth;
            op_dst_value += buffer_h[i];
        }

        for (size_t i{ 0 }; i < eff_digit_cnt_l; ++i) {
            op_dst_value <<= DigitWidth;
            op_dst_value += buffer_l[i];
        }

        break;
    }
    }

    if constexpr (is_signed) {
        value_out_of_range |= see0 && see1;

        if (eff_digit_cnt == 0) {
            return {
                .pad_zero = pad_zero,
                .digit_out_of_range = digit_out_of_range,
                .value_out_of_range = value_out_of_range,
                .value = 0,
            };
        }

        if constexpr (integral::WidthOf<DstIntegral> <
                      integral::WidthOf<OpUnsignedIntegral>) {
            op_dst_value %= integral_math::PowerOf2<OpUnsignedIntegral>(
                integral::WidthOf<DstIntegral>);
        }

        OpUnsignedIntegral special_value{
            integral_math::PowerOf2<OpUnsignedIntegral>(
                comparison_utils::BasicMin(DigitWidth * eff_digit_cnt,
                                           integral::WidthOf<DstIntegral>) -
                1)
        };

        if (op_dst_value < special_value) {
            dst_value = static_cast<DstIntegral>(op_dst_value);
        } else if (special_value < op_dst_value) {
            /*

            final is neg

            if (see0, see1) == (0, 0)
                pos -> neg

            if (see0, see1) == (0, 1)
                pos -> neg

            if (see0, see1) == (1, 0)
                pos -> neg

            if (see0, see1) == (1, 1)
                pos -> neg

            */

            dst_value =
                -static_cast<DstIntegral>(-op_dst_value % special_value);
        } else {
            dst_value = -static_cast<DstIntegral>(special_value - 1) -
                        static_cast<DstIntegral>(1);
        }
    } else {
        value_out_of_range |= see1;

        dst_value = static_cast<DstIntegral>(op_dst_value);
    }

    return {
        .pad_zero = pad_zero,
        .digit_out_of_range = digit_out_of_range,
        .value_out_of_range = value_out_of_range,
        .value = dst_value,
    };
}

}  // namespace zeta::core
