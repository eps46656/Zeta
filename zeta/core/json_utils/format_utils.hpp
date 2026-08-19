#pragma once

#include <zeta/core/elem_stream.hpp>

namespace zeta::core::json_utils::format_utils {

struct FormatConfig {
    struct Newline {
        enum struct TypeEnum : unsigned char {
            None = 0,
            LF = 1,
            CRLF = 2,
        };

        TypeEnum type;
    } newline;

    struct {
        bool before_colon;
        bool after_colon;

        bool before_comma;
        bool after_comma;
    } space;

    struct Indent {
        enum struct TypeEnum : unsigned char {
            None = 0,
            Space = 1,
            Tab = 2,
        };

        TypeEnum type;
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

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void MakeNewline(CodepointAcceptor&& cpa, FormatConfig const& config);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void MakeColon(CodepointAcceptor&& cpa, FormatConfig const& config);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void MakeComma(CodepointAcceptor&& cpa, FormatConfig const& config);

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void MakeIndent(CodepointAcceptor&& cpa, FormatConfig const& config,
                          unsigned indent_level);

}  // namespace zeta::core::json_utils::format_utils
