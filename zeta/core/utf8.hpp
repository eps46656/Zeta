#pragma once

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

struct Encoder {
    struct EncodeResultEnum {
        using Value = unsigned char;

        struct Success {
            static constexpr Value value{ 0 };
        };

        struct CodepointOutOfRange {
            static constexpr Value value{ 1 };
        };

        struct CodepointIsSurrogate {
            static constexpr Value value{ 2 };
        };
    };

    bool has_pulled_codepoint;
    unsigned char encoded_octet_cnt;

    unicode::unichar_t pulled_codepoint;
    unsigned char encoded_octets[4];

    size_t acc_pulled_codepoint_cnt;
    size_t acc_encoded_octet_cnt;

    constexpr Encoder();

    template <typename Provider>
    Encoder::EncodeResultEnum::Value Encode(Provider&& provider);

    template <typename Acceptor>
    bool Push(Acceptor&& acceptor, size_t max_pushed_octet_cnt);

    template <typename Provider, typename Acceptor>
    bool EncodeAndPush(Provider&& provider, Acceptor&& acceptor,
                       size_t max_pulled_codepoint_cnt,
                       size_t max_pushed_octet_cnt);
};

struct Decoder {
    struct DecodeResultEnum {
        using Value = unsigned char;

        struct Success {
            static constexpr Value value{ 0 };
        };

        struct ProviderExhausted {
            static constexpr Value value{ 1 };
        };

        struct OctetOutOfRange {
            static constexpr Value value{ 2 };
        };

        struct LeadingOctetPatternMismatch {
            static constexpr Value value{ 3 };
        };

        struct TrailingOctetPatternMismatch {
            static constexpr Value value{ 4 };
        };

        struct OverlongEncoding {
            static constexpr Value value{ 5 };
        };

        struct CodepointIsSurrogate {
            static constexpr Value value{ 6 };
        };
    };

    unsigned char pulled_octet_cnt;
    bool has_decoded_codepoint;

    unsigned char pulled_octets[4];
    unicode::unichar_t decoded_codepoint;

    size_t acc_pulled_octet_cnt;
    size_t acc_decoded_codepoint_cnt;

    constexpr Decoder();

    template <typename Provider>
    DecodeResultEnum::Value Decode(Provider&& provider,
                                   size_t max_pulled_octet_cnt);

    template <typename Acceptor>
    bool Push(Acceptor&& acceptor);

    template <typename Provider, typename Acceptor>
    bool DecodeAndPush(Provider&& provider, Acceptor&& acceptor,
                       size_t max_pulled_octet_cnt,
                       size_t max_pushed_codepoint_cnt);
};

}  // namespace zeta::core::utf8
