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
        bool prefer_uppercase_hex;
        bool prefer_escape_unicode;
    } string;
};

struct FormatContext {
    FormatConfig config;
    unsigned int indent_level;
};

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void MakeNewline(CodepointProvider&& cpp, FormatConfig const& config);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void MakeColon(CodepointProvider&& cpp, FormatConfig const& config);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void MakeComma(CodepointProvider&& cpp, FormatConfig const& config);

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void MakeIndent(CodepointProvider&& cpp, FormatConfig const& config,
                          unsigned indent_level);

}  // namespace zeta::core::json_utils::format_utils
