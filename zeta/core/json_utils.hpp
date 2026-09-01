#pragma once

#include <zeta/core/elem_stream.hpp>
#include <zeta/core/unicode.hpp>
#include <zeta/core/utils.hpp>

namespace zeta::core::json_utils {

struct FormatConfig {
    struct Newline {
        enum struct Type : unsigned char {
            None = 0,
            LF = 1,
            CRLF = 2,
        };

        Type type;
    } newline;

    struct {
        bool before_colon;
        bool after_colon;

        bool before_comma;
        bool after_comma;
    } space;

    struct Indent {
        enum struct Type : unsigned char {
            None = 0,
            Space = 1,
            Tab = 2,
        };

        Type type;
        unsigned char cnt;
    } indent;

    struct String {
        bool prefer_unicode_escape;
        bool prefer_uppercase_hex;
    } string;
};

struct FormatContext {
    FormatConfig config;
    unsigned int indent_level;
};

enum struct EncInnerState : unsigned char {
    ReceivingValue = 0,
    // SendSign -> ReceivingNumericIntPartDigit

    ReceivingNumericIntPartDigit = 31,
    // SendIntPartDigit
    //   if 0: -> ReceivingNumericFracPartDigitOrNext
    //   else: -> ReceivingNumericIntPartDigitOrNext

    ReceivingNumericIntPartDigitOrNext = 32,
    // SendIntPartDigit -> ReceivingNumericIntPartDigitOrNext
    // SendFracPartDigit -> ReceivingNumericFracPartDigitOrNext
    // SendExpPartE -> ReceivingNumericExpPartSignOrNext
    // SendFinish -> Finished

    ReceivingNumericFracPartDigitOrNext = 33,
    // SendFracPartDigit -> ReceivingNumericFracPartDigitOrNext
    // SendExpPartE -> ReceivingNumericExpPartSignOrNext
    // SendFinish -> Finished

    ReceivingNumericExpPartSignOrNext = 34,
    // SendExpPartSign -> ReceivingNumericExpPartDigit
    // SendExpPartDigit -> ReceivingNumericExpPartDigitOrNext

    ReceivingNumericExpPartDigit = 35,
    // SendExpPartDigit -> ReceivingNumericExpPartDigitOrNext

    ReceivingNumericExpPartDigitOrNext = 36,
    // SendExpPartDigit -> ReceivingNumericExpPartDigitOrNext
    // SendFinish -> Finished

    ReceivingStringCharOrFinish = 40,

    ReceivingArrayElemOrFinishLead = 50,
    ReceivingArrayElemOrFinishTrail = 51,

    ReceivingObjectKeyOrFinishLead = 60,
    ReceivingObjectKeyOrFinishTrail = 61,
    ReceivingObjectValue = 62,

    Finished = 127,
};

enum struct EncState : unsigned char {
    ReceivingValue = meta::ToUnderlying(EncInnerState::ReceivingValue),

    ReceivingNumericIntPartDigit =
        meta::ToUnderlying(EncInnerState::ReceivingNumericIntPartDigit),

    ReceivingNumericIntPartDigitOrNext =
        meta::ToUnderlying(EncInnerState::ReceivingNumericIntPartDigitOrNext),

    ReceivingNumericFracPartDigitOrNext =
        meta::ToUnderlying(EncInnerState::ReceivingNumericFracPartDigitOrNext),

    ReceivingNumericExpPartSignOrNext =
        meta::ToUnderlying(EncInnerState::ReceivingNumericExpPartSignOrNext),

    ReceivingNumericExpPartDigit =
        meta::ToUnderlying(EncInnerState::ReceivingNumericExpPartDigit),

    ReceivingNumericExpPartDigitOrNext =
        meta::ToUnderlying(EncInnerState::ReceivingNumericExpPartDigitOrNext),

    ReceivingStringCharOrFinish = 40,

    ReceivingArrayElemOrFinish =
        meta::ToUnderlying(EncInnerState::ReceivingArrayElemOrFinishLead),

    ReceivingObjectKeyOrFinish =
        meta::ToUnderlying(EncInnerState::ReceivingObjectKeyOrFinishLead),

    ReceivingObjectValue =
        meta::ToUnderlying(EncInnerState::ReceivingObjectValue),

    Finished = meta::ToUnderlying(EncInnerState::Finished),
};

enum struct EncReason : unsigned char {
    UnexpectedChar = 1,
};

template <typename CodepointAcceptorLike>
struct Encoder {
    static constexpr size_t max_depth{ 64 };

