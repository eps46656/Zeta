#include <fstream>
#include <vector>
#include <zeta/core/object_state_notation.ipp>
#include <zeta/core/seq_cntr.ipp>

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

#define ZetaDir "D:/ZetaDevelops/ZetaDevelop/Zeta"

inline void main1(int num) {
    constexpr size_t str_buffer_size{ 1024 };
    unsigned char str_buffer[1024];

    std::string num_str{ std::to_string(num) };

    std::ifstream bin_fs{ std::string{ ZetaDir "/osn/test_osn_" } + num_str +
                              ".bin",
                          std::ios::binary };

    std::ifstream cri_bin_fs{ std::string{ ZetaDir "/osn/test_osn_" } +
                                  num_str + ".bin",
                              std::ios::binary };

    std::ofstream re_bin_fs{ std::string{ ZetaDir "/osn/test_osn_re_" } +
                                 num_str + ".bin",
                             std::ios::binary };

    struct {
        std::ifstream& bin_fs;

        void operator()(void* data, size_t elem_stride, size_t elem_cnt) {
            ZETA_Core_DebugAssert(elem_stride == 1);

            if (elem_stride == 1) {
                this->bin_fs.read(
                    static_cast<char*>(data),
                    static_cast<long long>(elem_stride * elem_cnt));
            } else {
                unsigned char read_buffer[1024];

                this->bin_fs.read(reinterpret_cast<char*>(read_buffer),
                                  static_cast<long long>(elem_cnt));

                zeta::core::utils::ElemCopy(data, read_buffer, 1, elem_stride,
                                            1, elem_cnt);
            }
        }
    } bin_fs_provider{ bin_fs };

    struct {
        std::istream& re_bin_fs;
        std::ofstream& bin_fs;

        void operator()(void const* data, size_t elem_stride, size_t elem_cnt) {
            unsigned char cri_buffer[1024];
            unsigned char write_buffer[1024];

            zeta::core::utils::ElemCopy(write_buffer, data, 1, 1, elem_stride,
                                        elem_cnt);

            this->re_bin_fs.read(reinterpret_cast<char*>(cri_buffer),
                                 static_cast<long long>(elem_cnt));

            int cmp{ std::memcmp(write_buffer, cri_buffer, elem_cnt) };

            if (cmp != 0) {
                ZETA_Core_Debug_PrintVar(cri_buffer[0]);
                ZETA_Core_Debug_PrintVar(cri_buffer[1]);
                ZETA_Core_Debug_PrintVar(cri_buffer[2]);
                ZETA_Core_Debug_PrintVar(cri_buffer[3]);

                ZETA_Core_Debug_PrintVar(elem_cnt);

                ZETA_Core_Debug_PrintVar(
                    static_cast<unsigned char const*>(write_buffer)[0]);

                ZETA_Core_Debug_PrintVar(
                    static_cast<unsigned char const*>(write_buffer)[1]);

                ZETA_Core_Debug_PrintVar(
                    static_cast<unsigned char const*>(write_buffer)[2]);

                ZETA_Core_Debug_PrintVar(
                    static_cast<unsigned char const*>(write_buffer)[3]);

                ZETA_Core_DebugAssert(cmp == 0);
            }

            this->bin_fs.write(reinterpret_cast<char const*>(write_buffer),
                               static_cast<long long>(elem_cnt));
        }
    } bin_fs_acceptor{ cri_bin_fs, re_bin_fs };

    zeta::core::object_state_notation::Header header;

    zeta::core::object_state_notation::DeserializeHeaderFromOctets(
        bin_fs_provider, header);

    auto config{ ({
        auto [is_valid, config]{
            zeta::core::object_state_notation::Config::FromHeader(header)
        };

        if (!is_valid) {
            ZETA_Core_PrintVar(header.magic[0]);
            ZETA_Core_PrintVar(header.magic[1]);
            ZETA_Core_PrintVar(header.magic[2]);
            ZETA_Core_PrintVar(header.magic[3]);
            ZETA_Core_PrintVar(header.region_attr_size);
            ZETA_Core_PrintVar(header.integral_descriptor_size);
            ZETA_Core_PrintVar(header.list_elem_cnt_size);

            ZETA_Core_DebugAssert(false);
            return;
        }

        config;
    }) };

    {
        auto [is_valid, header]{ config.ToHeader() };

        if (!is_valid) {
            ZETA_Core_DebugAssert(false);
            return;
        }

        zeta::core::object_state_notation::SerializeHeaderToOctets(
            bin_fs_acceptor, header);
    }

    zeta::core::object_state_notation::state_machine::
        DeserializeFromOctetsStateMachine<decltype(bin_fs_provider)>
            deserializer{ config, bin_fs_provider };

    zeta::core::object_state_notation::state_machine::
        SerializeToOctetsStateMachine<decltype(bin_fs_acceptor)>
            serializer{ config, bin_fs_acceptor };

    zeta::core::object_state_notation::IntegralDescriptor integral_descriptor;
    unsigned _BitInt(128) unsigned_integral;
    signed _BitInt(128) signed_integral;

    while (deserializer.state !=
           zeta::core::object_state_notation::state_machine::
               DeserializationStateMachineBase::StateEnum::Completed::value) {
        ZETA_Core_Debug_PrintCurPos;

        ZETA_Core_Debug_PrintVar(deserializer.state);
        ZETA_Core_Debug_PrintVar(serializer.state);

        switch (deserializer.state) {
        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingNodeTag::value: {
            ZETA_Core_Debug_PrintCurPos;

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
            DeserializationStateMachineBase::StateEnum::SendingNameString::
                value:
        case zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::SendingObjTypeString::
                value: {
            zeta::core::lin_seq_elem_stream::Acceptor str_reader{
                .data = str_buffer,
                .elem_size = 1,
                .elem_stride = 1,
                .elem_cnt = str_buffer_size,
            };

            size_t str_size{ deserializer.DeserializeString(str_reader,
                                                            str_buffer_size) };

            ZETA_Core_DebugAssert(str_size < str_buffer_size);

            if (!serializer.SerializeString(
                    zeta::core::lin_seq_elem_stream::Provider{
                        .data = str_buffer,
                        .elem_size = 1,
                        .elem_stride = 1,
                        .elem_cnt = str_size,
                    },
                    str_size)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            if (!serializer.TerminateSerializeString()) {
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
            if (!deserializer.DeserializeIntegralDescriptor(
                    integral_descriptor)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            if (!serializer.SerializeIntegralDescriptor(integral_descriptor)) {
                ZETA_Core_DebugAssert(false);
                return;
            }

            ZETA_Core_Debug_PrintVar(integral_descriptor.signedness);
            ZETA_Core_Debug_PrintVar(integral_descriptor.size);

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
            if (integral_descriptor.signedness) {
                if (!deserializer.DeserializeIntegral(signed_integral)) {
                    ZETA_Core_DebugAssert(false);
                    return;
                }

                if (!serializer.SerializeIntegral(signed_integral)) {
                    ZETA_Core_DebugAssert(false);
                    return;
                }
            } else {
                if (!deserializer.DeserializeIntegral(unsigned_integral)) {
                    ZETA_Core_DebugAssert(false);
                    return;
                }

                if (!serializer.SerializeIntegral(unsigned_integral)) {
                    ZETA_Core_DebugAssert(false);
                    return;
                }
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

    ZETA_Core_DebugAssert(
        serializer.state ==
        zeta::core::object_state_notation::state_machine::
            SerializationStateMachineBase::StateEnum::Completed::value);

    ZETA_Core_DebugAssert(
        deserializer.state ==
        zeta::core::object_state_notation::state_machine::
            DeserializationStateMachineBase::StateEnum::Completed::value);

    ZETA_Core_DebugAssert(!cri_bin_fs.eof());

    char dummy;

    cri_bin_fs.read(&dummy, 1);

    ZETA_Core_DebugAssert(cri_bin_fs.eof());

    ZETA_Core_PrintVar("complete");

    bin_fs.close();
    re_bin_fs.close();
}

int main() {
    for (int num{ 0 }; num < 100; ++num) {
        ZETA_Core_PrintVar(num);
        main1(num);
    }

    ZETA_Core_PrintVar("ok");
    return 0;
}
