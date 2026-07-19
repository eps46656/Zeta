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

constexpr unsigned min_integral_descriptor_size{ 2 };
constexpr unsigned max_integral_descriptor_size{ comparison_utils::BasicMin(
    8U, integral::RangeMaxOf<unsigned long long>) };
ZETA_Core_StaticAssert(min_integral_descriptor_size <=
                       max_integral_descriptor_size);

constexpr unsigned min_list_elem_cnt_size{ 4 };
constexpr unsigned max_list_elem_cnt_size{ comparison_utils::BasicMin(
    8U, integral::RangeMaxOf<size_t>) };
ZETA_Core_StaticAssert(min_list_elem_cnt_size <= max_list_elem_cnt_size);

constexpr bool IsValidRegionAttrSize(unsigned size);

constexpr bool IsValidIntegralDescriptorSize(unsigned size);

constexpr bool IsValidListElemCntSize(unsigned size);

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

    unsigned region_attr_size;          // 1
    unsigned integral_descriptor_size;  // 1
    unsigned list_elem_cnt_size;        // 1

    unsigned char reserved[21];  // 21

    constexpr bool Check() const;
};

struct Config {
    Version version;

    unsigned region_attr_size;
    unsigned integral_descriptor_size;
    unsigned list_elem_cnt_size;

    constexpr bool Check() const;

    static constexpr pair::Pair<bool, Config> FromHeader(Header const& header);
    constexpr pair::Pair<bool, Header> ToHeader() const;
};

struct NodeTag {
    NodeTypeEnum::Value node_type;
    bool has_obj_type;
    bool has_region;
    bool has_name;

    static constexpr unsigned char NodeTypeMask{ 0b0000'1111U };
    static constexpr unsigned char HasObjType{ 0b0010'0000U };
    static constexpr unsigned char HasRegion{ 0b0100'0000U };
    static constexpr unsigned char HasName{ 0b1000'0000U };

    constexpr bool Check() const;

    static constexpr pair::Pair<bool, NodeTag> FromIntegral(
        unsigned char value);

    constexpr pair::Pair<bool, unsigned char> ToIntegral() const;
};

struct IntegralDescriptor {
    bool signedness;
    unsigned long long size;

    constexpr bool Check() const;

    static constexpr pair::Pair<bool, IntegralDescriptor> FromIntegral(
        unsigned long long integral_descriptor,
        unsigned integral_descriptor_size);

    constexpr pair::Pair<bool, unsigned long long> ToIntegral(
        unsigned integral_descriptor_size) const;
};

template <typename Acceptor>
void SerializeHeaderToOctets(Acceptor&& acceptor, Header const& src_header);

template <typename Provider>
void DeserializeHeaderFromOctets(Provider&& provider, Header& dst_header);

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

        struct ReceivingObjTypeString {
            static constexpr Value value{ 2 };
        };

        struct ReceivingNameString {
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

        struct Corrupted {
            static constexpr Value value{ static_cast<unsigned char>(-1) };
        };
    };

    static constexpr StateEnum::Value FindNextState_(StateEnum::Value cur_state,
                                                     NodeTag const& node_tag);
};

template <typename Acceptor>
struct SerializeToOctetsStateMachine : public SerializationStateMachineBase {
    static constexpr unsigned max_depth{ 32 };

    Config config;

    StateEnum::Value state;

    unsigned short depth;

    size_t res_elem_cnts[max_depth];

    NodeTag node_tag_buffer;

    IntegralDescriptor integral_descriptor_buffer;

    unsigned char integral_chunk_buffer[255];
    unsigned char integral_chunk_elem_cnt;

    Acceptor& acceptor;

    constexpr SerializeToOctetsStateMachine(Config const& config,
                                            Acceptor& acceptor);

    bool SerializeNodeTag(NodeTag const& src_node_tag);

    template <typename Provider>
    bool SerializeString(Provider&& provider, size_t size);

    bool TerminateSerializeString();

    bool SerializeRegionAttr(unsigned long long region_beg,
                             unsigned long long region_size);

    bool SerializeIntegralDescriptor(
        IntegralDescriptor const& src_integral_descriptor);

    bool SerializeListElemCnt(size_t list_elem_cnt);

    template <typename Integral>
    bool SerializeIntegral(Integral src_integral);

    bool TerminateNode();
};

struct DeserializationStateMachineBase {
    struct StateEnum {
        using Value = unsigned char;

        struct SendingNodeTag {
            static constexpr Value value{ 0 };
        };

        struct SendingObjTypeString {
            static constexpr Value value{ 1 };
        };

        struct SendingNameString {
            static constexpr Value value{ 2 };
        };

        struct SendingRegionAttr {
            static constexpr Value value{ 3 };
        };

        struct SendingIntegralDescriptor {
            static constexpr Value value{ 4 };
        };

        struct SendingListElemCnt {
            static constexpr Value value{ 5 };
        };

        struct SendingIntegral {
            static constexpr Value value{ 6 };
        };

        struct SendingTermination {
            static constexpr Value value{ 7 };
        };

        struct Corrupted {
            static constexpr Value value{ static_cast<unsigned char>(-1) };
        };
    };

    static constexpr StateEnum::Value FindNextState_(StateEnum::Value cur_state,
                                                     NodeTag const& node_tag);
};

template <typename Provider>
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

    bool DeserializeNodeTag(NodeTag& dst_node_tag);

    template <typename Accetpr>
    size_t DeserializeString(Accetpr&& acceptor, size_t max_str_size);

    bool DeserializeRegionAttr(unsigned long long& region_beg,
                               unsigned long long& region_size);

    bool DeserializeIntegralDescriptor(
        IntegralDescriptor& dst_integral_descriptor);

    bool DeserializeListElemCnt(size_t& list_elem_cnt);

    template <typename Integral>
    bool DeserializeIntegral(Integral& dst_integral);

    bool TerminateNode();
};

}  // namespace state_machine

}  // namespace zeta::core::object_state_notation
