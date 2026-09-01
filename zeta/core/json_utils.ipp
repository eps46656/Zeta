#pragma once

#include <zeta/core/ascii.hpp>
#include <zeta/core/json_utils.hpp>
#include <zeta/core/lifecycle.hpp>
#include <zeta/core/utils.ipp>

namespace zeta::core {

namespace json_utils::detail {

constexpr bool IsInvisible_(unicode::unichar_t cp) {
    return cp == ascii::CharCodeTable::horizontal_tab ||
           cp == ascii::CharCodeTable::line_feed ||
           cp == ascii::CharCodeTable::carriage_return ||
           cp == ascii::CharCodeTable::space;
}

constexpr bool IsTokenEnd_(unicode::unichar_t cp) {
    return (IsInvisible_)(cp) || cp == ascii::CharCodeTable::comma ||
           cp == ascii::CharCodeTable::bracket_r ||
           cp == ascii::CharCodeTable::brace_r;
}

}  // namespace json_utils::detail

namespace json_utils::detail {

template <typename CodepointAcceptorLike>
constexpr void MakeNewline_(Encoder<CodepointAcceptorLike>& encoder) {
    auto& cpa{ meta::GetInstRef(encoder.cpa_like) };

    FormatConfig const& fmt_config{ encoder.fmt_ctx.config };

    constexpr unicode::unichar_t lf_str[1]{ ascii::CharCodeTable::line_feed };

    constexpr unicode::unichar_t crlf_str[2]{
        ascii::CharCodeTable::carriage_return, ascii::CharCodeTable::line_feed
    };

    switch (fmt_config.newline.type) {
    case FormatConfig::Newline::Type::None: break;

    case FormatConfig::Newline::Type::LF:
        elem_stream::acceptor::Transfer(cpa, lf_str, sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
        break;

    case FormatConfig::Newline::Type::CRLF:
        elem_stream::acceptor::Transfer(cpa, crlf_str,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 2);

        break;
    }
}

template <typename CodepointAcceptorLike>
constexpr void MakeColon_(Encoder<CodepointAcceptorLike>& encoder) {
    auto& cpa{ meta::GetInstRef(encoder.cpa_like) };

    FormatConfig const& fmt_config{ encoder.fmt_ctx.config };

    constexpr unicode::unichar_t colon_str[3]{
        ascii::CharCodeTable::space,
        ascii::CharCodeTable::colon,
        ascii::CharCodeTable::space,
    };

    unicode::unichar_t const* str_beg{ colon_str + 1 -
                                       fmt_config.space.before_colon };

    unicode::unichar_t const* str_end{ colon_str + 2 +
                                       fmt_config.space.after_colon };

    elem_stream::acceptor::Transfer(cpa, str_beg, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t),
                                    static_cast<size_t>(str_end - str_beg));
}

template <typename CodepointAcceptorLike>
constexpr void MakeComma_(Encoder<CodepointAcceptorLike>& encoder) {
    auto& cpa{ meta::GetInstRef(encoder.cpa_like) };

    FormatConfig const& fmt_config{ encoder.fmt_ctx.config };

    constexpr unicode::unichar_t comma_str[3]{
        ascii::CharCodeTable::space,
        ascii::CharCodeTable::comma,
        ascii::CharCodeTable::space,
    };

    unicode::unichar_t const* str_beg{ comma_str + 1 -
                                       fmt_config.space.before_colon };

    unicode::unichar_t const* str_end{ comma_str + 2 +
                                       (fmt_config.newline.type ==
                                            FormatConfig::Newline::Type::None &&
                                        fmt_config.space.after_colon) };

    elem_stream::acceptor::Transfer(cpa, str_beg, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t),
                                    static_cast<size_t>(str_end - str_beg));
}

template <typename CodepointAcceptorLike>
constexpr void MakeIndent_(Encoder<CodepointAcceptorLike>& encoder) {
    auto& cpa{ meta::GetInstRef(encoder.cpa_like) };

    FormatContext const& fmt_ctx{ encoder.fmt_ctx };

    FormatConfig const& fmt_config{ fmt_ctx.config };

    if (fmt_config.newline.type == FormatConfig::Newline::Type::None) {
        return;
    }

    unicode::unichar_t indent_char;

    switch (fmt_config.indent.type) {
    case FormatConfig::Indent::Type::None: return;
    case FormatConfig::Indent::Type::Space:
        indent_char = ascii::CharCodeTable::space;
        break;
    case FormatConfig::Indent::Type::Tab:
        indent_char = ascii::CharCodeTable::horizontal_tab;
        break;
    }

    elem_stream::acceptor::Transfer(
        cpa, &indent_char, sizeof(unicode::unichar_t), 0,
        static_cast<size_t>(fmt_ctx.indent_level) *
            static_cast<size_t>(fmt_config.indent.cnt));
}

template <typename CodepointAcceptorLike>
constexpr void SendLastElem_(Encoder<CodepointAcceptorLike>& encoder,
                             bool allow_as_obj_key) {
    ZETA_Core_DebugAssert(0 < encoder.depth);

    EncInnerState& state{ encoder.states[encoder.depth - 1] };

    ZETA_Core_Debug_PrintVar(static_cast<unsigned char>(state));

    switch (state) {
    case EncInnerState::ReceivingValue: {
        ZETA_Core_Debug_PrintCurPos;
        --encoder.depth;

        return;
    }

    case EncInnerState::ReceivingArrayElemOrFinishLead:
    case EncInnerState::ReceivingArrayElemOrFinishTrail: {
        if (state == EncInnerState::ReceivingArrayElemOrFinishTrail) {
            detail::MakeComma_(encoder);
        }

        detail::MakeNewline_(encoder);
        detail::MakeIndent_(encoder);

        state = EncInnerState::ReceivingArrayElemOrFinishTrail;

        return;
    }

    case EncInnerState::ReceivingObjectKeyOrFinishLead:
    case EncInnerState::ReceivingObjectKeyOrFinishTrail: {
        ZETA_Core_Debug_PrintCurPos;
        ZETA_Core_DebugAssert(allow_as_obj_key);

        if (state == EncInnerState::ReceivingObjectKeyOrFinishTrail) {
            detail::MakeComma_(encoder);
        }

        detail::MakeNewline_(encoder);
        detail::MakeIndent_(encoder);

        state = EncInnerState::ReceivingObjectValue;

        return;
    }

    case EncInnerState::ReceivingObjectValue: {
        detail::MakeColon_(encoder);

        state = EncInnerState::ReceivingObjectKeyOrFinishTrail;

        return;
    }

    default: ZETA_Core_Unreachable();
    }
}

}  // namespace json_utils::detail

