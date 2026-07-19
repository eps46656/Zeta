#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/unicode.hpp>
#include <zeta/core/utf8.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

namespace utf8::detail {

inline void CheckEncoder_(utf8::Encoder const& encoder) {
    ZETA_Core_DebugAssert(encoder.encoded_octet_cnt <= 4);
}

}  // namespace utf8::detail

inline constexpr utf8::Encoder::Encoder()
    : has_pulled_codepoint{ false },
      encoded_octet_cnt{ 0 },
      pulled_codepoint{ 0 },
      encoded_octets{ 0, 0, 0, 0 },
      acc_pulled_codepoint_cnt{ 0 },
      acc_encoded_octet_cnt{ 0 } {}

template <typename Provider>
utf8::Encoder::EncodeResultEnum::Value utf8::Encoder::Encode(
    Provider&& provider) {
    detail::CheckEncoder_(*this);

    if (!this->has_pulled_codepoint) {
        provider(&this->pulled_codepoint, 0, 1);
        this->has_pulled_codepoint = true;
        ++this->acc_pulled_codepoint_cnt;
    }

    unsigned level{ 1U + (range_maxs[1] < this->pulled_codepoint) +
                    (range_maxs[2] < this->pulled_codepoint) +
                    (range_maxs[3] < this->pulled_codepoint) +
                    (range_maxs[4] < this->pulled_codepoint) };

    if (level == 5) { return EncodeResultEnum::CodepointOutOfRange::value; }

    if (unicode::surrogate_range_min <= this->pulled_codepoint &&
        this->pulled_codepoint <= unicode::surrogate_range_max) {
        return EncodeResultEnum::CodepointIsSurrogate::value;
    }

    unicode::unichar_t codepoint{ this->pulled_codepoint };

    for (unsigned char* dst{ this->encoded_octets + level - 1 };
         dst != this->encoded_octets; --dst) {
        *dst = 0b1000'0000 + (codepoint % 64);
        codepoint /= 64;
    }

    switch (level) {
    case 1:
        ZETA_Core_DebugAssert(codepoint <= 0b0111'1111);

        this->encoded_octets[0] = static_cast<unsigned char>(codepoint);
        break;
    case 2:
        ZETA_Core_DebugAssert(codepoint <= 0b0001'1111);

        this->encoded_octets[0] =
            static_cast<unsigned char>(0b1100'0000 + codepoint);
        break;
    case 3:
        ZETA_Core_DebugAssert(codepoint <= 0b0000'1111);

        this->encoded_octets[0] =
            static_cast<unsigned char>(0b1110'0000 + codepoint);
        break;
    case 4:
        ZETA_Core_DebugAssert(codepoint <= 0b0000'0111);

        this->encoded_octets[0] =
            static_cast<unsigned char>(0b1111'0000 + codepoint);
        break;
    default: ZETA_Core_Unreachable();
    }

    switch (this->encoded_octets[0] / 16) {
    case 0b1000:
    case 0b1001:
    case 0b1010:
    case 0b1011: ZETA_Core_DebugAssert(false);
    }

    this->has_pulled_codepoint = false;
    this->encoded_octet_cnt = static_cast<unsigned char>(level);
    this->acc_encoded_octet_cnt += level;

    return EncodeResultEnum::Success::value;
}

template <typename Acceptor>
bool utf8::Encoder::Push(Acceptor&& acceptor, size_t max_pushed_octet_cnt) {
    detail::CheckEncoder_(*this);

    if (this->encoded_octet_cnt == 0 ||
        max_pushed_octet_cnt < this->encoded_octet_cnt) {
        return false;
    }

    acceptor(this->encoded_octets, 1, this->encoded_octet_cnt);

    this->encoded_octet_cnt = 0;

    return true;
}

template <typename Provider, typename Acceptor>
bool utf8::Encoder::EncodeAndPush(Provider&& provider, Acceptor&& acceptor,
                                  size_t max_pulled_codepoint_cnt,
                                  size_t max_pushed_octet_cnt) {
    bool any_progress{ false };

    bool push_progress{ true };
    bool encode_progress{ true };

    for (;;) {
        size_t old_encoded_octet_cnt{ this->encoded_octet_cnt };
        push_progress = this->Push(acceptor, max_pushed_octet_cnt);
        if (push_progress) { max_pushed_octet_cnt -= old_encoded_octet_cnt; }
        any_progress |= push_progress;
        if (!push_progress && !encode_progress) { break; }

        encode_progress =
            this->encoded_octet_cnt == 0 && 0 < max_pulled_codepoint_cnt &&
            this->Encode(provider) == EncodeResultEnum::Success::value;
        if (encode_progress) { --max_pulled_codepoint_cnt; }
        any_progress |= encode_progress;
        if (!push_progress && !encode_progress) { break; }
    }

    return any_progress;
}

namespace utf8::detail {

inline void CheckDecoder_(utf8::Decoder const& decoder) {
    ZETA_Core_DebugAssert(decoder.pulled_octet_cnt <= 4);
}

}  // namespace utf8::detail

inline constexpr utf8::Decoder::Decoder()
    : pulled_octet_cnt{ 0 },
      has_decoded_codepoint{ false },
      pulled_octets{ 0, 0, 0, 0 },
      decoded_codepoint{ 0 },
      acc_pulled_octet_cnt{ 0 },
      acc_decoded_codepoint_cnt{ 0 } {}

template <typename Provider>
utf8::Decoder::DecodeResultEnum::Value utf8::Decoder::Decode(
    Provider&& provider, size_t max_pulled_octet_cnt) {
    detail::CheckDecoder_(*this);

    DecodeResultEnum::Value result{ DecodeResultEnum::Success::value };

    unsigned pulled_octet_iter{ 0 };

    auto pull_octet{ [&]() -> unsigned char {
        if (pulled_octet_iter == this->pulled_octet_cnt) {
            if (max_pulled_octet_cnt == 0) {
                result = DecodeResultEnum::ProviderExhausted::value;
                return 0;
            }

            provider(this->pulled_octets + this->pulled_octet_cnt, 0, 1);
            --max_pulled_octet_cnt;
            ++this->pulled_octet_cnt;
            ++this->acc_pulled_octet_cnt;
        }

        unsigned char ret{ this->pulled_octets[pulled_octet_iter++] };

        if (255 < ret) {
            result = DecodeResultEnum::OctetOutOfRange::value;
            return 0;
        }

        return ret;
    } };

    unsigned char head{ pull_octet() };

    if (result != DecodeResultEnum::Success::value) { return result; }

    unsigned level;

    unicode::unichar_t codepoint{ head };

    switch (head / 16) {
    case 0b0000:
    case 0b0001:
    case 0b0010:
    case 0b0011:
    case 0b0100:
    case 0b0101:
    case 0b0110:
    case 0b0111: level = 1; break;

    case 0b1100:
    case 0b1101:
        level = 2;
        codepoint -= 0b1100'0000;
        break;

    case 0b1110:
        level = 3;
        codepoint -= 0b1110'0000;
        break;

    case 0b1111:
        if (codepoint <= 0b1111'0111) {
            level = 4;
            codepoint -= 0b1111'0000;
            break;
        }

        [[fallthrough]];
    default: return DecodeResultEnum::LeadingOctetPatternMismatch::value;
    }

    for (unsigned i{ 1 }; i < level; ++i) {
        unsigned char octet{ pull_octet() };

        if (result != DecodeResultEnum::Success::value) { return result; }

        if (octet / 64 != 0b10) {
            return DecodeResultEnum::TrailingOctetPatternMismatch::value;
        }

        codepoint = codepoint * 64 + (octet - 0b1000'0000);
    }

    if (codepoint < range_mins[level] || range_maxs[level] < codepoint) {
        return DecodeResultEnum::OverlongEncoding::value;
    }

    if (unicode::surrogate_range_min <= codepoint &&
        codepoint <= unicode::surrogate_range_max) {
        return DecodeResultEnum::CodepointIsSurrogate::value;
    }

    this->has_decoded_codepoint = true;
    this->decoded_codepoint = codepoint;
    ++this->acc_decoded_codepoint_cnt;

    utils::MemMove(this->pulled_octets, this->pulled_octets + pulled_octet_iter,
                   this->pulled_octet_cnt - pulled_octet_iter);

    this->pulled_octet_cnt -= pulled_octet_iter;

    return DecodeResultEnum::Success::value;
}

template <typename Acceptor>
bool utf8::Decoder::Push(Acceptor&& acceptor) {
    detail::CheckDecoder_(*this);

    if (!this->has_decoded_codepoint) { return false; }

    acceptor(&this->decoded_codepoint, 0, 1);

    this->has_decoded_codepoint = false;

    return true;
}

template <typename Provider, typename Acceptor>
bool utf8::Decoder::DecodeAndPush(Provider&& provider, Acceptor&& acceptor,
                                  size_t max_pulled_octet_cnt,
                                  size_t max_pushed_codepoint_cnt) {
    bool any_progress{ false };

    bool push_progress{ true };
    bool decode_progress{ true };

    for (;;) {
        push_progress = 0 < max_pushed_codepoint_cnt && this->Push(acceptor);
        if (push_progress) { --max_pushed_codepoint_cnt; }
        any_progress |= push_progress;
        if (!push_progress && !decode_progress) { break; }

        size_t old_acc_fetched_octet_cnt{ this->acc_pulled_octet_cnt };
        decode_progress = !this->has_decoded_codepoint &&
                          this->Decode(provider, max_pulled_octet_cnt) ==
                              DecodeResultEnum::Success::value;
        max_pulled_octet_cnt -=
            this->acc_pulled_octet_cnt - old_acc_fetched_octet_cnt;
        any_progress |= decode_progress;
        if (!push_progress && !decode_progress) { break; }
    }

    return any_progress;
}

}  // namespace zeta::core
