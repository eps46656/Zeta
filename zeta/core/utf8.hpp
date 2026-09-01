#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/unicode.hpp>

namespace zeta::core::utf8 {

constexpr unicode::unichar_t range_mins[]{
    0x0000,   // 0
    0x0000,   // 1
    0x0080,   // 2
    0x0800,   // 3
    0x10000,  // 4
};

constexpr unicode::unichar_t range_maxs[]{
    0x0000,    // 0
    0x007F,    // 1
    0x07FF,    // 2
    0xFFFF,    // 3
    0x10FFFF,  // 4
};

constexpr unsigned EvaluateLevelFromCodepoint(unicode::unichar_t codepoint);

constexpr unsigned EvaluateLevelFromLeadingOctet(unsigned char leading_octet);

struct EncodeResult {
    enum struct ResultType : unsigned char {
        Success = 0,
        CodepointOutOfRange = 1,
        CodepointIsSurrogate = 2,
    };

    ResultType type;

    unsigned char encoded_octet_cnt : 3;
    unsigned char encoded_octets[4];
};

constexpr EncodeResult Encode(unicode::unichar_t codepoint);

struct DecodeResult {
    enum struct ResultType : unsigned char {
        Success = 0,
        InsufficientOctet = 1,
        OctetOutOfRange = 2,
        LeadingOctetPatternMismatch = 3,
        TrailingOctetPatternMismatch = 4,
        OverlongEncoding = 5,
        CodepointIsSurrogate = 6,
    };

    ResultType type;

    unicode::unichar_t codepoint;
    unsigned char consumed_octet_cnt;
};

constexpr DecodeResult Decode(unsigned char const* octets, size_t octet_cnt);

struct Encoder {
    enum struct ResultEnum : unsigned char {
        Success = 0,
        ProviderExhausted = 1,
        InsufficientEncodedBuffer = 2,
        CodepointOutOfRange = 3,
        CodepointIsSurrogate = 4,
    };

    unsigned char encoded_octet_cnt : 3;
    bool has_pulled_codepoint : 1;

    unsigned char encoded_octets[7];
    unicode::unichar_t pulled_codepoint;

    constexpr Encoder();

    template <elem_stream::provider::IsProvider Provider>
    constexpr ResultEnum Encode(this Encoder& self, Provider&& provider);

    template <elem_stream::acceptor::IsAcceptor Acceptor>
    constexpr size_t Push(this Encoder& self, Acceptor&& acceptor);

    template <elem_stream::provider::IsProvider Provider,
              elem_stream::acceptor::IsAcceptor Acceptor>
    constexpr bool EncodeAndPush(this Encoder& self, Provider&& provider,
                                 Acceptor&& acceptor);
};

struct Decoder {
    enum struct ResultEnum : unsigned char {
        Success = 0,
        InsufficientOctet = 1,
        HasDecodedCodepoint = 2,
        OctetOutOfRange = 3,
        LeadingOctetPatternMismatch = 4,
        TrailingOctetPatternMismatch = 5,
        OverlongEncoding = 6,
        CodepointIsSurrogate = 7,
    };

    unsigned char pulled_octet_cnt;
    bool has_decoded_codepoint;

    unsigned char pulled_octets[4];
    unicode::unichar_t decoded_codepoint;

    constexpr Decoder();

    template <elem_stream::provider::IsProvider Provider>
    constexpr ResultEnum Decode(this Decoder& self, Provider&& provider);

    template <elem_stream::acceptor::IsAcceptor Acceptor>
    constexpr bool Push(this Decoder& self, Acceptor&& acceptor);

    template <elem_stream::provider::IsProvider Provider,
              elem_stream::acceptor::IsAcceptor Acceptor>
    constexpr bool DecodeAndPush(this Decoder& self, Provider&& provider,
                                 Acceptor&& acceptor);
};

}  // namespace zeta::core::utf8