template <typename CodepointAcceptorLike>
constexpr json_utils::EncState
json_utils::Encoder<CodepointAcceptorLike>::GetState(this Encoder& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    EncInnerState& state{ self.states[self.depth - 1] };

    switch (state) {
    case EncInnerState::ReceivingValue: return EncState::ReceivingValue;

    case EncInnerState::ReceivingNumericIntPartDigit:
        return EncState::ReceivingNumericIntPartDigit;

    case EncInnerState::ReceivingNumericIntPartDigitOrNext:
        return EncState::ReceivingNumericIntPartDigitOrNext;

    case EncInnerState::ReceivingNumericFracPartDigitOrNext:
        return EncState::ReceivingNumericFracPartDigitOrNext;

    case EncInnerState::ReceivingNumericExpPartSignOrNext:
        return EncState::ReceivingNumericExpPartSignOrNext;

    case EncInnerState::ReceivingNumericExpPartDigit:
        return EncState::ReceivingNumericExpPartDigit;

    case EncInnerState::ReceivingNumericExpPartDigitOrNext:
        return EncState::ReceivingNumericExpPartDigitOrNext;

    case EncInnerState::ReceivingStringCharOrFinish:
        return EncState::ReceivingStringCharOrFinish;

    case EncInnerState::ReceivingArrayElemOrFinishLead:
    case EncInnerState::ReceivingArrayElemOrFinishTrail:
        return EncState::ReceivingArrayElemOrFinish;

    case EncInnerState::ReceivingObjectKeyOrFinishLead:
    case EncInnerState::ReceivingObjectKeyOrFinishTrail:
        return EncState::ReceivingObjectKeyOrFinish;

    case EncInnerState::ReceivingObjectValue:
        return EncState::ReceivingObjectValue;
    }
}

