#pragma once

#include <zeta/core/debug_utils.ipp>
#include <zeta/core/json_utils/array_serde.hpp>
#include <zeta/core/json_utils/format_utils.ipp>
#include <zeta/core/json_utils/utils.ipp>

namespace zeta::core::json_utils {

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void array_serde::serialize::SendStart(
    CodepointAcceptor&& cpa, StateEnum& state,
    format_utils::FormatContext& fmt_ctx) {
    constexpr unicode::unichar_t bracket_l{ ascii::CharCodeTable::bracket_l };

    elem_stream::acceptor::Transfer(cpa, &bracket_l, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    ++fmt_ctx.indent_level;

    state = StateEnum::ReceivingElemOrFinishLead;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void array_serde::serialize::SendElem(
    CodepointAcceptor&& cpa, StateEnum& state,
    format_utils::FormatContext& fmt_ctx) {
    ZETA_Core_DebugAssert(state == StateEnum::ReceivingElemOrFinishLead ||
                          state == StateEnum::ReceivingElemOrFinishTail);

    if (state == StateEnum::ReceivingElemOrFinishTail) {
        format_utils::MakeComma(cpa, fmt_ctx.config);
    }

    format_utils::MakeNewline(cpa, fmt_ctx.config);
    format_utils::MakeIndent(cpa, fmt_ctx.config, fmt_ctx.indent_level);

    state = StateEnum::ReceivingElemOrFinishTail;
}

template <elem_stream::acceptor::IsAcceptor CodepointAcceptor>
constexpr void array_serde::serialize::SendFinish(
    CodepointAcceptor&& cpa, StateEnum& state,
    format_utils::FormatContext& fmt_ctx) {
    constexpr unicode::unichar_t bracket_r{ ascii::CharCodeTable::bracket_r };

    ZETA_Core_DebugAssert(state == StateEnum::ReceivingElemOrFinishLead ||
                          state == StateEnum::ReceivingElemOrFinishTail);

    --fmt_ctx.indent_level;

    if (state == StateEnum::ReceivingElemOrFinishTail) {
        format_utils::MakeNewline(cpa, fmt_ctx.config);

        format_utils::MakeIndent(cpa, fmt_ctx.config, fmt_ctx.indent_level);
    }

    elem_stream::acceptor::Transfer(cpa, &bracket_r, sizeof(unicode::unichar_t),
                                    sizeof(unicode::unichar_t), 1);

    state = StateEnum::Finished;
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void array_serde::deserialize::ReceiveStart(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp != ascii::CharCodeTable::bracket_l) {
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

    if (cp == ascii::CharCodeTable::bracket_r) {
        state = StateEnum::Finished;
    } else {
        state = StateEnum::SendingElem;
        bcpp.Revert();
    }
}

template <elem_stream::provider::IsProvider CodepointProvider>
constexpr void array_serde::deserialize::ReceivedElem(
    BufferedCodepointProvider<CodepointProvider>& bcpp, StateEnum& state) {
    ZETA_Core_DebugAssert(state == StateEnum::SendingElem);

    unicode::unichar_t cp;

    do {
        if (bcpp.IsEnd()) {
            state = StateEnum::Corrupted;
            return;
        }

        cp = bcpp.Fetch();
    } while ((IsInvisible)(cp));

    if (cp == ascii::CharCodeTable::comma) {
        state = StateEnum::SendingElem;
    } else if (cp == ascii::CharCodeTable::bracket_r) {
        state = StateEnum::Finished;
    } else {
        state = StateEnum::Corrupted;
        bcpp.Revert();
    }
}

}  // namespace zeta::core::json_utils
