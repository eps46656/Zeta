#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/object_state_notation.hpp>
#include <zeta/core/serde_utils.ipp>
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
    if (!(self.node_type == NodeTypeEnum::Null::value ||
          self.node_type == NodeTypeEnum::Integral::value ||
          self.node_type == NodeTypeEnum::IntegralList::value ||
          self.node_type == NodeTypeEnum::NodeList::value ||
          self.node_type == NodeTypeEnum::Terminator::value)) {
        return false;
    }

    return true;
}

constexpr pair::Pair<bool, object_state_notation::NodeTag>
object_state_notation::NodeTag::FromEncodedValue(unsigned char encoded_value) {
    NodeTag node_tag{
        .node_type = static_cast<unsigned char>(encoded_value & NodeTypeMask),
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
                       self.node_type +                         //
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

template <typename Integral, typename DigitCntLike,
          elem_stream::acceptor::IsAcceptor Acceptor>
bool NormalSerializeIntegral_(Integral src_value, DigitCntLike digit_cnt_like,
                              Acceptor&& acceptor) {
    return serde_utils::SerializeIntegral(
        src_value, serde_utils::EndiannessEnum::Little{},
        meta::TypeWrapper<unsigned char>{},
        value_wrapper::StaticValueWrapper<size_t, 8>{}, digit_cnt_like, false,
        acceptor, nullptr);
}

template <typename Integral, typename DigitCntLike,
          elem_stream::provider::IsProvider Provider>
bool NormalDeserializeIntegral_(Integral& dst_value,
                                DigitCntLike digit_cnt_like,
                                Provider&& provider) {
    return serde_utils::DeserializeIntegral(
        dst_value, serde_utils::EndiannessEnum::Little{},
        meta::TypeWrapper<unsigned char>{},
        value_wrapper::StaticValueWrapper<size_t, 8>{}, digit_cnt_like, false,
        provider, nullptr);
}

template <typename Integral, elem_stream::acceptor::IsAcceptor Acceptor>
bool LEB128SerializeIntegral_(Integral src_value, Acceptor&& acceptor) {
    return vlq_utils::SerializeIntegral(
        src_value, serde_utils::EndiannessEnum::Little{},
        meta::TypeWrapper<unsigned char>{},
        value_wrapper::StaticValueWrapper<size_t, 8>{}, false, acceptor,
        nullptr);
}

template <typename Integral, elem_stream::provider::IsProvider Provider>
bool LEB128DeserializeIntegral_(Integral& dst_value, Provider&& provider) {
    return vlq_utils::DeserializeIntegral(
        dst_value, serde_utils::EndiannessEnum::Little{},
        meta::TypeWrapper<unsigned char>{},
        value_wrapper::StaticValueWrapper<size_t, 8>{}, false, provider,
        nullptr);
}

}  // namespace object_state_notation::detail

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool object_state_notation::SerializeHeaderToOctets(
    Acceptor&& acceptor, object_state_notation::Header const& src_header) {
    if (!src_header.Check()) { return false; }

    elem_stream::acceptor::Transfer(acceptor, src_header.magic, 1, 1,
                                    sizeof(src_header.magic));

    elem_stream::acceptor::Transfer(acceptor, &src_header.version.major, 1, 1,
                                    1);
    elem_stream::acceptor::Transfer(acceptor, &src_header.version.minor, 1, 1,
                                    1);
    elem_stream::acceptor::Transfer(acceptor, &src_header.version.patch, 1, 1,
                                    2);

    detail::NormalSerializeIntegral_(
        src_header.region_attr_size,
        value_wrapper::StaticValueWrapper<size_t, 1>{}, acceptor);

    elem_stream::acceptor::Transfer(acceptor, src_header.reserved, 1, 1,
                                    sizeof(src_header.reserved));

    return true;
}

template <elem_stream::provider::IsProvider Provider>
bool object_state_notation::DeserializeHeaderFromOctets(
    Provider&& provider, object_state_notation::Header& dst_header) {
    elem_stream::provider::Transfer(provider, dst_header.magic, 1, 1,
                                    sizeof(dst_header.magic));

    elem_stream::provider::Transfer(provider, &dst_header.version.major, 1, 1,
                                    1);
    elem_stream::provider::Transfer(provider, &dst_header.version.minor, 1, 1,
                                    1);
    elem_stream::provider::Transfer(provider, &dst_header.version.patch, 1, 1,
                                    2);

    detail::NormalDeserializeIntegral_(
        dst_header.region_attr_size,
        value_wrapper::StaticValueWrapper<size_t, 1>{}, provider);

    elem_stream::provider::Transfer(provider, dst_header.reserved, 1, 1,
                                    sizeof(dst_header.reserved));

    return true;
}

constexpr object_state_notation::state_machine::SerializationStateMachineBase::
    StateEnum::Value
    object_state_notation::state_machine::SerializationStateMachineBase::
        SerializationStateMachineBase::FindNextState_(
            StateEnum::Value cur_state, NodeTag const& node_tag) {
    switch (cur_state) {
    case StateEnum::ReceivingNodeTagOrTermination::value:
    case StateEnum::ReceivingNodeTag::value:
        if (node_tag.has_name) { return StateEnum::ReceivingNameString::value; }

        [[fallthrough]];

    case StateEnum::ReceivingNameString::value:
        if (node_tag.has_obj_type) {
            return StateEnum::ReceivingObjTypeString::value;
        }

        [[fallthrough]];

    case StateEnum::ReceivingObjTypeString::value:
        if (node_tag.has_region) {
            return StateEnum::ReceivingRegionAttr::value;
        }

        [[fallthrough]];

    case StateEnum::ReceivingRegionAttr::value:
        switch (node_tag.node_type) {
        case NodeTypeEnum::Null::value:
            return StateEnum::ReceivingTermination::value;

        case NodeTypeEnum::Integral::value:
        case NodeTypeEnum::IntegralList::value:
            return StateEnum::ReceivingIntegralDescriptor::value;

        case NodeTypeEnum::NodeList::value:
            return StateEnum::ReceivingListElemCnt::value;

        default: ZETA_Core_Unreachable();
        }

        break;

    default: ZETA_Core_Unreachable();
    }
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr object_state_notation::state_machine::
    SerializeToOctetsStateMachine<Acceptor>::SerializeToOctetsStateMachine(
        Config const& config, unsigned char* integral_chunk_buffer_data,
        unsigned short integral_chunk_buffer_max_octet_cnt, Acceptor& acceptor)
    : config{ config },
      state{ StateEnum::ReceivingNodeTag::value },
      depth{ 0 },
      integral_chunk_buffer{
          .data = integral_chunk_buffer_data,
          .elem_cnt = 0,
          .octet_cnt = 0,
          .max_octet_cnt = integral_chunk_buffer_max_octet_cnt,
      },
      acceptor{ acceptor } {}

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeNodeTag(this SerializeToOctetsStateMachine& self,
                                NodeTag const& src_node_tag) {
    if (!(self.state == StateEnum::ReceivingNodeTag::value ||
          self.state == StateEnum::ReceivingNodeTagOrTermination::value)) {
        return false;
    }

    if (!src_node_tag.Check()) { return false; }

    if (!(self.depth < max_depth)) { return false; }

    if (!(src_node_tag.node_type != NodeTypeEnum::Terminator::value)) {
        return false;
    }

    if (0 < self.depth) {
        if (self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
            --self.res_elem_cnts[self.depth - 1];
        }
    }

    auto [is_valid, node_tag_encoded_value]{ src_node_tag.ToEncodedValue() };

    if (!is_valid) { return false; }

    if (!detail::NormalSerializeIntegral_(
            node_tag_encoded_value,
            value_wrapper::StaticValueWrapper<size_t, 1>{}, self.acceptor)) {
        return false;
    }

    ++self.depth;

    self.node_tag_buffer = src_node_tag;

    switch (self.node_tag_buffer.node_type) {
    case NodeTypeEnum::Null::value:
        self.res_elem_cnts[self.depth - 1] = 0;
        break;

    case NodeTypeEnum::Integral::value:
        self.res_elem_cnts[self.depth - 1] = 1;
        break;
    }

    self.state = (FindNextState_)(self.state, self.node_tag_buffer);

    return true;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
template <elem_stream::provider::IsProvider Provider>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeString(this SerializeToOctetsStateMachine& self,
                               Provider&& provider, size_t size) {
    if (!(self.state == StateEnum::ReceivingNameString::value ||
          self.state == StateEnum::ReceivingObjTypeString::value)) {
        return false;
    }

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

    return true;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::TerminateSerializeString(this SerializeToOctetsStateMachine&
                                            self) {
    if (!(self.state == StateEnum::ReceivingNameString::value ||
          self.state == StateEnum::ReceivingObjTypeString::value)) {
        return false;
    }

    unsigned char zero_octet{ 0 };

    elem_stream::acceptor::Transfer(self.acceptor, &zero_octet, 1, 1, 1);

    self.state = (FindNextState_)(self.state, self.node_tag_buffer);

    return true;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
template <integral::IsIntegral Integral>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeRegionAttr(this SerializeToOctetsStateMachine& self,
                                   Integral region_beg, Integral region_size) {
    if (!(self.state == StateEnum::ReceivingRegionAttr::value)) {
        return false;
    }

    detail::NormalSerializeIntegral_(region_beg,
                                     value_wrapper::DynamicValueWrapper<size_t>{
                                         self.config.region_attr_size },
                                     self.acceptor);

    detail::NormalSerializeIntegral_(region_size,
                                     value_wrapper::DynamicValueWrapper<size_t>{
                                         self.config.region_attr_size },
                                     self.acceptor);

    self.state = (FindNextState_)(self.state, self.node_tag_buffer);

    return true;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool object_state_notation::state_machine::
    SerializeToOctetsStateMachine<Acceptor>::SerializeIntegralDescriptor(
        this SerializeToOctetsStateMachine& self,
        IntegralDescriptor const& src_integral_descriptor) {
    if (!(self.state == StateEnum::ReceivingIntegralDescriptor::value)) {
        return false;
    }

    auto [is_valid, integral_descriptor_encoded_value]{
        src_integral_descriptor.ToEncodedValue()
    };

    if (!is_valid) { return false; }

    if (!detail::LEB128SerializeIntegral_(integral_descriptor_encoded_value,
                                          self.acceptor)) {
        return false;
    }

    self.integral_descriptor_buffer = src_integral_descriptor;

    switch (self.node_tag_buffer.node_type) {
    case NodeTypeEnum::Integral::value:
        self.state = StateEnum::ReceivingIntegral::value;
        break;

    case NodeTypeEnum::IntegralList::value:
        self.state = StateEnum::ReceivingListElemCnt::value;
        break;

    default: ZETA_Core_Unreachable();
    }

    return true;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeListElemCnt(this SerializeToOctetsStateMachine& self,
                                    size_t list_elem_cnt) {
    if (!(self.state == StateEnum::ReceivingListElemCnt::value)) {
        return false;
    }

    ZETA_Core_Debug_PrintVar(list_elem_cnt);

    detail::LEB128SerializeIntegral_(list_elem_cnt + static_cast<size_t>(1),
                                     self.acceptor);

    self.res_elem_cnts[self.depth - 1] = list_elem_cnt;

    if (list_elem_cnt == 0) {
        self.state = StateEnum::ReceivingTermination::value;
        return true;
    }

    switch (self.node_tag_buffer.node_type) {
    case NodeTypeEnum::IntegralList::value:
        self.state = list_elem_cnt == static_cast<size_t>(-1)
                         ? StateEnum::ReceivingIntegralOrTermination::value
                         : StateEnum::ReceivingIntegral::value;
        break;

    case NodeTypeEnum::NodeList::value:
        self.state = list_elem_cnt == static_cast<size_t>(-1)
                         ? StateEnum::ReceivingNodeTagOrTermination::value
                         : StateEnum::ReceivingNodeTag::value;
        break;

    default: ZETA_Core_Unreachable();
    }

    return true;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
template <integral::IsIntegral Integral>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeIntegral(this SerializeToOctetsStateMachine& self,
                                 Integral src_integral) {
    if (!(self.state == StateEnum::ReceivingIntegral::value ||
          self.state == StateEnum::ReceivingIntegralOrTermination::value)) {
        return false;
    }

    if (self.integral_descriptor_buffer.is_signed !=
        integral::IsSignedIntegral<Integral>) {
        return false;
    }

    ZETA_Core_Debug_PrintVar(src_integral);
    ZETA_Core_Debug_PrintVar(self.res_elem_cnts[self.depth - 1]);

    size_t integral_size{ self.integral_descriptor_buffer.size };

    if (self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        if (integral_size == 0) {
            detail::LEB128SerializeIntegral_(src_integral, self.acceptor);
        } else {
            detail::NormalSerializeIntegral_(
                src_integral,
                value_wrapper::DynamicValueWrapper<size_t>{ integral_size },
                self.acceptor);
        }

        if (--self.res_elem_cnts[self.depth - 1] == 0) {
            self.state = StateEnum::ReceivingTermination::value;
        }

        return true;
    }

    size_t est_integral_size{
        integral_size == 0
            ? vlq_utils::EstimateSerializedUnitCnt(
                  src_integral, value_wrapper::StaticValueWrapper<size_t, 8>{})
            : integral_size
    };

    if (self.integral_chunk_buffer.elem_cnt + 1 == 255 ||
        self.integral_chunk_buffer.max_octet_cnt <
            self.integral_chunk_buffer.octet_cnt + est_integral_size) {
        detail::NormalSerializeIntegral_(
            self.integral_chunk_buffer.elem_cnt + 1,
            value_wrapper::StaticValueWrapper<size_t, 1>{}, self.acceptor);

        if (0 < self.integral_chunk_buffer.elem_cnt) {
            elem_stream::acceptor::Transfer(
                self.acceptor, self.integral_chunk_buffer.data, 1, 1,
                self.integral_chunk_buffer.octet_cnt);

            self.integral_chunk_buffer.elem_cnt = 0;
            self.integral_chunk_buffer.octet_cnt = 0;
        }

        if (integral_size == 0) {
            detail::LEB128SerializeIntegral_(src_integral, self.acceptor);
        } else {
            ZETA_Core_Debug_PrintVar(src_integral);
            ZETA_Core_Debug_PrintVar(integral_size);

            detail::NormalSerializeIntegral_(
                src_integral,
                value_wrapper::DynamicValueWrapper<size_t>{ integral_size },
                self.acceptor);
        }

        return true;
    }

    lin_seq_elem_stream::Acceptor integral_chunk_elem_stream_acceptor{
        .data = self.integral_chunk_buffer.data +
                self.integral_chunk_buffer.octet_cnt,
        .elem_size = 1,
        .elem_stride = 1,
        .elem_cnt = est_integral_size,
    };

    if (integral_size == 0) {
        detail::LEB128SerializeIntegral_(src_integral,
                                         integral_chunk_elem_stream_acceptor);
    } else {
        detail::NormalSerializeIntegral_(
            src_integral,
            value_wrapper::DynamicValueWrapper<size_t>{ integral_size },
            integral_chunk_elem_stream_acceptor);
    }

    ++self.integral_chunk_buffer.elem_cnt;
    self.integral_chunk_buffer.octet_cnt += est_integral_size;

    return true;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::TerminateNode(this SerializeToOctetsStateMachine& self) {
    if (!(self.state == StateEnum::ReceivingNodeTagOrTermination::value ||
          self.state == StateEnum::ReceivingIntegralOrTermination::value ||
          self.state == StateEnum::ReceivingTermination::value)) {
        return false;
    }

    switch (self.node_tag_buffer.node_type) {
    case NodeTypeEnum::Null::value: break;

    case NodeTypeEnum::Integral::value: break;

    case NodeTypeEnum::IntegralList::value:
        if (self.res_elem_cnts[self.depth - 1] == static_cast<size_t>(-1)) {
            if (0 < self.integral_chunk_buffer.elem_cnt) {
                detail::NormalSerializeIntegral_(
                    self.integral_chunk_buffer.elem_cnt,
                    value_wrapper::StaticValueWrapper<size_t, 1>{},
                    self.acceptor);

                elem_stream::acceptor::Transfer(
                    self.acceptor, self.integral_chunk_buffer.data, 1, 1,
                    self.integral_chunk_buffer.octet_cnt);

                self.integral_chunk_buffer.elem_cnt = 0;
                self.integral_chunk_buffer.octet_cnt = 0;
            }

            detail::NormalSerializeIntegral_(
                0, value_wrapper::StaticValueWrapper<size_t, 1>{},
                self.acceptor);
        }

        break;

    case NodeTypeEnum::NodeList::value:

        if (self.res_elem_cnts[self.depth - 1] == static_cast<size_t>(-1)) {
            unsigned char node_tag{ NodeTag{
                .node_type = NodeTypeEnum::Terminator::value,
                .has_name = false,
                .has_obj_type = false,
                .has_region = false,
            }
                                        .ToEncodedValue()
                                        .second };

            elem_stream::acceptor::Transfer(self.acceptor, &node_tag, 1, 1, 1);
        }

        break;

    default: ZETA_Core_Unreachable();
    }

    --self.depth;

    if (self.depth == 0) {
        self.state = StateEnum::Completed::value;
        return true;
    }

    self.node_tag_buffer = NodeTag{
        .node_type = NodeTypeEnum::NodeList::value,
        .has_name = false,
        .has_obj_type = false,
        .has_region = false,
    };

    switch (self.res_elem_cnts[self.depth - 1]) {
    case 0: self.state = StateEnum::ReceivingTermination::value; break;

    case static_cast<size_t>(-1):
        self.state = StateEnum::ReceivingNodeTagOrTermination::value;
        break;

    default: self.state = StateEnum::ReceivingNodeTag::value; break;
    }

    return true;
}

constexpr object_state_notation::state_machine::
    DeserializationStateMachineBase::StateEnum::Value
    object_state_notation::state_machine::DeserializationStateMachineBase::
        FindNextState_(StateEnum::Value cur_state, NodeTag const& node_tag) {
    switch (cur_state) {
    case StateEnum::SendingNodeTag::value:
        if (node_tag.has_name) { return StateEnum::SendingNameString::value; }

        [[fallthrough]];

    case StateEnum::SendingNameString::value:
        if (node_tag.has_obj_type) {
            return StateEnum::SendingObjTypeString::value;
        }

        [[fallthrough]];

    case StateEnum::SendingObjTypeString::value:
        if (node_tag.has_region) { return StateEnum::SendingRegionAttr::value; }

        [[fallthrough]];

    case StateEnum::SendingRegionAttr::value:
        switch (node_tag.node_type) {
        case NodeTypeEnum::Null::value:
            return StateEnum::SendingTermination::value;

        case NodeTypeEnum::Integral::value:
        case NodeTypeEnum::IntegralList::value:
            return StateEnum::SendingIntegralDescriptor::value;

        case NodeTypeEnum::NodeList::value:
            return StateEnum::SendingListElemCnt::value;

        default: ZETA_Core_Unreachable();
        }

        break;

    default: ZETA_Core_Unreachable();
    }
}

template <elem_stream::provider::IsProvider Provider>
constexpr object_state_notation::state_machine::
    DeserializeFromOctetsStateMachine<
        Provider>::DeserializeFromOctetsStateMachine(Config const& config,
                                                     Provider& provider)
    : config{ config },
      state{ StateEnum::SendingNodeTag::value },
      depth{ 0 },
      node_tag_buffer_store_nxt{ false },
      provider{ provider } {}

template <elem_stream::provider::IsProvider Provider>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeNodeTag(this DeserializeFromOctetsStateMachine& self,
                                  NodeTag& dst_node_tag) {
    if (!(self.state == StateEnum::SendingNodeTag::value)) { return false; }

    if (!(self.depth < max_depth)) { return false; }

    if (self.node_tag_buffer_store_nxt) {
        self.node_tag_buffer_store_nxt = false;
    } else {
        unsigned char node_tag_encoded_value;

        if (!detail::NormalDeserializeIntegral_(
                node_tag_encoded_value,
                value_wrapper::StaticValueWrapper<size_t, 1>{},
                self.provider)) {
            self.state = StateEnum::Corrupted::value;
            return false;
        }

        auto [is_valid,
              node_tag]{ NodeTag::FromEncodedValue(node_tag_encoded_value) };

        if (!is_valid) {
            self.state = StateEnum::Corrupted::value;
            return false;
        }

        self.node_tag_buffer = node_tag;
    }

    if (0 < self.depth &&
        self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        --self.res_elem_cnts[self.depth - 1];
    }

    ++self.depth;

    dst_node_tag = self.node_tag_buffer;

    if (!(self.node_tag_buffer.Check() &&
          self.node_tag_buffer.node_type != NodeTypeEnum::Terminator::value)) {
        self.state = StateEnum::Corrupted::value;
        return false;
    }

    switch (self.node_tag_buffer.node_type) {
    case NodeTypeEnum::Null::value:
        self.res_elem_cnts[self.depth - 1] = 0;
        break;

    case NodeTypeEnum::Integral::value:
        self.res_elem_cnts[self.depth - 1] = 1;
        break;
    }

    self.state = self.FindNextState_(self.state, self.node_tag_buffer);

    return true;
}

template <elem_stream::provider::IsProvider Provider>
template <elem_stream::acceptor::IsAcceptor Acceptor>
size_t object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeString(this DeserializeFromOctetsStateMachine& self,
                                 Acceptor&& acceptor, size_t max_str_size) {
    if (!(self.state == StateEnum::SendingObjTypeString::value ||
          self.state == StateEnum::SendingNameString::value)) {
        return false;
    }

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
        self.state = self.FindNextState_(self.state, self.node_tag_buffer);
    }

    return acc_str_size;
}

template <elem_stream::provider::IsProvider Provider>
template <integral::IsIntegral Integral>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeRegionAttr(this DeserializeFromOctetsStateMachine&
                                         self,
                                     Integral& region_beg,
                                     Integral& region_size) {
    if (!(self.state == StateEnum::SendingRegionAttr::value)) { return false; }

    detail::NormalDeserializeIntegral_(
        region_beg,
        value_wrapper::DynamicValueWrapper<size_t>{
            self.config.region_attr_size },
        self.provider);

    detail::NormalDeserializeIntegral_(
        region_size,
        value_wrapper::DynamicValueWrapper<size_t>{
            self.config.region_attr_size },
        self.provider);

    self.state = self.FindNextState_(self.state, self.node_tag_buffer);

    return true;
}

template <elem_stream::provider::IsProvider Provider>
bool object_state_notation::state_machine::
    DeserializeFromOctetsStateMachine<Provider>::DeserializeIntegralDescriptor(
        this DeserializeFromOctetsStateMachine& self,
        IntegralDescriptor& dst_integral_descriptor) {
    if (!(self.state == StateEnum::SendingIntegralDescriptor::value)) {
        return false;
    }

    size_t integral_descriptor_encoded_value;

    if (!detail::LEB128DeserializeIntegral_(integral_descriptor_encoded_value,
                                            self.provider)) {
        self.state = StateEnum::Corrupted::value;
        return false;
    }

    IntegralDescriptor integral_descriptor{
        IntegralDescriptor::FromEncodedValue(integral_descriptor_encoded_value)
    };

    self.integral_descriptor_buffer = integral_descriptor;

    dst_integral_descriptor = integral_descriptor;

    switch (self.node_tag_buffer.node_type) {
    case NodeTypeEnum::Integral::value:
        self.state = StateEnum::SendingIntegral::value;
        break;

    case NodeTypeEnum::IntegralList::value:
        self.state = StateEnum::SendingListElemCnt::value;
        break;

    default: ZETA_Core_Unreachable();
    }

    return true;
}

template <elem_stream::provider::IsProvider Provider>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeListElemCnt(this DeserializeFromOctetsStateMachine&
                                          self,
                                      size_t& dst_list_elem_cnt) {
    if (!(self.state == StateEnum::SendingListElemCnt::value)) { return false; }

    size_t list_elem_cnt_encoded_value;

    if (!detail::LEB128DeserializeIntegral_(list_elem_cnt_encoded_value,
                                            self.provider)) {
        self.state = StateEnum::Corrupted::value;
        return false;
    }

    size_t list_elem_cnt{ list_elem_cnt_encoded_value - 1 };

    dst_list_elem_cnt = list_elem_cnt;

    self.res_elem_cnts[self.depth - 1] = list_elem_cnt;

    if (self.node_tag_buffer.node_type == NodeTypeEnum::IntegralList::value &&
        list_elem_cnt == static_cast<size_t>(-1)) {
        if (!detail::NormalDeserializeIntegral_(
                self.integral_chunk_res_elem_cnt,
                value_wrapper::StaticValueWrapper<size_t, 1>{},
                self.provider)) {
            self.state = StateEnum::Corrupted::value;
            return false;
        }

        self.state = self.integral_chunk_res_elem_cnt == 0
                         ? StateEnum::SendingTermination::value
                         : StateEnum::SendingIntegral::value;

        return true;
    }

    if (list_elem_cnt == 0) {
        self.state = StateEnum::SendingTermination::value;
        return true;
    }

    if (self.node_tag_buffer.node_type == NodeTypeEnum::IntegralList::value) {
        self.state = StateEnum::SendingIntegral::value;
        return true;
    }

    unsigned char node_tag_encoded_value;

    if (!detail::NormalDeserializeIntegral_(
            node_tag_encoded_value,
            value_wrapper::StaticValueWrapper<size_t, 1>{}, self.provider)) {
        self.state = StateEnum::Corrupted::value;
        return false;
    }

    auto [is_valid,
          node_tag]{ NodeTag::FromEncodedValue(node_tag_encoded_value) };

    if (!is_valid) {
        self.state = StateEnum::Corrupted::value;
        return false;
    }

    if (node_tag.node_type == NodeTypeEnum::Terminator::value) {
        self.state = StateEnum::SendingTermination::value;
    } else {
        self.node_tag_buffer_store_nxt = true;
        self.node_tag_buffer = node_tag;

        self.state = StateEnum::SendingNodeTag::value;
    }

    return true;
}

template <elem_stream::provider::IsProvider Provider>
template <integral::IsIntegral Integral>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeIntegral(this DeserializeFromOctetsStateMachine& self,
                                   Integral& dst_integral) {
    if (!(self.state == StateEnum::SendingIntegral::value)) { return false; }

    if (self.integral_descriptor_buffer.is_signed !=
        integral::IsSignedIntegral<Integral>) {
        return false;
    }

    if (self.integral_descriptor_buffer.size == 0) {
        detail::LEB128DeserializeIntegral_(dst_integral, self.provider);
    } else {
        detail::NormalDeserializeIntegral_(
            dst_integral,
            value_wrapper::DynamicValueWrapper<size_t>{
                self.integral_descriptor_buffer.size },
            self.provider);
    }

    ZETA_Core_Debug_PrintVar(self.integral_descriptor_buffer.is_signed);
    ZETA_Core_Debug_PrintVar(self.integral_descriptor_buffer.size);
    ZETA_Core_Debug_PrintVar(dst_integral);

    if (self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        --self.res_elem_cnts[self.depth - 1];

        if (self.res_elem_cnts[self.depth - 1] == 0) {
            self.state = StateEnum::SendingTermination::value;
        }

        return true;
    }

    if (0 < --self.integral_chunk_res_elem_cnt) { return true; }

    if (!detail::NormalDeserializeIntegral_(
            self.integral_chunk_res_elem_cnt,
            value_wrapper::StaticValueWrapper<size_t, 1>{}, self.provider)) {
        self.state = StateEnum::Corrupted::value;
        return false;
    }

    if (self.integral_chunk_res_elem_cnt == 0) {
        self.state = StateEnum::SendingTermination::value;
    }

    return true;
}

template <elem_stream::provider::IsProvider Provider>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::TerminateNode(this DeserializeFromOctetsStateMachine& self) {
    if (!(self.state == StateEnum::SendingTermination::value)) { return false; }

    --self.depth;

    if (self.depth == 0) {
        self.state = StateEnum::Completed::value;
        return true;
    }

    if (self.res_elem_cnts[self.depth - 1] != static_cast<size_t>(-1)) {
        if (0 < self.res_elem_cnts[self.depth - 1]) {
            self.state = StateEnum::SendingNodeTag::value;
        }

        return true;
    }

    unsigned char node_tag_encoded_value;

    if (!detail::NormalDeserializeIntegral_(
            node_tag_encoded_value,
            value_wrapper::StaticValueWrapper<size_t, 1>{}, self.provider)) {
        self.state = StateEnum::Corrupted::value;
        return false;
    }

    auto [is_valid,
          node_tag]{ NodeTag::FromEncodedValue(node_tag_encoded_value) };

    if (!is_valid) {
        self.state = StateEnum::Corrupted::value;
        return false;
    }

    if (node_tag.node_type != NodeTypeEnum::Terminator::value) {
        self.node_tag_buffer_store_nxt = true;
        self.node_tag_buffer = node_tag;

        self.state = StateEnum::SendingNodeTag::value;
    }

    return true;
}

}  // namespace zeta::core