template <typename CodepointAcceptorLike>
template <typename CodeAcceptorLikeInitArg>
constexpr json_utils::Encoder<CodepointAcceptorLike>::Encoder(
    CodeAcceptorLikeInitArg&& cpa_like_init_arg, FormatConfig const& fmt_config)
    : cpa_like{ ZETA_Core_Lifecycle_UnpackInitArg(
          CodepointAcceptorLike, CodeAcceptorLikeInitArg, cpa_like_init_arg) },
      fmt_ctx{ fmt_config, 0 } {
    this->depth = 1;
    this->states[0] = EncInnerState::ReceivingValue;
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendNull(this Encoder& self) {
    constexpr unicode::unichar_t null_str[]{ ascii::CharCodeTable::n,
                                             ascii::CharCodeTable::u,
                                             ascii::CharCodeTable::l,
                                             ascii::CharCodeTable::l };

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, false);

    elem_stream::acceptor::Transfer(self.cpa_like, &null_str,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 4);

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendBoolean(this Encoder& self,
                                                        bool value) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, false);

    constexpr unicode::unichar_t false_str[]{ ascii::CharCodeTable::f,
                                              ascii::CharCodeTable::a,
                                              ascii::CharCodeTable::l,
                                              ascii::CharCodeTable::s,
                                              ascii::CharCodeTable::e };

    constexpr unicode::unichar_t true_str[]{ ascii::CharCodeTable::t,
                                             ascii::CharCodeTable::r,
                                             ascii::CharCodeTable::u,
                                             ascii::CharCodeTable::e };

    elem_stream::acceptor::Transfer(self.cpa_like, value ? true_str : false_str,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), value ? 4 : 5);

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendNumericSign(this Encoder& self,
                                                            bool is_neg) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, false);

    constexpr unicode::unichar_t minus{ ascii::CharCodeTable::minus };

    if (is_neg) {
        elem_stream::acceptor::Transfer(self.cpa_like, &minus,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    self.states[self.depth++] = EncInnerState::ReceivingNumericIntPartDigit;

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendNumericIntPartDigit(
    this Encoder& self, unsigned char digit) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_Debug_PrintVar(
        static_cast<unsigned char>(self.states[self.depth - 1]));

    EncInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(
        state == EncInnerState::ReceivingNumericIntPartDigit ||
        state == EncInnerState::ReceivingNumericIntPartDigitOrNext);

    if (digit < 0 || 9 < digit) {
        return { utils::TryResultReasonTag{}, EncReason::UnexpectedChar };
    }

    unicode::unichar_t cp{ static_cast<unicode::unichar_t>(
        static_cast<unsigned char>(ascii::CharCodeTable::num_0 + digit)) };

    elem_stream::acceptor::Transfer(self.cpa_like, &cp,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    if (state == EncInnerState::ReceivingNumericIntPartDigit) {
        state = digit == 0 ? EncInnerState::ReceivingNumericFracPartDigitOrNext
                           : EncInnerState::ReceivingNumericIntPartDigitOrNext;
    }

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendNumericFracPartDigit(
    this Encoder& self, unsigned char digit) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_Debug_PrintVar(
        static_cast<unsigned char>(self.states[self.depth - 1]));

    EncInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(
        state == EncInnerState::ReceivingNumericIntPartDigitOrNext ||
        state == EncInnerState::ReceivingNumericFracPartDigitOrNext);

    if (digit < 0 || 9 < digit) {
        return { utils::TryResultReasonTag{}, EncReason::UnexpectedChar };
    }

    unicode::unichar_t digit_cp{ static_cast<unicode::unichar_t>(
        static_cast<unsigned char>(ascii::CharCodeTable::num_0 + digit)) };

    if (state == EncInnerState::ReceivingNumericIntPartDigitOrNext) {
        unicode::unichar_t cp[]{ ascii::CharCodeTable::point, digit_cp };

        elem_stream::acceptor::Transfer(self.cpa_like, cp,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 2);

        state = EncInnerState::ReceivingNumericFracPartDigitOrNext;
    } else {
        elem_stream::acceptor::Transfer(self.cpa_like, &digit_cp,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendNumericExpPartE(
    this Encoder& self, unicode::unichar_t e) {
    ZETA_Core_DebugAssert(0 < self.depth);

    EncInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(
        state == EncInnerState::ReceivingNumericIntPartDigitOrNext ||
        state == EncInnerState::ReceivingNumericFracPartDigitOrNext);

    if (e != ascii::CharCodeTable::e && e != ascii::CharCodeTable::E) {
        return { utils::TryResultReasonTag{}, EncReason::UnexpectedChar };
    }

    unicode::unichar_t cp{ static_cast<unicode::unichar_t>(e) };

    elem_stream::acceptor::Transfer(self.cpa_like, &cp,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    state = EncInnerState::ReceivingNumericExpPartSignOrNext;

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendNumericExpPartSign(
    this Encoder& self, unicode::unichar_t sign) {
    ZETA_Core_DebugAssert(0 < self.depth);

    EncInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state ==
                          EncInnerState::ReceivingNumericExpPartSignOrNext);

    if (sign != unicode::null_codepoint &&
        sign != ascii::CharCodeTable::minus &&
        sign != ascii::CharCodeTable::plus) {
        return { utils::TryResultReasonTag{}, EncReason::UnexpectedChar };
    }

    if (sign != ascii::CharCodeTable::empty) {
        unicode::unichar_t cp{ static_cast<unicode::unichar_t>(sign) };

        elem_stream::acceptor::Transfer(self.cpa_like, &cp,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    state = EncInnerState::ReceivingNumericExpPartDigit;

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendNumericExpPartDigit(
    this Encoder& self, unsigned char digit) {
    ZETA_Core_DebugAssert(0 < self.depth);

    EncInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(
        state == EncInnerState::ReceivingNumericExpPartSignOrNext ||
        state == EncInnerState::ReceivingNumericExpPartDigit ||
        state == EncInnerState::ReceivingNumericExpPartDigitOrNext);

    if (digit < 0 || 9 < digit) {
        return { utils::TryResultReasonTag{}, EncReason::UnexpectedChar };
    }

    unicode::unichar_t cp{ static_cast<unicode::unichar_t>(
        static_cast<unsigned char>(ascii::CharCodeTable::num_0 + digit)) };

    elem_stream::acceptor::Transfer(self.cpa_like, &cp,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    state = EncInnerState::ReceivingNumericExpPartDigitOrNext;

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendStringStart(
    this Encoder& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, true);

    constexpr unicode::unichar_t cp{ ascii::CharCodeTable::double_quote };

    elem_stream::acceptor::Transfer(self.cpa_like, &cp,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    self.states[self.depth++] = EncInnerState::ReceivingStringCharOrFinish;

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendStringChar(
    this Encoder& self, unicode::unichar_t cp) {
    ZETA_Core_DebugAssert(0 < self.depth);

    EncInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == EncInnerState::ReceivingStringCharOrFinish);

    if (0x10FFFF < cp || (0xD800 <= cp && cp <= 0xDFFF)) {
        return { utils::TryResultReasonTag{}, EncReason::UnexpectedChar };
    }

    unicode::unichar_t send_cp_buffer[12];

    bool hit_escape{ true };

    switch (cp) {
    case ascii::CharCodeTable::backspace:
        send_cp_buffer[1] = ascii::CharCodeTable::b;
        break;

    case ascii::CharCodeTable::horizontal_tab:
        send_cp_buffer[1] = ascii::CharCodeTable::t;
        break;

    case ascii::CharCodeTable::line_feed:
        send_cp_buffer[1] = ascii::CharCodeTable::n;
        break;

    case ascii::CharCodeTable::form_feed:
        send_cp_buffer[1] = ascii::CharCodeTable::f;
        break;

    case ascii::CharCodeTable::carriage_return:
        send_cp_buffer[1] = ascii::CharCodeTable::r;
        break;

    case ascii::CharCodeTable::double_quote:
        send_cp_buffer[1] = ascii::CharCodeTable::double_quote;
        break;

    case ascii::CharCodeTable::backslash:
        send_cp_buffer[1] = ascii::CharCodeTable::backslash;
        break;

    default: hit_escape = false; break;
    }

    if (hit_escape) {
        send_cp_buffer[0] = ascii::CharCodeTable::backslash;

        elem_stream::acceptor::Transfer(self.cpa_like, send_cp_buffer,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 2);

        return utils::TryResultValueTag{};
    }

    bool prefer_unicode_escape{
        self.fmt_ctx.config.string.prefer_unicode_escape
    };

    bool prefer_uppercase_hex{
        self.fmt_ctx.config.string.prefer_uppercase_hex
    };

    auto to_hex_4{ [prefer_uppercase_hex](unicode::unichar_t* dst,
                                          unicode::unichar_t cp) -> void {
        dst += 4;

        for (int i{ 0 }; i < 4; ++i, cp /= 16) {
            --dst;

            unicode::unichar_t digit{ cp % 16 };

            if (digit < 10) {
                *dst = ascii::CharCodeTable::num_0 + (cp % 16);
            } else if (prefer_uppercase_hex) {
                *dst = ascii::CharCodeTable::A + (digit - 10);
            } else {
                *dst = ascii::CharCodeTable::a + (digit - 10);
            }
        }
    } };

    if (cp <= 0x1F || cp == 0x7F) { goto SINGLE_U_ESCAPE; }
    if (cp <= 0x7F) { goto DIRECT; }
    if (!prefer_unicode_escape) { goto DIRECT; }
    if (cp <= 0xFFFF) { goto SINGLE_U_ESCAPE; }
    goto DOUBLE_U_ESCAPE;

DIRECT: {
    elem_stream::acceptor::Transfer(self.cpa_like, &cp,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    return utils::TryResultValueTag{};
}

SINGLE_U_ESCAPE: {
    send_cp_buffer[0] = ascii::CharCodeTable::backslash;
    send_cp_buffer[1] = ascii::CharCodeTable::u;

    to_hex_4(send_cp_buffer + 2, cp);

    elem_stream::acceptor::Transfer(self.cpa_like, send_cp_buffer,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 6);

    return utils::TryResultValueTag{};
}

DOUBLE_U_ESCAPE: {
    cp -= 0x10000;

    send_cp_buffer[0] = ascii::CharCodeTable::backslash;
    send_cp_buffer[1] = ascii::CharCodeTable::u;

    to_hex_4(send_cp_buffer + 2, cp / 0x400 + unicode::surrogate_h_range_min);

    send_cp_buffer[6] = ascii::CharCodeTable::backslash;
    send_cp_buffer[7] = ascii::CharCodeTable::u;

    to_hex_4(send_cp_buffer + 8, cp % 0x400 + unicode::surrogate_l_range_min);

    elem_stream::acceptor::Transfer(self.cpa_like, send_cp_buffer,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 12);

    return utils::TryResultValueTag{};
}
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendArrayStart(this Encoder& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, false);

    constexpr unicode::unichar_t bracket_l{ ascii::CharCodeTable::bracket_l };

    elem_stream::acceptor::Transfer(self.cpa_like, &bracket_l,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    ++self.fmt_ctx.indent_level;

    self.states[self.depth++] = EncInnerState::ReceivingArrayElemOrFinishLead;

    ZETA_Core_Debug_PrintVar(
        static_cast<unsigned char>(self.states[self.depth - 1]));

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendObjectStart(
    this Encoder& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(self.depth < self.max_depth);

    detail::SendLastElem_(self, false);

    constexpr unicode::unichar_t brace_l{ ascii::CharCodeTable::brace_l };

    elem_stream::acceptor::Transfer(self.cpa_like, &brace_l,
                                    sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    ++self.fmt_ctx.indent_level;

    self.states[self.depth++] = EncInnerState::ReceivingObjectKeyOrFinishLead;

    return utils::TryResultValueTag{};
}

template <typename CodepointAcceptorLike>
constexpr utils::TryResult<meta::Monostate, json_utils::EncReason>
json_utils::Encoder<CodepointAcceptorLike>::SendFinish(this Encoder& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    EncInnerState& state{ self.states[self.depth - 1] };

    switch (state) {
    case EncInnerState::ReceivingNumericIntPartDigitOrNext:
    case EncInnerState::ReceivingNumericFracPartDigitOrNext:
    case EncInnerState::ReceivingNumericExpPartDigitOrNext: break;

    case EncInnerState::ReceivingStringCharOrFinish: {
        constexpr unicode::unichar_t cp{ ascii::CharCodeTable::double_quote };

        elem_stream::acceptor::Transfer(self.cpa_like, &cp,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);

        break;
    }

    case EncInnerState::ReceivingArrayElemOrFinishLead:
    case EncInnerState::ReceivingArrayElemOrFinishTrail: {
        constexpr unicode::unichar_t bracket_r{
            ascii::CharCodeTable::bracket_r
        };

        --self.fmt_ctx.indent_level;

        if (state == EncInnerState::ReceivingArrayElemOrFinishTrail) {
            detail::MakeNewline_(self);

            detail::MakeIndent_(self);
        }

        elem_stream::acceptor::Transfer(self.cpa_like, &bracket_r,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);

        break;
    }

    case EncInnerState::ReceivingObjectKeyOrFinishLead:
    case EncInnerState::ReceivingObjectKeyOrFinishTrail: {
        constexpr unicode::unichar_t brace_r{ ascii::CharCodeTable::brace_r };

        --self.fmt_ctx.indent_level;

        if (state == EncInnerState::ReceivingObjectKeyOrFinishTrail) {
            detail::MakeNewline_(self);
            detail::MakeIndent_(self);
        }

        elem_stream::acceptor::Transfer(self.cpa_like, &brace_r,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);

        break;
    }

    default: ZETA_Core_Unreachable();
    }

    ZETA_Core_Debug_PrintVar(static_cast<unsigned char>(state));
    ZETA_Core_Debug_PrintVar(self.depth);

    if (self.depth == 1) {
        state = EncInnerState::Finished;
    } else {
        --self.depth;
    }

    return utils::TryResultValueTag{};
}

namespace json_utils::detail {

template <typename CodepointProviderLike>
constexpr void CheckValueDeserializer_(
    json_utils::Decoder<CodepointProviderLike> const& td) {
    ZETA_Core_DebugAssert(td.depth <= td.max_depth);
}

template <elem_stream::provider::IsProvider CodepointProviderLike>
constexpr bool CodepointIsEnd_(
    json_utils::Decoder<CodepointProviderLike> const& decoder) {
    auto& cpp{ meta::GetInstRef(decoder.cpp_like) };

    return decoder.buffer_state != BufferState::HasCodepointUnfetched &&
           elem_stream::provider::IsEnd(cpp);
}

template <typename CodepointProviderLike>
constexpr unicode::unichar_t CodepointFetch_(
    json_utils::Decoder<CodepointProviderLike>& decoder) {
    auto& cpp{ meta::GetInstRef(decoder.cpp_like) };

    if (decoder.buffer_state == BufferState::Empty ||
        decoder.buffer_state == BufferState::HasCodepointFetched) {
        elem_stream::provider::Transfer(cpp, &decoder.buffer_codepoint,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    decoder.buffer_state = BufferState::HasCodepointFetched;

    return decoder.buffer_codepoint;
}

template <typename CodepointProviderLike>
constexpr void CodepointRevert_(
    json_utils::Decoder<CodepointProviderLike>& decoder) {
    ZETA_Core_DebugAssert(decoder.buffer_state ==
                          BufferState::HasCodepointFetched);

    decoder.buffer_state = BufferState::HasCodepointUnfetched;
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, DecReason> DiscoverNextElem_(
    json_utils::Decoder<CodepointProviderLike>& decoder) {
    if (decoder.depth == decoder.max_depth - 1) {
        ZETA_Core_Debug_PrintCurPos;
        decoder.states[decoder.depth++] = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::DepthOverflow };
    }

    unicode::unichar_t cp;

    do {
        if (detail::CodepointIsEnd_(decoder)) {
            if (0 < decoder.depth) {
                ZETA_Core_Debug_PrintCurPos;
                decoder.states[decoder.depth++] = DecInnerState::Corrupted;
                return { utils::TryResultReasonTag{},
                         DecReason::UnexpectedEnd };
            }

            decoder.states[decoder.depth++] = DecInnerState::Finished;

            return { utils::TryResultReasonTag{}, DecReason::SequenceEnd };
        }

        cp = detail::CodepointFetch_(decoder);
    } while (detail::IsInvisible_(cp));

    detail::CodepointRevert_(decoder);

    switch (cp) {
    case ascii::CharCodeTable::n: {
        decoder.states[decoder.depth++] = DecInnerState::SendingNull;
        return utils::TryResultValueTag{};
    }

    case ascii::CharCodeTable::t:
    case ascii::CharCodeTable::f: {
        decoder.states[decoder.depth++] = DecInnerState::SendingBoolean;
        return utils::TryResultValueTag{};
    }

    case ascii::CharCodeTable::minus: {
        decoder.states[decoder.depth++] = DecInnerState::SendingNumericSign;
        return utils::TryResultValueTag{};
    }

    case ascii::CharCodeTable::double_quote: {
        decoder.states[decoder.depth++] = DecInnerState::SendingStringStart;
        return utils::TryResultValueTag{};
    }

    case ascii::CharCodeTable::bracket_l: {
        decoder.states[decoder.depth++] = DecInnerState::SendingArrayStart;
        return utils::TryResultValueTag{};
    }

    case ascii::CharCodeTable::bracket_r: {
        if (0 < decoder.depth && decoder.states[decoder.depth - 1] ==
                                     DecInnerState::SendingArrayElemTrail) {
            decoder.states[decoder.depth - 1] =
                DecInnerState::SendingArrayFinish;
            return { utils::TryResultReasonTag{}, DecReason::SequenceEnd };
        }

        ZETA_Core_Debug_PrintCurPos;

        decoder.states[decoder.depth++] = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }

    case ascii::CharCodeTable::brace_l: {
        decoder.states[decoder.depth++] = DecInnerState::SendingObjectStart;
        return utils::TryResultValueTag{};
    }

    case ascii::CharCodeTable::brace_r: {
        ZETA_Core_Debug_PrintCurPos;

        ZETA_Core_Debug_PrintVar(
            static_cast<unsigned char>(decoder.states[decoder.depth - 1]));

        if (0 < decoder.depth && decoder.states[decoder.depth - 1] ==
                                     DecInnerState::SendingObjectValue) {
            ZETA_Core_Debug_PrintCurPos;

            decoder.states[decoder.depth - 1] =
                DecInnerState::SendingObjectFinish;
            return { utils::TryResultReasonTag{}, DecReason::SequenceEnd };
        }

        ZETA_Core_Debug_PrintCurPos;

        decoder.states[decoder.depth++] = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }
    }

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        decoder.states[decoder.depth++] = DecInnerState::SendingNumericSign;
        return utils::TryResultValueTag{};
    }

    ZETA_Core_Debug_PrintCurPos;

    ZETA_Core_Debug_PrintVar(static_cast<unsigned char>(cp));

    decoder.states[decoder.depth++] = DecInnerState::Corrupted;

    return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
}

}  // namespace json_utils::detail

template <typename CodepointProviderLike>
template <typename... CodeProviderLikeInitArgs>
constexpr json_utils::Decoder<CodepointProviderLike>::Decoder(
    CodeProviderLikeInitArgs&&... cpp_like_init_args)
    : depth{ 0 },
      buffer_state{ BufferState::Empty },
      cpp_like{ ZETA_Core_Lifecycle_UnpackInitArgs(CodepointProviderLike,
                                                   CodeProviderLikeInitArgs,
                                                   cpp_like_init_args) } {
    detail::DiscoverNextElem_(*this).Discard();
}

template <typename CodepointProviderLike>
constexpr json_utils::DecState
json_utils::Decoder<CodepointProviderLike>::GetState(this Decoder& self) {
    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    switch (state) {
    case DecInnerState::SendingNull: return DecState::SendingNull;

    case DecInnerState::SendingBoolean: return DecState::SendingBoolean;

    case DecInnerState::SendingNumericSign: return DecState::SendingNumericSign;

    case DecInnerState::SendingNumericIntPartDigitLead:
    case DecInnerState::SendingNumericIntPartDigitTrail:
    case DecInnerState::SendingNumericIntPartDigitFinish:
        return DecState::SendingNumericIntPartDigit;

    case DecInnerState::SendingNumericFracPartDigit:
        return DecState::SendingNumericFracPartDigit;

    case DecInnerState::SendingNumericExpPartE:
        return DecState::SendingNumericExpPartE;

    case DecInnerState::SendingNumericExpPartSign:
        return DecState::SendingNumericExpPartSign;

    case DecInnerState::SendingNumericExpPartDigitLead:
    case DecInnerState::SendingNumericExpPartDigitTrail:
        return DecState::SendingNumericExpPartDigit;

    case DecInnerState::SendingNumericFinish:
        return DecState::SendingNumericFinish;

    case DecInnerState::SendingStringStart: return DecState::SendingStringStart;

    case DecInnerState::SendingStringChar: return DecState::SendingStringChar;

    case DecInnerState::SendingStringFinish:
        return DecState::SendingStringFinish;

    case DecInnerState::SendingArrayStart: return DecState::SendingArrayStart;

    case DecInnerState::SendingArrayElemLead:
    case DecInnerState::SendingArrayElemTrail:
        return DecState::SendingArrayElem;

    case DecInnerState::SendingArrayFinish: return DecState::SendingArrayFinish;

    case DecInnerState::SendingObjectStart: return DecState::SendingObjectStart;

    case DecInnerState::SendingObjectKeyLead:
    case DecInnerState::SendingObjectKeyTrail:
        return DecState::SendingObjectKey;

    case DecInnerState::SendingObjectValue: return DecState::SendingObjectValue;

    case DecInnerState::SendingObjectFinish:
        return DecState::SendingObjectFinish;

    case DecInnerState::Finished: return DecState::Finished;

    case DecInnerState::Corrupted: return DecState::Corrupted;
    }
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveNull(this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingNull);

    ZETA_Core_DebugAssert(!detail::CodepointIsEnd_(self));

    constexpr unicode::unichar_t expected_cp[]{
        ascii::CharCodeTable::n,  //
        ascii::CharCodeTable::u,  //
        ascii::CharCodeTable::l,  //
        ascii::CharCodeTable::l,  //
    };

    for (int i{ 0 }; i < 4; ++i) {
        if (detail::CodepointIsEnd_(self)) {
            ZETA_Core_DebugAssert(i != 0);
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
        }

        if (detail::CodepointFetch_(self) != expected_cp[i]) {
            ZETA_Core_DebugAssert(i != 0);
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
        }
    }

    if (self.depth == 1) {
        state = DecInnerState::Finished;
    } else {
        --self.depth;
    }

    return utils::TryResultValueTag{};
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<bool, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveBoolean(this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingBoolean);

    ZETA_Core_DebugAssert(!detail::CodepointIsEnd_(self));

    constexpr unicode::unichar_t expected_false_cp[]{
        ascii::CharCodeTable::f,  //
        ascii::CharCodeTable::a,  //
        ascii::CharCodeTable::l,  //
        ascii::CharCodeTable::s,  //
        ascii::CharCodeTable::e,  //
    };

    constexpr unicode::unichar_t expected_true_cp[]{
        ascii::CharCodeTable::t,  //
        ascii::CharCodeTable::r,  //
        ascii::CharCodeTable::u,  //
        ascii::CharCodeTable::e,  //
    };

    unicode::unichar_t const* expected_cp;
    int expected_cp_len;
    bool expected_bool_value;

    switch (detail::CodepointFetch_(self)) {
    case ascii::CharCodeTable::f: {
        expected_cp = expected_false_cp + 1;
        expected_cp_len = 5 - 1;
        expected_bool_value = false;
        break;
    }

    case ascii::CharCodeTable::t: {
        expected_cp = expected_true_cp + 1;
        expected_cp_len = 4 - 1;
        expected_bool_value = true;
        break;
    }

    default: {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }
    }

    for (int i{ 0 }; i < expected_cp_len; ++i) {
        if (detail::CodepointIsEnd_(self)) {
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
        }

        if (detail::CodepointFetch_(self) != expected_cp[i]) {
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
        }
    }

    if (self.depth == 1) {
        state = DecInnerState::Finished;
    } else {
        --self.depth;
    }

    return { utils::TryResultValueTag{}, expected_bool_value };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<bool, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveNumericSign(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingNumericSign);

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    unicode::unichar_t cp{ detail::CodepointFetch_(self) };

    bool is_neg{ cp == ascii::CharCodeTable::minus };

    if (is_neg) {
        state = DecInnerState::SendingNumericIntPartDigitLead;
        return { utils::TryResultValueTag{}, is_neg };
    }

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        state = DecInnerState::SendingNumericIntPartDigitLead;
        detail::CodepointRevert_(self);

        return { utils::TryResultValueTag{}, is_neg };
    }

    state = DecInnerState::Corrupted;

    return { utils::TryResultValueTag{}, is_neg };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<unsigned char, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveNumericIntPartDigit(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(
        state == DecInnerState::SendingNumericIntPartDigitLead ||
        state == DecInnerState::SendingNumericIntPartDigitTrail ||
        state == DecInnerState::SendingNumericIntPartDigitFinish);

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    unicode::unichar_t cp{ detail::CodepointFetch_(self) };

    switch (state) {
    case DecInnerState::SendingNumericIntPartDigitLead:
        if (cp == ascii::CharCodeTable::num_0) {
            state = DecInnerState::SendingNumericIntPartDigitFinish;
            return { utils::TryResultValueTag{},
                     static_cast<unsigned char>(0) };
        }

        if (ascii::CharCodeTable::num_1 <= cp &&
            cp <= ascii::CharCodeTable::num_9) {
            state = DecInnerState::SendingNumericIntPartDigitTrail;
            return { utils::TryResultValueTag{},
                     static_cast<unsigned char>(cp -
                                                ascii::CharCodeTable::num_0) };
        }

        state = DecInnerState::Corrupted;

        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };

    case DecInnerState::SendingNumericIntPartDigitTrail:
        if (ascii::CharCodeTable::num_0 <= cp &&
            cp <= ascii::CharCodeTable::num_9) {
            return { utils::TryResultValueTag{},
                     static_cast<unsigned char>(cp -
                                                ascii::CharCodeTable::num_0) };
        }

        [[fallthrough]];

    case DecInnerState::SendingNumericIntPartDigitFinish:
        if (cp == ascii::CharCodeTable::point) {
            state = DecInnerState::SendingNumericFracPartDigit;
            return { utils::TryResultReasonTag{}, DecReason::NoValue };
        }

        if (cp == ascii::CharCodeTable::e || cp == ascii::CharCodeTable::E) {
            state = DecInnerState::SendingNumericExpPartE;
            detail::CodepointRevert_(self);

            return { utils::TryResultReasonTag{}, DecReason::NoValue };
        }

        if (detail::IsTokenEnd_(cp)) {
            state = DecInnerState::SendingNumericFinish;
            detail::CodepointRevert_(self);

            return { utils::TryResultReasonTag{}, DecReason::NoValue };
        }

        state = DecInnerState::Corrupted;

        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };

    default: ZETA_Core_Unreachable();
    }
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<unsigned char, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveNumericFracPartDigit(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingNumericFracPartDigit);

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    unicode::unichar_t cp{ detail::CodepointFetch_(self) };

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        return { utils::TryResultValueTag{},
                 static_cast<unsigned char>(cp - ascii::CharCodeTable::num_0) };
    }

    if (cp == ascii::CharCodeTable::e || cp == ascii::CharCodeTable::E) {
        state = DecInnerState::SendingNumericExpPartE;
        detail::CodepointRevert_(self);
        return { utils::TryResultReasonTag{}, DecReason::NoValue };
    }

    if (detail::IsTokenEnd_(cp)) {
        state = DecInnerState::SendingNumericFinish;
        detail::CodepointRevert_(self);

        return { utils::TryResultReasonTag{}, DecReason::NoValue };
    }

    state = DecInnerState::Corrupted;

    return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<unicode::unichar_t, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveNumericExpPartE(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_Debug_PrintVar(static_cast<unsigned char>(state));

    ZETA_Core_DebugAssert(state == DecInnerState::SendingNumericExpPartE);

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    unicode::unichar_t cp{ detail::CodepointFetch_(self) };

    unicode::unichar_t e{ cp };

    ZETA_Core_Debug_PrintVar(cp);

    ZETA_Core_DebugAssert(cp == ascii::CharCodeTable::e ||
                          cp == ascii::CharCodeTable::E);

    state = DecInnerState::SendingNumericExpPartSign;

    return { utils::TryResultValueTag{}, e };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<unicode::unichar_t, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveNumericExpPartSign(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);
    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingNumericExpPartSign);

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    unicode::unichar_t cp{ detail::CodepointFetch_(self) };

    if (cp == ascii::CharCodeTable::minus || cp == ascii::CharCodeTable::plus) {
        state = DecInnerState::SendingNumericExpPartDigitLead;
        return { utils::TryResultValueTag{}, cp };
    }

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        state = DecInnerState::SendingNumericExpPartDigitLead;
        detail::CodepointRevert_(self);
        return { utils::TryResultReasonTag{}, DecReason::NoValue };
    }

    state = DecInnerState::Corrupted;

    return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<unsigned char, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveNumericExpPartDigit(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);
    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(
        state == DecInnerState::SendingNumericExpPartDigitLead ||
        state == DecInnerState::SendingNumericExpPartDigitTrail);

    if (detail::CodepointIsEnd_(self)) {
        ZETA_Core_Debug_PrintCurPos;
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    unicode::unichar_t cp{ detail::CodepointFetch_(self) };

    if (ascii::CharCodeTable::num_0 <= cp &&
        cp <= ascii::CharCodeTable::num_9) {
        ZETA_Core_Debug_PrintCurPos;
        state = DecInnerState::SendingNumericExpPartDigitTrail;

        return { utils::TryResultValueTag{},
                 static_cast<unsigned char>(cp - ascii::CharCodeTable::num_0) };
    }

    if (state == DecInnerState::SendingNumericExpPartDigitTrail &&
        detail::IsTokenEnd_(cp)) {
        ZETA_Core_Debug_PrintCurPos;

        state = DecInnerState::SendingNumericFinish;
        detail::CodepointRevert_(self);

        return { utils::TryResultReasonTag{}, DecReason::NoValue };
    }

    ZETA_Core_Debug_PrintCurPos;

    state = DecInnerState::Corrupted;

    return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveStringStart(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    unicode::unichar_t cp;

    do {
        if (detail::CodepointIsEnd_(self)) {
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
        }

        cp = detail::CodepointFetch_(self);
    } while (detail::IsInvisible_(cp));

    if (cp == ascii::CharCodeTable::double_quote) {
        state = DecInnerState::SendingStringChar;
        return utils::TryResultValueTag{};
    }

    state = DecInnerState::Corrupted;

    return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<unicode::unichar_t, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveStringChar(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingStringChar);

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    unicode::unichar_t cp;

    cp = detail::CodepointFetch_(self);

    if (cp == ascii::CharCodeTable::double_quote) {
        state = DecInnerState::SendingStringFinish;
        return { utils::TryResultReasonTag{}, DecReason::SequenceEnd };
    }

    if (cp < 0x0020 || 0x10FFFF < cp) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }

    if (cp != ascii::CharCodeTable::backslash) {
        return { utils::TryResultValueTag{}, cp };
    }

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    cp = detail::CodepointFetch_(self);

    if (cp != ascii::CharCodeTable::u) {
        switch (cp) {
        case ascii::CharCodeTable::b:
            return { utils::TryResultValueTag{},
                     ascii::CharCodeTable::backspace };

        case ascii::CharCodeTable::t:
            return { utils::TryResultValueTag{},
                     ascii::CharCodeTable::horizontal_tab };

        case ascii::CharCodeTable::n:
            return { utils::TryResultValueTag{},
                     ascii::CharCodeTable::line_feed };

        case ascii::CharCodeTable::f:
            return { utils::TryResultValueTag{},
                     ascii::CharCodeTable::form_feed };

        case ascii::CharCodeTable::r:
            return { utils::TryResultValueTag{},
                     ascii::CharCodeTable::carriage_return };

        case ascii::CharCodeTable::double_quote:
            return { utils::TryResultValueTag{},
                     ascii::CharCodeTable::double_quote };

        case ascii::CharCodeTable::slash:
            return { utils::TryResultValueTag{}, ascii::CharCodeTable::slash };

        case ascii::CharCodeTable::backslash:
            return { utils::TryResultValueTag{},
                     ascii::CharCodeTable::backslash };
        }

        state = DecInnerState::Corrupted;

        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }

    auto try_from_hex_4{
        [&self, &state]() -> utils::TryResult<unicode::unichar_t, DecReason> {
            unicode::unichar_t ret{ 0 };

            for (int i{ 0 }; i < 4; ++i) {
                if (detail::CodepointIsEnd_(self)) {
                    state = DecInnerState::Corrupted;
                    return { utils::TryResultReasonTag{},
                             DecReason::UnexpectedEnd };
                }

                unicode::unichar_t cp{ detail::CodepointFetch_(self) };

                unicode::unichar_t k;

                if (ascii::CharCodeTable::num_0 <= cp &&
                    cp <= ascii::CharCodeTable::num_9) {
                    k = cp - ascii::CharCodeTable::num_0;
                } else if (ascii::CharCodeTable::a <= cp &&
                           cp <= ascii::CharCodeTable::f) {
                    k = cp - ascii::CharCodeTable::a + 10;
                } else if (ascii::CharCodeTable::A <= cp &&
                           cp <= ascii::CharCodeTable::F) {
                    k = cp - ascii::CharCodeTable::A + 10;
                } else {
                    state = DecInnerState::Corrupted;
                    return { utils::TryResultReasonTag{},
                             DecReason::UnexpectedChar };
                }

                ret = ret * 16 + k;
            }

            return { utils::TryResultValueTag{}, ret };
        }
    };

    utils::TryResult<unicode::unichar_t, DecReason> try_unichar_buffer_h{
        try_from_hex_4()
    };

    if (try_unichar_buffer_h.HasReason()) { return try_unichar_buffer_h; }

    unicode::unichar_t unichar_buffer_h{ try_unichar_buffer_h.GetValue() };

    if (unichar_buffer_h < unicode::surrogate_range_min ||
        unicode::surrogate_range_max < unichar_buffer_h) {
        return { utils::TryResultValueTag{}, unichar_buffer_h };
    }

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    if (detail::CodepointFetch_(self) != ascii::CharCodeTable::backslash) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }

    if (detail::CodepointIsEnd_(self)) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
    }

    if (detail::CodepointFetch_(self) != ascii::CharCodeTable::u) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }

    utils::TryResult<unicode::unichar_t, DecReason> try_unichar_buffer_l{
        try_from_hex_4()
    };

    if (try_unichar_buffer_l.HasReason()) { return try_unichar_buffer_l; }

    unicode::unichar_t unichar_buffer_l{ try_unichar_buffer_l.GetValue() };

    if (unichar_buffer_l < unicode::surrogate_l_range_min ||
        unicode::surrogate_l_range_max < unichar_buffer_l) {
        state = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }

    return { utils::TryResultValueTag{},
             0x10000 +
                 ((unichar_buffer_h - unicode::surrogate_h_range_min) << 10) +
                 (unichar_buffer_l - unicode::surrogate_l_range_min) };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveArrayStart(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingArrayStart);

    unicode::unichar_t cp;

    do {
        if (detail::CodepointIsEnd_(self)) {
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
        }

        cp = detail::CodepointFetch_(self);
    } while (detail::IsInvisible_(cp));

    if (cp == ascii::CharCodeTable::bracket_l) {
        state = DecInnerState::SendingArrayElemLead;
        return utils::TryResultValueTag{};
    }

    state = DecInnerState::Corrupted;

    return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveArrayElem(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingArrayElemLead ||
                          state == DecInnerState::SendingArrayElemTrail);

    unicode::unichar_t cp;

    do {
        if (detail::CodepointIsEnd_(self)) {
            ZETA_Core_Debug_PrintCurPos;
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
        }

        cp = detail::CodepointFetch_(self);
    } while (detail::IsInvisible_(cp));

    if (cp == ascii::CharCodeTable::bracket_r) {
        state = DecInnerState::SendingArrayFinish;
        return { utils::TryResultReasonTag{}, DecReason::SequenceEnd };
    }

    switch (state) {
    case DecInnerState::SendingArrayElemLead:
        state = DecInnerState::SendingArrayElemTrail;

        detail::CodepointRevert_(self);

        break;

    case DecInnerState::SendingArrayElemTrail:
        if (cp != ascii::CharCodeTable::comma) {
            state = DecInnerState::Corrupted;
            ZETA_Core_Debug_PrintVar(cp);
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
        }

        break;

    default: ZETA_Core_Unreachable();
    }

    ZETA_Core_Debug_PrintCurPos;

    return detail::DiscoverNextElem_(self);
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveObjectStart(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingObjectStart);

    unicode::unichar_t cp;

    do {
        if (detail::CodepointIsEnd_(self)) {
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
        }

        cp = detail::CodepointFetch_(self);
    } while (detail::IsInvisible_(cp));

    if (cp == ascii::CharCodeTable::brace_l) {
        state = DecInnerState::SendingObjectKeyLead;
        return utils::TryResultValueTag{};
    }

    state = DecInnerState::Corrupted;

    return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveObjectKey(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingObjectKeyLead ||
                          state == DecInnerState::SendingObjectKeyTrail);

    unicode::unichar_t cp;

    do {
        if (detail::CodepointIsEnd_(self)) {
            ZETA_Core_Debug_PrintCurPos;
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
        }

        cp = detail::CodepointFetch_(self);
    } while (detail::IsInvisible_(cp));

    if (cp == ascii::CharCodeTable::brace_r) {
        state = DecInnerState::SendingObjectFinish;
        return { utils::TryResultReasonTag{}, DecReason::SequenceEnd };
    }

    switch (state) {
    case DecInnerState::SendingObjectKeyLead:
        detail::CodepointRevert_(self);
        break;

    case DecInnerState::SendingObjectKeyTrail:
        if (cp != ascii::CharCodeTable::comma) {
            ZETA_Core_Debug_PrintVar(cp);
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
        }

        break;

    default: ZETA_Core_Unreachable();
    }

    state = DecInnerState::SendingObjectValue;

    auto ret{ detail::DiscoverNextElem_(self) };

    if (self.states[self.depth - 1] != DecInnerState::Corrupted &&
        self.states[self.depth - 1] != DecInnerState::SendingObjectFinish &&
        self.states[self.depth - 1] != DecInnerState::SendingStringStart) {
        ZETA_Core_Debug_PrintVar(
            static_cast<unsigned char>(self.states[self.depth - 1]));
        self.states[self.depth - 1] = DecInnerState::Corrupted;
        return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
    }

    ZETA_Core_Debug_PrintCurPos;

    return ret;
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveObjectValue(
    this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    DecInnerState& state{ self.states[self.depth - 1] };

    ZETA_Core_DebugAssert(state == DecInnerState::SendingObjectValue);

    unicode::unichar_t cp;

    do {
        if (detail::CodepointIsEnd_(self)) {
            ZETA_Core_Debug_PrintCurPos;
            state = DecInnerState::Corrupted;
            return { utils::TryResultReasonTag{}, DecReason::UnexpectedEnd };
        }

        cp = detail::CodepointFetch_(self);
    } while (detail::IsInvisible_(cp));

    if (cp == ascii::CharCodeTable::colon) {
        state = DecInnerState::SendingObjectKeyTrail;
        return detail::DiscoverNextElem_(self);
    }

    state = DecInnerState::Corrupted;

    return { utils::TryResultReasonTag{}, DecReason::UnexpectedChar };
}

template <typename CodepointProviderLike>
constexpr utils::TryResult<meta::Monostate, json_utils::DecReason>
json_utils::Decoder<CodepointProviderLike>::ReceiveFinish(this Decoder& self) {
    detail::CheckValueDeserializer_(self);

    ZETA_Core_DebugAssert(0 < self.depth);

    ZETA_Core_DebugAssert(
        self.states[self.depth - 1] == DecInnerState::SendingNumericFinish ||
        self.states[self.depth - 1] == DecInnerState::SendingStringFinish ||
        self.states[self.depth - 1] == DecInnerState::SendingArrayFinish ||
        self.states[self.depth - 1] == DecInnerState::SendingObjectFinish);

    if (self.depth == 1) {
        self.states[0] = DecInnerState::Finished;
    } else {
        --self.depth;
    }

    return utils::TryResultValueTag{};
}

}  // namespace zeta::core
