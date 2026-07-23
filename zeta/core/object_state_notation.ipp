#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/integral_math.ipp>
#include <zeta/core/object_state_notation.hpp>
#include <zeta/core/serde_utils.ipp>

namespace zeta::core {

constexpr bool object_state_notation::IsValidRegionAttrSize(
    unsigned region_attr_size) {
    return min_region_attr_size <= region_attr_size &&
           region_attr_size <= max_region_attr_size;
}

constexpr bool object_state_notation::IsValidIntegralDescriptorSize(
    unsigned integral_descriptor_size) {
    return min_integral_descriptor_size <= integral_descriptor_size &&
           integral_descriptor_size <= max_integral_descriptor_size;
}

constexpr bool object_state_notation::IsValidListElemCntSize(
    unsigned list_elem_cnt_size) {
    return min_list_elem_cnt_size <= list_elem_cnt_size &&
           list_elem_cnt_size <= max_list_elem_cnt_size;
}

constexpr size_t object_state_notation::GetMaxListElemCntWithoutVarying(
    unsigned list_elem_cnt_size) {
    ZETA_Core_DebugAssert((IsValidListElemCntSize)(list_elem_cnt_size));

    return (1ULL << (list_elem_cnt_size * 8 - 1)) - 2;
}

constexpr size_t object_state_notation::GetVaryingListElemCnt(
    unsigned list_elem_cnt_size) {
    ZETA_Core_DebugAssert((IsValidListElemCntSize)(list_elem_cnt_size));

    return (1ULL << (list_elem_cnt_size * 8 - 1)) - 1;
}

constexpr unsigned long long object_state_notation::GetMaxIntegralSize(
    unsigned integral_descriptor_size) {
    ZETA_Core_DebugAssert(
        (IsValidIntegralDescriptorSize)(integral_descriptor_size));

    return integral_math::PowerOf2Minus1<unsigned long long>(
        integral_descriptor_size * 8 - 1);
}

constexpr bool object_state_notation::Header::Check() const {
    if (!(this->magic[0] == 'O' && this->magic[1] == 'S' &&
          this->magic[2] == 'N' && this->magic[3] == '\0')) {
        return false;
    }

    if (!(IsValidRegionAttrSize)(this->region_attr_size)) { return false; }

    if (!(IsValidIntegralDescriptorSize)(this->integral_descriptor_size)) {
        return false;
    }

    if (!(IsValidListElemCntSize)(this->list_elem_cnt_size)) { return false; }

    bool is_all_zero{ true };

    for (size_t i{ 0 }; i < sizeof(this->reserved) / sizeof(this->reserved[0]);
         ++i) {
        is_all_zero &= this->reserved[i] == 0;
    }

    if (!is_all_zero) { return false; }

    return true;
}

constexpr bool object_state_notation::Config::Check() const {
    if (!(IsValidRegionAttrSize)(this->region_attr_size)) { return false; }

    if (!(IsValidIntegralDescriptorSize)(this->integral_descriptor_size)) {
        return false;
    }

    if (!(IsValidListElemCntSize)(this->list_elem_cnt_size)) { return false; }

    return true;
}

constexpr pair::Pair<bool, object_state_notation::Config>
object_state_notation::Config::FromHeader(Header const& header) {
    Config config{
        .version = header.version,
        .region_attr_size = header.region_attr_size,
        .integral_descriptor_size = header.integral_descriptor_size,
        .list_elem_cnt_size = header.list_elem_cnt_size,
    };

    bool is_valid{ true };

    is_valid &= header.Check();
    is_valid &= config.Check();

    return { is_valid, config };
}

constexpr pair::Pair<bool, object_state_notation::Header>
object_state_notation::Config::ToHeader() const {
    if (!this->Check()) { return { false, {} }; }

    Header header{
        .magic{ 'O', 'S', 'N', '\0' },
        .version = this->version,
        .region_attr_size = this->region_attr_size,
        .integral_descriptor_size = this->integral_descriptor_size,
        .list_elem_cnt_size = this->list_elem_cnt_size,
        .reserved{ 0 },
    };

    return { true, header };
}

constexpr bool object_state_notation::NodeTag::Check() const {
    if (!(this->node_type == NodeTypeEnum::Null::value ||
          this->node_type == NodeTypeEnum::Integral::value ||
          this->node_type == NodeTypeEnum::IntegralList::value ||
          this->node_type == NodeTypeEnum::NodeList::value ||
          this->node_type == NodeTypeEnum::Terminator::value)) {
        return false;
    }

    return true;
}

constexpr pair::Pair<bool, object_state_notation::NodeTag>
object_state_notation::NodeTag::FromIntegral(unsigned char value) {
    NodeTag node_tag{
        .node_type = static_cast<unsigned char>(value & NodeTypeMask),
        .has_name = (value & HasName) != 0,
        .has_obj_type = (value & HasObjType) != 0,
        .has_region = (value & HasRegion) != 0,
    };

    return { node_tag.Check(), node_tag };
}

constexpr pair::Pair<bool, unsigned char>
object_state_notation::NodeTag::ToIntegral() const {
    if (!this->Check()) { return { false, 0 }; }

    return { true, static_cast<unsigned char>(
                       this->node_type +                         //
                       (this->has_name ? HasName : 0U) +         //
                       (this->has_obj_type ? HasObjType : 0U) +  //
                       (this->has_region ? HasRegion : 0U)) };
}

constexpr bool object_state_notation::IntegralDescriptor::Check() const {
    if (!(0 < this->size)) { return false; }

    if (!(this->size <= integral::WidthOf<unsigned long long>)) {
        return false;
    }

    return true;
}

constexpr pair::Pair<bool, object_state_notation::IntegralDescriptor>
object_state_notation::IntegralDescriptor::FromIntegral(
    unsigned long long integral_descriptor_integral,
    unsigned integral_descriptor_size) {
    if (!(IsValidIntegralDescriptorSize)(integral_descriptor_size)) {
        return { false, {} };
    }

    unsigned long long k{ 1ULL << (integral_descriptor_size * 8 - 1) };

    bool signedness{ k <= integral_descriptor_integral };
    unsigned long long size{ signedness ? integral_descriptor_integral - k
                                        : integral_descriptor_integral };

    return { true, IntegralDescriptor{
                       .signedness = signedness,
                       .size = size,
                   } };
}

constexpr pair::Pair<bool, unsigned long long>
object_state_notation::IntegralDescriptor::ToIntegral(
    unsigned integral_descriptor_size) const {
    if (!this->Check()) { return { false, 0 }; }

    if (!(IsValidIntegralDescriptorSize)(integral_descriptor_size)) {
        return { false, 0 };
    }

    unsigned long long k{ 1ULL << (integral_descriptor_size * 8 - 1) };

    if (!(this->size < k)) { return { false, 0 }; }

    return { true, this->signedness ? k + this->size : this->size };
}

template <typename Acceptor>
bool object_state_notation::SerializeHeaderToOctets(
    Acceptor&& acceptor, object_state_notation::Header const& src_header) {
    if (!src_header.Check()) { return false; }

    acceptor(src_header.magic, 1, sizeof(src_header.magic));

    acceptor(&src_header.version.major, 1, 1);
    acceptor(&src_header.version.minor, 1, 1);
    acceptor(&src_header.version.patch, 1, 2);

    serde_utils::SerializeIntegral(
        acceptor, src_header.region_attr_size,
        value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
        serde_utils::EndiannessEnum::Little{}, false, nullptr);

    serde_utils::SerializeIntegral(
        acceptor, src_header.integral_descriptor_size,
        value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
        serde_utils::EndiannessEnum::Little{}, false, nullptr);

    serde_utils::SerializeIntegral(
        acceptor, src_header.list_elem_cnt_size,
        value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
        serde_utils::EndiannessEnum::Little{}, false, nullptr);

    acceptor(src_header.reserved, 1, sizeof(src_header.reserved));

    return true;
}

template <typename Provider>
bool object_state_notation::DeserializeHeaderFromOctets(
    Provider&& provider, object_state_notation::Header& dst_header) {
    provider(dst_header.magic, 1, sizeof(dst_header.magic));

    provider(&dst_header.version.major, 1, 1);
    provider(&dst_header.version.minor, 1, 1);
    provider(&dst_header.version.patch, 1, 2);

    serde_utils::DeserializeIntegral(
        provider, dst_header.region_attr_size,
        value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
        serde_utils::EndiannessEnum::Little{}, false, nullptr);

    serde_utils::DeserializeIntegral(
        provider, dst_header.integral_descriptor_size,
        value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
        serde_utils::EndiannessEnum::Little{}, false, nullptr);

    serde_utils::DeserializeIntegral(
        provider, dst_header.list_elem_cnt_size,
        value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
        serde_utils::EndiannessEnum::Little{}, false, nullptr);

    provider(dst_header.reserved, 1, sizeof(dst_header.reserved));

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

template <typename Acceptor>
constexpr object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeToOctetsStateMachine(Config const& config,
                                             Acceptor& acceptor)
    : config{ config },
      state{ StateEnum::ReceivingNodeTag::value },
      depth{ 0 },
      integral_chunk_elem_cnt{ 0 },
      acceptor{ acceptor } {}

template <typename Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeNodeTag(NodeTag const& src_node_tag) {
    if (!(this->state == StateEnum::ReceivingNodeTag::value ||
          this->state == StateEnum::ReceivingNodeTagOrTermination::value)) {
        return false;
    }

    if (!src_node_tag.Check()) { return false; }

    if (!(this->depth < max_depth)) { return false; }

    if (!(src_node_tag.node_type != NodeTypeEnum::Terminator::value)) {
        return false;
    }

    if (0 < this->depth) {
        if (this->res_elem_cnts[this->depth - 1] != static_cast<size_t>(-1)) {
            --this->res_elem_cnts[this->depth - 1];
        }
    }

    auto [is_valid, node_tag_integral]{ src_node_tag.ToIntegral() };

    if (!is_valid) { return false; }

    if (!serde_utils::SerializeIntegral(
            this->acceptor, node_tag_integral,
            value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
            serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
        return false;
    }

    ++this->depth;

    this->node_tag_buffer = src_node_tag;

    switch (this->node_tag_buffer.node_type) {
    case NodeTypeEnum::Null::value:
        this->res_elem_cnts[this->depth - 1] = 0;
        break;

    case NodeTypeEnum::Integral::value:
        this->res_elem_cnts[this->depth - 1] = 1;
        break;
    }

    this->state = this->FindNextState_(this->state, this->node_tag_buffer);

    return true;
}

template <typename Acceptor>
template <typename Provider>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeString(Provider&& provider, size_t size) {
    if (!(this->state == StateEnum::ReceivingNameString::value ||
          this->state == StateEnum::ReceivingObjTypeString::value)) {
        return false;
    }

    constexpr size_t buffer_size{ 256 };
    unsigned char buffer[256];

    for (; buffer_size <= size; size -= buffer_size) {
        provider(buffer, 1, buffer_size);
        this->acceptor(buffer, 1, buffer_size);
    }

    if (0 < size) {
        provider(buffer, 1, size);
        this->acceptor(buffer, 1, size);
    }

    return true;
}

template <typename Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::TerminateSerializeString() {
    if (!(this->state == StateEnum::ReceivingNameString::value ||
          this->state == StateEnum::ReceivingObjTypeString::value)) {
        return false;
    }

    unsigned char zero_octet{ 0 };

    this->acceptor(&zero_octet, 1, 1);

    this->state = this->FindNextState_(this->state, this->node_tag_buffer);

    return true;
}

template <typename Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeRegionAttr(unsigned long long region_beg,
                                   unsigned long long region_size) {
    if (!(this->state == StateEnum::ReceivingRegionAttr::value)) {
        return false;
    }

    serde_utils::SerializeIntegral(
        this->acceptor, region_beg,
        value_wrapper::DynamicValueWrapper<unsigned long long>{
            this->config.region_attr_size },
        serde_utils::EndiannessEnum::Little{}, true, nullptr);

    serde_utils::SerializeIntegral(
        this->acceptor, region_size,
        value_wrapper::DynamicValueWrapper<unsigned long long>{
            this->config.region_attr_size },
        serde_utils::EndiannessEnum::Little{}, true, nullptr);

    this->state = this->FindNextState_(this->state, this->node_tag_buffer);

    return true;
}

template <typename Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeIntegralDescriptor(IntegralDescriptor const&
                                               src_integral_descriptor) {
    if (!(this->state == StateEnum::ReceivingIntegralDescriptor::value)) {
        return false;
    }

    auto [is_valid,
          integral_descriptor_integral]{ src_integral_descriptor.ToIntegral(
        this->config.integral_descriptor_size) };

    if (!is_valid) { return false; }

    if (!serde_utils::SerializeIntegral(
            this->acceptor, integral_descriptor_integral,
            value_wrapper::DynamicValueWrapper<unsigned long long>{
                this->config.integral_descriptor_size },
            serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
        return false;
    }

    this->integral_descriptor_buffer = src_integral_descriptor;

    switch (this->node_tag_buffer.node_type) {
    case NodeTypeEnum::Integral::value:
        this->state = StateEnum::ReceivingIntegral::value;
        break;

    case NodeTypeEnum::IntegralList::value:
        this->state = StateEnum::ReceivingListElemCnt::value;
        break;

    default: ZETA_Core_Unreachable();
    }

    return true;
}

template <typename Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeListElemCnt(size_t list_elem_cnt) {
    if (!(this->state == StateEnum::ReceivingListElemCnt::value)) {
        return false;
    }

    size_t max_list_elem_cnt_without_varying{ (
        GetMaxListElemCntWithoutVarying)(this->config.list_elem_cnt_size) };

    size_t varying_list_elem_cnt{ (
        GetVaryingListElemCnt)(this->config.list_elem_cnt_size) };

    if (!(list_elem_cnt <= max_list_elem_cnt_without_varying ||
          list_elem_cnt == static_cast<size_t>(-1))) {
        return false;
    }

    unsigned long long target_list_elem_cnt{ list_elem_cnt ==
                                                     static_cast<size_t>(-1)
                                                 ? varying_list_elem_cnt
                                                 : list_elem_cnt };

    serde_utils::SerializeIntegral(
        this->acceptor, target_list_elem_cnt,
        value_wrapper::DynamicValueWrapper<unsigned long long>{
            this->config.list_elem_cnt_size },
        serde_utils::EndiannessEnum::Little{}, false, nullptr);

    this->res_elem_cnts[this->depth - 1] = list_elem_cnt;

    if (list_elem_cnt == 0) {
        this->state = StateEnum::ReceivingTermination::value;
        return true;
    }

    switch (this->node_tag_buffer.node_type) {
    case NodeTypeEnum::IntegralList::value:
        this->state = list_elem_cnt == static_cast<size_t>(-1)
                          ? StateEnum::ReceivingIntegralOrTermination::value
                          : StateEnum::ReceivingIntegral::value;
        break;

    case NodeTypeEnum::NodeList::value:
        this->state = list_elem_cnt == static_cast<size_t>(-1)
                          ? StateEnum::ReceivingNodeTagOrTermination::value
                          : StateEnum::ReceivingNodeTag::value;
        break;

    default: ZETA_Core_Unreachable();
    }

    return true;
}

template <typename Acceptor>
template <typename Integral>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::SerializeIntegral(Integral src_integral) {
    if (!(this->state == StateEnum::ReceivingIntegral::value ||
          this->state == StateEnum::ReceivingIntegralOrTermination::value)) {
        return false;
    }

    if (this->integral_descriptor_buffer.signedness) {
        if (!integral::IsSignedIntegral<Integral>) { return false; }
    } else {
        if (!integral::IsUnsignedIntegral<Integral>) { return false; }
    }

    size_t integral_size{ this->integral_descriptor_buffer.size };

    if (this->res_elem_cnts[this->depth - 1] != static_cast<size_t>(-1)) {
        serde_utils::SerializeIntegral(
            this->acceptor, src_integral,
            value_wrapper::DynamicValueWrapper<unsigned long long>{
                integral_size },
            serde_utils::EndiannessEnum::Little{}, true, nullptr);

        if (--this->res_elem_cnts[this->depth - 1] == 0) {
            this->state = StateEnum::ReceivingTermination::value;
        }
    } else if (255 <= integral_size * (this->integral_chunk_elem_cnt + 1)) {
        serde_utils::SerializeIntegral(
            this->acceptor, this->integral_chunk_elem_cnt + 1,
            value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
            serde_utils::EndiannessEnum::Little{}, false, nullptr);

        if (0 < this->integral_chunk_elem_cnt) {
            this->acceptor(this->integral_chunk_buffer, 1,
                           integral_size * this->integral_chunk_elem_cnt);

            this->integral_chunk_elem_cnt = 0;
        }

        serde_utils::SerializeIntegral(
            this->acceptor, src_integral,
            value_wrapper::DynamicValueWrapper<unsigned long long>{
                integral_size },
            serde_utils::EndiannessEnum::Little{}, true, nullptr);
    } else {
        serde_utils::SerializeIntegral(
            lin_seq_elem_stream::Acceptor{
                .data = this->integral_chunk_buffer +
                        integral_size * this->integral_chunk_elem_cnt,
                .elem_size = 1,
                .elem_stride = 1,
            },
            src_integral,
            value_wrapper::DynamicValueWrapper<unsigned long long>{
                integral_size },
            serde_utils::EndiannessEnum::Little{}, true, nullptr);

        ++this->integral_chunk_elem_cnt;
    }

    return true;
}

template <typename Acceptor>
bool object_state_notation::state_machine::SerializeToOctetsStateMachine<
    Acceptor>::TerminateNode() {
    if (!(this->state == StateEnum::ReceivingNodeTagOrTermination::value ||
          this->state == StateEnum::ReceivingIntegralOrTermination::value ||
          this->state == StateEnum::ReceivingTermination::value)) {
        return false;
    }

    switch (this->node_tag_buffer.node_type) {
    case NodeTypeEnum::Null::value: break;

    case NodeTypeEnum::Integral::value: break;

    case NodeTypeEnum::IntegralList::value:

        if (this->res_elem_cnts[this->depth - 1] == static_cast<size_t>(-1)) {
            if (0 < this->integral_chunk_elem_cnt) {
                serde_utils::SerializeIntegral(
                    this->acceptor, this->integral_chunk_elem_cnt,
                    value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
                    serde_utils::EndiannessEnum::Little{}, false, nullptr);

                this->acceptor(this->integral_chunk_buffer, 1,
                               this->integral_descriptor_buffer.size *
                                   this->integral_chunk_elem_cnt);

                this->integral_chunk_elem_cnt = 0;
            }

            serde_utils::SerializeIntegral(
                this->acceptor, 0,
                value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
                serde_utils::EndiannessEnum::Little{}, false, nullptr);
        }

        break;

    case NodeTypeEnum::NodeList::value:

        if (this->res_elem_cnts[this->depth - 1] == static_cast<size_t>(-1)) {
            unsigned char node_tag{ NodeTag{
                .node_type = NodeTypeEnum::Terminator::value,
                .has_name = false,
                .has_obj_type = false,
                .has_region = false,
            }
                                        .ToIntegral()
                                        .second };

            this->acceptor(&node_tag, 1, 1);
        }

        break;

    default: ZETA_Core_Unreachable();
    }

    --this->depth;

    if (this->depth == 0) {
        this->state = StateEnum::Completed::value;
        return true;
    }

    this->node_tag_buffer = NodeTag{
        .node_type = NodeTypeEnum::NodeList::value,
        .has_name = false,
        .has_obj_type = false,
        .has_region = false,
    };

    switch (this->res_elem_cnts[this->depth - 1]) {
    case 0: this->state = StateEnum::ReceivingTermination::value; break;

    case static_cast<size_t>(-1):
        this->state = StateEnum::ReceivingNodeTagOrTermination::value;
        break;

    default: this->state = StateEnum::ReceivingNodeTag::value; break;
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

template <typename Provider>
constexpr object_state_notation::state_machine::
    DeserializeFromOctetsStateMachine<
        Provider>::DeserializeFromOctetsStateMachine(Config const& config,
                                                     Provider& provider)
    : config{ config },
      state{ StateEnum::SendingNodeTag::value },
      depth{ 0 },
      node_tag_buffer_store_nxt{ false },
      provider{ provider } {}

template <typename Provider>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeNodeTag(NodeTag& dst_node_tag) {
    if (!(this->state == StateEnum::SendingNodeTag::value)) { return false; }

    if (!(this->depth < max_depth)) { return false; }

    if (this->node_tag_buffer_store_nxt) {
        this->node_tag_buffer_store_nxt = false;
    } else {
        unsigned char node_tag_integral;

        if (!serde_utils::DeserializeIntegral(
                this->provider, node_tag_integral,
                value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
                serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
            this->state = StateEnum::Corrupted::value;
            return false;
        }

        auto [is_valid, node_tag]{ NodeTag::FromIntegral(node_tag_integral) };

        if (!is_valid) {
            this->state = StateEnum::Corrupted::value;
            return false;
        }

        this->node_tag_buffer = node_tag;
    }

    if (0 < this->depth &&
        this->res_elem_cnts[this->depth - 1] != static_cast<size_t>(-1)) {
        --this->res_elem_cnts[this->depth - 1];
    }

    ++this->depth;

    dst_node_tag = this->node_tag_buffer;

    if (!(this->node_tag_buffer.Check() &&
          this->node_tag_buffer.node_type != NodeTypeEnum::Terminator::value)) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    switch (this->node_tag_buffer.node_type) {
    case NodeTypeEnum::Null::value:
        this->res_elem_cnts[this->depth - 1] = 0;
        break;

    case NodeTypeEnum::Integral::value:
        this->res_elem_cnts[this->depth - 1] = 1;
        break;
    }

    this->state = this->FindNextState_(this->state, this->node_tag_buffer);

    return true;
}

template <typename Provider>
template <typename Acceptor>
size_t object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeString(Acceptor&& acceptor, size_t max_str_size) {
    if (!(this->state == StateEnum::SendingObjTypeString::value ||
          this->state == StateEnum::SendingNameString::value)) {
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
            this->provider(buffer + buffer_elem_cnt, 1, 1);

            if (buffer[buffer_elem_cnt] == 0) {
                string_is_terminated = true;
                break;
            }
        }

        acceptor(buffer, 1, buffer_elem_cnt);

        max_str_size -= buffer_elem_cnt;
        acc_str_size += buffer_elem_cnt;
    }

    if (string_is_terminated) {
        this->state = this->FindNextState_(this->state, this->node_tag_buffer);
    }

    return acc_str_size;
}

template <typename Provider>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeRegionAttr(unsigned long long& region_beg,
                                     unsigned long long& region_size) {
    if (!(this->state == StateEnum::SendingRegionAttr::value)) { return false; }

    serde_utils::DeserializeIntegral(
        this->provider, region_beg,
        value_wrapper::DynamicValueWrapper<unsigned long long>{
            this->config.region_attr_size },
        serde_utils::EndiannessEnum::Little{}, true, nullptr);

    serde_utils::DeserializeIntegral(
        this->provider, region_size,
        value_wrapper::DynamicValueWrapper<unsigned long long>{
            this->config.region_attr_size },
        serde_utils::EndiannessEnum::Little{}, true, nullptr);

    this->state = this->FindNextState_(this->state, this->node_tag_buffer);

    return true;
}

template <typename Provider>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeIntegralDescriptor(IntegralDescriptor&
                                                 dst_integral_descriptor) {
    if (!(this->state == StateEnum::SendingIntegralDescriptor::value)) {
        return false;
    }

    unsigned long long integral_descriptor_integral;

    if (!serde_utils::DeserializeIntegral(
            this->provider, integral_descriptor_integral,
            value_wrapper::DynamicValueWrapper<unsigned long long>{
                this->config.integral_descriptor_size },
            serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    auto [is_valid, integral_descriptor]{ IntegralDescriptor::FromIntegral(
        integral_descriptor_integral, this->config.integral_descriptor_size) };

    if (!is_valid) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    this->integral_descriptor_buffer = integral_descriptor;

    dst_integral_descriptor = integral_descriptor;

    switch (this->node_tag_buffer.node_type) {
    case NodeTypeEnum::Integral::value:
        this->state = StateEnum::SendingIntegral::value;
        break;

    case NodeTypeEnum::IntegralList::value:
        this->state = StateEnum::SendingListElemCnt::value;
        break;

    default: ZETA_Core_Unreachable();
    }

    return true;
}

template <typename Provider>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeListElemCnt(size_t& dst_list_elem_cnt) {
    if (!(this->state == StateEnum::SendingListElemCnt::value)) {
        return false;
    }

    unsigned long long list_elem_cnt_integral;

    if (!serde_utils::DeserializeIntegral(
            this->provider, list_elem_cnt_integral,
            value_wrapper::DynamicValueWrapper<unsigned long long>{
                this->config.list_elem_cnt_size },
            serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    size_t max_list_elem_cnt_without_varying{ (
        GetMaxListElemCntWithoutVarying)(this->config.list_elem_cnt_size) };

    size_t varying_list_elem_cnt{ (
        GetVaryingListElemCnt)(this->config.list_elem_cnt_size) };

    if (!(list_elem_cnt_integral <= max_list_elem_cnt_without_varying ||
          list_elem_cnt_integral == varying_list_elem_cnt)) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    size_t list_elem_cnt{ list_elem_cnt_integral == varying_list_elem_cnt
                              ? static_cast<size_t>(-1)
                              : static_cast<size_t>(list_elem_cnt_integral) };

    dst_list_elem_cnt = list_elem_cnt;

    this->res_elem_cnts[this->depth - 1] = list_elem_cnt;

    if (this->node_tag_buffer.node_type == NodeTypeEnum::IntegralList::value &&
        list_elem_cnt == static_cast<size_t>(-1)) {
        if (!serde_utils::DeserializeIntegral(
                this->provider, this->integral_chunk_res_elem_cnt,
                value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
                serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
            this->state = StateEnum::Corrupted::value;
            return false;
        }

        this->state = this->integral_chunk_res_elem_cnt == 0
                          ? StateEnum::SendingTermination::value
                          : StateEnum::SendingIntegral::value;

        return true;
    }

    if (list_elem_cnt == 0) {
        this->state = StateEnum::SendingTermination::value;
        return true;
    }

    if (this->node_tag_buffer.node_type == NodeTypeEnum::IntegralList::value) {
        this->state = StateEnum::SendingIntegral::value;
        return true;
    }

    unsigned char node_tag_integral;

    if (!serde_utils::DeserializeIntegral(
            this->provider, node_tag_integral,
            value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
            serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    auto [is_valid, node_tag]{ NodeTag::FromIntegral(node_tag_integral) };

    if (!is_valid) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    if (node_tag.node_type == NodeTypeEnum::Terminator::value) {
        this->state = StateEnum::SendingTermination::value;
    } else {
        this->node_tag_buffer_store_nxt = true;
        this->node_tag_buffer = node_tag;

        this->state = StateEnum::SendingNodeTag::value;
    }

    return true;
}

template <typename Provider>
template <typename Integral>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::DeserializeIntegral(Integral& dst_integral) {
    if (!(this->state == StateEnum::SendingIntegral::value)) { return false; }

    if (this->integral_descriptor_buffer.signedness) {
        if (!integral::IsSignedIntegral<Integral>) { return false; }
    } else {
        if (!integral::IsUnsignedIntegral<Integral>) { return false; }
    }

    serde_utils::DeserializeIntegral(
        this->provider, dst_integral,
        value_wrapper::DynamicValueWrapper<unsigned long long>{
            this->integral_descriptor_buffer.size },
        serde_utils::EndiannessEnum::Little{}, true, nullptr);

    if (this->res_elem_cnts[this->depth - 1] != static_cast<size_t>(-1)) {
        --this->res_elem_cnts[this->depth - 1];

        if (this->res_elem_cnts[this->depth - 1] == 0) {
            this->state = StateEnum::SendingTermination::value;
        }

        return true;
    }

    if (0 < --this->integral_chunk_res_elem_cnt) { return true; }

    if (!serde_utils::DeserializeIntegral(
            this->provider, this->integral_chunk_res_elem_cnt,
            value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
            serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    if (this->integral_chunk_res_elem_cnt == 0) {
        this->state = StateEnum::SendingTermination::value;
    }

    return true;
}

template <typename Provider>
bool object_state_notation::state_machine::DeserializeFromOctetsStateMachine<
    Provider>::TerminateNode() {
    if (!(this->state == StateEnum::SendingTermination::value)) {
        return false;
    }

    --this->depth;

    if (this->depth == 0) {
        this->state = StateEnum::Completed::value;
        return true;
    }

    if (this->res_elem_cnts[this->depth - 1] != static_cast<size_t>(-1)) {
        if (0 < this->res_elem_cnts[this->depth - 1]) {
            this->state = StateEnum::SendingNodeTag::value;
        }

        return true;
    }

    unsigned char node_tag_integral;

    if (!serde_utils::DeserializeIntegral(
            this->provider, node_tag_integral,
            value_wrapper::StaticValueWrapper<unsigned long long, 1>{},
            serde_utils::EndiannessEnum::Little{}, false, nullptr)) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    auto [is_valid, node_tag]{ NodeTag::FromIntegral(node_tag_integral) };

    if (!is_valid) {
        this->state = StateEnum::Corrupted::value;
        return false;
    }

    if (node_tag.node_type != NodeTypeEnum::Terminator::value) {
        this->node_tag_buffer_store_nxt = true;
        this->node_tag_buffer = node_tag;

        this->state = StateEnum::SendingNodeTag::value;
    }

    return true;
}

}  // namespace zeta::core
