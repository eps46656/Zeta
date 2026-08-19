#pragma once

#include <zeta/core/ascii.hpp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/json_utils/format_utils.hpp>
#include <zeta/core/unicode.hpp>

namespace zeta::core::json_utils {

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void format_utils::MakeNewline(CodepointAcceptor&& cpa,
                                         FormatConfig const& config) {
    constexpr unicode::unichar_t lf_str[1]{ ascii::CharCodeTable::line_feed };

    constexpr unicode::unichar_t crlf_str[2]{
        ascii::CharCodeTable::carriage_return, ascii::CharCodeTable::line_feed
    };

    switch (config.newline.type) {
    case FormatConfig::Newline::TypeEnum::None: break;

    case FormatConfig::Newline::TypeEnum::LF:
        elem_stream::acceptor::Transfer(cpa, lf_str, sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
        break;

    case FormatConfig::Newline::TypeEnum::CRLF:
        elem_stream::acceptor::Transfer(cpa, crlf_str,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 2);

        break;
    }
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void format_utils::MakeColon(CodepointAcceptor&& cpa,
                                       FormatConfig const& config) {
    constexpr unicode::unichar_t colon_str[3]{
        ascii::CharCodeTable::space,
        ascii::CharCodeTable::colon,
        ascii::CharCodeTable::space,
    };

    unicode::unichar_t const* str_beg{ colon_str + 1 -
                                       config.space.before_colon };

    unicode::unichar_t const* str_end{ colon_str + 2 +
                                       config.space.after_colon };

    elem_stream::acceptor::Transfer(cpa, str_beg, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t),
                                    static_cast<size_t>(str_end - str_beg));
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void format_utils::MakeComma(CodepointAcceptor&& cpa,
                                       FormatConfig const& config) {
    constexpr unicode::unichar_t comma_str[3]{
        ascii::CharCodeTable::space,
        ascii::CharCodeTable::comma,
        ascii::CharCodeTable::space,
    };

    unicode::unichar_t const* str_beg{ comma_str + 1 -
                                       config.space.before_colon };

    unicode::unichar_t const* str_end{
        comma_str + 2 +
        (config.newline.type == FormatConfig::Newline::TypeEnum::None &&
         config.space.after_colon)
    };

    elem_stream::acceptor::Transfer(cpa, str_beg, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t),
                                    static_cast<size_t>(str_end - str_beg));
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void format_utils::MakeIndent(CodepointAcceptor&& cpa,
                                        FormatConfig const& config,
                                        unsigned indent_level) {
    if (config.newline.type == FormatConfig::Newline::TypeEnum::None) {
        return;
    }

    unicode::unichar_t indent_char;

    switch (config.indent.type) {
    case FormatConfig::Indent::TypeEnum::None: return;
    case FormatConfig::Indent::TypeEnum::Space:
        indent_char = ascii::CharCodeTable::space;
        break;
    case FormatConfig::Indent::TypeEnum::Tab:
        indent_char = ascii::CharCodeTable::horizontal_tab;
        break;
    }

    elem_stream::acceptor::Transfer(cpa, &indent_char,
                                    sizeof(unicode::unichar_t), 0,
                                    static_cast<size_t>(indent_level) *
                                        static_cast<size_t>(config.indent.cnt));
}

}  // namespace zeta::core::json_utils

// namespace zeta::core::json_utils
