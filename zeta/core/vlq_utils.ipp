#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/serde_utils.ipp>
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
    bool no_lossy;
    bool is_end;

    static constexpr size_t GetElemSize() { return sizeof(UnitIntegral); }

    constexpr bool IsEnd(this Provider_ const& provider) {
        return provider.is_end;
    }

    constexpr size_t Transfer(this Provider_& provider, void* dst,
                              size_t dst_elem_size, size_t dst_elem_stride,
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

            ZETA_Core_Debug_PrintVar(transferred_cnt);

            if (transferred_cnt == 0) {
                provider.is_end = true;
                break;
            }

            ZETA_Core_Debug_PrintVar(buffer);

            auto [canon_value, cur_no_lossy]{ serde_utils::CanonicalizeIntegral(
                buffer, digit_range_max) };

            ZETA_Core_Debug_PrintVar(canon_value);

            provider.no_lossy &= cur_no_lossy;

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
                              size_t src_elem_size, size_t src_elem_stride,
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

template <integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          elem_stream::provider::IsProvider InnerProvider>
struct elem_stream::provider::ProviderTraits<
    vlq_utils::detail::Provider_<UnitIntegral, UnitWidth, InnerProvider>>
    : public elem_stream::provider::DefaultProviderTraits<
          vlq_utils::detail::Provider_<UnitIntegral, UnitWidth,
                                       InnerProvider>> {};

template <integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          elem_stream::acceptor::IsAcceptor InnerAcceptor>
struct elem_stream::acceptor::AcceptorTraits<
    vlq_utils::detail::Acceptor_<UnitIntegral, UnitWidth, InnerAcceptor>>
    : public elem_stream::acceptor::DefaultAcceptorTraits<
          vlq_utils::detail::Acceptor_<UnitIntegral, UnitWidth,
                                       InnerAcceptor>> {};

template <integral::IsIntegral Integral, typename EndiannessLike,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          typename Acceptor>
    requires requires {
        requires 2 <= UnitWidth;

        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
bool vlq_utils::SerializeIntegral(
    Integral src_value, EndiannessLike endianness_like,
    meta::TypeWrapper<UnitIntegral> unit_integral,
    value_wrapper::StaticValueWrapper<size_t, UnitWidth>, bool allow_lossy,
    Acceptor&& acceptor, error::Error* dst_error) {
    detail::Acceptor_<UnitIntegral, UnitWidth, Acceptor> vlq_acceptor{
        .inner_acceptor = acceptor,
        .buffer = {},
        .buffer_has_value = false,
    };

    size_t src_value_digit_cnt;

    if (0 <= src_value) {
        src_value_digit_cnt = integral_math::CeilDiv(
            src_value == integral::RangeMaxOf<Integral>
                ? integral::WidthOf<Integral>
                : integral_math::CeilLog2(src_value + 1) + 1,
            UnitWidth - 1);
    } else {
        src_value_digit_cnt = integral_math::CeilDiv(
            src_value == integral::RangeMinOf<Integral>
                ? integral::WidthOf<Integral>
                : integral_math::CeilLog2(-src_value) + 1,
            UnitWidth - 1);
    }

    ZETA_Core_Debug_PrintVar(src_value_digit_cnt);

    bool no_lossy{ serde_utils::SerializeIntegral(
        src_value, endianness_like, unit_integral,
        value_wrapper::StaticValueWrapper<size_t, UnitWidth - 1>{},
        value_wrapper::DynamicValueWrapper<size_t>{ src_value_digit_cnt },
        allow_lossy, vlq_acceptor, dst_error) };

    if (vlq_acceptor.buffer_has_value) {
        elem_stream::acceptor::Transfer(
            vlq_acceptor.inner_acceptor, &vlq_acceptor.buffer,
            sizeof(vlq_acceptor.buffer), sizeof(vlq_acceptor.buffer), 1);

        vlq_acceptor.buffer_has_value = false;
    }

    return no_lossy;
}

template <integral::IsIntegral Integral, typename EndiannessLike,
          integral::IsUnsignedIntegral UnitIntegral, size_t UnitWidth,
          typename Provider>
    requires requires {
        requires 2 <= UnitWidth;

        requires UnitWidth <= integral::WidthOf<UnitIntegral>;
    }
bool vlq_utils::DeserializeIntegral(
    Integral& dst_value, EndiannessLike endianness_like,
    meta::TypeWrapper<UnitIntegral> unit_integral,
    value_wrapper::StaticValueWrapper<size_t, UnitWidth>, bool allow_lossy,
    Provider&& provider, error::Error* dst_error) {
    detail::Provider_<UnitIntegral, UnitWidth, Provider> vlq_provider{
        .inner_provider = provider,
        .no_lossy = true,
        .is_end = false,
    };

    bool no_lossy{ serde_utils::DeserializeIntegral(
        dst_value, endianness_like, unit_integral,
        value_wrapper::StaticValueWrapper<size_t, UnitWidth - 1>{},
        serde_utils::VariableOctetCntTag{}, allow_lossy, vlq_provider,
        dst_error) };

    no_lossy &= vlq_provider.no_lossy;

    return no_lossy;
}

}  // namespace zeta::core
