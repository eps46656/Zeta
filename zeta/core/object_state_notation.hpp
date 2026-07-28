#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/seq_cntr.hpp>

namespace zeta::core::object_state_notation {

struct NodeTypeEnum {
    using Value = unsigned char;

    struct Null {
        static constexpr Value value{ 0 };
    };

    struct Integral {
        static constexpr Value value{ 1 };
    };

    struct IntegralList {
        static constexpr Value value{ 2 };
    };

    struct NodeList {
        static constexpr Value value{ 3 };
    };

    struct Terminator {
        static constexpr Value value{ 4 };
    };
};

constexpr unsigned min_region_attr_size{ 4 };
constexpr unsigned max_region_attr_size{ comparison_utils::BasicMin(
    8U, (integral::WidthOf<unsigned long long> + 7) / 8) };
ZETA_Core_StaticAssert(min_region_attr_size <= max_region_attr_size);

constexpr size_t max_integral_size{ integral::RangeMaxOf<size_t> / 2 };

constexpr bool IsValidRegionAttrSize(unsigned size);

constexpr bool IsValidIntegralDescriptorSize(unsigned size);

constexpr bool IsValidIntegralSize(size_t integral_size);

constexpr size_t GetMaxListElemCntWithoutVarying(unsigned list_elem_cnt_size);

constexpr size_t GetVaryingListElemCnt(unsigned list_elem_cnt_size);

constexpr unsigned long long GetMaxIntegralSize(
    unsigned integral_descriptor_size);

struct Version {
    unsigned char major;
    unsigned char minor;
    unsigned char patch[2];
};

struct Header {
    unsigned char magic[4];  // 4

    Version version;  // 4

    unsigned region_attr_size;  // 1

    unsigned char reserved[23];  // 23

    constexpr bool Check(this Header const& self);
};

struct Config {
    Version version;

    unsigned region_attr_size;

    constexpr bool Check(this Config const& self);

    static constexpr pair::Pair<bool, Config> FromHeader(Header const& header);
    constexpr pair::Pair<bool, Header> ToHeader(this Config const& self);
};

struct NodeTag {
    NodeTypeEnum::Value node_type;
    bool has_name;
    bool has_obj_type;
    bool has_region;

