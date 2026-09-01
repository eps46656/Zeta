#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/integral_endec.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/utils.ipp>
#include <zeta/core/vlq_utils.hpp>

namespace zeta::core {

namespace vlq_utils::detail {

template <integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          elem_stream::provider::IsProvider InnerProvider>
struct Provider_ {
    ZETA_Core_StaticAssert(2 <= UnitWidth);
    ZETA_Core_StaticAssert(UnitWidth <= integral::WidthOf<UnitIntegral>);

    InnerProvider& inner_provider;
    bool digit_out_of_range : 1;
    bool is_end : 1;

    static constexpr size_t GetElemSize() { return sizeof(UnitIntegral); }

    constexpr bool IsEnd(this Provider_ const& provider) {
        return provider.is_end;
    }

    constexpr size_t Transfer(this Provider_& provider, void* dst,
                              size_t dst_elem_size, ptrdiff_t dst_elem_stride,
                              size_t cnt) {
        size_t transfer_elem_size{ comparison_utils::BasicMin(
            dst_elem_size, sizeof(UnitIntegral)) };

        size_t transfer_cnt{ 0 };

        UnitIntegral buffer;

        constexpr UnitIntegral digit_range_max{
            integral_math::PowerOf2Minus1<UnitIntegral>(UnitWidth)
        };

        constexpr UnitIntegral special_value{
            integral_math::PowerOf2<UnitIntegral>(UnitWidth - 1)
        };

        for (; !provider.is_end && transfer_cnt < cnt; ++transfer_cnt) {
            size_t transferred_cnt{ elem_stream::provider::Transfer(
                provider.inner_provider, &buffer, sizeof(buffer),
                sizeof(buffer), 1) };

            if (transferred_cnt == 0) {
                provider.is_end = true;
                break;
            }

            auto [cur_digit_out_of_range, canon_value]{
                integral_endec::CanonicalizeIntegral(buffer, digit_range_max)
            };

            provider.digit_out_of_range |= cur_digit_out_of_range;

            if (special_value <= canon_value) {
                canon_value -= special_value;
            } else {
                provider.is_end = true;
            }

            utils::MemCopy(dst, &canon_value, transfer_elem_size);

            dst = static_cast<unsigned char*>(dst) + dst_elem_stride;
        }

        return transfer_cnt;
    }
};

template <integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          elem_stream::acceptor::IsAcceptor InnerAcceptor>
struct Acceptor_ {
    ZETA_Core_StaticAssert(2 <= UnitWidth);
    ZETA_Core_StaticAssert(UnitWidth <= integral::WidthOf<UnitIntegral>);

    InnerAcceptor& inner_acceptor;
    UnitIntegral buffer;
    bool buffer_has_value;

    static constexpr size_t GetElemSize() { return sizeof(UnitIntegral); }

    constexpr bool IsEnd(this Acceptor_ const& acceptor) {
        return elem_stream::acceptor::IsEnd(acceptor.inner_acceptor);
    }

