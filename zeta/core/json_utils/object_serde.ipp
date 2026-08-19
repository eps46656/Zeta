#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/json_utils/format_utils.ipp>
#include <zeta/core/json_utils/object_serde.hpp>
#include <zeta/core/json_utils/utils.ipp>

namespace zeta::core::json_utils {

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void object_serde::serialize::SendStart(
    CodepointAcceptor&& cpa, StateEnum& state,
    format_utils::FormatContext& fmt_ctx) {
    constexpr unicode::unichar_t brace_l{ ascii::CharCodeTable::brace_l };

    elem_stream::acceptor::Transfer(cpa, &brace_l, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    ++fmt_ctx.indent_level;

    state = StateEnum::ReceivingKeyOrFinishLead;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void object_serde::serialize::SendKey(
    CodepointAcceptor&& cpa, StateEnum& state,
    format_utils::FormatContext& fmt_ctx) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingKeyOrFinishLead ||
                          state == StateEnum::ReceivingKeyOrFinishTail);

    if (state == StateEnum::ReceivingKeyOrFinishTail) {
        format_utils::MakeComma(cpa, fmt_ctx.config);
    }

    format_utils::MakeNewline(cpa, fmt_ctx.config);
    format_utils::MakeIndent(cpa, fmt_ctx.config, fmt_ctx.indent_level);

    state = StateEnum::ReceivingValue;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void object_serde::serialize::SendValue(
    CodepointAcceptor&& cpa, StateEnum& state,
    format_utils::FormatContext& fmt_ctx) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingValue);

    format_utils::MakeColon(cpa, fmt_ctx.config);

    state = StateEnum::ReceivingKeyOrFinishTail;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void object_serde::serialize::SendFinish(
    CodepointAcceptor&& cpa, StateEnum& state,
    format_utils::FormatContext& fmt_ctx) {
    constexpr unicode::unichar_t brace_r{ ascii::CharCodeTable::brace_r };

    ZETA_Core_DebugAssert(state == StateEnum::ReceivingKeyOrFinishLead ||
                          state == StateEnum::ReceivingKeyOrFinishTail);

    --fmt_ctx.indent_level;

    if (state == StateEnum::ReceivingKeyOrFinishTail) {
        format_utils::MakeNewline(cpa, fmt_ctx.config);
        format_utils::MakeIndent(cpa, fmt_ctx.config, fmt_ctx.indent_level);
    }

    elem_stream::acceptor::Transfer(cpa, &brace_r, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    state = StateEnum::Finished;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void object_serde::deserialize::ReceiveStart(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp != ascii::CharCodeTable::brace_l) {
        state = StateEnum::Corrupted;
        bcpp.Revert();
        return;
    }

    do {
        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp == ascii::CharCodeTable::brace_r) {
        state = StateEnum::Finished;
    } else {
        state = StateEnum::SendingKey;
        bcpp.Revert();
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void object_serde::deserialize::ReceivedKey(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingKey);

    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp == ascii::CharCodeTable::colon) {
        state = StateEnum::SendingValue;
    } else {
        state = StateEnum::Corrupted;
        bcpp.Revert();
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void object_serde::deserialize::ReceivedValue(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingValue);

    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp == ascii::CharCodeTable::comma) {
        state = StateEnum::SendingKey;
    } else if (cp == ascii::CharCodeTable::brace_r) {
        state = StateEnum::Finished;
    } else {
        state = StateEnum::Corrupted;
        bcpp.Revert();
    }
}

}  // namespace zeta::core::json_utils