    static constexpr unsigned char NodeTypeMask{ 0b0000'1111U };
    static constexpr unsigned char HasName{ 0b0001'0000U };
    static constexpr unsigned char HasObjType{ 0b0010'0000U };
    static constexpr unsigned char HasRegion{ 0b0100'0000U };

    constexpr bool Check(this NodeTag const& self);

    static constexpr pair::Pair<bool, NodeTag> FromEncodedValue(
        unsigned char value);

    constexpr pair::Pair<bool, unsigned char> ToEncodedValue(
        this NodeTag const& self);
};

struct IntegralDescriptor {
    bool is_signed;
    size_t size;

    constexpr bool Check(this IntegralDescriptor const& self);

    static constexpr IntegralDescriptor FromEncodedValue(size_t encoded_value);

    constexpr pair::Pair<bool, size_t> ToEncodedValue(
        this IntegralDescriptor const& self);
};

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool SerializeHeaderToOctets(Acceptor&& acceptor, Header const& src_header);

template <elem_stream::provider::IsProvider Provider>
bool DeserializeHeaderFromOctets(Provider&& provider, Header& dst_header);

namespace state_machine {

struct SerializationStateMachineBase {
    struct StateEnum {
        using Value = unsigned char;

        struct ReceivingNodeTag {
            static constexpr Value value{ 0 };
        };

        struct ReceivingNodeTagOrTermination {
            static constexpr Value value{ 1 };
        };

        struct ReceivingNameString {
            static constexpr Value value{ 2 };
        };

        struct ReceivingObjTypeString {
            static constexpr Value value{ 3 };
        };

        struct ReceivingRegionAttr {
            static constexpr Value value{ 4 };
        };

        struct ReceivingIntegralDescriptor {
            static constexpr Value value{ 5 };
        };

        struct ReceivingListElemCnt {
            static constexpr Value value{ 6 };
        };

        struct ReceivingIntegral {
            static constexpr Value value{ 7 };
        };

        struct ReceivingIntegralOrTermination {
            static constexpr Value value{ 8 };
        };

        struct ReceivingTermination {
            static constexpr Value value{ 9 };
        };

        struct Completed {
            static constexpr Value value{ 10 };
        };

        struct Corrupted {
            static constexpr Value value{ static_cast<unsigned char>(-1) };
        };
    };

    static constexpr StateEnum::Value FindNextState_(StateEnum::Value cur_state,
                                                     NodeTag const& node_tag);
};

template <elem_stream::acceptor::IsAcceptor Acceptor>
struct SerializeToOctetsStateMachine : public SerializationStateMachineBase {
    static constexpr unsigned max_depth{ 32 };

    Config config;

    StateEnum::Value state;

    unsigned short depth;

    size_t res_elem_cnts[max_depth];

    NodeTag node_tag_buffer;

    IntegralDescriptor integral_descriptor_buffer;

    struct {
        unsigned char* data;
        unsigned char elem_cnt;
        unsigned char octet_cnt;
        unsigned short max_octet_cnt;
    } integral_chunk_buffer;

    Acceptor& acceptor;

    constexpr SerializeToOctetsStateMachine(
        Config const& config, unsigned char* integral_chunk_buffer_data,
        unsigned short integral_chunk_buffer_max_octet_cnt, Acceptor& acceptor);

    bool SerializeNodeTag(this SerializeToOctetsStateMachine& self,
                          NodeTag const& src_node_tag);

    template <elem_stream::provider::IsProvider Provider>
    bool SerializeString(this SerializeToOctetsStateMachine& self,
                         Provider&& provider, size_t size);

    bool TerminateSerializeString(this SerializeToOctetsStateMachine& self);

    template <integral::IsIntegral Integral>
    bool SerializeRegionAttr(this SerializeToOctetsStateMachine& self,
                             Integral region_beg, Integral region_size);

    bool SerializeIntegralDescriptor(
        this SerializeToOctetsStateMachine& self,
        IntegralDescriptor const& src_integral_descriptor);

    bool SerializeListElemCnt(this SerializeToOctetsStateMachine& self,
                              size_t list_elem_cnt);

    template <integral::IsIntegral Integral>
    bool SerializeIntegral(this SerializeToOctetsStateMachine& self,
                           Integral src_integral);

    bool TerminateNode(this SerializeToOctetsStateMachine& self);
};

struct DeserializationStateMachineBase {
    struct StateEnum {
        using Value = unsigned char;

        struct SendingNodeTag {
            static constexpr Value value{ 0 };
        };

        struct SendingNameString {
            static constexpr Value value{ 2 };
        };

        struct SendingObjTypeString {
            static constexpr Value value{ 3 };
        };

        struct SendingRegionAttr {
            static constexpr Value value{ 4 };
        };

        struct SendingIntegralDescriptor {
            static constexpr Value value{ 5 };
        };

        struct SendingListElemCnt {
            static constexpr Value value{ 6 };
        };

        struct SendingIntegral {
            static constexpr Value value{ 7 };
        };

        struct SendingTermination {
            static constexpr Value value{ 9 };
        };

        struct Completed {
            static constexpr Value value{ 10 };
        };

        struct Corrupted {
            static constexpr Value value{ static_cast<unsigned char>(-1) };
        };
    };

    static constexpr StateEnum::Value FindNextState_(StateEnum::Value cur_state,
                                                     NodeTag const& node_tag);
};

template <elem_stream::provider::IsProvider Provider>
struct DeserializeFromOctetsStateMachine
    : public DeserializationStateMachineBase {
    static constexpr unsigned max_depth{ 32 };

    Config config;

    StateEnum::Value state;

    unsigned short depth;

    size_t res_elem_cnts[max_depth];

    bool node_tag_buffer_store_nxt;
    NodeTag node_tag_buffer;

    IntegralDescriptor integral_descriptor_buffer;

    unsigned char integral_chunk_res_elem_cnt;

    Provider& provider;

    constexpr DeserializeFromOctetsStateMachine(Config const& config,
                                                Provider& provider);

    bool DeserializeNodeTag(this DeserializeFromOctetsStateMachine& self,
                            NodeTag& dst_node_tag);

    template <elem_stream::acceptor::IsAcceptor Accetpr>
    size_t DeserializeString(this DeserializeFromOctetsStateMachine& self,
                             Accetpr&& acceptor, size_t max_str_size);

    template <integral::IsIntegral Integral>
    bool DeserializeRegionAttr(this DeserializeFromOctetsStateMachine& self,
                               Integral& region_beg, Integral& region_size);

    bool DeserializeIntegralDescriptor(
        this DeserializeFromOctetsStateMachine& self,
        IntegralDescriptor& dst_integral_descriptor);

    bool DeserializeListElemCnt(this DeserializeFromOctetsStateMachine& self,
                                size_t& list_elem_cnt);

    template <integral::IsIntegral Integral>
    bool DeserializeIntegral(this DeserializeFromOctetsStateMachine& self,
                             Integral& dst_integral);

    bool TerminateNode(this DeserializeFromOctetsStateMachine& self);
};

}  // namespace state_machine

}  // namespace zeta::core::object_state_notation
