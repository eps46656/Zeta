#pragma once

#include <zeta/core/comparison_utils.ipp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/unicode.hpp>
#include <zeta/core/utf8.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core {

namespace utf8::detail {

constexpr void CheckEncoder_(utf8::Encoder const& encoder) {
    ZETA_Core_DebugAssert(encoder.encoded_octet_cnt <=
                          sizeof(encoder.encoded_octets) /
                              sizeof(encoder.encoded_octets[0]));
}

}  // namespace utf8::detail

constexpr utf8::Encoder::Encoder()
    : encoded_octet_cnt{ 0 }, has_pulled_codepoint{ false } {}

template <elem_stream::provider::IsProvider Provider>
constexpr utf8::Encoder::ResultEnum utf8::Encoder::Encode(this Encoder& self,
                                                          Provider&& provider) {
    detail::CheckEncoder_(self);

    if (!self.has_pulled_codepoint) {
        if (elem_stream::provider::IsEnd(provider)) {
            return ResultEnum::ProviderExhausted;
        }

        elem_stream::provider::Transfer(provider, &self.pulled_codepoint,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);

        self.has_pulled_codepoint = true;
    }

    unsigned level{ 1U + (range_maxs[1] < self.pulled_codepoint) +
                    (range_maxs[2] < self.pulled_codepoint) +
                    (range_maxs[3] < self.pulled_codepoint) +
                    (range_maxs[4] < self.pulled_codepoint) };

    constexpr size_t encoded_octets_capabity{ sizeof(self.encoded_octets) /
                                              sizeof(self.encoded_octets[0]) };

    if (level == 5) { return ResultEnum::CodepointOutOfRange; }

    if (unicode::surrogate_range_min <= self.pulled_codepoint &&
        self.pulled_codepoint <= unicode::surrogate_range_max) {
        return ResultEnum::CodepointIsSurrogate;
    }

    if (encoded_octets_capabity - self.encoded_octet_cnt < level) {
        return ResultEnum::InsufficientEncodedBuffer;
    }

    unsigned char* cur_encoded_octet{ self.encoded_octets +
                                      self.encoded_octet_cnt };

    unicode::unichar_t codepoint{ self.pulled_codepoint };

    for (unsigned char* dst{ cur_encoded_octet + level - 1 };
         dst != cur_encoded_octet; --dst) {
        *dst = 0b1000'0000 + (codepoint % 64);
        codepoint /= 64;
    }

    switch (level) {
    case 1:
        ZETA_Core_DebugAssert(codepoint <= 0b0111'1111);

        cur_encoded_octet[0] = static_cast<unsigned char>(codepoint);
        break;
    case 2:
        ZETA_Core_DebugAssert(codepoint <= 0b0001'1111);

        cur_encoded_octet[0] =
            static_cast<unsigned char>(0b1100'0000 + codepoint);
        break;
    case 3:
        ZETA_Core_DebugAssert(codepoint <= 0b0000'1111);

        cur_encoded_octet[0] =
            static_cast<unsigned char>(0b1110'0000 + codepoint);
        break;
    case 4:
        ZETA_Core_DebugAssert(codepoint <= 0b0000'0111);

        cur_encoded_octet[0] =
            static_cast<unsigned char>(0b1111'0000 + codepoint);
        break;
    default: ZETA_Core_Unreachable();
    }

    switch (cur_encoded_octet[0] / 16) {
    case 0b1000:
    case 0b1001:
    case 0b1010:
    case 0b1011: ZETA_Core_DebugAssert(false);
    }

    self.has_pulled_codepoint = false;
    self.encoded_octet_cnt += static_cast<unsigned char>(level);

    return ResultEnum::Success;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr size_t utf8::Encoder::Push(this Encoder& self, Acceptor&& acceptor) {
    detail::CheckEncoder_(self);

    size_t pushed_octet_cnt{ 0 };

    for (; pushed_octet_cnt < self.encoded_octet_cnt &&
           !elem_stream::acceptor::IsEnd(acceptor);
         ++pushed_octet_cnt) {
        elem_stream::acceptor::Transfer(
            acceptor, self.encoded_octets + pushed_octet_cnt, 1, 1, 1);
    }

    if (0 < pushed_octet_cnt) {
        utils::MemMove(self.encoded_octets,
                       self.encoded_octets + pushed_octet_cnt,
                       self.encoded_octet_cnt - pushed_octet_cnt);

        self.encoded_octet_cnt -= pushed_octet_cnt;
    }

    return pushed_octet_cnt;
}

template <elem_stream::provider::IsProvider Provider,
          elem_stream::acceptor::IsAcceptor Acceptor>
constexpr bool utf8::Encoder::EncodeAndPush(this Encoder& self,
                                            Provider&& provider,
                                            Acceptor&& acceptor) {
    bool any_progress{ false };

    bool push_progress{ true };
    bool encode_progress{ true };

    for (;;) {
        push_progress = 0 < self.Push(acceptor);
        any_progress |= push_progress;
        if (!push_progress && !encode_progress) { break; }

        encode_progress = self.Encode(provider) == ResultEnum::Success;
        any_progress |= encode_progress;
        if (!push_progress && !encode_progress) { break; }
    }

    return any_progress;
}

namespace utf8::detail {

constexpr void CheckDecoder_(utf8::Decoder const& decoder) {
    ZETA_Core_DebugAssert(decoder.pulled_octet_cnt <= 4);
}

}  // namespace utf8::detail

constexpr utf8::Decoder::Decoder()
    : pulled_octet_cnt{ 0 }, has_decoded_codepoint{ false } {}

template <elem_stream::provider::IsProvider Provider>
constexpr utf8::Decoder::ResultEnum utf8::Decoder::Decode(this Decoder& self,
                                                          Provider&& provider) {
    detail::CheckDecoder_(self);

    if (self.has_decoded_codepoint) { return ResultEnum::HasDecodedCodepoint; }

    ResultEnum result{ ResultEnum::Success };

    unsigned pulled_octet_iter{ 0 };

    auto pull_octet{ [&]() -> unsigned char {
        if (pulled_octet_iter == self.pulled_octet_cnt) {
            if (elem_stream::provider::IsEnd(provider)) {
                result = ResultEnum::InsufficientOctet;
                return 0;
            }

            elem_stream::provider::Transfer(
                provider, self.pulled_octets + self.pulled_octet_cnt, 1, 1, 1);

            ++self.pulled_octet_cnt;
        }

        unsigned char ret{ self.pulled_octets[pulled_octet_iter++] };

        if (255 < ret) {
            result = ResultEnum::OctetOutOfRange;
            return 0;
        }

        return ret;
    } };

    unsigned char head{ pull_octet() };

    if (result != ResultEnum::Success) { return result; }

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
    default: return ResultEnum::LeadingOctetPatternMismatch;
    }

    for (unsigned i{ 1 }; i < level; ++i) {
        unsigned char octet{ pull_octet() };

        if (result != ResultEnum::Success) { return result; }

        if (octet / 64 != 0b10) {
            return ResultEnum::TrailingOctetPatternMismatch;
        }

        codepoint = codepoint * 64 + (octet - 0b1000'0000);
    }

    if (codepoint < range_mins[level] || range_maxs[level] < codepoint) {
        return ResultEnum::OverlongEncoding;
    }

    if (unicode::surrogate_range_min <= codepoint &&
        codepoint <= unicode::surrogate_range_max) {
        return ResultEnum::CodepointIsSurrogate;
    }

    self.has_decoded_codepoint = true;
    self.decoded_codepoint = codepoint;

    utils::MemMove(self.pulled_octets, self.pulled_octets + pulled_octet_iter,
                   self.pulled_octet_cnt - pulled_octet_iter);

    self.pulled_octet_cnt -= pulled_octet_iter;

    return ResultEnum::Success;
}

template <elem_stream::acceptor::IsAcceptor Acceptor>
constexpr bool utf8::Decoder::Push(this Decoder& self, Acceptor&& acceptor) {
    detail::CheckDecoder_(self);

    if (!self.has_decoded_codepoint || elem_stream::acceptor::IsEnd(acceptor)) {
        return false;
    }

    elem_stream::acceptor::Transfer(acceptor, &self.decoded_codepoint,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    self.has_decoded_codepoint = false;

    return true;
}

template <elem_stream::provider::IsProvider Provider,
          elem_stream::acceptor::IsAcceptor Acceptor>
constexpr bool utf8::Decoder::DecodeAndPush(this Decoder& self,
                                            Provider&& provider,
                                            Acceptor&& acceptor) {
    bool any_progress{ false };

    bool push_progress{ true };
    bool decode_progress{ true };

    for (;;) {
        push_progress = self.Push(acceptor);
        any_progress |= push_progress;
        if (!push_progress && !decode_progress) { break; }

        decode_progress = self.Decode(provider) == ResultEnum::Success;
        any_progress |= decode_progress;
        if (!push_progress && !decode_progress) { break; }
    }

    return any_progress;
}

}  // namespace zeta::core
