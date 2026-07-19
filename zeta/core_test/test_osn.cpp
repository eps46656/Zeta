#include <fstream>
#include <vector>
#include <zeta/core/object_state_notation.ipp>

/*
struct Node {
    zeta::core::object_state_notation::NodeHeader node_header;

    union {
        unsigned _BitInt(64) u64_integral;
        signed _BitInt(64) s64_integral;

        std::vector<unsigned _BitInt(64)> u64_integral_list;
        std::vector<signed _BitInt(64)> s64_integral_list;

        std::vector<Node> node_list;
    };

    Node(zeta::core::object_state_notation::NodeHeader const& node_header_)
        : node_header{ node_header_ } {
        switch (this->node_header.node_tag.node_type) {
        case zeta::core::object_state_notation::NodeTypeEnum::IntegralList::
            value:

            if (this->node_header.integral_descriptor.signedness) {
                new (
                    &this->s64_integral_list)
decltype(this->s64_integral_list){}; } else { new ( &this->u64_integral_list)
decltype(this->u64_integral_list){};
            }

            break;

        case zeta::core::object_state_notation::NodeTypeEnum::NodeList::value:
            new (&this->node_list) decltype(this->node_list){};
            break;
        }
    }

    Node(Node&& node) : node_header{ node.node_header } {
        switch (this->node_header.node_tag.node_type) {
        case zeta::core::object_state_notation::NodeTypeEnum::IntegralList::
            value:

            if (this->node_header.integral_descriptor.signedness) {
                new (
                    &this->s64_integral_list) decltype(this->s64_integral_list){
                    std::move(node.s64_integral_list)
                };
            } else {
                new (
                    &this->u64_integral_list) decltype(this->u64_integral_list){
                    std::move(node.u64_integral_list)
                };
            }

            break;

        case zeta::core::object_state_notation::NodeTypeEnum::NodeList::value:
            new (&this->node_list) decltype(this->node_list){ std::move(
                node.node_list) };
            break;
        }
    }

    ~Node() {
        switch (this->node_header.node_tag.node_type) {
        case zeta::core::object_state_notation::NodeTypeEnum::IntegralList::
            value:

            if (this->node_header.integral_descriptor.signedness) {
                this->s64_integral_list.~vector();
            } else {
                this->u64_integral_list.~vector();
            }

            break;

        case zeta::core::object_state_notation::NodeTypeEnum::NodeList::value:
            this->node_list.~vector();
            break;
        }
    }
};
*/

inline void main1() {
    constexpr size_t str_buffer_size{ 1024 };
    unsigned char str_buffer[1024];

    std::ifstream ifs{ ZetaDir "/osn/test_osn_0.bin", std::ios::binary };
    std::ofstream ofs{ ZetaDir "/osn/test_osn_re_0.bin", std::ios::binary };

    struct {
        std::ifstream& ifs;

        void operator()(void* data, size_t elem_size, size_t elem_cnt) {
            ifs.read(static_cast<char*>(data), elem_size * elem_cnt);
        }
    } ifs_provider{ ifs };

    struct {
        std::ofstream& ofs;

        void operator()(void const* data, size_t elem_size, size_t elem_cnt) {
            ofs.write(static_cast<char const*>(data), elem_size * elem_cnt);
        }
    } ofs_acceptor{ ofs };

    zeta::core::object_state_notation::Header header;

    zeta::core::object_state_notation::DeserializeHeaderFromOctets(ifs_provider,
                                                                   header);

    auto config{ ({
        auto [is_valid, config]{
            zeta::core::object_state_notation::Config::FromHeader(header)
        };

        if (!is_valid) {
            ZETA_Core_PrintVar("invalid config");
            return;
        }

        config;
    }) };

    {
        auto [is_valid, header]{ config.ToHeader() };

        if (!is_valid) {
            ZETA_Core_PrintVar("invalid config");
            return;
        }

        zeta::core::object_state_notation::SerializeHeaderToOctets(ofs_acceptor,
                                                                   header);
    }

    zeta::core::object_state_notation::state_machine::
        DeserializeFromOctetsStateMachine<decltype(ifs_provider)>
            deserializer{ config, ifs_provider };

    zeta::core::object_state_notation::state_machine::
        SerializeToOctetsStateMachine<decltype(ofs_acceptor)>
            serializer{ config, ofs_acceptor };

    for (;;) {
        switch (deserializer.state) {
        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingNodeTag::value: {
            zeta::core::object_state_notation::NodeTag node_tag;

            if (!deserializer.DeserializeNodeTag(node_tag)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            if (!serializer.SerializeNodeTag(node_tag)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            break;
        }

        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingObjTypeString::
                value:
        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingNameString::
                value: {
            zeta::core::seq_cntr::MemReader str_reader{
                .data = str_buffer,
                .elem_size = 1,
                .elem_stride = 1,
            };

            size_t str_size{ deserializer.DeserializeString(str_reader,
                                                            str_buffer_size) };

            ZETA_Core_DebugAssert(str_size < str_buffer_size);

            if (!serializer.SerializeString(
                    zeta::core::seq_cntr::MemWriter{
                        .data = str_buffer,
                        .elem_size = 1,
                        .elem_stride = 1,
                    },
                    str_size)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            break;
        }

        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingRegionAttr::
                value: {
            unsigned long long region_beg;
            unsigned long long region_size;

            if (!deserializer.DeserializeRegionAttr(region_beg, region_size)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            if (!serializer.SerializeRegionAttr(region_beg, region_size)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            break;
        }

        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::
                SendingIntegralDescriptor::value: {
            zeta::core::object_state_notation::IntegralDescriptor
                integral_descriptor;

            if (!deserializer.DeserializeIntegralDescriptor(
                    integral_descriptor)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            if (!serializer.SerializeIntegralDescriptor(integral_descriptor)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            break;
        }

        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingListElemCnt::
                value: {
            size_t list_elem_cnt;

            if (!deserializer.DeserializeListElemCnt(list_elem_cnt)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            if (!serializer.SerializeListElemCnt(list_elem_cnt)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            break;
        }

        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingIntegral::
                value: {
            unsigned long long integral;

            if (!deserializer.DeserializeIntegral(integral)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            if (!serializer.SerializeIntegral(integral)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            break;
        }

        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingTermination::
                value: {
            if (!deserializer.TerminateNode()) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            if (!serializer.TerminateNode()) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            break;
        }

        default:
            ZETA_Core_Debug_PrintVar(deserializer.state);

            ZETA_Core_DebugAssert(false);
            return;
        }

        ZETA_Core_DebugAssert(
            deserializer.state !=
            zeta::core::object_state_notation::state_machine::
                DeserializationStateMachineBase::StateEnum::Corrupted::value);

        ZETA_Core_DebugAssert(
            serializer.state !=
            zeta::core::object_state_notation::state_machine::
                SerializationStateMachineBase::StateEnum::Corrupted::value);

        zeta::core::debug_utils::ClearDebugStrStream();
    }

    ifs.close();
    ofs.close();
}

int main() {
    main1();
    ZETA_Core_PrintVar("ok");
    return 0;
}
