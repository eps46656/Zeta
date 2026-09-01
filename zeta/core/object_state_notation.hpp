#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/elem_stream.hpp>
#include <zeta/core/integral.hpp>
#include <zeta/core/meta.hpp>
#include <zeta/core/pair.hpp>
#include <zeta/core/unicode.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::object_state_notation {

enum struct NodeType : unsigned char {
    Null = 0,
    Integral = 1,
    IntegralList = 2,
    NodeList = 3,
    Terminator = 4,
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
    NodeType node_type;
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
constexpr bool EncodeHeaderToOctets(Acceptor&& acceptor,
                                    Header const& src_header);

template <elem_stream::provider::IsProvider Provider>
constexpr bool DecodeHeaderFromOctets(Provider&& provider, Header& dst_header);

enum struct EncoderState : unsigned char {
    ReceivingNodeTag = 0,
    ReceivingNodeTagOrFinish = 1,
    ReceivingNameString = 2,
    ReceivingObjTypeString = 3,
    ReceivingRegionAttr = 4,
    ReceivingIntegralDescriptor = 5,
    ReceivingListElemCnt = 6,
    ReceivingIntegral = 7,
    ReceivingIntegralOrFinish = 8,
    ReceivingFinish = 9,
    Finished = 10,
    Corrupted = static_cast<unsigned char>(-1),
};

template <elem_stream::acceptor::IsAcceptor Acceptor>
struct Encoder {
    static constexpr unsigned max_depth{ 32 };

    Config config;

    EncoderState state;

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

    constexpr Encoder(Config const& config,
                      unsigned char* integral_chunk_buffer_data,
                      unsigned short integral_chunk_buffer_max_octet_cnt,
                      Acceptor& acceptor);

    constexpr utils::TryResult<meta::Monostate, meta::Monostate> SendNodeTag(
        this Encoder& self, NodeTag const& src_node_tag);

    template <elem_stream::provider::IsProvider Provider>
    constexpr utils::TryResult<meta::Monostate, meta::Monostate>
    SendStringOctet(this Encoder& self, Provider&& provider, size_t size);

    template <integral::IsIntegral Integral>
    constexpr utils::TryResult<meta::Monostate, meta::Monostate> SendRegionAttr(
        this Encoder& self, Integral region_beg, Integral region_size);

    constexpr utils::TryResult<meta::Monostate, meta::Monostate>
    SendIntegralDescriptor(this Encoder& self,
                           IntegralDescriptor const& src_integral_descriptor);

    constexpr utils::TryResult<meta::Monostate, meta::Monostate>
    SendListElemCnt(this Encoder& self, size_t list_elem_cnt);

    template <integral::IsIntegral Integral>
    constexpr utils::TryResult<meta::Monostate, meta::Monostate> SendIntegral(
        this Encoder& self, Integral src_integral);

    constexpr utils::TryResult<meta::Monostate, meta::Monostate> SendFinish(
        this Encoder& self);
};

enum struct DecoderState : unsigned char {
    SendingNodeTag = 0,
    SendingNameString = 2,
    SendingObjTypeString = 3,
    SendingRegionAttr = 4,
    SendingIntegralDescriptor = 5,
    SendingListElemCnt = 6,
    SendingIntegral = 7,
    SendingFinish = 9,
    Finished = 10,
    Corrupted = static_cast<unsigned char>(-1),
};

template <elem_stream::provider::IsProvider Provider>
struct Decoder {
    static constexpr unsigned max_depth{ 32 };

    Config config;

    DecoderState state;

    unsigned short depth;

    size_t res_elem_cnts[max_depth];

    bool has_buffer_node_tag;
    NodeTag buffer_node_tag;

    IntegralDescriptor integral_descriptor_buffer;

    unsigned char integral_chunk_res_elem_cnt;

    Provider& provider;

    constexpr Decoder(Config const& config, Provider& provider);

    constexpr utils::TryResult<NodeTag, meta::Monostate> ReceiveNodeTag(
        this Decoder& self);

    template <elem_stream::acceptor::IsAcceptor Accetpr>
    constexpr utils::TryResult<unicode::unichar_t, meta::Monostate>
    ReceiveStringChar(this Decoder& self);

    template <integral::IsIntegral Integral>
    constexpr utils::TryResult<pair::Pair<Integral, Integral>, meta::Monostate>
    ReceiveRegionAttr(this Decoder& self);

    constexpr utils::TryResult<IntegralDescriptor, meta::Monostate>
    ReceiveIntegralDescriptor(this Decoder& self);

    constexpr utils::TryResult<size_t, meta::Monostate> ReceiveListElemCnt(
        this Decoder& self);

    template <integral::IsIntegral Integral>
    constexpr utils::TryResult<Integral, meta::Monostate> ReceiveIntegral(
        this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, meta::Monostate> ReceiveFinish(
        this Decoder& self);
};

}  // namespace zeta::core::object_state_notation
