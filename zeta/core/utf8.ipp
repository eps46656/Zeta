#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/elem_stream.ipp>
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

template <elem_stream::provider::IsProvider Provider>
utf8::Encoder::EncodeResultEnum::Value utf8::Encoder::Encode(
    this Encoder& self, Provider&& provider) {
    detail::CheckEncoder_(self);

    if (!self.has_pulled_codepoint) {
        elem_stream::provider::Transfer(provider, &self.pulled_codepoint,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);

        self.has_pulled_codepoint = true;
        ++self.acc_pulled_codepoint_cnt;
    }

    unsigned level{ 1U + (range_maxs[1] < self.pulled_codepoint) +
                    (range_maxs[2] < self.pulled_codepoint) +
                    (range_maxs[3] < self.pulled_codepoint) +
                    (range_maxs[4] < self.pulled_codepoint) };

    if (level == 5) { return EncodeResultEnum::CodepointOutOfRange::value; }

    if (unicode::surrogate_range_min <= self.pulled_codepoint &&
        self.pulled_codepoint <= unicode::surrogate_range_max) {
        return EncodeResultEnum::CodepointIsSurrogate::value;
    }

    unicode::unichar_t codepoint{ self.pulled_codepoint };

    for (unsigned char* dst{ self.encoded_octets + level - 1 };
         dst != self.encoded_octets; --dst) {
        *dst = 0b1000'0000 + (codepoint % 64);
        codepoint /= 64;
    }

    switch (level) {
    case 1:
        ZETA_Core_DebugAssert(codepoint <= 0b0111'1111);

        self.encoded_octets[0] = static_cast<unsigned char>(codepoint);
        break;
    case 2:
        ZETA_Core_DebugAssert(codepoint <= 0b0001'1111);

        self.encoded_octets[0] =
            static_cast<unsigned char>(0b1100'0000 + codepoint);
        break;
    case 3:
        ZETA_Core_DebugAssert(codepoint <= 0b0000'1111);

        self.encoded_octets[0] =
            static_cast<unsigned char>(0b1110'0000 + codepoint);
        break;
    case 4:
        ZETA_Core_DebugAssert(codepoint <= 0b0000'0111);

        self.encoded_octets[0] =
            static_cast<unsigned char>(0b1111'0000 + codepoint);
        break;
    default: ZETA_Core_Unreachable();
    }

    switch (self.encoded_octets[0] / 16) {
    case 0b1000:
    case 0b1001:
    case 0b1010:
    case 0b1011: ZETA_Core_DebugAssert(false);
    }

    self.has_pulled_codepoint = false;
    self.encoded_octet_cnt = static_cast<unsigned char>(level);
    self.acc_encoded_octet_cnt += level;

    return EncodeResultEnum::Success::value;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool utf8::Encoder::Push(this Encoder& self, Acceptor&& acceptor,
                         size_t max_pushed_octet_cnt) {
    detail::CheckEncoder_(self);

    if (self.encoded_octet_cnt == 0 ||
        max_pushed_octet_cnt < self.encoded_octet_cnt) {
        return false;
    }

    elem_stream::acceptor::Transfer(acceptor, self.encoded_octets, 1, 1,
                                    self.encoded_octet_cnt);

    self.encoded_octet_cnt = 0;

    return true;
}

template <elem_stream::provider::IsProvider Provider,
          elem_stream::acceptor::IsAcceptor Acceptor>
bool utf8::Encoder::EncodeAndPush(this Encoder& self, Provider&& provider,
                                  Acceptor&& acceptor,
                                  size_t max_pulled_codepoint_cnt,
                                  size_t max_pushed_octet_cnt) {
    bool any_progress{ false };

    bool push_progress{ true };
    bool encode_progress{ true };

    for (;;) {
        size_t old_encoded_octet_cnt{ self.encoded_octet_cnt };
        push_progress = self.Push(acceptor, max_pushed_octet_cnt);
        if (push_progress) { max_pushed_octet_cnt -= old_encoded_octet_cnt; }
        any_progress |= push_progress;
        if (!push_progress && !encode_progress) { break; }

        encode_progress =
            self.encoded_octet_cnt == 0 && 0 < max_pulled_codepoint_cnt &&
            self.Encode(provider) == EncodeResultEnum::Success::value;
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

template <elem_stream::provider::IsProvider Provider>
utf8::Decoder::DecodeResultEnum::Value utf8::Decoder::Decode(
    this Decoder& self, Provider&& provider, size_t max_pulled_octet_cnt) {
    detail::CheckDecoder_(self);

    DecodeResultEnum::Value result{ DecodeResultEnum::Success::value };

    unsigned pulled_octet_iter{ 0 };

    auto pull_octet{ [&]() -> unsigned char {
        if (pulled_octet_iter == self.pulled_octet_cnt) {
            if (max_pulled_octet_cnt == 0) {
                result = DecodeResultEnum::ProviderExhausted::value;
                return 0;
            }

            elem_stream::provider::Transfer(
                provider, self.pulled_octets + self.pulled_octet_cnt, 1, 1, 1);

            --max_pulled_octet_cnt;
            ++self.pulled_octet_cnt;
            ++self.acc_pulled_octet_cnt;
        }

        unsigned char ret{ self.pulled_octets[pulled_octet_iter++] };

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

    self.has_decoded_codepoint = true;
    self.decoded_codepoint = codepoint;
    ++self.acc_decoded_codepoint_cnt;

    utils::MemMove(self.pulled_octets, self.pulled_octets + pulled_octet_iter,
                   self.pulled_octet_cnt - pulled_octet_iter);

    self.pulled_octet_cnt -= pulled_octet_iter;

    return DecodeResultEnum::Success::value;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
bool utf8::Decoder::Push(this Decoder& self, Acceptor&& acceptor) {
    detail::CheckDecoder_(self);

    if (!self.has_decoded_codepoint) { return false; }

    elem_stream::acceptor::Transfer(acceptor, &self.decoded_codepoint,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    self.has_decoded_codepoint = false;

    return true;
}

template <elem_stream::provider::IsProvider Provider,
          elem_stream::acceptor::IsAcceptor Acceptor>
bool utf8::Decoder::DecodeAndPush(this Decoder& self, Provider&& provider,
                                  Acceptor&& acceptor,
                                  size_t max_pulled_octet_cnt,
                                  size_t max_pushed_codepoint_cnt) {
    bool any_progress{ false };

    bool push_progress{ true };
    bool decode_progress{ true };

    for (;;) {
        push_progress = 0 < max_pushed_codepoint_cnt && self.Push(acceptor);
        if (push_progress) { --max_pushed_codepoint_cnt; }
        any_progress |= push_progress;
        if (!push_progress && !decode_progress) { break; }

        size_t old_acc_fetched_octet_cnt{ self.acc_pulled_octet_cnt };
        decode_progress = !self.has_decoded_codepoint &&
                          self.Decode(provider, max_pulled_octet_cnt) ==
                              DecodeResultEnum::Success::value;
        max_pulled_octet_cnt -=
            self.acc_pulled_octet_cnt - old_acc_fetched_octet_cnt;
        any_progress |= decode_progress;
        if (!push_progress && !decode_progress) { break; }
    }

    return any_progress;
}

}  // namespace zeta::core