    constexpr size_t Transfer(this Acceptor_& acceptor, void const* src,
                              size_t src_elem_size, ptrdiff_t src_elem_stride,
                              size_t cnt) {
        size_t transfer_elem_size{ comparison_utils::BasicMin(
            src_elem_size, sizeof(UnitIntegral)) };

        constexpr UnitIntegral special_value{
            integral_math::PowerOf2<UnitIntegral>(UnitWidth - 1)
        };

        size_t transfer_cnt{ 0 };

        while (transfer_cnt < cnt) {
            size_t cur_transfer_cnt;

            if (acceptor.buffer_has_value) {
                acceptor.buffer += special_value;

                cur_transfer_cnt = elem_stream::acceptor::Transfer(
                    acceptor.inner_acceptor, &acceptor.buffer,
                    sizeof(acceptor.buffer), sizeof(acceptor.buffer), 1);

                acceptor.buffer_has_value = false;

                if (cur_transfer_cnt == 0) { return transfer_cnt; }
            } else {
                utils::MemCopy(&acceptor.buffer, src, transfer_elem_size);
                acceptor.buffer_has_value = true;

                src = static_cast<unsigned char const*>(src) + src_elem_stride;

                ++transfer_cnt;
            }
        }

        return transfer_cnt;
    }
};

}  // namespace vlq_utils::detail

template <integral::IsIntegral Integral, size_t UnitWidth>
    requires requires { requires 2 <= UnitWidth; }
constexpr size_t vlq_utils::EstimateEncodedUnitCnt(
    Integral value, meta::ValueWrapper<size_t, UnitWidth>) {
    size_t need_bit_cnt;

    if constexpr (integral::IsSignedIntegral<Integral>) {
        need_bit_cnt = value < 0
                           ? (value == integral::RangeMinOf<Integral>
                                  ? integral::WidthOf<Integral>
                                  : integral_math::CeilLog2(-value) + 1)
                           : (value == integral::RangeMaxOf<Integral>
                                  ? integral::WidthOf<Integral>
                                  : integral_math::CeilLog2(value + 1) + 1);
    } else {
        need_bit_cnt = value == integral::RangeMaxOf<Integral>
                           ? integral::WidthOf<Integral>
                           : integral_math::CeilLog2(value + 1);

        if (need_bit_cnt == 0) { need_bit_cnt = 1; }
    }

    return integral_math::CeilDiv(need_bit_cnt, UnitWidth - 1);
}

template <elem_stream::acceptor::IsAcceptor Acceptor,
          integral_endec::IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          integral::IsIntegral SrcIntegral>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
constexpr void vlq_utils::Encode(
    Acceptor&& acceptor, EndiannessLike endianness,
    meta::TypeWrapper<UnitIntegral> unit_integral,
    meta::ValueWrapper<size_t, UnitWidth> unit_width, SrcIntegral src_value) {
    detail::Acceptor_<UnitIntegral, UnitWidth, Acceptor> vlq_acceptor{
        .inner_acceptor = acceptor,
        .buffer = {},
        .buffer_has_value = false,
    };

    integral_endec::EncodeResult encode_result{ integral_endec::Encode(
        vlq_acceptor, endianness, unit_integral,
        meta::ValueWrapper<size_t, UnitWidth - 1>{},
        (EstimateEncodedUnitCnt)(src_value, unit_width), src_value) };

    if (vlq_acceptor.buffer_has_value) {
        elem_stream::acceptor::Transfer(
            vlq_acceptor.inner_acceptor, &vlq_acceptor.buffer,
            sizeof(vlq_acceptor.buffer), sizeof(vlq_acceptor.buffer), 1);

        vlq_acceptor.buffer_has_value = false;
    }

    ZETA_Core_DebugAssert(!encode_result.value_out_of_range);
}

template <elem_stream::provider::IsProvider Provider,
          integral_endec::IsEndiannessLike EndiannessLike,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          integral::IsIntegral DstIntegral>
    requires requires {
        requires 2 <= UnitWidth;
        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
constexpr vlq_utils::DecodeResult<DstIntegral> vlq_utils::Decode(
    Provider&& provider, EndiannessLike endianness,
    meta::TypeWrapper<UnitIntegral> unit_integral,
    meta::ValueWrapper<size_t, UnitWidth>, meta::TypeWrapper<DstIntegral>) {
    detail::Provider_<UnitIntegral, UnitWidth, Provider> vlq_provider{
        .inner_provider = provider,
        .digit_out_of_range = false,
        .is_end = false,
    };

    auto ret{ integral_endec::Decode(
        vlq_provider, endianness, unit_integral,
        meta::ValueWrapper<size_t, UnitWidth - 1>{},
        integral_endec::VariableOctetCntTag{},
        meta::TypeWrapper<DstIntegral>{}) };

    return {
        .digit_out_of_range =
            vlq_provider.digit_out_of_range || ret.digit_out_of_range,
        .value_out_of_range = ret.value_out_of_range,
        .value = ret.value,
    };
}

}  // namespace zeta::core
