#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_endec.ipp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/lin_seq_elem_stream.ipp>
#include <zeta/core/object_state_notation.hpp>
#include <zeta/core/pair.ipp>
#include <zeta/core/unicode.hpp>
#include <zeta/core/vlq_utils.ipp>

namespace zeta::core {

constexpr bool object_state_notation::IsValidRegionAttrSize(
    unsigned region_attr_size) {
    return min_region_attr_size <= region_attr_size &&
           region_attr_size <= max_region_attr_size;
}

constexpr bool object_state_notation::IsValidIntegralSize(
    size_t integral_size) {
    return integral_size <= max_integral_size;
}

constexpr bool object_state_notation::Header::Check(this Header const& self) {
    if (!(self.magic[0] == 'O' && self.magic[1] == 'S' &&
          self.magic[2] == 'N' && self.magic[3] == '\0')) {
        return false;
    }

    if (!(IsValidRegionAttrSize)(self.region_attr_size)) { return false; }

    bool is_all_zero{ true };

    for (size_t i{ 0 }; i < sizeof(self.reserved) / sizeof(self.reserved[0]);
         ++i) {
        is_all_zero &= self.reserved[i] == 0;
    }

    if (!is_all_zero) { return false; }

    return true;
}

constexpr bool object_state_notation::Config::Check(this Config const& self) {
    if (!(IsValidRegionAttrSize)(self.region_attr_size)) { return false; }

    return true;
}

constexpr pair::Pair<bool, object_state_notation::Config>
object_state_notation::Config::FromHeader(Header const& header) {
    Config config{
        .version = header.version,
        .region_attr_size = header.region_attr_size,
    };

    bool is_valid{ true };

    is_valid &= header.Check();
    is_valid &= config.Check();

    return { is_valid, config };
}

constexpr pair::Pair<bool, object_state_notation::Header>
object_state_notation::Config::ToHeader(this Config const& self) {
    if (!self.Check()) { return { false, {} }; }

    Header header{
        .magic{ 'O', 'S', 'N', '\0' },
        .version = self.version,
        .region_attr_size = self.region_attr_size,
        .reserved{ 0 },
    };

    return { true, header };
}

constexpr bool object_state_notation::NodeTag::Check(this NodeTag const& self) {
    if (!(self.node_type == NodeType::Null ||
          self.node_type == NodeType::Integral ||
          self.node_type == NodeType::IntegralList ||
          self.node_type == NodeType::NodeList ||
          self.node_type == NodeType::Terminator)) {
        return false;
    }

    return true;
}

constexpr pair::Pair<bool, object_state_notation::NodeTag>
object_state_notation::NodeTag::FromEncodedValue(unsigned char encoded_value) {
    NodeTag node_tag{
        .node_type = static_cast<NodeType>(encoded_value & NodeTypeMask),
        .has_name = (encoded_value & HasName) != 0,
        .has_obj_type = (encoded_value & HasObjType) != 0,
        .has_region = (encoded_value & HasRegion) != 0,
    };

    return { node_tag.Check(), node_tag };
}

constexpr pair::Pair<bool, unsigned char>
object_state_notation::NodeTag::ToEncodedValue(this NodeTag const& self) {
    if (!self.Check()) { return { false, 0 }; }

    return { true, static_cast<unsigned char>(
                       meta::ToUnderlying(self.node_type) +     //
                       (self.has_name ? HasName : 0U) +         //
                       (self.has_obj_type ? HasObjType : 0U) +  //
                       (self.has_region ? HasRegion : 0U)) };
}

constexpr bool object_state_notation::IntegralDescriptor::Check(
    this IntegralDescriptor const& self) {
    if (!(IsValidIntegralSize)(self.size)) { return false; }

    return true;
}

constexpr object_state_notation::IntegralDescriptor
object_state_notation::IntegralDescriptor::FromEncodedValue(
    size_t encoded_value) {
    return {
        .is_signed = encoded_value % 2 == 1,
        .size = encoded_value / 2,
    };
}

constexpr pair::Pair<bool, size_t>
object_state_notation::IntegralDescriptor::ToEncodedValue(
    this IntegralDescriptor const& self) {
    if (!self.Check()) { return { false, 0 }; }

    return { true, self.size * 2 + self.is_signed };
}

namespace object_state_notation::detail {

template <elem_stream::acceptor::IsAcceptor Acceptor, typename DigitCntLike,
          typename SrcIntegral>
constexpr integral_endec::EncodeResult NormalEncodeIntegral_(
    Acceptor&& acceptor, DigitCntLike digit_cnt_like,
    SrcIntegral const& src_value) {
    return integral_endec::Encode(
        acceptor, meta::AutoValueWrapper<integral_endec::Endianness::Little>{},
        meta::TypeWrapper<unsigned char>{}, meta::ValueWrapper<size_t, 8>{},
        digit_cnt_like, src_value);
}

template <elem_stream::provider::IsProvider Provider, typename DigitCntLike,
          typename DstIntegral>
constexpr integral_endec::DecodeResult<DstIntegral> NormalDecodeIntegral_(
    Provider&& provider, DigitCntLike digit_cnt_like,
    meta::TypeWrapper<DstIntegral>) {
    return integral_endec::Decode(
        provider, meta::AutoValueWrapper<integral_endec::Endianness::Little>{},
        meta::TypeWrapper<unsigned char>{}, meta::ValueWrapper<size_t, 8>{},
        digit_cnt_like, meta::TypeWrapper<DstIntegral>{});
}

template <elem_stream::acceptor::IsAcceptor Acceptor, typename SrcIntegral>
constexpr void LEB128EncodeIntegral_(Acceptor&& acceptor,
                                     SrcIntegral const& src_value) {
    return vlq_utils::Encode(
        acceptor, meta::AutoValueWrapper<integral_endec::Endianness::Little>{},
        meta::TypeWrapper<unsigned char>{}, meta::ValueWrapper<size_t, 8>{},
        src_value);
}

template <elem_stream::provider::IsProvider Provider, typename DstIntegral>
constexpr vlq_utils::DecodeResult<DstIntegral> LEB128DecodeIntegral_(
    Provider&& provider, meta::TypeWrapper<DstIntegral>) {
    return vlq_utils::Decode(
        provider, meta::AutoValueWrapper<integral_endec::Endianness::Little>{},
        meta::TypeWrapper<unsigned char>{}, meta::ValueWrapper<size_t, 8>{},
        meta::TypeWrapper<DstIntegral>{});
}

}  // namespace object_state_notation::detail

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr bool object_state_notation::EncodeHeaderToOctets(
    Acceptor&& acceptor, object_state_notation::Header const& src_header) {
    ZETA_Core_DebugAssert(src_header.Check());

    elem_stream::acceptor::Transfer(acceptor, src_header.magic, 1, 1,
                                    sizeof(src_header.magic));

    elem_stream::acceptor::Transfer(acceptor, &src_header.version.major, 1, 1,
                                    1);
    elem_stream::acceptor::Transfer(acceptor, &src_header.version.minor, 1, 1,
                                    1);
    elem_stream::acceptor::Transfer(acceptor, &src_header.version.patch, 1, 1,
                                    2);

    src_header.region_attr_size =
        detail::NormalEncodeIntegral_(acceptor, meta::ValueWrapper<size_t, 1>{},
                                      meta::TypeWrapper<unsigned char>{})
            .value;

    elem_stream::acceptor::Transfer(acceptor, src_header.reserved, 1, 1,
                                    sizeof(src_header.reserved));

    return true;
}

template <elem_stream::provider::IsProvider Provider>
constexpr bool object_state_notation::DecodeHeaderFromOctets(
    Provider&& provider, object_state_notation::Header& dst_header) {
    elem_stream::provider::Transfer(provider, dst_header.magic, 1, 1,
                                    sizeof(dst_header.magic));

    elem_stream::provider::Transfer(provider, &dst_header.version.major, 1, 1,
                                    1);
    elem_stream::provider::Transfer(provider, &dst_header.version.minor, 1, 1,
                                    1);
    elem_stream::provider::Transfer(provider, &dst_header.version.patch, 1, 1,
                                    2);

    detail::NormalDecodeIntegral_(dst_header.region_attr_size,
                                  meta::ValueWrapper<size_t, 1>{}, provider);

    elem_stream::provider::Transfer(provider, dst_header.reserved, 1, 1,
                                    sizeof(dst_header.reserved));

    return true;
}

namespace object_state_notation::detail {

constexpr EncoderState FindNextState_(EncoderState cur_state,
                                      NodeTag const& node_tag) {
    switch (cur_state) {
    case EncoderState::ReceivingNodeTag:
    case EncoderState::ReceivingNodeTagOrFinish:
        if (node_tag.has_name) { return EncoderState::ReceivingNameString; }

        [[fallthrough]];

    case EncoderState::ReceivingNameString:
        if (node_tag.has_obj_type) {
            return EncoderState::ReceivingObjTypeString;
        }

        [[fallthrough]];

    case EncoderState::ReceivingObjTypeString:
        if (node_tag.has_region) { return EncoderState::ReceivingRegionAttr; }

        [[fallthrough]];

    case EncoderState::ReceivingRegionAttr: break;

    case EncoderState::ReceivingIntegralDescriptor:
    case EncoderState::ReceivingListElemCnt:
    case EncoderState::ReceivingIntegral:
    case EncoderState::ReceivingIntegralOrFinish:
    case EncoderState::ReceivingFinish:
    case EncoderState::Finished:
    case EncoderState::Corrupted: ZETA_Core_Unreachable();
    }

    // when after ReceivingRegionAttr

    switch (node_tag.node_type) {
    case NodeType::Null: return EncoderState::ReceivingFinish;

    case NodeType::Integral:
    case NodeType::IntegralList:
        return EncoderState::ReceivingIntegralDescriptor;

    case NodeType::NodeList: return EncoderState::ReceivingListElemCnt;

    case NodeType::Terminator: ZETA_Core_Unreachable();
    }
}

}  // namespace object_state_notation::detail

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr object_state_notation::Encoder<Acceptor>::Encoder(
    Config const& config, unsigned char* integral_chunk_buffer_data,
    unsigned short integral_chunk_buffer_max_octet_cnt, Acceptor& acceptor)
    : config{ config },
      state{ EncoderState::ReceivingNodeTag },
      depth{ 0 },
      integral_chunk_buffer{
          .data = integral_chunk_buffer_data,
          .elem_cnt = 0,
          .octet_cnt = 0,
          .max_octet_cnt = integral_chunk_buffer_max_octet_cnt,
      },
      acceptor{ acceptor } {}

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr utils::TryResult<meta::Monostate, meta::Monostate>
object_state_notation::Encoder<Acceptor>::SendNodeTag(
    this Encoder& self, NodeTag const& src_node_tag) {
    ZETA_Core_DebugAssert(self.state == EncoderState::ReceivingNodeTag ||
                          self.state == EncoderState::ReceivingNodeTagOrFinish);

    if (!src_node_tag.Check()) { return utils::TryResultReasonTag{}; }

    ZETA_Core_DebugAssert(self.depth < max_depth);

    ZETA_Core_DebugAssert(src_node_tag.node_type != NodeType::Terminator);

    if (0 < self.depth &&
        self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        --self.res_elem_cnts[self.depth - 1];
    }

    auto [is_valid, node_tag_encoded_value]{ src_node_tag.ToEncodedValue() };

    if (!is_valid) { return utils::TryResultReasonTag{}; }

    if (detail::NormalEncodeIntegral_(self.acceptor,
                                      meta::ValueWrapper<size_t, 1>{},
                                      node_tag_encoded_value)
            .value_out_of_range) {
        return utils::TryResultReasonTag{};
    }

    ++self.depth;

    self.node_tag_buffer = src_node_tag;

    switch (self.node_tag_buffer.node_type) {
    case NodeType::Null: self.res_elem_cnts[self.depth - 1] = 0; break;
    case NodeType::Integral: self.res_elem_cnts[self.depth - 1] = 1; break;
    case NodeType::IntegralList:
    case NodeType::NodeList:
    case NodeType::Terminator:
    }

    self.state = detail::FindNextState_(self.state, self.node_tag_buffer);

    return utils::TryResultValueTag{};
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
template <elem_stream::provider::IsProvider Provider>
constexpr utils::TryResult<meta::Monostate, meta::Monostate>
object_state_notation::Encoder<Acceptor>::SendStringOctet(this Encoder& self,
                                                          Provider&& provider,
                                                          size_t size) {
    ZETA_Core_DebugAssert(self.state == EncoderState::ReceivingNameString ||
                          self.state == EncoderState::ReceivingObjTypeString);

    constexpr size_t buffer_size{ 256 };
    unsigned char buffer[256];

    for (; buffer_size <= size; size -= buffer_size) {
        elem_stream::provider::Transfer(provider, buffer, 1, 1, buffer_size);
        elem_stream::acceptor::Transfer(self.acceptor, buffer, 1, 1,
                                        buffer_size);
    }

    if (0 < size) {
        elem_stream::provider::Transfer(provider, buffer, 1, 1, size);
        elem_stream::acceptor::Transfer(self.acceptor, buffer, 1, 1, size);
    }

    return utils::TryResultValueTag{};
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
template <integral::IsIntegral Integral>
constexpr utils::TryResult<meta::Monostate, meta::Monostate>
object_state_notation::Encoder<Acceptor>::SendRegionAttr(this Encoder& self,
                                                         Integral region_beg,
                                                         Integral region_size) {
    ZETA_Core_DebugAssert(self.state == EncoderState::ReceivingRegionAttr);

    detail::NormalEncodeIntegral_(
        self.acceptor, static_cast<size_t>(self.config.region_attr_size),
        region_beg);

    detail::NormalEncodeIntegral_(
        self.acceptor, static_cast<size_t>(self.config.region_attr_size),
        region_size);

    self.state = detail::FindNextState_(self.state, self.node_tag_buffer);

    return utils::TryResultValueTag{};
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr utils::TryResult<meta::Monostate, meta::Monostate>
object_state_notation::Encoder<Acceptor>::SendIntegralDescriptor(
    this Encoder& self, IntegralDescriptor const& src_integral_descriptor) {
    ZETA_Core_DebugAssert(self.state ==
                          EncoderState::ReceivingIntegralDescriptor);

    auto [is_valid, integral_descriptor_encoded_value]{
        src_integral_descriptor.ToEncodedValue()
    };

    if (!is_valid) { return utils::TryResultReasonTag{}; }

    detail::LEB128EncodeIntegral_(integral_descriptor_encoded_value,
                                  self.acceptor);

    self.integral_descriptor_buffer = src_integral_descriptor;

    switch (self.node_tag_buffer.node_type) {
    case NodeType::Integral:
        self.state = EncoderState::ReceivingIntegral;
        break;

    case NodeType::IntegralList:
        self.state = EncoderState::ReceivingListElemCnt;
        break;

    case NodeType::Null:
    case NodeType::NodeList:
    case NodeType::Terminator: ZETA_Core_Unreachable();
    }

    return utils::TryResultValueTag{};
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr utils::TryResult<meta::Monostate, meta::Monostate>
object_state_notation::Encoder<Acceptor>::SendListElemCnt(
    this Encoder& self, size_t list_elem_cnt) {
    ZETA_Core_DebugAssert(self.state == EncoderState::ReceivingListElemCnt);

    detail::LEB128EncodeIntegral_(list_elem_cnt + static_cast<size_t>(1),
                                  self.acceptor);

    self.res_elem_cnts[self.depth - 1] = list_elem_cnt;

    if (list_elem_cnt == 0) {
        self.state = EncoderState::ReceivingFinish;
        return utils::TryResultValueTag{};
    }

    switch (self.node_tag_buffer.node_type) {
    case NodeType::IntegralList:
        self.state = list_elem_cnt == static_cast<size_t>(-1)
                         ? EncoderState::ReceivingIntegralOrFinish
                         : EncoderState::ReceivingIntegral;
        break;

    case NodeType::NodeList:
        self.state = list_elem_cnt == static_cast<size_t>(-1)
                         ? EncoderState::ReceivingNodeTagOrFinish
                         : EncoderState::ReceivingNodeTag;
        break;

    case NodeType::Null:
    case NodeType::Integral:
    case NodeType::Terminator: ZETA_Core_Unreachable();
    }

    return utils::TryResultValueTag{};
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
template <integral::IsIntegral Integral>
constexpr utils::TryResult<meta::Monostate, meta::Monostate>
object_state_notation::Encoder<Acceptor>::SendIntegral(this Encoder& self,
                                                       Integral src_integral) {
    ZETA_Core_DebugAssert(self.state == EncoderState::ReceivingIntegral ||
                          self.state ==
                              EncoderState::ReceivingIntegralOrFinish);

    if (self.integral_descriptor_buffer.is_signed !=
        integral::IsSignedIntegral<Integral>) {
        return utils::TryResultReasonTag{};
    }

    size_t integral_size{ self.integral_descriptor_buffer.size };

    if (self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        if (integral_size == 0) {
            detail::LEB128EncodeIntegral_(self.acceptor, src_integral);
        } else {
            detail::NormalEncodeIntegral_(self.acceptor,
                                          static_cast<size_t>(integral_size),
                                          src_integral);
        }

        if (--self.res_elem_cnts[self.depth - 1] == 0) {
            self.state = EncoderState::ReceivingFinish;
        }

        return utils::TryResultValueTag{};
    }

    size_t est_integral_size{
        integral_size == 0 ? vlq_utils::EstimateEncodedUnitCnt(
                                 src_integral, meta::ValueWrapper<size_t, 8>{})
                           : integral_size
    };

    if (self.integral_chunk_buffer.elem_cnt + 1 == 255 ||
        self.integral_chunk_buffer.max_octet_cnt <
            self.integral_chunk_buffer.octet_cnt + est_integral_size) {
        detail::NormalEncodeIntegral_(self.acceptor,
                                      meta::ValueWrapper<size_t, 1>{},
                                      self.integral_chunk_buffer.elem_cnt + 1);

        if (0 < self.integral_chunk_buffer.elem_cnt) {
            elem_stream::acceptor::Transfer(
                self.acceptor, self.integral_chunk_buffer.data, 1, 1,
                self.integral_chunk_buffer.octet_cnt);

            self.integral_chunk_buffer.elem_cnt = 0;
            self.integral_chunk_buffer.octet_cnt = 0;
        }

        if (integral_size == 0) {
            detail::LEB128EncodeIntegral_(self.acceptor, src_integral);
        } else {
            detail::NormalEncodeIntegral_(self.acceptor,
                                          static_cast<size_t>(integral_size),
                                          src_integral);
        }

        return utils::TryResultValueTag{};
    }

    lin_seq_elem_stream::Acceptor integral_chunk_elem_stream_acceptor{
        .data = self.integral_chunk_buffer.data +
                self.integral_chunk_buffer.octet_cnt,
        .elem_size = 1,
        .elem_stride = 1,
        .elem_cnt = est_integral_size,
    };

    if (integral_size == 0) {
        detail::LEB128EncodeIntegral_(integral_chunk_elem_stream_acceptor,
                                      src_integral);
    } else {
        detail::NormalEncodeIntegral_(integral_chunk_elem_stream_acceptor,
                                      static_cast<size_t>(integral_size),
                                      src_integral);
    }

    ++self.integral_chunk_buffer.elem_cnt;
    self.integral_chunk_buffer.octet_cnt += est_integral_size;

    return utils::TryResultValueTag{};
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr utils::TryResult<meta::Monostate, meta::Monostate>
object_state_notation::Encoder<Acceptor>::SendFinish(this Encoder& self) {
    ZETA_Core_DebugAssert(
        self.state == EncoderState::ReceivingNodeTagOrFinish ||
        self.state == EncoderState::ReceivingNameString ||
        self.state == EncoderState::ReceivingObjTypeString ||
        self.state == EncoderState::ReceivingIntegralOrFinish ||
        self.state == EncoderState::ReceivingFinish);

    if (self.state == EncoderState::ReceivingNameString ||
        self.state == EncoderState::ReceivingObjTypeString) {
        constexpr unsigned char zero_octet{ 0 };

        elem_stream::acceptor::Transfer(self.acceptor, &zero_octet, 1, 1, 1);

        self.state = detail::FindNextState_(self.state, self.node_tag_buffer);

        return utils::TryResultValueTag{};
    }

    switch (self.node_tag_buffer.node_type) {
    case NodeType::Null: break;

    case NodeType::Integral: break;

    case NodeType::IntegralList:
        if (self.res_elem_cnts[self.depth - 1] == static_cast<size_t>(-1)) {
            if (0 < self.integral_chunk_buffer.elem_cnt) {
                detail::NormalEncodeIntegral_(
                    self.acceptor, meta::ValueWrapper<size_t, 1>{},
                    self.integral_chunk_buffer.elem_cnt);

                elem_stream::acceptor::Transfer(
                    self.acceptor, self.integral_chunk_buffer.data, 1, 1,
                    self.integral_chunk_buffer.octet_cnt);

                self.integral_chunk_buffer.elem_cnt = 0;
                self.integral_chunk_buffer.octet_cnt = 0;
            }

            detail::NormalEncodeIntegral_(self.acceptor,
                                          meta::ValueWrapper<size_t, 1>{}, 0);
        }

        break;

    case NodeType::NodeList:
        if (self.res_elem_cnts[self.depth - 1] == static_cast<size_t>(-1)) {
            unsigned char node_tag{ NodeTag{
                .node_type = NodeType::Terminator,
                .has_name = false,
                .has_obj_type = false,
                .has_region = false,
            }
                                        .ToEncodedValue()
                                        .second };

            elem_stream::acceptor::Transfer(self.acceptor, &node_tag, 1, 1, 1);
        }

        break;

    case NodeType::Terminator: ZETA_Core_Unreachable();
    }

    --self.depth;

    if (self.depth == 0) {
        self.state = EncoderState::Finished;
        return utils::TryResultValueTag{};
    }

    self.node_tag_buffer = NodeTag{
        .node_type = NodeType::NodeList,
        .has_name = false,
        .has_obj_type = false,
        .has_region = false,
    };

    switch (self.res_elem_cnts[self.depth - 1]) {
    case 0: self.state = EncoderState::ReceivingFinish; break;

    case static_cast<size_t>(-1):
        self.state = EncoderState::ReceivingNodeTagOrFinish;
        break;

    default: self.state = EncoderState::ReceivingNodeTag; break;
    }

    return utils::TryResultValueTag{};
}

namespace object_state_notation::detail {

constexpr DecoderState FindNextState_(DecoderState cur_state,
                                      NodeTag const& node_tag) {
    switch (cur_state) {
    case DecoderState::SendingNodeTag:
        if (node_tag.has_name) { return DecoderState::SendingNameString; }

        [[fallthrough]];

    case DecoderState::SendingNameString:
        if (node_tag.has_obj_type) {
            return DecoderState::SendingObjTypeString;
        }

        [[fallthrough]];

    case DecoderState::SendingObjTypeString:
        if (node_tag.has_region) { return DecoderState::SendingRegionAttr; }

        [[fallthrough]];

    case DecoderState::SendingRegionAttr: break;

    case DecoderState::SendingIntegralDescriptor:
    case DecoderState::SendingListElemCnt:
    case DecoderState::SendingIntegral:
    case DecoderState::SendingFinish:
    case DecoderState::Finished:
    case DecoderState::Corrupted: ZETA_Core_Unreachable();
    }

    // when after SendingRegionAttr

    switch (node_tag.node_type) {
    case NodeType::Null: return DecoderState::SendingFinish;

    case NodeType::Integral:
    case NodeType::IntegralList: return DecoderState::SendingIntegralDescriptor;

    case NodeType::NodeList: return DecoderState::SendingListElemCnt;

    case NodeType::Terminator: ZETA_Core_Unreachable();
    }
}

}  // namespace object_state_notation::detail

template <elem_stream::provider::IsProvider Provider>
constexpr object_state_notation::Decoder<Provider>::Decoder(
    Config const& config, Provider& provider)
    : config{ config },
      depth{ 0 },
      has_buffer_node_tag{ false },
      provider{ provider } {
    if (elem_stream::provider::IsEnd(this->provider)) {
        state = DecoderState::Finished;
    } else {
        state = DecoderState::SendingNodeTag;
    }
}

template <elem_stream::provider::IsProvider Provider>
constexpr utils::TryResult<object_state_notation::NodeTag, meta::Monostate>
object_state_notation::Decoder<Provider>::ReceiveNodeTag(this Decoder& self) {
    ZETA_Core_DebugAssert(self.state == DecoderState::SendingNodeTag);

    ZETA_Core_DebugAssert(self.depth < max_depth);

    if (self.has_buffer_node_tag) {
        self.has_buffer_node_tag = false;
    } else {
        unsigned char node_tag_encoded_value;

        if (!detail::NormalDecodeIntegral_(node_tag_encoded_value,
                                           meta::ValueWrapper<size_t, 1>{},
                                           self.provider)) {
            self.state = DecoderState::Corrupted;
            return utils::TryResultReasonTag{};
        }

        auto [is_valid,
              node_tag]{ NodeTag::FromEncodedValue(node_tag_encoded_value) };

        if (!is_valid) {
            self.state = DecoderState::Corrupted;
            return utils::TryResultReasonTag{};
        }

        self.buffer_node_tag = node_tag;
    }

    if (0 < self.depth &&
        self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        --self.res_elem_cnts[self.depth - 1];
    }

    ++self.depth;

    if (!self.buffer_node_tag.Check() ||
        self.buffer_node_tag.node_type == NodeType::Terminator) {
        self.state = DecoderState::Corrupted;
        return utils::TryResultReasonTag{};
    }

    switch (self.buffer_node_tag.node_type) {
    case NodeType::Null: self.res_elem_cnts[self.depth - 1] = 0; break;
    case NodeType::Integral: self.res_elem_cnts[self.depth - 1] = 1; break;
    case NodeType::IntegralList:
    case NodeType::NodeList:
    case NodeType::Terminator: break;
    }

    self.state = detail::FindNextState_(self.state, self.buffer_node_tag);

    return { utils::TryResultValueTag{}, self.buffer_node_tag };
}

template <elem_stream::provider::IsProvider Provider>
template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr utils::TryResult<unicode::unichar_t, meta::Monostate>
object_state_notation::Decoder<Provider>::ReceiveStringChar(
    this Decoder& self) {
    ZETA_Core_DebugAssert(self.state == DecoderState::SendingObjTypeString ||
                          self.state == DecoderState::SendingNameString);

    //

    constexpr unsigned buffer_size{ 256 };

    unsigned char buffer[buffer_size];

    size_t acc_str_size{ 0 };

    bool string_is_terminated{ false };

    while (!string_is_terminated && 0 < max_str_size) {
        size_t cur_size{ comparison_utils::BasicMin(buffer_size,
                                                    max_str_size) };

        unsigned buffer_elem_cnt{ 0 };

        for (; buffer_elem_cnt < cur_size; ++buffer_elem_cnt) {
            elem_stream::provider::Transfer(self.provider,
                                            buffer + buffer_elem_cnt, 1, 1, 1);

            if (buffer[buffer_elem_cnt] == 0) {
                string_is_terminated = true;
                break;
            }
        }

        elem_stream::acceptor::Transfer(acceptor, buffer, 1, 1,
                                        buffer_elem_cnt);

        max_str_size -= buffer_elem_cnt;
        acc_str_size += buffer_elem_cnt;
    }

    if (string_is_terminated) {
        self.state = detail::FindNextState_(self.state, self.buffer_node_tag);
    }

    return { utils::TryResultValueTag{}, acc_str_size };
}

template <elem_stream::provider::IsProvider Provider>
template <integral::IsIntegral Integral>
constexpr utils::TryResult<pair::Pair<Integral, Integral>, meta::Monostate>
object_state_notation::Decoder<Provider>::ReceiveRegionAttr(
    this Decoder& self) {
    ZETA_Core_DebugAssert(self.state == DecoderState::SendingRegionAttr);

    Integral region_beg;
    Integral region_size;

    detail::NormalDecodeIntegral_(
        region_beg, static_cast<size_t>(self.config.region_attr_size),
        self.provider);

    detail::NormalDecodeIntegral_(
        region_size, static_cast<size_t>(self.config.region_attr_size),
        self.provider);

    self.state = detail::FindNextState_(self.state, self.buffer_node_tag);

    return { utils::TryResultValueTag{},
             pair::Pair<Integral, Integral>{ .first = region_beg,
                                             .second = region_size } };
}

template <elem_stream::provider::IsProvider Provider>
constexpr utils::TryResult<object_state_notation::IntegralDescriptor,
                           meta::Monostate>
object_state_notation::Decoder<Provider>::ReceiveIntegralDescriptor(
    this Decoder& self) {
    ZETA_Core_DebugAssert(self.state ==
                          DecoderState::SendingIntegralDescriptor);

    size_t integral_descriptor_encoded_value{ ({
        vlq_utils::DecodeResult<size_t> tmp{ detail::LEB128DecodeIntegral_(
            self.provider, integral_descriptor_encoded_value) };

        if (tmp.digit_out_of_range || tmp.value_out_of_range) {
            self.state = DecoderState::Corrupted;
            return utils::TryResultReasonTag{};
        }

        tmp.value;
    }) };

    self.integral_descriptor_buffer =
        IntegralDescriptor::FromEncodedValue(integral_descriptor_encoded_value);

    switch (self.buffer_node_tag.node_type) {
    case NodeType::Integral: self.state = DecoderState::SendingIntegral; break;

    case NodeType::IntegralList:
        self.state = DecoderState::SendingListElemCnt;
        break;

    case NodeType::Null:
    case NodeType::NodeList:
    case NodeType::Terminator: ZETA_Core_Unreachable();
    }

    return { utils::TryResultValueTag{}, self.integral_descriptor_buffer };
}

template <elem_stream::provider::IsProvider Provider>
constexpr utils::TryResult<size_t, meta::Monostate>
object_state_notation::Decoder<Provider>::ReceiveListElemCnt(
    this Decoder& self) {
    ZETA_Core_DebugAssert(self.state == DecoderState::SendingListElemCnt);

    size_t list_elem_cnt_encoded_value{ ({
        vlq_utils::DecodeResult<size_t> tmp{ detail::LEB128DecodeIntegral_(
            self.provider, list_elem_cnt_encoded_value) };

        if (tmp.digit_out_of_range || tmp.value_out_of_range) {
            self.state = DecoderState::Corrupted;
            return utils::TryResultReasonTag{};
        }

        tmp.value;
    }) };

    size_t list_elem_cnt{ list_elem_cnt_encoded_value - 1 };

    self.res_elem_cnts[self.depth - 1] = list_elem_cnt;

    if (self.buffer_node_tag.node_type == NodeType::IntegralList &&
        list_elem_cnt == static_cast<size_t>(-1)) {
        if (detail::NormalDecodeIntegral_(self.integral_chunk_res_elem_cnt,
                                          meta::ValueWrapper<size_t, 1>{},
                                          self.provider)) {
            self.state = self.integral_chunk_res_elem_cnt == 0
                             ? DecoderState::SendingFinish
                             : DecoderState::SendingIntegral;
        } else {
            self.state = DecoderState::Corrupted;
        }

        return { utils::TryResultValueTag{}, list_elem_cnt };
    }

    if (list_elem_cnt == 0) {
        self.state = DecoderState::SendingFinish;
        return { utils::TryResultValueTag{}, list_elem_cnt };
    }

    if (self.buffer_node_tag.node_type == NodeType::IntegralList) {
        self.state = DecoderState::SendingIntegral;
        return { utils::TryResultValueTag{}, list_elem_cnt };
    }

    unsigned char node_tag_encoded_value;

    if (!detail::NormalDecodeIntegral_(node_tag_encoded_value,
                                       meta::ValueWrapper<size_t, 1>{},
                                       self.provider)) {
        self.state = DecoderState::Corrupted;
        return { utils::TryResultValueTag{}, list_elem_cnt };
    }

    auto [is_valid,
          node_tag]{ NodeTag::FromEncodedValue(node_tag_encoded_value) };

    if (!is_valid) {
        self.state = DecoderState::Corrupted;
        return { utils::TryResultValueTag{}, list_elem_cnt };
    }

    if (node_tag.node_type == NodeType::Terminator) {
        self.state = DecoderState::SendingFinish;
    } else {
        self.has_buffer_node_tag = true;
        self.buffer_node_tag = node_tag;

        self.state = DecoderState::SendingNodeTag;
    }

    return { utils::TryResultValueTag{}, list_elem_cnt };
}

template <elem_stream::provider::IsProvider Provider>
template <integral::IsIntegral Integral>
constexpr utils::TryResult<Integral, meta::Monostate>
object_state_notation::Decoder<Provider>::ReceiveIntegral(this Decoder& self) {
    ZETA_Core_DebugAssert(self.state == DecoderState::SendingIntegral);

    if (self.integral_descriptor_buffer.is_signed !=
        integral::IsSignedIntegral<Integral>) {
        return utils::TryResultReasonTag{};
    }

    Integral integral{ self.integral_descriptor_buffer.size == 0
                           ? detail::LEB128DecodeIntegral_(
                                 self.provider, meta::TypeWrapper<Integral>{})
                                 .value
                           : detail::NormalDecodeIntegral_(
                                 self.provider,
                                 static_cast<size_t>(
                                     self.integral_descriptor_buffer.size),
                                 meta::TypeWrapper<Integral>{})
                                 .value };

    if (self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        --self.res_elem_cnts[self.depth - 1];

        if (self.res_elem_cnts[self.depth - 1] == 0) {
            self.state = DecoderState::SendingFinish;
        }

        return { utils::TryResultValueTag{}, integral };
    }

    if (0 < --self.integral_chunk_res_elem_cnt) {
        return { utils::TryResultValueTag{}, integral };
    }

    self.integral_chunk_res_elem_cnt = ({
        integral_endec::DecodeResult<size_t> tmp{ detail::LEB128DecodeIntegral_(
            self.provider, meta::ValueWrapper<size_t, 1>{}) };

        if (tmp.digit_out_of_range || tmp.value_out_of_range) {
            self.state = DecoderState::Corrupted;
            return utils::TryResultReasonTag{};
        }

        tmp.value;
    });

    if (self.integral_chunk_res_elem_cnt == 0) {
        self.state = DecoderState::SendingFinish;
    }

    return { utils::TryResultValueTag{}, integral };
}

template <elem_stream::provider::IsProvider Provider>
constexpr utils::TryResult<meta::Monostate, meta::Monostate>
object_state_notation::Decoder<Provider>::ReceiveFinish(this Decoder& self) {
    ZETA_Core_DebugAssert(self.state == DecoderState::SendingFinish);

    --self.depth;

    if (self.depth == 0) {
        self.state = DecoderState::Finished;
        return utils::TryResultValueTag{};
    }

    if (self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        if (0 < self.res_elem_cnts[self.depth - 1]) {
            self.state = DecoderState::SendingNodeTag;
        }

        return utils::TryResultValueTag{};
    }

    unsigned char node_tag_encoded_value{ ({
        integral_endec::DecodeResult<unsigned char> tmp{
            detail::LEB128DecodeIntegral_(self.provider,
                                          meta::ValueWrapper<size_t, 1>{},
                                          meta::TypeWrapper<unsigned char>{})
        };

        if (tmp.digit_out_of_range || tmp.value_out_of_range) {
            self.state = DecoderState::Corrupted;
            return utils::TryResultReasonTag{};
        }

        tmp.value;
    }) };

    auto [is_valid,
          node_tag]{ NodeTag::FromEncodedValue(node_tag_encoded_value) };

    if (!is_valid) {
        self.state = DecoderState::Corrupted;
        return utils::TryResultValueTag{};
    }

    if (node_tag.node_type != NodeType::Terminator) {
        self.has_buffer_node_tag = true;
        self.buffer_node_tag = node_tag;

        self.state = DecoderState::SendingNodeTag;
    }

    return utils::TryResultValueTag{};
}

}  // namespace zeta::core
