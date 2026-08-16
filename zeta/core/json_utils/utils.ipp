#pragma once

#include <zeta/core/ascii.hpp>
#include <zeta/core/debug_utils.ipp>
#include <zeta/core/elem_stream.ipp>
#include <zeta/core/json_utils/utils.hpp>

namespace zeta::core {

constexpr bool json_utils::IsInvisible(unicode::unichar_t cp) {
    return cp == ascii::CharCodeTable::horizontal_tab ||
           cp == ascii::CharCodeTable::line_feed ||
           cp == ascii::CharCodeTable::carriage_return ||
           cp == ascii::CharCodeTable::space;
}

constexpr bool json_utils::IsTokenEnd(unicode::unichar_t cp) {
    return IsInvisible(cp) || cp == ascii::CharCodeTable::comma ||
           cp == ascii::CharCodeTable::bracket_r ||
           cp == ascii::CharCodeTable::brace_r;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr json_utils::BufferedCodepointProvider<
    CodepointProvider>::BufferedCodepointProvider(CodepointProvider& cpp)
    : buffer_state{ BufferStateEnum::Empty }, cpp{ cpp } {}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr bool json_utils::BufferedCodepointProvider<CodepointProvider>::IsEnd(
    this BufferedCodepointProvider& self) {
    return self.buffer_state != BufferStateEnum::HasCodepointUnfetched &&
           elem_stream::provider::IsEnd(self.cpp);
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unicode::unichar_t
json_utils::BufferedCodepointProvider<CodepointProvider>::Fetch(
    this BufferedCodepointProvider& self) {
    if (self.buffer_state == BufferStateEnum::Empty ||
        self.buffer_state == BufferStateEnum::HasCodepointFetched) {
        elem_stream::provider::Transfer(self.cpp, &self.buffer_codepoint,
                                        sizeof(unicode::unichar_t),
                                        sizeof(unicode::unichar_t), 1);
    }

    self.buffer_state = BufferStateEnum::HasCodepointFetched;

    return self.buffer_codepoint;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr unicode::unichar_t
json_utils::BufferedCodepointProvider<CodepointProvider>::FetchSkipUnvisible(
    this BufferedCodepointProvider& self) {
    while (true) {
        if (self.IsEnd()) { return null_cp; }
        unicode::unichar_t cp{ self.Fetch() };
        if (!(IsInvisible)(cp)) { return cp; }
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void json_utils::BufferedCodepointProvider<CodepointProvider>::Revert(
    this BufferedCodepointProvider& self) {
    ZETA_Core_DebugAssert(self.buffer_state ==
                          BufferStateEnum::HasCodepointFetched);

    self.buffer_state = BufferStateEnum::HasCodepointUnfetched;
}

}  // namespace zeta::core