    EncInnerState states[max_depth];

    size_t depth;

    CodepointAcceptorLike cpa_like;

    FormatContext fmt_ctx;

    template <typename CodeAcceptorLikeInitArg>
    constexpr Encoder(CodeAcceptorLikeInitArg&& cpa_like_init_arg,
                      FormatConfig const& fmt_config);

    constexpr EncState GetState(this Encoder& self);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendNull(
        this Encoder& self);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendBoolean(
        this Encoder& self, bool value);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendNumericSign(
        this Encoder& self, bool is_neg);

    constexpr utils::TryResult<meta::Monostate, EncReason>
    SendNumericIntPartDigit(this Encoder& self, unsigned char digit);

    constexpr utils::TryResult<meta::Monostate, EncReason>
    SendNumericFracPartDigit(this Encoder& self, unsigned char digit);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendNumericExpPartE(
        this Encoder& self, unicode::unichar_t e);

    constexpr utils::TryResult<meta::Monostate, EncReason>
    SendNumericExpPartSign(this Encoder& self, unicode::unichar_t sign);

    constexpr utils::TryResult<meta::Monostate, EncReason>
    SendNumericExpPartDigit(this Encoder& self, unsigned char digit);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendStringStart(
        this Encoder& self);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendStringChar(
        this Encoder& self, unicode::unichar_t cp);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendArrayStart(
        this Encoder& self);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendObjectStart(
        this Encoder& self);

    constexpr utils::TryResult<meta::Monostate, EncReason> SendFinish(
        this Encoder& self);
};

enum struct DecInnerState : unsigned char {
    SendingNull = 10,

    SendingBoolean = 20,

    SendingNumericSign = 30,
    // ReceiveNumericSign
    //   if "-": SendingNumericIntPartDigitLead
    //   if "0" ~ "9": revert, to SendingNumericFracPartDigit
    //   else: Corrupted

    SendingNumericIntPartDigitLead = 31,
    // ReceiveNumericIntPartDigit
    //   if "0": SendingNumericIntPartDigitFinish
    //   if "1" ~ "9": SendingNumericIntPartDigitTrail
    //   else: Corrupted

    SendingNumericIntPartDigitTrail = 32,
    // ReceiveNumericIntPartDigit
    //   if "0" ~ "9": SendingNumericIntPartDigitTrail
    //   if ".": SendingNumericFracPartDigit
    //   if "e" or "E": SendingNumericExpPartE
    //   if terminator: SendingNumericFinish
    //   else: Corrupted

    SendingNumericIntPartDigitFinish = 33,
    // ReceiveNumericIntPartDigit
    //   if ".": SendingNumericFracPartDigit
    //   if "e" or "E": SendingNumericExpPartE
    //   if terminator: SendingNumericFinish
    //   else: Corrupted

    SendingNumericFracPartDigit = 34,
    // ReceiveNumericFracPartDigit
    //   if "0" ~ "9": SendingNumericFracPartDigit
    //   if "e" or "E": SendingNumericExpPartE
    //   if terminator: SendingNumericFinish
    //   else: Corrupted

    SendingNumericExpPartE = 35,
    // ReceiveNumericExpPartSign
    //   if "e" or "E": SendingNumericExpPartSign, always

    SendingNumericExpPartSign = 36,
    // ReceiveNumericExpPartDigit
    //   if "+" or "-": SendingNumericExpPartDigitLead
    //   if "0" ~ "9": SendingNumericExpPartDigitLead
    //   else: Corrupted

    SendingNumericExpPartDigitLead = 37,
    // ReceiveNumericExpPartDigit
    //   if "0" ~ "9": SendingNumericExpPartDigitTrail
    //   else: Corrupted

    SendingNumericExpPartDigitTrail = 38,
    // ReceiveNumericExpPartDigit
    //   if "0" ~ "9": SendingNumericExpPartDigitTrail
    //   if terminator: SendingNumericFinish
    //   else: Corrupted

    SendingNumericFinish = 39,

    SendingStringStart = 40,
    SendingStringChar = 41,
    SendingStringFinish = 42,

    SendingArrayStart = 50,
    SendingArrayElemLead = 51,
    SendingArrayElemTrail = 52,
    SendingArrayFinish = 53,

    SendingObjectStart = 60,
    SendingObjectKeyLead = 61,
    SendingObjectKeyTrail = 62,
    SendingObjectValue = 63,
    SendingObjectFinish = 64,

    Finished = 127,

    Corrupted = 255,
};

enum struct DecState : unsigned char {
    SendingNull = meta::ToUnderlying(DecInnerState::SendingNull),

    SendingBoolean = meta::ToUnderlying(DecInnerState::SendingBoolean),

    SendingNumericSign = meta::ToUnderlying(DecInnerState::SendingNumericSign),

    SendingNumericIntPartDigit =
        meta::ToUnderlying(DecInnerState::SendingNumericIntPartDigitLead),

    SendingNumericFracPartDigit =
        meta::ToUnderlying(DecInnerState::SendingNumericFracPartDigit),

    SendingNumericExpPartE =
        meta::ToUnderlying(DecInnerState::SendingNumericExpPartE),

    SendingNumericExpPartSign =
        meta::ToUnderlying(DecInnerState::SendingNumericExpPartSign),

    SendingNumericExpPartDigit =
        meta::ToUnderlying(DecInnerState::SendingNumericExpPartDigitLead),

    SendingNumericFinish =
        meta::ToUnderlying(DecInnerState::SendingNumericFinish),

    SendingStringStart = meta::ToUnderlying(DecInnerState::SendingStringStart),
    SendingStringChar = meta::ToUnderlying(DecInnerState::SendingStringChar),
    SendingStringFinish =
        meta::ToUnderlying(DecInnerState::SendingStringFinish),

    SendingArrayStart = meta::ToUnderlying(DecInnerState::SendingArrayStart),
    SendingArrayElem = meta::ToUnderlying(DecInnerState::SendingArrayElemLead),
    SendingArrayFinish = meta::ToUnderlying(DecInnerState::SendingArrayFinish),

    SendingObjectStart = meta::ToUnderlying(DecInnerState::SendingObjectStart),
    SendingObjectKey = meta::ToUnderlying(DecInnerState::SendingObjectKeyLead),
    SendingObjectValue = meta::ToUnderlying(DecInnerState::SendingObjectValue),
    SendingObjectFinish =
        meta::ToUnderlying(DecInnerState::SendingObjectFinish),

    Finished = meta::ToUnderlying(DecInnerState::Finished),

    Corrupted = meta::ToUnderlying(DecInnerState::Corrupted),
};

enum struct DecReason : unsigned char {
    UnexpectedEnd = 0,
    UnexpectedChar = 1,
    NoValue = 2,
    SequenceEnd = 3,
    DepthOverflow = 4,
};

enum BufferState : unsigned char {
    Empty = 0,
    HasCodepointFetched = 1,
    HasCodepointUnfetched = 2,
};

template <typename CodepointProviderLike>
struct Decoder {
    static constexpr size_t max_depth{ 64 };

    DecInnerState states[max_depth];

    size_t depth;

    BufferState buffer_state;
    unicode::unichar_t buffer_codepoint;

    CodepointProviderLike cpp_like;

    template <typename... CodeProviderLikeInitArgs>
    constexpr Decoder(CodeProviderLikeInitArgs&&... cpp_like_init_args);

    constexpr DecState GetState(this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, DecReason> ReceiveNull(
        this Decoder& self);

    constexpr utils::TryResult<bool, DecReason> ReceiveBoolean(
        this Decoder& self);

    constexpr utils::TryResult<bool, DecReason> ReceiveNumericSign(
        this Decoder& self);

    constexpr utils::TryResult<unsigned char, DecReason>
    ReceiveNumericIntPartDigit(this Decoder& self);

    constexpr utils::TryResult<unsigned char, DecReason>
    ReceiveNumericFracPartDigit(this Decoder& self);

    constexpr utils::TryResult<unicode::unichar_t, DecReason>
    ReceiveNumericExpPartE(this Decoder& self);

    constexpr utils::TryResult<unicode::unichar_t, DecReason>
    ReceiveNumericExpPartSign(this Decoder& self);

    constexpr utils::TryResult<unsigned char, DecReason>
    ReceiveNumericExpPartDigit(this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, DecReason> ReceiveStringStart(
        this Decoder& self);

    constexpr utils::TryResult<unicode::unichar_t, DecReason> ReceiveStringChar(
        this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, DecReason> ReceiveArrayStart(
        this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, DecReason> ReceiveArrayElem(
        this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, DecReason> ReceiveObjectStart(
        this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, DecReason> ReceiveObjectKey(
        this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, DecReason> ReceiveObjectValue(
        this Decoder& self);

    constexpr utils::TryResult<meta::Monostate, DecReason> ReceiveFinish(
        this Decoder& self);
};

}  // namespace zeta::core::json_utils
